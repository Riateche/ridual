#include "Volume.h"

#include <QtCore/QCoreApplication>
#undef signals // Collides with GTK symbols
#include "gio/gio.h"

using namespace gio;


Volume::Volume(GVolume *src) {
  char* p_name = g_volume_get_name(src);
  name = QString::fromLocal8Bit(p_name);
  g_free(p_name);
  GMount* mount = g_volume_get_mount(src);
  mounted = mount != 0;
  if (mount) {
    g_object_unref(mount);
  }

  //GIcon* gicon = g_volume_get_icon(src);
  //g_object_unref(gicon);
}

