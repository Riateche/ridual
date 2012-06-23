#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QObject>
#include <QFileSystemWatcher>
#include "File_info.h"
#include <QFileIconProvider>


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
  
};

#endif // DIRECTORY_H
