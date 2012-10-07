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

Action::Action(const Action_data &p_data)
: data(p_data)
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

/*QString Action::get_real_dir(QString uri) {
  if (uri.startsWith("/")) return uri;
  foreach(Gio_mount mount, mounts) {
    if (!mount.uri.isEmpty() && uri.startsWith(mount.uri)) {
      //convert uri e.g. "ftp://user@host/path"
      //to real path e.g. "/home/user/.gvfs/FTP as user on host/path"
      QString real_dir = mount.path + "/" + uri.mid(mount.uri.length());
      return real_dir;
    }
  }
  return uri;
}
*/

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
    foreach(File_info target, data.targets) {
      QString root_path = Directory::find_real_path(target.uri, mounts);
      if (root_path.endsWith("/")) root_path = root_path.left(root_path.length() - 1);
      QStack<QDirIterator*> stack;
      try {
        if (prepare) {
          prepare_one(root_path, root_path, QDir(root_path).exists());
        } else {
          process_one(root_path, root_path, QDir(root_path).exists(), true);
        }
      } catch(Skip_exception) {
        continue;
      }
      if (QDir(root_path).exists()) {
        stack.push(new QDirIterator(root_path, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags));
        while(!stack.isEmpty()) {
          while(stack.top()->hasNext()) {
            QString path = stack.top()->next();
            bool is_dir = QDir(path).exists();
            try {
              if (prepare) {
                prepare_one(path, root_path, is_dir);
              } else {
                process_one(path, root_path, is_dir, true);
              }
            } catch(Skip_exception) {
              continue;
            }
            if (is_dir) {
              stack.push(new QDirIterator(path, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags));
            }
          }
          delete stack.pop();
          if (!prepare && !stack.isEmpty()) {
            QString path = stack.top()->filePath();
            try {
              process_one(path, root_path, true, false);
            } catch(Skip_exception) {
              continue;
            }
          }
        }
        if (!prepare) {
          try {
            process_one(root_path, root_path, QDir(root_path).exists(), false);
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
  emit started();
  state.queue_id = queue->get_id();
  if (!data.destination.isEmpty()) {
    normalized_destination = Directory::find_real_path(data.destination, mounts);
    if (normalized_destination.endsWith("/")) {
      normalized_destination = normalized_destination.left(normalized_destination.length() - 1);
    }
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
    QString new_path = normalized_destination + "/" + relative_path;
    //qDebug() << "new_path: " << new_path;
    if (normalized_destination.startsWith(path)) {
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
              if (!QDir(path).isReadable()) {
                //Error_reaction r =
                ask_question(Question_data(tr("Directory '%1' is not readable").arg(path), Error_type::read_failed, true));

              }
              bool failure1 = QDir(new_path).exists();
              bool failure2 = QFile(new_path).exists();
              if (failure1 || failure2) {
                //Error_reaction r =
                ask_question(Question_data(tr("%1 '%2' already exists").arg(failure1? "Directory": "File").arg(new_path), Error_type::exists, true));
              }
              QString mkdir_error;
              if (!ridual_mkdir(new_path, mkdir_error)) {
                //Error_reaction r =
                ask_question(Question_data(tr("Failed to create directory '%1': %2").arg(new_path).arg(mkdir_error), Error_type::create_failed, true));
              }
            } // if dir_before
          } //copy or move
          if (data.type == Action_type::remove || data.type == Action_type::move) {
            if (!dir_before) {
              QString rmdir_error;
              if (!ridual_rmdir(path, rmdir_error)) {
                ask_question(Question_data(tr("Failed to remove directory '%1': %2").arg(path).arg(rmdir_error), Error_type::delete_failed, true));
              }
            }
          } //remove or move
        } else {
          if (data.type == Action_type::copy || data.type == Action_type::move) {
            std::ifstream file1;
            file1.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            std::ofstream file2;
            file2.exceptions(std::ofstream::failbit | std::ofstream::badbit | std::ofstream::eofbit);
            try {
              file1.open(path.toLocal8Bit(), std::ifstream::in | std::ifstream::binary);
            } catch (std::ios_base::failure e) {
              ask_question(Question_data(tr("Failed to read from file '%1': %2").arg(path).arg(errno_to_string()), Error_type::read_failed, false));
            }

            if (QFile(new_path).exists()) {
              ask_question(Question_data(tr("File '%1' already exists").arg(new_path), Error_type::exists, false));
            }

            try {
              file2.open(new_path.toLocal8Bit(), std::ifstream::out | std::ifstream::trunc | std::ifstream::binary);
            } catch (std::ios_base::failure e) {
              ask_question(Question_data(tr("Failed to create file '%1': %2").arg(new_path).arg(errno_to_string()), Error_type::create_failed, false));
            }

            int file_size = 0;
            try {
              file1.seekg(0, std::ios::end);
              file_size = file1.tellg();
              file1.seekg(0);
            } catch (std::ios_base::failure e) {
              ask_question(Question_data(tr("Failed to determine size of file '%1': %2").arg(path).arg(errno_to_string()), Error_type::read_failed, false));
            }

            while(file1.tellg() != file_size) {
              if (signal_timer.elapsed() > signal_interval) {
                process_events();
                state.current_action = tr("Copying '%1'").arg(path);
                state.current_progress = 1.0 * file1.tellg() / file_size;
                if (total_size > 0) {
                  state.total_progress = 1.0 * current_size / total_size;
                } else {
                  state.total_progress_text = tr("Files count: %1").arg(current_count);
                  state.total_progress = Action_state::UNKNOWN;
                }
                emit state_changed(state);
                signal_timer.restart();
              }
              char copy_buffer[BUFFER_SIZE];
              int count = 0;
              try {
                count = file1.readsome(copy_buffer, BUFFER_SIZE);
              } catch (std::ios_base::failure e) {
                ask_question(Question_data(tr("Failed to read from file '%1': %2").arg(path).arg(errno_to_string()), Error_type::read_failed, false));
              }
              try {
                file2.write(copy_buffer, count);
              } catch (std::ios_base::failure e) {
                ask_question(Question_data(tr("Failed to write to file '%1': %2").arg(new_path).arg(errno_to_string()), Error_type::write_failed, false));
              }
              current_size += count;
            } //end if file
            if (!QFile(new_path).setPermissions(QFile(path).permissions())) {
              qDebug() << "Failed to copy permissions";
            }

          } //copy or move
          if (data.type == Action_type::remove || data.type == Action_type::move) {
            QFile f(path);
            if (!f.remove()) {
              ask_question(Question_data(tr("Failed to remove file '%1': %2").arg(path).arg(f.errorString()), Error_type::delete_failed, false));
            }
          } //remove or move
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
