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


Action::Action(const Action_data &p_data)
: data(p_data)
, fs_engine(0)
{
  total_size = 0;
  current_size = 0;
  total_count = 0;
  current_count = 0;
  paused = false;
  cancelled = false;
  error_reaction = Error_reaction::undefined;
  queue = 0;

  qRegisterMetaType<Action_state>("Action_state");
  qRegisterMetaType<Question_data>("Question_data");
}

Action::~Action() {

}

void Action::set_queue(Action_queue *q) {
  queue = q;
}

void Action::set_fs_engine(File_system_engine *v) {
  fs_engine = v;
}

Error_reaction::Enum Action::ask_question(Question_data data) {
  state.current_action = tr("Waiting for answer");
  if (state.current_progress == Action_state::UNKNOWN) {
    state.current_progress = Action_state::DISABLED;
  }
  if (state.total_progress == Action_state::UNKNOWN) {
    state.total_progress = Action_state::DISABLED;
  }
  emit state_changed(state);
  error_reaction = Error_reaction::undefined;
  data.action = this;
  emit question(data);
  while(error_reaction == Error_reaction::undefined) {
    if (cancelled) throw Abort_exception();
    queue->msleep(sleep_interval);
    QApplication::processEvents();
  }
  if (error_reaction == Error_reaction::abort) throw Abort_exception();
  if (error_reaction == Error_reaction::retry) throw Retry_exception();
  if (error_reaction == Error_reaction::skip)  throw Skip_exception();
  return error_reaction;
}

void Action::process_events() {
  do {
    if (cancelled) throw Abort_exception();
    queue->msleep(sleep_interval);
    QApplication::processEvents();
  } while(paused);
}

void Action::iterate_all(bool prepare) {
  try {
    foreach(QString target_uri, data.targets) {
      //QString root_path = Directory::find_real_path(target_uri, mounts);
      if (target_uri.endsWith("/")) target_uri = target_uri.left(target_uri.length() - 1);
      QStack<File_system_engine::Iterator*> stack;
      try {
        if (prepare) {
          prepare_one(target_uri, target_uri, QDir(target_uri).exists());
        } else {
          process_one(target_uri, target_uri, QDir(target_uri).exists(), true);
        }
      } catch(Skip_exception) {
        continue;
      }
      //todo: catch list() exceptions here
      if (QDir(target_uri).exists()) {
        stack.push(fs_engine->list(target_uri)); //new QDirIterator(target_uri, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags));
        while(!stack.isEmpty()) {
          while(stack.top()->has_next()) {
            File_info i = stack.top()->get_next();
            //QString path = i.uri; // stack.top()->next();
            //bool is_dir = QDir(path).exists();
            try {
              if (prepare) {
                prepare_one(i.uri, target_uri, i.is_folder);
              } else {
                process_one(i.uri, target_uri, i.is_folder, true);
              }
            } catch(Skip_exception) {
              continue;
            }
            if (i.is_folder) {
              stack.push(fs_engine->list(i.uri)); //new QDirIterator(path, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags));
            }
          }
          delete stack.pop();
          if (!prepare && !stack.isEmpty()) {
            File_info i = stack.top()->get_current();
            try {
              process_one(i.uri, target_uri, true, false);
            } catch(Skip_exception) {
              continue;
            }
          }
        }
        if (!prepare) {
          try {
            process_one(target_uri, target_uri, QDir(target_uri).exists(), false);
          } catch(Skip_exception) {
            continue;
          }
        }
      }
    }
  } catch(Prepare_finished_exception) {
    return;
  }
}

void Action::prepare_one(const QString &path, const QString &root_path, bool is_dir) {
  if (data.recursive_fetch_option == recursive_fetch_auto && data.type == Action_type::remove) {
    //by default recursive fetch is disabled for remove tasks
    total_size = 0;
    total_count = 0;
    throw Prepare_finished_exception();
  }

  if (data.recursive_fetch_option == recursive_fetch_auto && total_count > auto_recursive_fetch_max) {
    total_size = 0;
    total_count = 0;
    throw Prepare_finished_exception();
  }
  if (signal_timer.elapsed() > signal_interval) {
    process_events();
    state.current_action = tr("Recursive fetch at '%1'").arg(path);
    state.current_progress = Action_state::DISABLED;
    state.current_progress_text = "";
    state.total_progress_text = tr("Files count: %1").arg(total_count);
    state.total_progress = Action_state::UNKNOWN;
    emit state_changed(state);
    signal_timer.restart();
  }
  total_count++;
  if (data.type != Action_type::remove) {
    if (!is_dir) total_size += QFile(path).size();
  }
  Q_UNUSED(root_path);
}

void Action::run() {
  if (!fs_engine) {
    qFatal("no fs engine for action");
    return;
  }
  fs_engine->moveToThread(thread()); //dangerous! need to test
  emit started();
  state.queue_id = queue->get_id();
  if (data.destination.endsWith("/")) {
    data.destination = data.destination.left(data.destination.length() - 1);
  }

  state.current_action = tr("Starting");
  emit state_changed(state);
  signal_timer.restart();

  try {
    if (data.recursive_fetch_option != recursive_fetch_off) {
      iterate_all(true);
    }
    iterate_all(false);
  } catch (Abort_exception) {
    return;
  }
}

void Action::process_one(const QString& path, const QString& root_path, bool is_dir, bool dir_before) {
  try {
    //qDebug() << "path: " << path;
    int index = root_path.lastIndexOf("/");
    QString relative_path = path.mid(index + 1);
    QString new_path = data.destination + "/" + relative_path;
    //qDebug() << "new_path: " << new_path;
    if (data.destination.startsWith(path)) {
      ask_question(Question_data(tr("Failed to copy '%1' to '%2': can't copy folder inside itself.").arg(path).arg(new_path), Error_type::destination_inside_source, is_dir));
    }

    bool retry_asked;
    do { //for retry
      retry_asked = false;
      try { //also for retry
        if (signal_timer.elapsed() > signal_interval) {
          process_events();
          if (data.type == Action_type::copy || data.type == Action_type::move) {
            if (data.type == Action_type::copy) {
              state.current_action = tr("Copying");
            } else {
              state.current_action = tr("Moving");
            }
            state.current_action += tr(" '%1'").arg(path);
            state.current_progress = Action_state::DISABLED;
            state.current_progress_text = "";
            if (total_size > 0) {
              state.total_progress = 1.0 * current_size / total_size;
            } else {
              state.total_progress_text = tr("Files count: %1").arg(current_count);
              state.total_progress = Action_state::UNKNOWN;
            }
          } else if (data.type == Action_type::remove) {
            state.current_action = tr("Deleting '%1'").arg(path);
            state.current_progress = Action_state::DISABLED;
            state.current_progress_text = "";
            if (total_count > 0) {
              state.total_progress = 1.0 * current_count / total_count;
            } else {
              state.total_progress_text = tr("Files count: %1").arg(current_count);
              state.total_progress = Action_state::UNKNOWN;
            }
          }
          emit state_changed(state);
          signal_timer.restart();
        }

        if (is_dir) {
          if (data.type == Action_type::copy || data.type == Action_type::move) {
            if (dir_before) {
              fs_engine->make_directory(new_path);
            } // if dir_before
          } //copy or move
          if (data.type == Action_type::remove || data.type == Action_type::move) {
            if (!dir_before) {
              fs_engine->remove(path);
              /*QString rmdir_error;
              if (!ridual_rmdir(path, rmdir_error)) {
                ask_question(Question_data(tr("Failed to remove directory '%1': %2").arg(path).arg(rmdir_error), Error_type::delete_failed, true));
              }*/
            }
          } //remove or move
        } else { // above - for dirs, below - for files
          File_system_engine::Operation* operation = 0;
          if (data.type == Action_type::copy) {
            operation = fs_engine->copy(path, new_path);
          } else if (data.type == Action_type::move) {
            operation = fs_engine->move(path, new_path);
          } else if (data.type == Action_type::remove) {
            fs_engine->remove(path);
          } //remove or move
          if (operation) {
            while(!operation->is_finished()) {
              if (signal_timer.elapsed() > signal_interval) {
                process_events();
                //todo: add real operation name here
                state.current_action = tr("Processing '%1'").arg(path);
                state.current_progress = operation->get_progress();
                if (total_size > 0) {
                  //todo: reimplement progress calculation
                  state.total_progress = 1.0 * current_size / total_size;
                } else {
                  state.total_progress_text = tr("Files count: %1").arg(current_count);
                  state.total_progress = Action_state::UNKNOWN;
                }
                emit state_changed(state);
                signal_timer.restart();
              }
              operation->run_iteration();
            }
            delete operation;
          }
        } //is file
        current_count++;
      } catch (Retry_exception) {
        retry_asked = true;
      }
    } while(retry_asked);
  } catch (Skip_exception) {
    if (is_dir) throw;
    return;
  }
}


void Action::question_answered(Error_reaction::Enum reaction) {
  error_reaction = reaction;
}

void Action::set_paused(bool v) {
  paused = v;
}

void Action::abort() {
  cancelled = true;
}
