#include "Action.h"
#include "Directory.h"
#include <QDir>
#include <QQueue>
#include <QDebug>
#include "Action_queue.h"
#include <QApplication>
#include <QStack>
#include <QDirIterator>
#include <QFSFileEngine>
#include "mkdir.h"
#include "Core.h"
#include <fstream>
#include <errno.h>
#include "File_system_engine.h"

Action::Action(Action_queue *q, const Action_data &p_data)
: data(p_data)
, fs_engine(q->get_core()->get_new_file_system_engine())
, queue(q)
{
  total_size = 0;
  total_count = 0;

  current_size = 0;
  current_count = 0;

  paused = false;
  blocked = false;

  //state_delivery_in_process = false;
  pending_operation = 0;

  qRegisterMetaType<Action_state>("Action_state");
  qRegisterMetaType<Question_data>("Question_data");
  /*connect(&iteration_timer, SIGNAL(timeout()), this, SLOT(run_iteration()));
  iteration_timer.setInterval(0);
  iteration_timer.setSingleShot(true); */

  fs_engine->moveToThread(queue);
  moveToThread(queue);
}

Action::~Action() {
  delete fs_engine;
}

void Action::ask_question(Question_data data) {
  qDebug() << "ask_question: " << data.get_message();
  //error_reaction = Error_reaction::undefined;
  //data.action = this;
  emit question(data);
  blocked = true;
  update_iteration_timer();
  send_state();
}

void Action::run() {
  if (!fs_engine) {
    qFatal("no fs engine for action");
    return;
  }
  //fs_engine->moveToThread(thread()); //dangerous! need to test
  emit started();
  if (data.destination.endsWith("/")) {
    data.destination = data.destination.left(data.destination.length() - 1);
  }

  if (data.recursive_fetch_option == recursive_fetch_auto &&
      data.type == Action_type::remove) {
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
  update_iteration_timer();
}

void Action::process_one(const File_info &file_info) {
  if (phase == phase_preparing) {
    if (data.recursive_fetch_option == recursive_fetch_auto && total_count > auto_recursive_fetch_max) {
      total_size = 0;
      total_count = 0;
      end_preparing();
    }
    total_count++;
    if (data.type != Action_type::remove) {
      if (file_info.is_file()) total_size += file_info.file_size;
    }
  } else {
    QStringList new_path_parts;
    new_path_parts << data.destination;
    foreach(File_system_engine::Iterator* i, fs_iterators_stack) {
      new_path_parts << i->get_current().file_name();
    }
    QString new_path = new_path_parts.join("/");
    if (file_info.is_folder) {
      if (data.type == Action_type::copy || data.type == Action_type::move) {
        fs_engine->make_directory(new_path);
      }
    } else {
      if (pending_operation != 0) {
        qWarning("pending_operation must be NULL in Action::process_one");
      }
      if (data.type == Action_type::copy) {
        pending_operation = fs_engine->copy(file_info.uri, new_path);
      } else if (data.type == Action_type::move) {
        pending_operation = fs_engine->move(file_info.uri, new_path);
      } else if (data.type == Action_type::remove) {
        fs_engine->remove(file_info.uri);
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
  qDebug() << "emit state_changed(state)";
  //state_delivery_in_process = true;
  signal_timer.restart();
}

void Action::run_iteration() {
  while(!(paused || blocked || phase == phase_finished)) {
    QApplication::processEvents();
    if (signal_timer.elapsed() > signal_interval) {
      send_state();
    }

    try {
      if (pending_operation) {
        process_pending_operation();
      } else if (fs_iterators_stack.top()->has_next()) {
        File_info fi = fs_iterators_stack.top()->get_next();
        current_file = fi;
        process_one(fi);
        if (fi.is_folder) {
          fs_iterators_stack.push(fs_engine->list(fi.uri));
        }
      } else {
        fs_iterators_stack.pop();
        if (fs_iterators_stack.isEmpty()) {
          if (phase == phase_preparing) {
            end_preparing();
          } else if (phase == phase_processing) {
            phase = phase_finished;
            emit finished();
            update_iteration_timer();
          } else {
            qWarning("unexpected value of phase");
          }
        } else {
          File_info fi = fs_iterators_stack.top()->get_current();
          postprocess_directory(fi);
        }
      }
    } catch (File_system_engine::Exception& e) {
      ask_question(Question_data(this, e));
    }
  }
}

void Action::end_preparing() {
  while(!fs_iterators_stack.isEmpty()) {
    delete fs_iterators_stack.pop();
  }
  fs_iterators_stack.push(new File_system_engine::Simple_iterator(data.targets));
  phase = phase_processing;
}

void Action::update_iteration_timer() {
  if (paused || blocked || phase == phase_finished) {
    //iteration_timer.stop();
  } else {
    run_iteration();
    //iteration_timer.start();
  }
}


void Action::question_answered(Error_reaction::Enum reaction) {
  //.....
}

void Action::set_paused(bool v) {
  paused = v;
  update_iteration_timer();
}

void Action::abort() {
  phase = phase_finished;
  //todo: display warning if answer was automatic
  emit finished();
  update_iteration_timer();
}

void Action::state_delivered() {
  qDebug() << "Action::state_delivered";
  //state_delivery_in_process = false;
}
