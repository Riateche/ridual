#include "Directory.h"
#include <QDir>
#include "Task.h"
#include <QDebug>
#include "Main_window.h"

Directory::Directory(Main_window* mw, QString p_uri) :
  main_window(mw),
  uri(p_uri)
{
  if (uri.startsWith("~")) {
    uri = QDir::homePath() + uri.mid(1);
  }

  if (uri.startsWith("/")) {
    watcher.addPath(uri);
  }
  connect(&watcher, SIGNAL(directoryChanged(QString)), this, SLOT(refresh()));

  if (uri == "places/mounts") {
    connect(main_window, SIGNAL(gio_mounts_changed()), this, SLOT(refresh()));
  }
}

QString Directory::get_parent_uri() {
  //todo: add special cases
  if (uri == "/" || uri == "places") {
    return "places";
  }
  QStringList m = uri.split("/"); //uri separator must always be "/"
  if (!m.isEmpty()) m.removeLast();
  if (m.count() == 1 && m.first().isEmpty()) return "/"; //root
  return m.join("/");

}

void Directory::refresh() {
  if (uri.isEmpty()) {
    uri = "places";
  }
  if (uri.startsWith("/")) {
    // regular directory
    Task task(this, SLOT(thread_ready(QVariant)), task_directory_list, uri);
    main_window->add_task(task);
    return;
  }
  foreach(gio::Mount mount, main_window->get_gio_mounts()) {
    if (!mount.uri.isEmpty() && uri.startsWith(mount.uri)) {
      QString real_dir = mount.path + "/" + uri.mid(mount.uri.length());
      Task task(this, SLOT(thread_ready(QVariant)), task_directory_list, real_dir);
      main_window->add_task(task);
      return;
    }
  }
  if (uri == "places") {
    QList<File_info> r;
    File_info fi;
    fi.caption = tr("Filesystem root");
    fi.file_path = "/";
    fi.uri = "/";
    fi.is_file = false;
    r << fi;
    fi = File_info();
    fi.caption = tr("Mounted filesystems");
    fi.uri = "places/mounts";
    r << fi;
    emit ready(r);
    return;
  }
  if (uri == "places/mounts") {
    QList<File_info> r;
    foreach (gio::Mount m, main_window->get_gio_mounts()) {
      File_info i;
      i.caption = m.name;
      i.uri = m.default_location;
      r << i;
    }
    foreach (gio::Volume v, main_window->get_gio_volumes()) {
      if (!v.mounted) {
        File_info i;
        i.caption = v.name + tr(" (unmounted)");
        i.uri = "/"; //todo: change it
        r << i;
      }
    }
    emit ready(r);
    return;
  }
  qDebug() << "unknown uri: " << uri;
  //todo: error: unknown uri
}

void Directory::thread_ready(QVariant result) {
  if (result.canConvert<Task_error>()) {
    Task_error error = result.value<Task_error>();
    qDebug() << "task error received: " << error.message;
    return;
  }
  Q_ASSERT(result.canConvert< QList<File_info> >());

  QList<File_info> r = result.value< QList<File_info> >();
  for(int i = 0; i < r.count(); i++) {
    r[i].uri = r[i].file_path; //todo: it will be different for GIO folders
    //can't get icons in non-gui thread
    //r[i].icon = icon_provider.icon(QFileInfo(r[i].file_path));
  }
  emit ready(r);
}
