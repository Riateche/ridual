#ifndef GIO_VOLUME_H
#define GIO_VOLUME_H

#include <QString>

struct _GVolume;
typedef _GVolume GVolume;

namespace gio {
  
  class Volume {
  public:
    Volume(GVolume* src);
    ~Volume();
    QString name;
    bool mounted;

    GVolume* get_gvolume() { return gvolume; }

  private:
    GVolume* gvolume;
    Q_DISABLE_COPY(Volume)
  };
  
} // namespace gio

#endif // GIO_VOLUME_H
