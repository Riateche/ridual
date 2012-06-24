#ifndef GIO_VOLUME_H
#define GIO_VOLUME_H

#include <QString>

struct _GVolume;
typedef _GVolume GVolume;

namespace gio {

  /*!
    This class represents GIO volume. We must use it to
    mount unmounted partitions. For mounted partitions
    we can use gio::Mount.
    */
  class Volume {
  public:

    /*! Constructs an object for given GVolume and
      gets all needed information from it.
       */
    Volume(GVolume* src);
    ~Volume();

    /*! Human-readable name. */
    QString name;

    /*! Is this volume already mounted. */
    bool mounted;

    /*! Get GVolume object passed to the constructor.
      */
    GVolume* get_gvolume() { return gvolume; }

  private:
    GVolume* gvolume;

    // we must not copy an object that store GIO pointer
    Q_DISABLE_COPY(Volume)
  };
  
} // namespace gio

#endif // GIO_VOLUME_H
