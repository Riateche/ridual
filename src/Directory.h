#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QObject>
#include "File_info.h"
#include <QFileIconProvider>

typedef struct _GObject GObject;
typedef struct _GAsyncResult GAsyncResult;
typedef void* gpointer;



class Main_window;

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

*/
class Directory : public QObject {
  Q_OBJECT
public:
  /*! Constructs an object for given uri. If you want to get the contents
    of directory, you must call Directory::refresh then.

    URI passed to constructor can be changed internally. More specificly,
    "~" is expanded to home path, "/" can be added to the end or removed from
    the end to satisfy 'uri rules', etc. use Directory::get_uri to get canonical URI.
  */
  explicit Directory(Main_window* mw, QString uri);
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
  QString get_parent_uri();

  
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

private:
  Main_window* main_window;
  QString uri;
  QString path; //real path
  QFileIconProvider icon_provider;

  void create_task(QString path);

  static const int watcher_refresh_timeout = 1000; //ms

  bool need_update;
  bool watcher_created;

  enum Async_result_type {
    async_result_unexpected,
    async_result_mount_location,
    async_result_mount_volume
  };

  // async_result is used for several purposes, we need to determine what was requested
  Async_result_type async_result_type;

  static void async_result(GObject *source_object, GAsyncResult *res, gpointer _this);
  
};

#endif // DIRECTORY_H
