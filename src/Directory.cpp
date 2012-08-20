#include "Directory.h"
#include <QDir>
#include <QDebug>
#include "Main_window.h"
#include "Directory_list_task.h"
#include "Special_uri.h"
#include "Directory_watcher.h"
#include <QThreadPool>
#include "qt_gtk.h"


Directory::Directory(Main_window* mw, QString p_uri) :
  main_window(mw),
  uri(p_uri)
{
  connect(this, SIGNAL(watch(QString)), main_window->get_watcher(), SLOT(add(QString)));
  connect(this, SIGNAL(unwatch(QString)), main_window->get_watcher(), SLOT(remove(QString)));
  connect(main_window->get_watcher(), SIGNAL(directory_changed(QString)), this, SLOT(directory_changed(QString)));

  async_result_type = async_result_unexpected;
  need_update = false;
  watcher_created = false;
  uri = canonize(uri);

  Special_uri special_uri(uri);
  if (special_uri.name() == Special_uri::mounts) {
    connect(main_window, SIGNAL(gio_mounts_changed()), this, SLOT(refresh()));
  }

  if (special_uri.name() == Special_uri::bookmarks) {
    connect(main_window->get_bookmarks(), SIGNAL(changed()), this, SLOT(refresh()));
  }

  if (special_uri.name() == Special_uri::userdirs) {
    connect(main_window->get_user_dirs(), SIGNAL(changed()), this, SLOT(refresh()));
  }


  QTimer* t = new QTimer(this);
  connect(t, SIGNAL(timeout()), this, SLOT(refresh_timeout()));
  t->start(watcher_refresh_timeout);
}

Directory::~Directory() {
  if (watcher_created) {
    emit unwatch(path);
  }
}

QString Directory::canonize(QString uri) {
  if (uri.startsWith("~")) {
    uri = QDir::homePath() + uri.mid(1);
  }

  if (uri.contains("//")) {
    if (uri.startsWith("/")) {
      uri = "/" + uri.split("/", QString::SkipEmptyParts).join("/");
    } else {
      int start_index = uri.indexOf("//") + 2;
      uri = uri.left(start_index) +
          uri.mid(start_index).split("/", QString::SkipEmptyParts).join("/");
    }
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
  return uri;
}

QString Directory::get_parent_uri() {
  QRegExp network_root("^[^\\:]*\\://[^/]*/$");
  if (network_root.indexIn(uri) == 0) {
    //we are in network root such as "ftp://user@host/"
    return Special_uri(Special_uri::mounts).uri();
  }
  if (uri == "/" || Special_uri(uri).name() == Special_uri::places) {
    return Special_uri(Special_uri::places).uri();
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
  if (uri.isEmpty()) {
    emit error(tr("Address is empty"));
    return;
  }
  if (uri.startsWith("/")) {
    // regular directory
    path = uri;
    create_task(uri);
    return;
  }
  Special_uri special_uri(uri);
  if (special_uri.name() == Special_uri::places) {
    //the root of our virtual directory tree
    File_info_list r;
    File_info fi;
    fi.name = tr("Filesystem root");
    fi.uri = "/";
    r << fi;
    fi = File_info();
    fi.name = tr("Home");
    fi.uri = QDir::homePath();
    r << fi;
    fi = File_info();
    fi.name = Special_uri(Special_uri::mounts).caption();
    fi.uri = Special_uri(Special_uri::mounts).uri();
    r << fi;
    fi = File_info();
    fi.name = Special_uri(Special_uri::bookmarks).caption();
    fi.uri = Special_uri(Special_uri::bookmarks).uri();
    r << fi;
    fi = File_info();
    fi.name = Special_uri(Special_uri::userdirs).caption();
    fi.uri = Special_uri(Special_uri::userdirs).uri();
    r << fi;
    r.columns << column_name << column_uri;
    emit ready(r);
    return;
  }
  if (special_uri.name() == Special_uri::mounts) { // list of mounts
    File_info_list r;
    foreach (Gio_mount m, main_window->get_gio_mounts()) {
      File_info i;
      i.name = m.name;
      i.uri = m.default_location;
      r << i;
    }
    int id = 0;
    foreach (Gio_volume* v, main_window->get_gio_volumes()) {
      //we must show only unmounted volumes because
      //mounted volumes have been listed as gio::Mount's
      if (!v->mounted) {
        File_info i;
        i.name = v->name + tr(" (unmounted)");
        i.uri = QString("places/mounts/%1").arg(id); //use number of volume in list as id
        r << i;
      }
      id++;
    }
    r.columns << column_name << column_uri;
    emit ready(r);
    return;
  }

  if (uri.startsWith(Special_uri(Special_uri::mounts).uri())) { //mounting of unmounted volume was requested
    int id = uri.mid(Special_uri(Special_uri::mounts).uri().length() + 1).toInt(); //uri is something like "places/mounts/42"
    if (id < 0 || id >= main_window->get_gio_volumes().count()) {
      emit error(tr("Invalid volume id"));
      return;
    }
    GVolume* volume = main_window->get_gio_volumes().at(id)->get_gvolume();
    async_result_type = async_result_mount_volume;
    g_volume_mount(volume, GMountMountFlags(), 0, 0, async_result, this);
    return;
  }

  if (special_uri.name() == Special_uri::bookmarks) { // list of bookmarks
    File_info_list list = main_window->get_bookmarks()->get_all();
    list.columns << column_name << column_uri;
    emit ready(list);
    return;
  }
  if (special_uri.name() == Special_uri::userdirs) { // list of xdg bookmarks
    File_info_list list = main_window->get_user_dirs()->get_all();
    list.columns << column_name << column_uri;
    emit ready(list);
    return;
  }

  foreach(Gio_mount mount, main_window->get_gio_mounts()) {
    if (!mount.uri.isEmpty() && uri.startsWith(mount.uri)) {
      //convert uri e.g. "ftp://user@host/path"
      //to real path e.g. "/home/user/.gvfs/FTP as user on host/path"
      path = mount.path + "/" + uri.mid(mount.uri.length());
      //qDebug() << "This location is recognized as existing mount";
      create_task(path);
      return;
    }
  }

  //address not recognized. try to pass it to gio
  GFile* file = g_file_new_for_uri(uri.toLocal8Bit());
  async_result_type = async_result_mount_location;
  qDebug() << "Trying to mount this location";
  g_file_mount_enclosing_volume(file, GMountMountFlags(), 0, 0, async_result, this);
}

void Directory::task_ready(File_info_list r) {
  QString uri_prefix = uri.endsWith("/")? uri: (uri + "/");
  for(int i = 0; i < r.count(); i++) {
    r[i].uri = uri_prefix + QFileInfo(r[i].full_path).fileName();
    //we can't get icons in non-gui thread, because QFileIconProvider uses QPixmap
    //and it produces warning. We must do it in gui thread, it's bad because
    //it causes GUI to freeze.
    r[i].icon = icon_provider.icon(QFileInfo(r[i].full_path));
    //todo: this is slow for network fs
  }
  emit ready(r);
}

void Directory::refresh_timeout() {
  if (!need_update) return;
  need_update = false;
  refresh();
}

void Directory::directory_changed(QString changed_path) {
  qDebug() << "directory_changed" << path << changed_path;
  if (!path.isEmpty() && path == changed_path) {
    need_update = true;
  }
}

void Directory::create_task(QString path) {
  Directory_list_task* task = new Directory_list_task(path);
  connect(task, SIGNAL(ready(File_info_list)), this, SLOT(task_ready(File_info_list)));
  connect(task, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
  //connect(task, SIGNAL(error(QString)), this, SLOT(test(QString)));
  //main_window->add_task(task);

  QThreadPool::globalInstance()->start(task);


  if (!watcher_created) {
    //Directory_watch_task* task2 = new Directory_watch_task(path);
    //connect(task2, SIGNAL(changed()), this, SLOT(watcher_event()));
    //main_window->add_task(task2);
    emit watch(path);
    watcher_created = true;
  }
}


void Directory::async_result(GObject *source_object, GAsyncResult *res, gpointer p_this) {
  Directory* _this = static_cast<Directory*>(p_this);
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
        emit _this->error( tr("Error %1: %2").arg(e->message)
                           .arg(QString::fromLocal8Bit(e->message)) );
      }
      g_error_free(e);
      return;
    }
    g_object_unref(source_object); // source_object is GFile created by us
    // location is mounted now, retry
    _this->refresh();

  } else if (_this->async_result_type == _this->async_result_mount_volume) {
    GVolume* volume = reinterpret_cast<GVolume*>(source_object);
    g_volume_mount_finish(volume, res, &e);
    if (e) {
      emit _this->error( tr("Error %1: %2").arg(e->message)
                         .arg(QString::fromLocal8Bit(e->message)) );
      g_error_free(e);
      return;
    }
    GMount* mount = g_volume_get_mount(volume);
    if (!mount) {
      qWarning("mount == null");
      emit _this->error(tr("Unexpected failure while mounting a volume."));
      return;
    }
    GFile* f = g_mount_get_root(mount);
    if (!f) {
      qWarning("f == null");
      emit _this->error(tr("Unexpected failure while mounting a volume."));
      return;
    }
    char* path = g_file_get_path(f);
    _this->uri = QString::fromLocal8Bit(path);
    g_free(path);
    g_object_unref(f);
    g_object_unref(mount);
    _this->refresh();
  }
  _this->async_result_type = _this->async_result_unexpected;
}
