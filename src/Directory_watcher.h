#ifndef DIRECTORY_WATCHER_H
#define DIRECTORY_WATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QHash>

class Directory_watcher : public QObject {
  Q_OBJECT
public:
  explicit Directory_watcher(QObject *parent = 0);
  
signals:
  void directory_changed(QString path);

public slots:
  void add(QString path);
  void remove(QString path);

private:
  QFileSystemWatcher watcher;
  QHash<QString, int> counter;
  
};

#endif // DIRECTORY_WATCHER_H
