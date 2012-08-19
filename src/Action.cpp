#include "Action.h"
#include "Directory.h"
#include "Main_window.h"
#include <QDir>
#include <QQueue>
#include <QDebug>
#include "Action_queue.h"
#include <QApplication>
#include <QStack>

Action::Action(Main_window *mw, const Action_data &p_data):
  main_window(mw),
  data(p_data)
{
  total_size = 0;
  current_size = 0;
  total_count = 0;
  current_count = 0;
  paused = false;
  cancelled = false;
  error_reaction = error_reaction_undefined;
  queue = 0;

  qRegisterMetaType<Action_state>("Action_state");
  qRegisterMetaType<Question_data>("Question_data");
  //connect(this, SIGNAL(error(QString)), main_window, SLOT(fatal_error(QString)));
  //connect(this, SIGNAL(error(QString)), this, SIGNAL(finished()));
  //connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

Action::~Action()
{
}

void Action::set_queue(Action_queue *q) {
  queue = q;
}

QString Action::get_real_dir(QString uri) {
  if (uri.startsWith("/")) return uri;
  foreach(gio::Mount mount, mounts) {
    if (!mount.uri.isEmpty() && uri.startsWith(mount.uri)) {
      //convert uri e.g. "ftp://user@host/path"
      //to real path e.g. "/home/user/.gvfs/FTP as user on host/path"
      QString real_dir = mount.path + "/" + uri.mid(mount.uri.length());
      return real_dir;
    }
  }
  return uri;
}

Error_reaction Action::ask_question(Question_data data) {
  error_reaction = error_reaction_undefined;
  data.action = this;
  emit question(data);
  while(error_reaction == error_reaction_undefined) {
    if (cancelled) throw Action_abort_exception();
    queue->msleep(sleep_interval);
    QApplication::processEvents();
  }
  if (error_reaction == error_reaction_abort) throw Action_abort_exception();
  if (error_reaction == error_reaction_retry) throw Action_retry_exception();
  if (error_reaction == error_reaction_skip)  throw Action_skip_exception();
  return error_reaction;
}

void Action::process_events() {
  do {
    if (cancelled) throw Action_abort_exception();
    queue->msleep(sleep_interval);
    QApplication::processEvents();
  } while(paused);
}

void Action::run() {
  //todo: disallow copy folder inside itself
  Action_state state;
  state.queue_id = queue->get_id();

  mounts = main_window->get_gio_mounts(); //thread-safe
  if (!data.destination.isEmpty()) {
    normalized_destination = get_real_dir(data.destination);
    if (normalized_destination.endsWith("/")) {
      normalized_destination = normalized_destination.left(normalized_destination.length() - 1);
    }
  }

  if (data.type != action_copy) {
    //emit error("not implemented");
    //iteration_timer.stop();
    qWarning("Action: unknown data.type");
    return;
  }

  state.current_action = tr("Starting");
  emit state_changed(state);

  try {
    foreach(File_info target, data.targets) {
      QString root_path = get_real_dir(target.full_path);
      if (root_path.endsWith("/")) root_path = root_path.left(root_path.length() - 1);
      QStack<QDirIterator*> stack;
      try {
        process_one(root_path, root_path, state);
      } catch(Action_skip_exception) {
        continue;
      }
      if (QDir(root_path).exists()) {
        stack.push(new QDirIterator(root_path, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags));
        while(!stack.isEmpty()) {
          while(stack.top()->hasNext()) {
            QString path = stack.top()->next();
            try {
              process_one(path, root_path, state);
            } catch(Action_skip_exception) {
              continue;
            }
            if (QDir(path).exists()) {
              stack.push(new QDirIterator(path, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags));
            }
          }
          delete stack.pop();
        }
      }

    }
  } catch (Action_abort_exception) {
    return;
  }

  deleteLater();
}

void Action::process_one(const QString& path, const QString& root_path, Action_state& state) {
  bool is_folder = QDir(path).exists();
  try {
    process_events();
    qDebug() << "path: " << path;
    int index = root_path.lastIndexOf("/");
    QString relative_path = path.mid(index + 1);
    QString new_path = normalized_destination + "/" + relative_path;
    qDebug() << "new_path: " << new_path;
    if (normalized_destination.startsWith(path)) {
      ask_question(Question_data(tr("Failed to copy '%1' to '%2': can't copy folder inside itself.").arg(path).arg(new_path), error_type_destination_inside_source, is_folder));
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

        if (is_folder) {
          if (!QDir(path).isReadable()) {
            //Error_reaction r =
            ask_question(Question_data(tr("Directory '%1' is not readable").arg(path), error_type_read_failed, true));

          }
          bool failure1 = QDir(new_path).exists();
          bool failure2 = QFile(new_path).exists();
          if (failure1 || failure2) {
            //Error_reaction r =
            ask_question(Question_data(tr("%1 '%2' already exists").arg(failure1? "Directory": "File").arg(new_path), error_type_exists, true));
          }
          if (!QDir().mkdir(new_path)) {
            //Error_reaction r =
            ask_question(Question_data(tr("Failed to create directory '%1'.").arg(new_path), error_type_create_failed, true));
          }
        } else {
          QFile file1(path);
          QFile file2(new_path);
          if (file2.exists()) {
            ask_question(Question_data(tr("File '%1' already exists").arg(new_path), error_type_exists, false));
          }
          if (!file1.open(QFile::ReadOnly)) {
            ask_question(Question_data(tr("Failed to read from file '%1'.").arg(path), error_type_read_failed, false));
          }
          if (!file2.open(QFile::WriteOnly)) {
            ask_question(Question_data(tr("Failed to create file '%1'.").arg(new_path), error_type_create_failed, false));
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
              ask_question(Question_data(tr("Failed to read from file '%1'.").arg(path), error_type_read_failed, false));
            }
            int count2 = file2.write(copy_buffer, count);
            if (count2 < count) {
              ask_question(Question_data(tr("Failed to write to file '%1'.").arg(new_path), error_type_write_failed, false));
            }
            current_size += count;
          }
        } //end if file
        current_count++;
      } catch (Action_retry_exception) {
        retry_asked = true;
      }
    } while(retry_asked);
  } catch (Action_skip_exception) {
    if (is_folder) throw;
    return;
  }
}


void Action::question_answered(Error_reaction reaction) {
  error_reaction = reaction;
}

void Action::toggle_pause() {
  paused = !paused;
}

void Action::abort() {
  cancelled = true;
}
