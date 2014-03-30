#include "Action.h"
#include "Directory.h"
#include <QDir>
#include <QQueue>
#include <QDebug>
#include "Action_queue.h"
#include <QApplication>
#include <QStack>
#include <QDirIterator>
#include "Core.h"
#include <fstream>
#include <errno.h>
#include "File_system_engine.h"
#include "gio/Gio_file_system_engine.h"



Action::Action(Action_queue *q, File_system_engine *fs, const Action_data &p_data)
: data(p_data)
, fs_engine(fs)
, queue(q)
{
  phase = phase_queued;
  total_size = 0;
  total_count = 0;

  current_size = 0;
  current_count = 0;

  paused = false;
  blocked = false;
  postprocess_running = false;

  //state_delivery_in_process = false;
  pending_operation = 0;

  qRegisterMetaType<Action_state>("Action_state");
  qRegisterMetaType<Question_data>("Question_data");
  /*connect(&iteration_timer, SIGNAL(timeout()), this, SLOT(run_iteration()));
  iteration_timer.setInterval(0);
  iteration_timer.setSingleShot(true); */

  //fs_engine->moveToThread(queue);
  if (queue) {
    moveToThread(queue);
  }
}

Action::~Action() {
  //delete fs_engine;
}

void Action::ask_question(Question_data data) {
  qDebug() << "ask_question: " << data.get_message();
  //error_reaction = Error_reaction::undefined;
  //data.action = this;
  emit question(data);
  blocked = true;
  //update_iteration_timer();
  send_state();
}

void Action::run() {
  //if (!fs_engine) {
  //  qFatal("no fs engine for action");
  //  return;
  //}
  //fs_engine->moveToThread(thread()); //dangerous! need to test
  emit started();
  if (data.destination.endsWith("/")) {
    data.destination = data.destination.left(data.destination.length() - 1);
  }

  if (data.recursive_fetch_option == recursive_fetch_auto &&
      (data.type == Action_type::remove || data.type == Action_type::trash) ) {
    phase = phase_processing;
  } else if (data.recursive_fetch_option == recursive_fetch_off) {
    phase = phase_processing;
  } else {
    phase = phase_preparing;
  }

  fs_iterators_stack.push(new File_system_engine::Simple_iterator(data.targets));

  foreach(File_info fi, data.targets) {
    QStringList path_parts = fi.uri.split("/");
    QStringList destination_parts = data.destination.split("/");
    if (destination_parts.mid(0, path_parts.count()) == path_parts) {
      emit error(tr("Cannot copy directory '%1' inside itself.").arg(fi.uri));
      phase = phase_finished;
      emit finished();
      return;
    }
  }

  send_state();
  run_iterations();
}

void Action::process_current(Error_reaction::Enum error_reaction) {
  if (pending_operation != 0) {
    qWarning("pending_operation must be NULL in Action::process_current");
  }
  if (postprocess_running) {
    postprocess_directory(current_file);
    return;
  }
  if (phase == phase_preparing) {
    if (data.recursive_fetch_option == recursive_fetch_auto && total_count > auto_recursive_fetch_max) {
      total_size = 0;
      total_count = 0;
      end_preparing();
    }
    total_count++;
    if (data.type != Action_type::remove && data.type != Action_type::trash) {
      if (current_file.is_file()) { total_size += current_file.file_size; }
    }
  } else {
    if (data.type == Action_type::trash) {
      if (error_reaction == Error_reaction::delete_competely) {
        fs_engine->remove(current_file.uri);
      } else {
        Gio_file_system_engine::move_to_trash(current_file.uri);
      }
    } else if (data.type == Action_type::remove) {
      fs_engine->remove(current_file.uri);
    } else if (data.type == Action_type::copy || data.type == Action_type::move) {
      QStringList new_path_parts;
      new_path_parts << data.destination;
      foreach(File_system_engine::Iterator* i, fs_iterators_stack) {
        new_path_parts << i->get_current().file_name();
        //qDebug() << "uri in stack: " << i->get_current().uri;
      }
      if (data.destination_includes_filename) {
        new_path_parts.removeAt(1);
      }
      //qDebug() << "new_path_parts: " << new_path_parts;
      QString new_path = new_path_parts.join("/");
      if (error_reaction == Error_reaction::overwrite) {
        fs_engine->remove_recursively(new_path);
      } else if (error_reaction == Error_reaction::rename_new) {
        new_path = find_autorename_path(new_path);
      } else if (error_reaction == Error_reaction::rename_existing) {
        QString path_for_existing = find_autorename_path(new_path);
        fs_engine->move(new_path, path_for_existing);
      }

      if (current_file.is_folder) {
        if (error_reaction != Error_reaction::merge_dir) {
          fs_engine->make_directory(new_path);
        }
      } else {
        if (data.type == Action_type::copy) {
          bool append = error_reaction == Error_reaction::continue_writing;
          pending_operation = fs_engine->copy(current_file.uri, new_path, append);
        } else if (data.type == Action_type::move) {
          pending_operation = fs_engine->move(current_file.uri, new_path);
        }
      }
    }
    current_count++;
  }
}

void Action::postprocess_directory(const File_info &file_info) {
  if (!file_info.is_folder) {
    qWarning("Action::postprocess_one must be called only for folders");
    return;
  }
  if (data.type == Action_type::remove || data.type == Action_type::move) {
    fs_engine->remove(file_info.uri);
  }
}

void Action::process_pending_operation() {
  if (pending_operation->is_finished()) {
    delete pending_operation;
    pending_operation = 0;
    current_size += current_file.file_size;
    return;
  }

  pending_operation->run_iteration();
}


void Action::send_state() {
  Action_state state;
  state.queue_id = queue ? queue->get_id() : -1;

  if (phase == phase_processing && total_size > 0) {
    qint64 real_current_size = current_size;
    if (pending_operation) {
      real_current_size += current_file.file_size * pending_operation->get_progress();
    }
    state.total_progress = 1.0 * real_current_size / total_size;
  } else {
    int c = phase == phase_preparing ? total_count : current_count;
    state.total_progress_text = tr("Files count: %1").arg(c);
    state.total_progress = Action_state::UNKNOWN;
  }

  if (blocked) {
    state.current_action = tr("Waiting for an answer");
  } else if (current_file.uri.isEmpty()) {
    state.current_action = tr("Starting");
  } else if (phase == phase_preparing) {
    state.current_action = tr("Processing %1");
  } else if (data.type == Action_type::copy) {
    state.current_action = tr("Copying %1");
  } else if (data.type == Action_type::move) {
    state.current_action = tr("Moving %1");
  } else if (data.type == Action_type::trash) {
    state.current_action = tr("Moving to trash %1");
  } else if (data.type == Action_type::remove) {
    state.current_action = tr("Removing %1");
  } else {
    qWarning("unhandled action type in Action::construct_processing_state");
  }
  if (state.current_action.contains("%1")) {
    state.current_action = state.current_action.arg(current_file.uri);
  }

  if (pending_operation) {
    state.current_progress = pending_operation->get_progress();
  } else {
    state.current_progress = Action_state::DISABLED;
  }

  if (blocked) {
    if (state.current_progress == Action_state::UNKNOWN) {
      state.current_progress = Action_state::DISABLED;
    }
    if (state.total_progress == Action_state::UNKNOWN) {
      state.total_progress = Action_state::DISABLED;
    }
  }

  emit state_changed(state);
  //qDebug() << "emit state_changed(state)";
  //state_delivery_in_process = true;
  signal_timer.restart();
}

void Action::run_iterations() {
  while(!(paused || blocked || phase == phase_finished)) {
    if (signal_timer.elapsed() > signal_interval) {
      send_state();
    }

    try {

      //3a. Finalize processing after previous item is successfully processed or skipped
      if (postprocess_running) {
        postprocess_running = false;
      } else {
        if (current_file.is_folder) {
          if (data.type != Action_type::trash) { // trash must not be recursive
            fs_iterators_stack.push(fs_engine->list(current_file.uri));
          }
        }
      }


      if (pending_operation) {
        //3b. process already started operation
        process_pending_operation();
      } else {
        // 1. Find the next item and set `current_file` and `postprocess_running`
        if (fs_iterators_stack.top()->has_next()) {
          current_file = fs_iterators_stack.top()->get_next();
          //current_file = fi;
        } else {
          delete fs_iterators_stack.top();
          fs_iterators_stack.pop();
          if (fs_iterators_stack.isEmpty()) {
            if (phase == phase_preparing) {
              end_preparing();
            } else if (phase == phase_processing) {
              phase = phase_finished;
              emit finished();
              //update_iteration_timer();
            } else {
              qWarning("unexpected value of phase");
            }
            current_file = File_info(); // no processing required
          } else {
            current_file = fs_iterators_stack.top()->get_current();
            postprocess_running = true;
          }
        }

        //2. Actual processing
        if (!current_file.uri.isEmpty()) {
          process_current();
        }
      }
    } catch (File_system_engine::Exception& e) {
      ask_question(Question_data(this, e));
    }
    QApplication::processEvents();
  }
}

void Action::end_preparing() {
  while(!fs_iterators_stack.isEmpty()) {
    delete fs_iterators_stack.pop();
  }
  fs_iterators_stack.push(new File_system_engine::Simple_iterator(data.targets));
  phase = phase_processing;
}

/*void Action::update_iteration_timer() {
  if (paused || blocked || phase == phase_finished) {
    //iteration_timer.stop();
  } else {
    run_iteration();
    //iteration_timer.start();
  }
}*/

QString Action::find_autorename_path(const QString &path) {
  QStringList parts = path.split("/");
  QFileInfo filename(parts.last());
  QString suffix = filename.completeSuffix();
  for(int i = 1; i < 10000; i++) {
    QString new_filename;
    if (suffix.isEmpty()) {
      new_filename = QString("%1 (%2)").arg(filename.baseName()).arg(i);
    } else {
      new_filename = QString("%1 (%2).%3").arg(filename.baseName()).arg(i).arg(suffix);
    }
    parts.removeLast();
    parts.append(new_filename);
    QString new_path = parts.join("/");
    if (!fs_engine->is_file(new_path) && !fs_engine->is_directory(new_path)) {
      return new_path;
    }
  }
  qFatal("Action::find_autorename_path: no filenames available");
  return QString();
}


void Action::question_answered(Error_reaction::Enum reaction) {
  try {
    if (reaction == Error_reaction::retry) {
      if (pending_operation) {
        process_pending_operation();
      } else {
        process_current();
      }
    } else if (reaction == Error_reaction::skip) {
      if (pending_operation) {
        delete pending_operation;
        pending_operation = 0;
        current_size += current_file.file_size;
      } else {
        run_iterations();
      }
    } else if (reaction == Error_reaction::abort) {
      abort();
      return;
    } else if (reaction == Error_reaction::continue_writing ||
               reaction == Error_reaction::overwrite ||
               reaction == Error_reaction::rename_existing ||
               reaction == Error_reaction::rename_new ||
               reaction == Error_reaction::delete_competely ||
               reaction == Error_reaction::merge_dir) {
      process_current(reaction);
    } else if (reaction == Error_reaction::ask) {
      qFatal("Action::question_answered: 'ask' must not be here");
    } else if (reaction == Error_reaction::undefined) {
      qFatal("Action::question_answered: 'undefined' must not be here");
    } else {
      qFatal("Action::question_answered: unhandled answer");
    }

    blocked = false;
    //update_iteration_timer();
    send_state();
  } catch (File_system_engine::Exception& e) {
    ask_question(Question_data(this, e));
    return;
  }
  run_iterations();
}

void Action::set_paused(bool v) {
  paused = v;
  run_iterations();
  //update_iteration_timer();
}

void Action::abort() {
  if (phase != phase_queued) {
    if (pending_operation) {
      delete pending_operation;
      pending_operation = 0;
    }
    phase = phase_finished;
    //todo: display warning if answer was automatic
    emit finished();
  }
}

void Action::state_delivered() {
  //qDebug() << "Action::state_delivered";
  //state_delivery_in_process = false;
}
