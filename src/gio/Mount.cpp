#include "Mount.h"
#include <QDebug>

#include "qt_gtk.h"
#include "gio/gio.h"


using namespace gio;

Mount::Mount(GMount *src) {
  char* p_name = g_mount_get_name(src);
  name = QString::fromLocal8Bit(p_name);
  g_free(p_name);

  GFile* file = g_mount_get_root(src);
  char* p_path = g_file_get_path(file);
  if (p_path) {
    path = QString::fromLocal8Bit(p_path);
    g_free(p_path);
  }
  char* p_uri = g_file_get_uri(file);
  if (p_uri) {
    uri = QString::fromLocal8Bit(p_uri);
    if (uri.startsWith("file://")) {
      uri.remove(0, 7);
    }
    g_free(p_uri);
  }
  g_object_unref(file);

  file = g_mount_get_default_location(src);
  char* p_dl = g_file_get_uri(file);
  if (p_dl) {
    default_location = QString::fromLocal8Bit(p_dl);
    if (default_location.startsWith("file://")) {
      //modify uri accordingly to our 'uri rules':
      //local files are represented by their normal paths without 'file://'
      default_location.remove(0, 7);
    }
    g_free(p_dl);
  }
  GVolume* volume = g_mount_get_volume(src);
  if (volume) {
    g_object_unref(volume);
    default_location = path;
    uri = path;
  }
  qDebug() << "path" << path;
  qDebug() << "default_location" << default_location;
  qDebug() << "uri" << uri;
  g_object_unref(file);
}

