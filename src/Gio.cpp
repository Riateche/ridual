#include "Gio.h"
#include <QDebug>
#include <QApplication>

static void print_drive_info(GDrive *drive) {
  GIcon *icon;
  gchar *name, *icon_string;
  name = g_drive_get_name(drive);
  icon = g_drive_get_icon(drive);
  icon_string = g_icon_to_string(icon);
  g_object_unref(icon);
  qDebug() << "tname" << name << "ticon" << icon_string;
  g_free(name);
  g_free(icon_string);

}


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



Gio::Gio(QObject *parent) :
  QThread(parent)
{


}

void Gio::run() {
  int argc = QApplication::argc();
  char** argv = QApplication::argv();
  gtk_init(&argc, &argv);
  monitor = g_volume_monitor_get();
  g_signal_connect(monitor, "drive-changed",      G_CALLBACK(on_drive_changed), 0);
  g_signal_connect(monitor, "drive-connected",    G_CALLBACK(on_drive_connected), 0);
  g_signal_connect(monitor, "drive-disconnected", G_CALLBACK(on_drive_disconnected), 0);
  GList* list = g_volume_monitor_get_volumes(monitor);
  for(; list; list = list->next) {
    GVolume* volume = static_cast<GVolume*>(list->data);
    qDebug() << "Volume(name: " << QString::fromLocal8Bit(g_volume_get_name(volume)) <<
                ", uuid: " << g_volume_get_uuid(volume) << ")";
    GMount* mount = g_volume_get_mount(volume);
    if (mount) {
      GFile* file = g_mount_get_root(mount);
      qDebug() << "  Mounted to " <<  g_file_get_path(file);
    } else {
      qDebug() << "  Not mounted";
    }
    //print_drive_info(static_cast<GDrive*>(list->data));
  }

  list = g_volume_monitor_get_mounts(monitor);
  for(; list; list = list->next) {
    GMount* mount = static_cast<GMount*>(list->data);
    GFile* file = g_mount_get_root(mount);
    qDebug() << "Mount(" << g_mount_get_name(mount) << ", " << g_file_get_path(file) << ")";
  }

  gtk_main();
}
