#include "File_action_task.h"
#include "Directory.h"
#include "Main_window.h"
#include <QDir>
#include <QQueue>
#include <QDebug>

Action::Action(Main_window *mw, const Action_data &p_data):
  main_window(mw),
  data(p_data),
  total_size(0),
  current_size(0),
  total_count(0),
  current_count(0),
  tree(0),
  current_item(0),
  file1(0),
  file2(0),
  questioned_item(0)
{
  run_state = run_state_preparing;
  qRegisterMetaType<Action_state>("Action_state");
  connect(this, SIGNAL(error(QString)), main_window, SLOT(fatal_error(QString)));
  connect(this, SIGNAL(error(QString)), this, SIGNAL(finished()));
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

void Action::file_copy_iteration() {
  if (file1->atEnd()) {
    delete_file_objects();
    current_count++;
    run_state = run_state_main;
    current_item->is_processed = true;
    return;
  }
  if (signal_timer.elapsed() > signal_interval) {
    tmp_state.current_action = tr("Copying '%1'").arg(file1->fileName());
    tmp_state.current_progress = tr("%1%").arg(100.0 * file1->pos() / file1->size(), 0, 'f', 1);
    if (total_size > 0) {
      tmp_state.total_progress = tr("%1%").arg(100.0 * current_size / total_size, 0, 'f', 1);
    } else {
      tmp_state.total_progress = tr("Files count: %1").arg(current_count);
    }
    emit state_changed(tmp_state);
    signal_timer.restart();
  }
  int count = file1->read(copy_buffer, BUFFER_SIZE);
  if (count < 0) {
    current_item->error_type = error_type_read_failed;
    if (!questioned_item) {
      questioned_item = current_item;
      send_question(current_item);
    }
    delete_file_objects();
    run_state = run_state_main;
    return;
  }
  current_size += count;
  int count2 = file2->write(copy_buffer, count);
  if (count2 < count) {
    current_item->error_type = error_type_write_failed;
    if (!questioned_item) {
      questioned_item = current_item;
      send_question(current_item);
    }
    delete_file_objects();
    run_state = run_state_main;
    return;
  }
  //todo: ability to resume copying after error
}

void Action::preparing_iteration() {
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
    tmp_state.current_action = tr("Getting file list at '%1'").arg(path);
    tmp_state.current_progress = tr("Files count: %1").arg(total_count);
    tmp_state.total_progress = tr("Unknown");
    emit state_changed(tmp_state);
    signal_timer.restart();
  }
  if (current_item->is_folder && current_item->error_type == error_type_read_failed) {
    if (!questioned_item) {
      questioned_item = current_item;
      send_question(current_item);
      tmp_state.errors_count++;
    }
    return;
  }
  if (!current_item->is_folder) {
    QFile file(path);
    total_count++;
    total_size += file.size();
  }
}

void Action::main_iteration() {
  current_item = current_item->find_next();
  if (data.type != action_copy) {
    emit error("not implemented");
    iteration_timer.stop();
    return;
  }
  if (!current_item) {
    iteration_timer.stop();
    if (!questioned_item) {
      emit finished();
    }
    return;
  }
  if (current_item->is_processed) {
    qDebug() << "item is processed";
    return;
  }
  //qDebug() << "new current_item = " << current_item << current_item->name;

  QString path = current_item->get_absolute_path();
  if (signal_timer.elapsed() > signal_interval) {
    tmp_state.current_action = tr("Copying '%1'").arg(path);
    tmp_state.current_progress = tr("%1%").arg(0);
    if (total_size > 0) {
      tmp_state.total_progress = tr("%1%").arg(100.0 * current_size / total_size, 0, 'f', 1);
    } else {
      tmp_state.total_progress = tr("Files count: %1").arg(current_count);
    }
    emit state_changed(tmp_state);
    signal_timer.restart();
  }
  QString new_path = data.destination + "/" + current_item->get_relative_path();
  if (current_item->is_folder) {
    if (QDir(new_path).exists()) {
      current_item->error_type = error_type_exists;
      if (!questioned_item) {
        questioned_item = current_item;
        send_question(current_item);
      }
      tmp_state.errors_count++;
      return;
    }
    if (!QDir().mkdir(new_path)) {
      current_item->error_type = error_type_create_failed;
      if (!questioned_item) {
        questioned_item = current_item;
        send_question(current_item);
      }
      tmp_state.errors_count++;
      return;
    }
    current_item->is_processed = true;
  } else {
    Q_ASSERT(file1 == 0);
    Q_ASSERT(file2 == 0);
    file1 = new QFile(path);
    file2 = new QFile(new_path);
    if (file2->exists()) {
      current_item->error_type = error_type_exists;
      if (!questioned_item) {
        questioned_item = current_item;
        send_question(current_item);
      }
      tmp_state.errors_count++;
      delete_file_objects();
      return;
    }
    if (!file2->open(QFile::WriteOnly)) {
      current_item->error_type = error_type_create_failed;
      if (!questioned_item) {
        questioned_item = current_item;
        send_question(current_item);
      }
      tmp_state.errors_count++;
      delete_file_objects();
      return;
    }
    if (!file1->open(QFile::ReadOnly)) {
      current_item->error_type = error_type_read_failed;
      if (!questioned_item) {
        questioned_item = current_item;
        send_question(current_item);
      }
      tmp_state.errors_count++;
      delete_file_objects();
      return;
    }
    run_state = run_state_copy_file;
    return;
  } // if current_item is file
}

void Action::delete_file_objects() {
  delete file1;
  delete file2;
  file1 = 0;
  file2 = 0;
}

void Action::send_question(Directory_tree_item *item) {
  QString path = item->get_absolute_path();
  QString new_path = data.destination + "/" + item->get_relative_path();
  QString file_or_dir = item->is_folder? tr("Directory"): tr("File");
  QString error_string;
  if (item->error_type == no_error) {
    qWarning("Action::send_question: no error given");
    return;
  }
  if (item->error_type == error_type_not_found) {
    error_string = tr("%1 '%2' not found.").arg(file_or_dir).arg(path);
  } else if (item->error_type == error_type_read_failed) {
    error_string = tr("%1 '%2' can't be read.").arg(file_or_dir).arg(path);
  } else if (item->error_type == error_type_create_failed) {
    if (item->is_folder) {
      error_string = tr("Failed to create directory '%1'.").arg(new_path);
    } else {
      error_string = tr("Failed to create file '%1'.").arg(new_path);
    }
  } else if (item->error_type == error_type_write_failed) {
    Q_ASSERT(item->is_folder == false);
    error_string = tr("Failed to write file '%1'.").arg(new_path);
  } else if (item->error_type == error_type_delete_failed) {
    error_string = tr("%1 '%2' can't be deleted.").arg(file_or_dir).arg(path);
  } else if (item->error_type == error_type_exists) {
    error_string = tr("%1 '%2' already exists.").arg(file_or_dir).arg(new_path);
  }
  emit question(error_string, item->error_type, item->is_folder);

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

  tmp_state.current_action = tr("Starting");
  emit state_changed(tmp_state);
  iteration_timer.start();
}

void Action::iteration() {
  if (!iteration_timer.isActive()) return;

  if (run_state == run_state_preparing) {
    if (data.recursive_fetch_option == recursive_fetch_off) {
      run_state = run_state_main; // cancel preparing
      return;
    }
    preparing_iteration();
  } else if (run_state == run_state_main) {
    main_iteration();
  } else if (run_state == run_state_copy_file) {
    file_copy_iteration();
  } // if run_state
}

void Action::question_answered(Error_reaction reaction) {
  if (!questioned_item) {
    qWarning("warning: Action::question_answered called when there is no questioned_item");
    return;
  }
  //...

}

