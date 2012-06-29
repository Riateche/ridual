#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <QObject>
#include <QFileSystemWatcher>
#include "File_info.h"

class Bookmarks : public QObject {
  Q_OBJECT
public:
  explicit Bookmarks(QObject *parent = 0);
  inline QList<File_info> get_all() { return list; }
  inline QList<File_info> get_xdg() { return list_xdg; }

signals:
  void changed();
  
private:
  QString filename, filename_xdg;
  QFileSystemWatcher watcher, watcher_xdg;
  QList<File_info> list, list_xdg;

private slots:
  void read();
  void read_xdg();

};

#endif // BOOKMARKS_H
