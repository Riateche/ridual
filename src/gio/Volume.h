#ifndef GIO_VOLUME_H
#define GIO_VOLUME_H

#include <QString>

struct _GVolume;
typedef _GVolume GVolume;


/*!
  This class represents GIO volume. We must use it to
  mount unmounted partitions. For mounted partitions
  Gio_mount can be used.

  This class keeps GVolume pointer because we need it to
  request volume mount. This operation is performed by
  calling g_volume_mount in Directory::refresh.
  */
class Gio_volume {
public:

  /*! Constructs an object for given GVolume and
    gets all needed information from it.
     */
  Gio_volume(GVolume* src);
  ~Gio_volume();

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
  Q_DISABLE_COPY(Gio_volume)
};

#endif // GIO_VOLUME_H
