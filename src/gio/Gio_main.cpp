#include "Gio_main.h"
#include <QDebug>
#include <QApplication>
#include "Mount.h"
#include "Volume.h"

#include <QtCore/QCoreApplication>
#undef signals // Collides with GTK symbols
#include "gio/gio.h"
#include "gtk/gtk.h"


/*
static void on_drive_changed(GVolumeMonitor *volume_monitor, GDrive *drive, void* unused) {
  qDebug() << "Drive changed:";
  print_drive_info(drive);
}

static void on_drive_connected(GVolumeMonitor *volume_monitor, GDrive *drive, void* unused) {
  qDebug() << "Drive connected:";
  print_drive_info(drive);
}

static void on_drive_disconnected(GVolumeMonitor *volume_monitor, GDrive *drive, void* unused) {
  qDebug() << "Drive disconnected:";
  print_drive_info(drive);
}
*/

static void gio_main_universal_callback(GVolumeMonitor *volume_monitor, GDrive *drive, Gio_main* parent) {
  parent->fetch_all();
}


Gio_main::Gio_main(QObject *parent) :
  QThread(parent)
{
  qRegisterMetaType< QList<gio::Volume> >("QList<gio::Volume>");
  qRegisterMetaType< QList<gio::Mount> >("QList<gio::Mount>");
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
