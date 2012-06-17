#include "Mount.h"

#include <QtCore/QCoreApplication>
#undef signals // Collides with GTK symbols
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
  g_object_unref(file);

  file = g_mount_get_default_location(src);
  char* p_dl = g_file_get_path(file);
  if (p_dl) {
    default_location = QString::fromLocal8Bit(p_dl);
    g_free(p_dl);
  }
  g_object_unref(file);
}

