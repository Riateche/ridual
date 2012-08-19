#include "Directory_watcher.h"
#include <QDir>

Directory_watcher::Directory_watcher(QObject *parent) :
  QObject(parent)
{
  connect(&watcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(directory_changed(QString)));
}

void Directory_watcher::add(QString path) {
  if (QDir(path).exists()) {
    watcher.addPath(path);
  }
}

void Directory_watcher::remove(QString path) {
  watcher.removePath(path);
}
