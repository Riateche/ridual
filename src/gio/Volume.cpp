#include "Volume.h"

#include "qt_gtk.h"



using namespace gio;

Volume::~Volume() {
  g_object_unref(gvolume);
}


Volume::Volume(GVolume *src): gvolume(src) {
  g_object_ref(gvolume);
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

