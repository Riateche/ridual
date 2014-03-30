#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QObject>
#include "File_info.h"
#include "Elapsed_timer.h"
#include "Core_ally.h"
#include "qt_gtk.h"

typedef struct _GObject GObject;
typedef struct _GAsyncResult GAsyncResult;
typedef void* gpointer;

class Gio_mount;


/*!
  This class provides a way to get directory contents by uri.
  It also can give uri of parent directory.

  URI rules differs from the main meaning of URI. URI can be:

  1) path to a real file or folder starting with "/":
  - /
  - /usr/bin
  - /usr/bin/rm

  Path must not end with "/" unless it's root path.

  2) network location as used in GIO (<schema>://<source_data>/[<relative_path>]):

  ftp://user@host/
  ftp://user@host/some/path
  archive://encoded_archive_path/internal/path

  Relative path must not end with "/". If there is no relative path, URI
  must end with "/".

  sftp://user@host/          (valid)
  ftp://user@host/some/path  (valid)
  sftp://user@host           (invalid)
  ftp://user@host/some/path/ (invalid)

  3) special locations started with "places". They are used for displaying
  list of available locations. It's specific for our application.

  Directory object provides a way to read directory contents and watch for changes.
  To use Directory, pass the proper URI in Directory constructor,
  connect to ready() signal and call Directory::refresh(). Directory will asyncronically
  read directory contents and emit ready() signal with the result as an argument.
  If the directory's content is changed, Directory will send ready() signal again.
  But while the directory is changing too often, ready() signal is emitted not more often
  than once in a second. If an error has occured while retrieving the directory contents,
  error() signal will be emitted instead of ready().

  The URI of Directory object cannot be changed. Create another Directory for another URI.

  Directory class treats 'places' URIs specially and generates content for them itself.
  All other requests are passed to Directory_list_task which uses filesystem engine to
  get directory contents. Directory also is able to mount GIO volume. If the requested
  URI is like 'places/mounts/[id]', Directory will try to mount the volume asyncronically.
  If it's successful, Directory's URI will change to volume's mount point, then directory
  will be refreshed again and emit ready() signal when volume root directory is
  successfully read. Use Directory::get_uri to get actual Directory's URI.

  If GUI network resource (such as 'sftp://user@host/') is requested and there is no appropriate
  mounted FS for it, Directory will try to mount it. If mounting is successful, Directory will
  read files list and return it as usual.

  This class is reentrant but not thread safe. Use it only from GUI thread.
  Static methods are thread safe.

*/
class Directory : public QObject, Core_ally {
  Q_OBJECT
public:
  /*! Constructs an object for given uri. If you want to get the contents
    of directory, you must call Directory::refresh then.

    URI passed to constructor can be changed internally. More specificly,
    "~" is expanded to home path, "/" can be added to the end or removed from
    the end to satisfy 'uri rules', etc. use Directory::get_uri to get canonical URI.
  */
  explicit Directory(Core* c, QString uri);
  ~Directory();

  static QString canonize(QString uri);

  /*! Get current URI. It can differ from URI passed to constructor. But once constructor
    is executed, URI will not be changed.
  */
  inline QString get_uri() { return uri; }

  /*!
    Get parent URI. This function doesn't deal with filesystem. Its result is
    based on URI syntax and rules.
  */
  static QString get_parent_uri(QString target_uri);

  inline QString get_parent_uri() { return get_parent_uri(uri); }

  static bool is_relative(QString uri);
  
signals:
  /*!
    This signal is emitted after calling Directory::refresh (manually or automatically).
    \param files List of files in the directory.
  */
  void ready(File_info_list files);

  /*!
    This signal is emitted after calling Directory::refresh (manually or automatically).
    \param message Displayed message.
  */
  void error(QString message);


  void watch(QString path);
  void unwatch(QString path);




  
public slots:
  /*!
    Requests to get file list. After calling this function
    either Directory::ready or Directory::error signal will
    be emitted. But the pause between refresh() call and a signal
    can be big. Also signal can be emitted immediately, so you must
    connect both signals to your slots before calling this function.

    This function is called automatically when something is changed inside
    the directory. So, you can receive these two signals even if you don't
    call this function.
  */
  void refresh();


private slots:
  void task_ready(File_info_list result);
  void refresh_timeout();

  void directory_changed(QString path);

  void location_not_found();
  void gio_mounter_finished(bool success);

private:
  QString uri;
  Elapsed_timer refresh_timer;

  void interrupt_gio_operation();
  //todo: ability to interrupt any operation

  void create_task(QString uri);

  static const int watcher_refresh_timeout = 1000; //ms

  bool need_update;
  bool watcher_created;

  enum Async_result_type {
    async_result_unexpected,
    async_result_deadly_unexpected,
    async_result_mount_location,
    async_result_mount_volume
  };

  // async_result is used for several purposes, we need to determine what was requested
  Async_result_type async_result_type;
  GCancellable* gcancellable;

  static void async_result(GObject *source_object, GAsyncResult *res, gpointer _this);
  
};

#endif // DIRECTORY_H
