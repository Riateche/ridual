#include "Directory.h"
#include <QDir>
#include "Task.h"
#include <QDebug>
#include "Main_window.h"

#include "qt_gtk.h"
#include "gio/gio.h"

Directory::Directory(Main_window* mw, QString p_uri) :
  main_window(mw),
  uri(p_uri)
{
  async_result_type = async_result_unexpected;
  if (uri.startsWith("~")) {
    uri = QDir::homePath() + uri.mid(1);
  }

  if (uri.startsWith("/")) {
    watcher.addPath(uri); //todo: move watcher to separate thread
  }
  connect(&watcher, SIGNAL(directoryChanged(QString)), this, SLOT(refresh()));

  if (uri == "places/mounts") {
    connect(main_window, SIGNAL(gio_mounts_changed()), this, SLOT(refresh()));
  }
}

QString Directory::get_parent_uri() {
  QRegExp network_root("^[^\\:]*\\://[^/]*/$");
  if (!network_root.isValid()) {
    qDebug() << "regexp error" << network_root.errorString();
  }
  if (network_root.indexIn(uri) == 0) {
    return "places/mounts";
  }
  if (uri == "/" || uri == "places") {
    return "places";
  }
  QStringList m = uri.split("/"); //uri separator must always be "/"
  if (!m.isEmpty()) m.removeLast();
  if (m.count() == 1 && m.first().isEmpty()) return "/"; //root
  QString s = m.join("/");
  if (network_root.indexIn(s + "/") == 0) {
    s += "/";
  }
  return s;
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
  foreach(gio::Mount* mount, main_window->get_gio_mounts()) {
    if (!mount->uri.isEmpty() &&
        uri.startsWith(mount->uri) &&
        !uri.startsWith("file://")) {
      QString real_dir = mount->path + "/" + uri.mid(mount->uri.length());
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
    foreach (gio::Mount* m, main_window->get_gio_mounts()) {
      File_info i;
      i.caption = m->name;
      i.uri = m->default_location;
      r << i;
    }
    int id = 0;
    foreach (gio::Volume* v, main_window->get_gio_volumes()) {
      if (!v->mounted) {
        File_info i;
        i.caption = v->name + tr(" (unmounted)");
        i.uri = QString("places/mounts/%1").arg(id);
        r << i;
      }
      id++;
    }
    emit ready(r);
    return;
  }

  if (uri.startsWith("places/mounts/")) {
    int id = uri.mid(14).toInt();
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
  qDebug() << "trying to mount" << uri;
  GFile* file = g_file_new_for_uri(uri.toLocal8Bit());
  //GMountOperation* mount_operation = g_mount_operation_new();
  async_result_type = async_result_mount_location;
  g_file_mount_enclosing_volume(file, GMountMountFlags(), 0, 0, async_result, this);

  //qDebug() << "unknown uri: " << uri;
  //emit error(tr("Address not recognized"));
}

void Directory::thread_ready(QVariant result) {
  if (result.canConvert<Task_error>()) {
    Task_error e = result.value<Task_error>();
    emit error(e.message);
    return;
  }
  Q_ASSERT(result.canConvert< QList<File_info> >());

  QList<File_info> r = result.value< QList<File_info> >();
  QString uri_prefix = uri.endsWith("/")? uri: (uri + "/");
  for(int i = 0; i < r.count(); i++) {
    r[i].uri = uri_prefix + QFileInfo(r[i].file_path).fileName();
    //can't get icons in non-gui thread
    r[i].icon = icon_provider.icon(QFileInfo(r[i].file_path));
    //todo: this is slow for network fs
  }
  emit ready(r);
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

}
