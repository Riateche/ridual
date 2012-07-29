#include "File_action_task.h"
#include "Directory.h"
#include "Main_window.h"
#include <QDir>
#include <QQueue>
#include <QDebug>

Action::Action(Main_window *mw, const Action_data &p_data):
  main_window(mw),
  data(p_data),
  total_count(0),
  total_size(0),
  current_count(0),
  current_size(0),
  tree(0),
  current_item(0),
  file1(0),
  file2(0)
{
  run_state = run_state_preparing;
  qRegisterMetaType<Action_state>("Action_state");
  connect(this, SIGNAL(error(QString)), main_window, SLOT(fatal_error(QString)));
  connect(&iteration_timer, SIGNAL(timeout()), this, SLOT(iteration()));
  connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

Action::~Action() {
  if (tree) delete tree;
  tree = 0;
  qDebug() << "~Action";
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

void Action::run() {
  mounts = main_window->get_gio_mounts(); //thread-safe
  signal_timer.start();

  tree = new Directory_tree_item();
  tree->is_folder = true;
  tree->is_folder_read = true;

  QList<Directory_tree_item*> trees;
  foreach(File_info target, data.targets) {
    Directory_tree_item* item = new Directory_tree_item();
    item->is_folder = target.is_folder();
    item->parent_path = target.parent_folder;
    item->name = target.full_name;
    item->parent = tree;
    tree->children << item;
  }
  current_item = tree;
  iteration_timer.start();
}

void Action::iteration() {
  if (!iteration_timer.isActive()) return;
  Action_state state;
  state.queue_id = queue_id;
  if (run_state == run_state_preparing) {
    if (data.recursive_fetch_option == recursive_fetch_off) {
      run_state = run_state_main; // cancel preparing
      return;
    }
    current_item = current_item->find_next();
    if (!current_item) {
      run_state = run_state_main; // preparing iteration ended
      current_item = tree; //moving cursor to the start
      return;
    }
    if (data.recursive_fetch_option == recursive_fetch_auto &&
        total_count > auto_recursive_fetch_max) {
      total_count = 0;
      total_size = 0;
      run_state = run_state_main; // cancel preparing
      current_item = tree; //moving cursor to the start
      return;
    }

    QString path = current_item->get_absolute_path();
    if (signal_timer.elapsed() > signal_interval) {
      state.current_action = tr("Getting file list at '%1'").arg(path);
      state.current_progress = tr("Files count: %1").arg(total_count);
      state.total_progress = tr("Unknown");
      emit state_changed(state);
      signal_timer.restart();
    }
    if (current_item->is_folder && current_item->error == error_cannot_read) {
      emit error(tr("Directory '%1' is not readable").arg(path));
      iteration_timer.stop();
      //todo: errors
      return;
    }
    if (!current_item->is_folder) {
      QFile file(path);
      total_count++;
      total_size += file.size();
    }
  } else if (run_state == run_state_main) {
    current_item = current_item->find_next();
    if (data.type != action_copy) {
      emit error("not implemented");
      iteration_timer.stop();
      return;
    }
    if (!current_item) {
      emit finished();
      iteration_timer.stop();
      return;
    }
    qDebug() << "new current_item = " << current_item << current_item->name;

    QString path = current_item->get_absolute_path();
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
    QString new_path = data.destination + "/" + current_item->get_relative_path();
    if (current_item->is_folder) {
      if (QDir(new_path).exists()) {
        emit error(tr("Directory '%1' already exists").arg(new_path));
        iteration_timer.stop();
        return;
      }
      if (!QDir().mkdir(new_path)) {
        emit error(tr("Failed to create directory '%1'").arg(new_path));
        iteration_timer.stop();
        return;
      }
    } else {
      run_state = run_state_copy_file;
      Q_ASSERT(file1 == 0);
      Q_ASSERT(file2 == 0);
      file1 = new QFile(path);
      file2 = new QFile(new_path);
      if (file2->exists()) {
        emit error(tr("File '%1' already exists").arg(new_path));
        iteration_timer.stop();
        return; //todo: error
      }
      if (!file2->open(QFile::WriteOnly)) {
        emit error(tr("Failed to open file '%1' for writing").arg(new_path));
        iteration_timer.stop();
        return; //todo: error
      }
      if (!file1->open(QFile::ReadOnly)) {
        emit error(tr("Failed to open file '%1' for reading").arg(path));
        iteration_timer.stop();
        return; //todo: error
      }
      return;
    } // if current_item is file
  } else if (run_state == run_state_copy_file) {
    if (file1->atEnd()) {
      delete file1;
      delete file2;
      file1 = 0;
      file2 = 0;
      current_count++;
      run_state = run_state_main;
      return;
    }
    if (signal_timer.elapsed() > signal_interval) {
      state.current_action = tr("Copying '%1'").arg(file1->fileName());
      state.current_progress = tr("%1%").arg(100.0 * file1->pos() / file1->size(), 0, 'f', 1);
      if (total_size > 0) {
        state.total_progress = tr("%1%").arg(100.0 * current_size / total_size, 0, 'f', 1);
      } else {
        state.total_progress = tr("Files count: %1").arg(current_count);
      }
      emit state_changed(state);
      signal_timer.restart();
    }
    int count = file1->read(copy_buffer, BUFFER_SIZE);
    if (count > 0) {
      current_size += count;
      file2->write(copy_buffer, count);
    } else {
      emit error(tr("Failed to read from file '%1': %2")
                 .arg(file1->fileName())
                 .arg(file1->errorString()));
      iteration_timer.stop();
      return;
    }
  } // if run_state
}
