#ifndef GIO_MOUNT_H
#define GIO_MOUNT_H

#include <QString>

struct _GMount;
typedef _GMount GMount;

namespace gio {
  
  class Mount {
  public:
    Mount(GMount* src);
    QString name, path, default_location;

  };
  
} // namespace gio

#endif // GIO_MOUNT_H
