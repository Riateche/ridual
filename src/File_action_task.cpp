#include "File_action_task.h"
#include "Directory.h"
#include "Main_window.h"
#include <QDir>
#include <QQueue>

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

}

void File_action_task::run(File_action_queue *p_queue) {
  queue = p_queue;

  mounts = main_window->get_gio_mounts(); //thread-safe
  File_action_state state;
  signal_timer.start();
  if (recursive_fetch_option == recursive_fetch_on ||
      recursive_fetch_option == recursive_fetch_auto) {
    QQueue<QString> paths_queue;
    foreach(File_info target, targets) {
      paths_queue.enqueue(get_real_dir(target.full_path));
    }
    while(!paths_queue.isEmpty()) {
      if (recursive_fetch_option == recursive_fetch_auto &&
          total_count > 1000) {
        total_count = 0;
        total_size = 0;
        break;
      }

      QString path = paths_queue.dequeue();
      if (signal_timer.elapsed() > signal_interval) {
        state.current_action = tr("Getting file list at '%1'").arg(path);
        state.current_progress = -1;
        state.total_progress = -1;
        emit state_changed(state);
        signal_timer.restart();
      }
      QDir dir(path);
      if (dir.exists()) {
        if (!dir.isReadable()) {
          emit error(tr("Directory '%1'' is not readable").arg(path));
          return;
        }
        total_count++;
        foreach(QFileInfo fi, dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name)) {
          paths_queue.enqueue(fi.absoluteFilePath());
        }
      } else {
        QFile file(path);
        if (file.exists()) {
          if (file.isReadable()) {
            total_count++;
            total_size += file.size();
          } else {
            emit error(tr("File '%1'' is not readable").arg(path));
            return;
          }
        } else {
          emit error(tr("File or directory '%1' not found").arg(path));
          return;
        }
      }
    }
  }

  QQueue<QString> paths_queue;
  foreach(File_info target, targets) {
    paths_queue.enqueue(get_real_dir(target.full_path));
  }
  while(!paths_queue.isEmpty()) {
    QString path = paths_queue.dequeue();
    if (signal_timer.elapsed() > signal_interval) {
      state.current_action = tr("Copying '%1'").arg(path);
      state.current_progress = 0;
      state.total_progress = 1.0 * current_size / total_size;
      emit state_changed(state);
      signal_timer.restart();
    }
    QDir dir(path);
    if (dir.exists()) {
      if (!dir.isReadable()) {
        emit error(tr("Directory '%1'' is not readable").arg(path));
        return;
      }
      total_count++;
      foreach(QFileInfo fi, dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name)) {
        paths_queue.enqueue(fi.absoluteFilePath());
      }
    } else {
      QFile file(path);
      if (file.exists()) {
        if (file.isReadable()) {
          total_count++;
          total_size += file.size();
        } else {
          emit error(tr("File '%1'' is not readable").arg(path));
          return;
        }
      } else {
        emit error(tr("File or directory '%1' not found").arg(path));
        return;
      }
    }
  }




  emit error("not implemented");
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
