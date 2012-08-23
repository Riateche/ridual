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

QString Action::get_real_dir(QString uri) {
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

Error_reaction::Enum Action::ask_question(Question_data data) {
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
      QString root_path = get_real_dir(target.full_path);
      if (root_path.endsWith("/")) root_path = root_path.left(root_path.length() - 1);
      QStack<QDirIterator*> stack;
      try {
        if (prepare) {
          prepare_one(root_path, root_path, QDir(root_path).exists());
        } else {
          process_one(root_path, root_path, QDir(root_path).exists());
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
                process_one(path, root_path, is_dir);
              }
            } catch(Skip_exception) {
              continue;
            }
            if (is_dir) {
              stack.push(new QDirIterator(path, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags));
            }
          }
          delete stack.pop();
        }
      }
    }
  } catch(Prepare_finished_exception) {
    return;
  }
}

void Action::prepare_one(const QString &path, const QString &root_path, bool is_dir) {
  if (data.recursive_fetch_option == recursive_fetch_auto && total_count > auto_recursive_fetch_max) {
    total_size = 0;
    total_count = 0;
    throw Prepare_finished_exception();
  }
  if (signal_timer.elapsed() > signal_interval) {
    state.current_action = tr("Calculating at '%1'").arg(path);
    state.current_progress = tr("Unknown");
    state.total_progress = tr("%1 files found").arg(total_count);
    emit state_changed(state);
    signal_timer.restart();
  }
  total_count++;
  if (!is_dir) total_size += QFile(path).size();
  Q_UNUSED(root_path);
}

void Action::run() {
  state.queue_id = queue->get_id();
  if (!data.destination.isEmpty()) {
    normalized_destination = get_real_dir(data.destination);
    if (normalized_destination.endsWith("/")) {
      normalized_destination = normalized_destination.left(normalized_destination.length() - 1);
    }
  }

  if (data.type != Action_type::copy) {
    //emit error("not implemented");
    //iteration_timer.stop();
    qWarning("Action: unknown data.type");
    return;
  }

  state.current_action = tr("Starting");
  emit state_changed(state);
  signal_timer.start();

  try {
    if (data.recursive_fetch_option != recursive_fetch_off) {
      iterate_all(true);
    }
    iterate_all(false);
  } catch (Abort_exception) {
    return;
  }
}

void Action::process_one(const QString& path, const QString& root_path, bool is_dir) {
  try {
    process_events();
    //qDebug() << "path: " << path;
    int index = root_path.lastIndexOf("/");
    QString relative_path = path.mid(index + 1);
    QString new_path = normalized_destination + "/" + relative_path;
    //qDebug() << "new_path: " << new_path;
    if (normalized_destination.startsWith(path)) {
      ask_question(Question_data(tr("Failed to copy '%1' to '%2': can't copy folder inside itself.").arg(path).arg(new_path), Error_type::destination_inside_source, is_dir));
    }

    bool retry_asked = false;
    do { //for retry
      try { //also for retry
        if (signal_timer.elapsed() > signal_interval) {
          state.current_action = tr("Copying '%1'").arg(path);
          state.current_progress = tr("%1%").arg(0);
          if (total_size > 0) {
            state.total_progress = tr("%1%").arg(100.0 * current_size / total_size, 0, 'f', 1);
          } else {
            state.total_progress = tr("Files count: %1").arg(current_count);
          }
          emit state_changed(state);
          signal_timer.restart();
        }

        if (is_dir) {
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
        } else {
          QFile file1(path);
          QFile file2(new_path);
          if (file2.exists()) {
            ask_question(Question_data(tr("File '%1' already exists").arg(new_path), Error_type::exists, false));
          }
          if (!file1.open(QFile::ReadOnly)) {
            ask_question(Question_data(tr("Failed to read from file '%1': %2").arg(path).arg(file1.errorString()), Error_type::read_failed, false));
          }
          if (!file2.open(QFile::WriteOnly)) {
            ask_question(Question_data(tr("Failed to create file '%1': %2").arg(new_path).arg(file2.errorString()), Error_type::create_failed, false));
          }
          while(!file1.atEnd()) {
            process_events();
            if (signal_timer.elapsed() > signal_interval) {
              state.current_action = tr("Copying '%1'").arg(path);
              state.current_progress = tr("%1%").arg(100.0 * file1.pos() / file1.size(), 0, 'f', 1);
              if (total_size > 0) {
                state.total_progress = tr("%1%").arg(100.0 * current_size / total_size, 0, 'f', 1);
              } else {
                state.total_progress = tr("Files count: %1").arg(current_count);
              }
              emit state_changed(state);
              signal_timer.restart();
            }
            char copy_buffer[BUFFER_SIZE];
            int count = file1.read(copy_buffer, BUFFER_SIZE);
            if (count < 0) {
              ask_question(Question_data(tr("Failed to read from file '%1': %2").arg(path).arg(file1.errorString()), Error_type::read_failed, false));
            }
            int count2 = file2.write(copy_buffer, count);
            if (count2 < count) {
              ask_question(Question_data(tr("Failed to write to file '%1': %2").arg(new_path).arg(file2.errorString()), Error_type::write_failed, false));
            }
            current_size += count;
          }
        } //end if file
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

void Action::toggle_pause() {
  paused = !paused;
}

void Action::abort() {
  cancelled = true;
}
