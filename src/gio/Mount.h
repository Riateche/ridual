#ifndef GIO_MOUNT_H
#define GIO_MOUNT_H

#include <QString>

//forward declaration: we don't want to include gnome headers in header
struct _GMount;
typedef _GMount GMount;

/*!
  This class holds information about one GMount.
  It gets all information in constructor.
  It doesn't keep a pointer to GMount.
  */
class Gio_mount {
public:
  Gio_mount(GMount* src);

  //! Human-readable name of mount.
  QString name;

  /*! Actual root path in file system. Some common examples:
    - /media/disc_uuid
    - /home/username/.gvfs/FTP as username on servername
    */
  QString path;

  /*! Root uri (starting with 'ftp://', 'smb://' etc)
    or actual root path (for non-network mounts).
    See 'uri rules' for more information.
  */
  QString uri;

  /*! Uri where should we start browsing. It's often the same as Mount::uri,
    but it can be sub-folder of uri. For example, for SFTP mount
    uri points to root folder and default_location points to home folder.

    Examples:
    - /media/disc_uuid
    - ftp://username@servername/
    - sftp://username@servername/home/username
  */
  QString default_location;
};

#endif // GIO_MOUNT_H
