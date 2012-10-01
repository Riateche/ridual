#include "Directory.h"
#include <QDir>
#include <QDebug>
#include "Main_window.h"
#include "Directory_list_task.h"
#include "Special_uri.h"
#include "Directory_watcher.h"
#include <QThreadPool>
#include "qt_gtk.h"
#include "Core.h"
#include "Mount_manager.h"
#include "Bookmarks_file_parser.h"

Directory::Directory(Core *c, QString p_uri) :
  Core_ally(c),
  uri(p_uri)
{
  connect(this, SIGNAL(watch(QString)),
          core->get_directory_watcher(), SLOT(add(QString)));
  connect(this, SIGNAL(unwatch(QString)),
          core->get_directory_watcher(), SLOT(remove(QString)));
  connect(core->get_directory_watcher(), SIGNAL(directory_changed(QString)),
          this, SLOT(directory_changed(QString)));

  async_result_type = async_result_unexpected;
  need_update = false;
  watcher_created = false;
  uri = canonize(uri);

  Special_uri special_uri(uri);
  if (special_uri.name() == Special_uri::places) {
    connect(core->get_mount_manager(), SIGNAL(mounts_changed()), this, SLOT(refresh()));
    connect(core->get_bookmarks(), SIGNAL(changed()), this, SLOT(refresh()));
    connect(core->get_user_dirs(), SIGNAL(changed()), this, SLOT(refresh()));
  }


  QTimer* t = new QTimer(this);
  connect(t, SIGNAL(timeout()), this, SLOT(refresh_timeout()));
  t->start(watcher_refresh_timeout);
  refresh_timer.restart();
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

  /*if (uri.contains("//")) {
    if (uri.startsWith("/")) {
      uri = "/" + uri.split("/", QString::SkipEmptyParts).join("/");
    } else {
      int start_index = uri.indexOf("//") + 2;
      uri = uri.left(start_index) +
          uri.mid(start_index).split("/", QString::SkipEmptyParts).join("/");
    }
  }*/

  if (uri.left(10).contains("://")) {
    int index = uri.indexOf("://") + 3;
    int index2 = uri.indexOf("/", index);
    if (index2 > 0) {
      uri = uri.left(index2) + QDir::cleanPath(uri.mid(index2));
    }
  } else {
    uri = QDir::cleanPath(uri);
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

QString Directory::get_parent_uri(QString target_uri) {
  QRegExp network_root("^[^\\:]*\\://[^/]*/$");
  if (network_root.indexIn(target_uri) == 0) {
    //we are in network root such as "ftp://user@host/"
    return Special_uri(Special_uri::mounts).uri();
  }
  if (target_uri == "/" || Special_uri(target_uri).name() == Special_uri::places) {
    return Special_uri(Special_uri::places).uri();
  }
  QStringList m = target_uri.split("/"); //uri separator must always be "/"
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

QString Directory::find_real_path(QString uri, const QList<Gio_mount> &mounts) {
  if (uri.startsWith("/")) return uri;
  foreach(Gio_mount mount, mounts) {
    if (!mount.uri.isEmpty() && uri.startsWith(mount.uri)) {
      return mount.path + "/" + uri.mid(mount.uri.length());
    }
  }
  return uri;
}

bool Directory::is_relative(QString uri) {
  return !uri.startsWith("/") &&
      !uri.startsWith("~") &&
      !uri.left(10).contains("://") &&
      !uri.startsWith("places");
}


QString Directory::find_real_path(QString uri, Core *core) {
  return find_real_path(uri, core->get_mount_manager()->get_mounts());
}



void Directory::refresh() {
  need_update = false;
  refresh_timer.restart();
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
    fi.is_folder = true;
    r << fi;
    fi = File_info();
    fi.name = tr("Home");
    fi.uri = QDir::homePath();
    fi.is_folder = true;
    r << fi;

    File_info_list mounts;
    foreach (Gio_mount m, core->get_mount_manager()->get_mounts()) {
      File_info i;
      i.name = m.name;
      i.uri = m.default_location;
      i.is_folder = true;
      mounts << i;
    }
    if (!mounts.isEmpty()) {
      File_info fi;
      fi.name = QString("* ") + tr("Mounted filesystems");
      r << fi;
      r << mounts;
    }
    File_info_list volumes;
    int id = 0;
    foreach (Gio_volume* v, core->get_mount_manager()->get_volumes()) {
      //we must show only unmounted volumes because
      //mounted volumes have been listed as gio::Mount's
      if (!v->mounted) {
        File_info i;
        i.name = v->name + tr(" (unmounted)");
        i.is_folder = true;
        i.uri = QString("places/mounts/%1").arg(id); //use number of volume in list as id
        volumes << i;
      }
      id++;
    }
    if (!volumes.isEmpty()) {
      File_info fi;
      fi.name = QString("* ") + tr("Unmounted volumes");
      r << fi;
      r << volumes;
    }

    QStringList uris;
    foreach(File_info fi, r) {
      fi.uri = canonize(fi.uri);
      uris << fi.uri;
    }

    File_info_list bookmarks;
    foreach(File_info fi, core->get_bookmarks()->get_all()) {
      fi.uri = canonize(fi.uri);
      if (!uris.contains(fi.uri)) {
        uris << fi.uri;
        bookmarks << fi;
      }
    }
    if (!bookmarks.isEmpty()) {
      File_info fi;
      fi.name = QString("* ") + tr("Bookmarks");
      r << fi;
      r << bookmarks;
    }

    File_info_list user_dirs;
    foreach(File_info fi, core->get_user_dirs()->get_all()) {
      fi.uri = canonize(fi.uri);
      if (!uris.contains(fi.uri)) {
        uris << fi.uri;
        user_dirs << fi;
      }
    }
    if (!user_dirs.isEmpty()) {
      File_info fi;
      fi.name = QString("* ") + tr("Standard places");
      r << fi;
      r << user_dirs;
    }

    r.columns << Column::name << Column::uri;
    r.disable_sort = true;
    emit ready(r);
    return;
  }

  if (uri.startsWith(Special_uri(Special_uri::mounts).uri())) { //mounting of unmounted volume was requested
    int id = uri.mid(Special_uri(Special_uri::mounts).uri().length() + 1).toInt(); //uri is something like "places/mounts/42"
    QList<Gio_volume*> volumes = core->get_mount_manager()->get_volumes();
    if (id < 0 || id >= volumes.count()) {
      emit error(tr("Invalid volume id"));
      return;
    }
    GVolume* volume = volumes.at(id)->get_gvolume();
    async_result_type = async_result_mount_volume;
    g_volume_mount(volume, GMountMountFlags(), 0, 0, async_result, this);
    return;
  }

  QString real_path = find_real_path(uri, core);
  if (uri != real_path) {
    path = real_path;
    create_task(path);
    return;
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
    r[i].uri = uri_prefix + QFileInfo(r[i].path).fileName();
    //we can't get icons in non-gui thread, because QFileIconProvider uses QPixmap
    //and it produces warning. We must do it in gui thread, it's bad because
    //it causes GUI to freeze.
    r[i].icon = icon_provider.icon(r[i].path);
    //todo: this is slow for network fs
  }
  emit ready(r);
}

void Directory::refresh_timeout() {
  if (!need_update) return;
  refresh();
}

void Directory::directory_changed(QString changed_path) {
  //qDebug() << "directory_changed" << path << changed_path;
  if (!path.isEmpty() && path == changed_path) {
    if (refresh_timer.elapsed() < watcher_refresh_timeout) {
      need_update = true;
    } else {
      refresh();
    }
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
