#ifndef GIO_VOLUME_H
#define GIO_VOLUME_H

#include <QString>

struct _GVolume;
typedef _GVolume GVolume;


/*!
  This class represents GIO volume. We must use it to
  mount unmounted partitions. For mounted partitions
  we can use gio::Mount.
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
