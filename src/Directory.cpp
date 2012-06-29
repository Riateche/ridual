#include "Directory.h"
#include <QDir>
#include <QDebug>
#include "Main_window.h"
#include "Directory_list_task.h"
#include "Directory_watch_task.h"

#include "qt_gtk.h"
#include "gio/gio.h"

Directory::Directory(Main_window* mw, QString p_uri) :
  main_window(mw),
  uri(p_uri)
{
  async_result_type = async_result_unexpected;
  need_update = false;
  watcher_created = false;

  if (uri.startsWith("~")) {
    uri = QDir::homePath() + uri.mid(1);
  }

  QRegExp network_root("^[^\\:]*\\://[^/]*/$");
  if (network_root.indexIn(uri) < 0 && uri.endsWith("/") && uri != "/") {
    // remove trailing slash
    uri = uri.left(uri.length() - 1);
  }
  if (network_root.indexIn(uri + "/") == 0) {
    //add required slash
    uri += "/";
  }

  if (uri.startsWith("file://")) {
    uri = uri.mid(7);
  }

  if (uri == "places/mounts") {
    connect(main_window, SIGNAL(gio_mounts_changed()), this, SLOT(refresh()));
  }

  QTimer* t = new QTimer(this);
  connect(t, SIGNAL(timeout()), this, SLOT(refresh_timeout()));
  t->start(watcher_refresh_timeout);
}

QString Directory::get_parent_uri() {
  QRegExp network_root("^[^\\:]*\\://[^/]*/$");
  if (network_root.indexIn(uri) == 0) {
    //we are in network root such as "ftp://user@host/"
    return "places/mounts";
  }
  if (uri == "/" || uri == "places") {
    return "places";
  }
  QStringList m = uri.split("/"); //uri separator must always be "/"
  if (!m.isEmpty()) m.removeLast();
  if (m.count() == 1 && m.first().isEmpty()) return "/";
  QString s = m.join("/");
  if (network_root.indexIn(s + "/") == 0) {
    //we are near network root, e.g. "ftp://user@host/one_folder"
    //we must get "ftp://user@host/" instead of "ftp://user@host"
    s += "/";
  }
  return s;
}

void Directory::refresh() {
  //qDebug() << "Directory::refresh";
  if (uri.isEmpty()) {
    emit error(tr("Address is empty"));
    return;
  }
  if (uri.startsWith("/")) {
    // regular directory
    //Task task(this, SLOT(thread_ready(QVariant)), task_directory_list, uri);
    create_task(uri);
    return;
  }
  foreach(gio::Mount* mount, main_window->get_gio_mounts()) {
    if (!mount->uri.isEmpty() && uri.startsWith(mount->uri)) {
      //convert uri e.g. "ftp://user@host/path"
      //to real path e.g. "/home/user/.gvfs/FTP as user on host/path"
      QString real_dir = mount->path + "/" + uri.mid(mount->uri.length());
      create_task(real_dir);
      return;
    }
  }
  if (uri == "places") { //the root of our virtual directory tree
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
  if (uri == "places/mounts") { // list of mounts
    QList<File_info> r;
    foreach (gio::Mount* m, main_window->get_gio_mounts()) {
      File_info i;
      i.caption = m->name;
      i.uri = m->default_location;
      r << i;
    }
    int id = 0;
    foreach (gio::Volume* v, main_window->get_gio_volumes()) {
      //we must show only unmounted volumes because
      //mounted volumes have been listed as gio::Mount's
      if (!v->mounted) {
        File_info i;
        i.caption = v->name + tr(" (unmounted)");
        i.uri = QString("places/mounts/%1").arg(id); //use number of volume in list as id
        r << i;
      }
      id++;
    }
    emit ready(r);
    return;
  }

  if (uri.startsWith("places/mounts/")) { //mounting of unmounted volume was requested
    int id = uri.mid(14).toInt(); //uri is something like "places/mounts/42"
    if (id < 0 || id >= main_window->get_gio_volumes().count()) {
      emit error(tr("Invalid volume id"));
      return;
    }
    GVolume* volume = main_window->get_gio_volumes().at(id)->get_gvolume();
    async_result_type = async_result_mount_volume;
    g_volume_mount(volume, GMountMountFlags(), 0, 0, async_result, this);
    return;
  }

  //address not recognized. try to pass it to gio
  GFile* file = g_file_new_for_uri(uri.toLocal8Bit());
  async_result_type = async_result_mount_location;
  g_file_mount_enclosing_volume(file, GMountMountFlags(), 0, 0, async_result, this);
}

void Directory::task_ready(QList<File_info> r) {
  QString uri_prefix = uri.endsWith("/")? uri: (uri + "/");
  for(int i = 0; i < r.count(); i++) {
    r[i].uri = uri_prefix + QFileInfo(r[i].file_path).fileName();
    //we can't get icons in non-gui thread, because QFileIconProvider uses QPixmap
    //and it produces warning. We must do it in gui thread, it's bad because
    //it causes GUI to freeze.
    r[i].icon = icon_provider.icon(QFileInfo(r[i].file_path));
    //todo: this is slow for network fs
  }
  emit ready(r);
}

void Directory::watcher_event() {
  need_update = true;
}

void Directory::refresh_timeout() {
  if (!need_update) return;
  need_update = false;
  refresh();
}

void Directory::create_task(QString path) {
  Directory_list_task* task = new Directory_list_task(this, path);
  connect(task, SIGNAL(ready(QList<File_info>)), this, SLOT(task_ready(QList<File_info>)));
  connect(task, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
  //connect(task, SIGNAL(error(QString)), this, SLOT(test(QString)));
  main_window->add_task(task);

  if (!watcher_created) {
    Directory_watch_task* task2 = new Directory_watch_task(this, path);
    connect(task2, SIGNAL(changed()), this, SLOT(watcher_event()));
    main_window->add_task(task2);
    watcher_created = true;
  }
}


void Directory::async_result(GObject *source_object, GAsyncResult *res, gpointer p_this) {
  Directory* _this = static_cast<Directory*>(p_this);
  qDebug() << "async result";
  if (_this->async_result_type == _this->async_result_unexpected) {
    qWarning("Directory::async_result: unexpected call");
  }
  GError* e = 0;

  if (_this->async_result_type == _this->async_result_mount_location) {
    g_file_mount_enclosing_volume_finish(reinterpret_cast<GFile*>(source_object), res, &e);
    if (e) {
      if (e->code == G_IO_ERROR_NOT_SUPPORTED) {
        //error "volume doesn't implement mount"  occurs on invalid address
        emit _this->error(tr("Address not recognized"));
      } else {
        emit _this->error(QString::fromLocal8Bit(e->message));
        qDebug() << "error code: " << e->code << e->message;
      }
      return;
    }
    // location is mounted now, retry
    _this->refresh();

  } else if (_this->async_result_type == _this->async_result_mount_volume) {
    GVolume* volume = reinterpret_cast<GVolume*>(source_object);
    g_volume_mount_finish(volume, res, &e);
    GMount* mount = g_volume_get_mount(volume);
    if (!mount) {
      qWarning("mount == null");
      return;
    }
    GFile* f = g_mount_get_root(mount);
    char* path = g_file_get_path(f);
    _this->uri = QString::fromLocal8Bit(path);
    g_free(path);
    _this->refresh();
    g_object_unref(mount);
  }
  _this->async_result_type = _this->async_result_unexpected;

  //todo: g_unref, g_free and error reporting

}
