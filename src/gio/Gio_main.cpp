#include "Gio_main.h"
#include <QDebug>
#include <QApplication>
#include "Mount.h"
#include "Volume.h"

#include <QtCore/QCoreApplication>
#undef signals // Collides with GTK symbols
#include "gio/gio.h"
#include "gtk/gtk.h"



static void gio_main_universal_callback(GVolumeMonitor *volume_monitor, GDrive *drive, Gio_main* parent) {
  parent->fetch_all();
}


Gio_main::Gio_main(QObject *parent) :
  QThread(parent)
{
  qRegisterMetaType< QList<gio::Volume> >("QList<gio::Volume>");
  qRegisterMetaType< QList<gio::Mount> >("QList<gio::Mount>");
}

void async_result(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  qDebug() << "async result";
  GError* error = 0;
  g_file_mount_enclosing_volume_finish( (GFile*) source_object, res, &error);
  if (error) {
    qDebug() << "error: " << error->message;
  } else {
    qDebug() << "no error";
  }

  GMount* mount = g_file_find_enclosing_mount( (GFile*) source_object, 0, 0);
  if (mount) {
    GFile* f = g_mount_get_root(mount);
    qDebug() << "mount" << mount << g_file_get_path(f);
  } else {
    qDebug() << "mount: NULL";
  }
}

void Gio_main::run() {
  int argc = QApplication::argc();
  char** argv = QApplication::argv();
  gtk_init(&argc, &argv);
  monitor = g_volume_monitor_get();
  g_signal_connect(monitor, "volume-added",
                   G_CALLBACK(gio_main_universal_callback), this);
  g_signal_connect(monitor, "volume-changed",
                   G_CALLBACK(gio_main_universal_callback), this);
  g_signal_connect(monitor, "volume-removed",
                   G_CALLBACK(gio_main_universal_callback), this);
  g_signal_connect(monitor, "mount-added",
                   G_CALLBACK(gio_main_universal_callback), this);
  g_signal_connect(monitor, "mount-changed",
                   G_CALLBACK(gio_main_universal_callback), this);
  g_signal_connect(monitor, "mount-removed",
                   G_CALLBACK(gio_main_universal_callback), this);
  fetch_all();

/*  GFile* file = g_file_new_for_uri("...");
  GMountOperation* mount_operation = g_mount_operation_new();
  g_file_mount_enclosing_volume(file, GMountMountFlags(), mount_operation, 0, async_result, 0);
*/
  gtk_main();
}



void Gio_main::fetch_all() {
  volumes.clear();
  mounts.clear();

  GList* list = g_volume_monitor_get_volumes(monitor);
  for(; list; list = list->next) {
    GVolume* volume = static_cast<GVolume*>(list->data);
    volumes << gio::Volume(volume);
    g_object_unref(volume);
  }
  g_free(list);

  list = g_volume_monitor_get_mounts(monitor);
  for(; list; list = list->next) {
    GMount* mount = static_cast<GMount*>(list->data);
    mounts << gio::Mount(mount);
    g_object_unref(mount);
  }
  g_free(list);

  emit list_changed(volumes, mounts);
}
