#ifndef GIO_MOUNT_H
#define GIO_MOUNT_H

#include <QString>

//forward declaration: we don't want to include gnome headers in header
struct _GMount;
typedef _GMount GMount;

namespace gio { 
  class Mount {
  public:
    Mount(GMount* src);
    QString name; //human-readable name
    QString path; //actual root path in file system
    QString uri;  //root uri (starting with 'ftp://', 'file:///' etc)
    QString default_location; //uri where should we start browsing
  };  
} // namespace gio

#endif // GIO_MOUNT_H
