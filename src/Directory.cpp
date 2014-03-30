#include "Directory.h"
#include <QDir>
#include <QDebug>
#include "Main_window.h"
#include "Directory_list_task.h"
#include "Special_uri.h"
#include "Directory_watcher.h"
#include <QThreadPool>
#include "Core.h"
#include "Mount_manager.h"
#include "Bookmarks_file_parser.h"
#include "utils.h"
#include <stdexcept>
#include <QApplication>
#include "gio/Gio_mounter.h"


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
  gcancellable = 0;
  need_update = false;
  watcher_created = false;
  uri = canonize(uri);

  Special_uri special_uri(uri);
  if (special_uri.name() == Special_uri::places) {
    connect(core->get_mount_manager(), SIGNAL(mounts_changed(QList<Gio_mount>)), this, SLOT(refresh()));
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
    emit unwatch(uri);
  }
  interrupt_gio_operation();
}

QString Directory::canonize(QString uri) {
  if (uri.startsWith("~")) {
    uri = QDir::homePath() + uri.mid(1);
  }

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
    return Special_uri(Special_uri::places).uri();
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



bool Directory::is_relative(QString uri) {
  return !uri.startsWith("/") &&
      !uri.startsWith("~") &&
      !uri.startsWith("trash:") &&
      !uri.startsWith("network:") &&
      !uri.left(10).contains("://") &&
      !uri.startsWith("places");
}




void Directory::interrupt_gio_operation() {
  if (gcancellable) {
    g_cancellable_cancel(gcancellable);
    while(async_result_type != async_result_unexpected) {
      QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
  }
}



void Directory::refresh() {
  need_update = false;
  refresh_timer.restart();
  if (uri.isEmpty()) {
    emit error(tr("Address is empty"));
    return;
  }
  if (Special_uri(uri).name() == Special_uri::mounts) { //mounting of unmounted volume was requested
    //uri is something like "places/mounts/42"
    int id = uri.split("/").last().toInt();
    QList<Gio_volume*> volumes = core->get_mount_manager()->get_volumes();
    if (id < 0 || id >= volumes.count()) {
      emit error(tr("Invalid volume id"));
      return;
    }
    GVolume* volume = volumes.at(id)->get_gvolume();
    interrupt_gio_operation();
    gcancellable = g_cancellable_new();
    async_result_type = async_result_mount_volume;
    g_volume_mount(volume, GMountMountFlags(), 0, gcancellable, async_result, this);
    return;
  }

  create_task(uri);
}

void Directory::task_ready(File_info_list r) {
  //QString uri_prefix = uri.endsWith("/")? uri: (uri + "/");
  for(int i = 0; i < r.count(); i++) {
    if (!r[i].is_header_entry) {
      r[i].icon = get_file_icon(r[i].mime_type);
      if (r[i].icon.isNull()) {
        r[i].icon = get_file_icon("text/plain");
      }
    }
  }
  emit ready(r);
}

void Directory::refresh_timeout() {
  if (!need_update) return;
  refresh();
}

void Directory::directory_changed(QString changed_path) {
  if (changed_path == uri) {
    if (refresh_timer.elapsed() < watcher_refresh_timeout) {
      need_update = true;
    } else {
      refresh();
    }
  }
}

void Directory::location_not_found() {
  Gio_mounter* m = new Gio_mounter();
  connect(m, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
  connect(m, SIGNAL(finished(bool)), this, SLOT(gio_mounter_finished(bool)));
  m->start(uri);
}

void Directory::gio_mounter_finished(bool success) {
  if (success) {
    refresh();
  }
}

void Directory::create_task(QString uri) {
  Directory_list_task* task = new Directory_list_task(uri, core);
  task->setAutoDelete(true);
  connect(task, SIGNAL(ready(File_info_list)), this, SLOT(task_ready(File_info_list)));
  connect(task, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
  connect(task, SIGNAL(location_not_found()), this, SLOT(location_not_found()));

  QThreadPool::globalInstance()->start(task);

  if (Special_uri(uri).name() != Special_uri::places) {
    if (!watcher_created) {
      emit watch(uri);
      watcher_created = true;
    }
  }
}


void Directory::async_result(GObject *source_object, GAsyncResult *res, gpointer p_this) {
  Directory* _this = static_cast<Directory*>(p_this);
  g_object_unref(_this->gcancellable);
  _this->gcancellable = 0;
  Async_result_type t = _this->async_result_type;
  _this->async_result_type = async_result_unexpected;
  GError* e = 0;
  if (t == _this->async_result_mount_volume) {
    GVolume* volume = reinterpret_cast<GVolume*>(source_object);
    g_volume_mount_finish(volume, res, &e);
    if (e) {
      if (e->code == G_IO_ERROR_CANCELLED) {
        qDebug() << "Operation was cancelled";
      } else {
        emit _this->error( tr("Error %1: %2").arg(e->message)
                           .arg(QString::fromLocal8Bit(e->message)) );
      }
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
  } else {
    qWarning("Directory::async_result: unexpected call");
    qDebug() << "type: " << t;
    #ifdef TESTS_MODE
      throw std::runtime_error("Directory::async_result: unexpected call");
    #endif
  }
}
