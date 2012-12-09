#ifndef DIRECTORY_WATCHER_H
#define DIRECTORY_WATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QHash>
#include "Core_ally.h"

class File_system_engine;

class Directory_watcher : public QObject, public Core_ally {
  Q_OBJECT
public:
  explicit Directory_watcher(Core* c);
  
signals:
  void directory_changed(QString uri);

public slots:
  void add(QString uri);
  void remove(QString uri);

private:
  QFileSystemWatcher watcher;
  QHash<QString, int> counter;
  QHash<QString, QString> real_paths;
  File_system_engine* fs;

private slots:
  void slot_directory_changed(QString real_path);
  
};

#endif // DIRECTORY_WATCHER_H
