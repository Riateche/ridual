#ifndef GIO_VOLUME_H
#define GIO_VOLUME_H

#include <QString>

struct _GVolume;
typedef _GVolume GVolume;

namespace gio {
  
  class Volume {
  public:
    Volume(GVolume* src);
    QString name;
    bool mounted;
  };
  
} // namespace gio

#endif // GIO_VOLUME_H
