#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QObject>
#include <QFileSystemWatcher>
#include "File_info.h"
#include <QFileIconProvider>

typedef struct _GObject GObject;
typedef struct _GAsyncResult GAsyncResult;
typedef void* gpointer;



class Main_window;

class Directory : public QObject {
  Q_OBJECT
public:
  explicit Directory(Main_window* mw, QString uri);
  inline QString get_uri() { return uri; }
  QString get_parent_uri();

  
signals:
  void ready(QList<File_info> files);
  void error(QString message);

  
public slots:
  void refresh();

private slots:
  void thread_ready(QVariant result);

private:
  Main_window* main_window;
  QString uri;
  QFileSystemWatcher watcher;
  QFileIconProvider icon_provider;

  enum Async_result_type {
    async_result_unexpected,
    async_result_mount_location,
    async_result_mount_volume
  };

  Async_result_type async_result_type;

  static void async_result(GObject *source_object, GAsyncResult *res, gpointer _this);
  
};

#endif // DIRECTORY_H
