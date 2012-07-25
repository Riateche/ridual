#include "File_action_task.h"
#include "Directory.h"
#include "Main_window.h"
#include <QDir>
#include <QQueue>

#define BUFFER_SIZE 65536

File_action_task::File_action_task(Main_window *mw, const File_action_type &p_action_type, File_info_list p_targets, QString p_destination):
  main_window(mw),
  action_type(p_action_type),
  targets(p_targets),
  destination(p_destination),
  recursive_fetch_option(recursive_fetch_auto),
  link_type(link_type_soft_absolute),
  queue(0),
  total_count(0),
  total_size(0),
  current_count(0),
  current_size(0)
{
  qRegisterMetaType<File_action_state>("File_action_state");
  connect(this, SIGNAL(error(QString)), main_window, SLOT(fatal_error(QString)));
}

File_action_task::~File_action_task() {
  foreach(Directory_tree_item* i, trees) delete i;
  trees.clear();
}

void File_action_task::run(File_action_queue *p_queue) {
  queue = p_queue;

  mounts = main_window->get_gio_mounts(); //thread-safe
  File_action_state state;
  signal_timer.start();

  foreach(File_info target, targets) {
    Directory_tree_item* item = new Directory_tree_item();
    item->is_folder = target.is_folder();
    item->parent_path = target.parent_folder;
    item->name = target.full_name;
    trees << item;
  }

  if (recursive_fetch_option == recursive_fetch_on ||
      recursive_fetch_option == recursive_fetch_auto) {
    foreach(Directory_tree_item* tree, trees) {
      Directory_tree_item* item = 0;
      while(item = item? item->find_next(): tree) {
        if (recursive_fetch_option == recursive_fetch_auto &&
            total_count > auto_recursive_fetch_max) {
          total_count = 0;
          total_size = 0;
          break;
        }

        QString path = item->get_absolute_path();
        if (signal_timer.elapsed() > signal_interval) {
          state.current_action = tr("Getting file list at '%1'").arg(path);
          state.current_progress = tr("Files count: %1").arg(total_count);
          state.total_progress = tr("Unknown");
          emit state_changed(state);
          signal_timer.restart();
        }
        if (item->is_folder && item->error == error_cannot_read) {
          emit error(tr("Directory '%1' is not readable").arg(path));
          //todo: error reports
          return;
        }
        if (!item->is_folder) {
          QFile file(path);
          //if (file.isReadable()) {
            total_count++;
            total_size += file.size();
          //} else {
          //  emit error(tr("File '%1' is not readable").arg(path));
          //  return;
          //}
        }
      }
    }
  }

  if (action_type != file_action_copy) {
    emit error("not implemented");
  }

  foreach(Directory_tree_item* tree, trees) {
    Directory_tree_item* item = 0;
    while(item = item? item->find_next(): tree) {
      QString path = item->get_absolute_path();
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
      QString new_path = destination + "/" + item->get_relative_path();
      if (item->is_folder) {
        if (QDir(new_path).exists()) {
          emit error(tr("Directory '%1' already exists").arg(new_path));
          return;
        }
        if (!QDir().mkdir(new_path)) {
          emit error(tr("Failed to create directory '%1'").arg(new_path));
          return;
        }
      } else {
        QFile file1(path);
        QFile file2(new_path);
        if (file2.exists()) {
          emit error(tr("File '%1' already exists").arg(new_path));
          return;
        }
        if (!file2.open(QFile::WriteOnly)) {
          emit error(tr("Failed to open file '%1' for writing").arg(new_path));
          return;
        }
        if (!file1.open(QFile::ReadOnly)) {
          emit error(tr("Failed to open file '%1' for reading").arg(path));
          return;
        }
        char buffer[BUFFER_SIZE];
        while(!file1.atEnd()) {
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
          int count = file1.read(buffer, BUFFER_SIZE);
          if (count > 0) {
            current_size += count;
            file2.write(buffer, count);
          } else {
            emit error(tr("Failed to read from file '%1': %2")
                       .arg(path)
                       .arg(file1.errorString()));
            return;
          }
        }
      }
      current_count++;
    }
  }

  deleteLater();
}

QString File_action_task::get_real_dir(QString uri) {
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
