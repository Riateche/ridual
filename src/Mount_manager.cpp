#include "Mount_manager.h"
#include "qt_gtk.h"
#include <QApplication>

Mount_manager::Mount_manager(QObject *parent) :
  QObject(parent)
{
  init_connects();
  fetch();

}

Mount_manager::~Mount_manager() {
  foreach(gulong id, connects) {
    g_signal_handler_disconnect (volume_monitor, id);
  }
  connects.clear();
}

void Mount_manager::init_connects() {
  //int argc = QApplication::argc();
  //char** argv = QApplication::argv();
  //gtk_init(&argc, &argv);
  gtk_init(0, 0);
  volume_monitor = g_volume_monitor_get();
  connects << g_signal_connect(volume_monitor, "volume-added",
                   G_CALLBACK(gio_callback), this);
  connects << g_signal_connect(volume_monitor, "volume-changed",
                   G_CALLBACK(gio_callback), this);
  connects << g_signal_connect(volume_monitor, "volume-removed",
                   G_CALLBACK(gio_callback), this);
  connects << g_signal_connect(volume_monitor, "mount-added",
                   G_CALLBACK(gio_callback), this);
  connects << g_signal_connect(volume_monitor, "mount-changed",
                   G_CALLBACK(gio_callback), this);
  connects << g_signal_connect(volume_monitor, "mount-removed",
                   G_CALLBACK(gio_callback), this);
}

void Mount_manager::fetch() {
  QMutexLocker locker(&mounts_mutex);
  //foreach (gio::Mount* m, mounts) delete m;
  foreach (Gio_volume* m, volumes) delete m;
  volumes.clear();
  mounts.clear();

  GList* list = g_volume_monitor_get_volumes(volume_monitor);
  for(; list; list = list->next) {
    GVolume* volume = static_cast<GVolume*>(list->data);
    volumes << new Gio_volume(volume);
    g_object_unref(volume);
  }
  g_free(list);

  list = g_volume_monitor_get_mounts(volume_monitor);
  for(; list; list = list->next) {
    GMount* mount = static_cast<GMount*>(list->data);
    mounts << Gio_mount(mount);
    g_object_unref(mount);
  }
  g_free(list);
  emit mounts_changed(mounts);
}

void Mount_manager::gio_callback(GVolumeMonitor*, GDrive*, Mount_manager* _this) {
  _this->fetch();
}
