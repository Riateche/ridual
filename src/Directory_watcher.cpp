#include "Directory_watcher.h"
#include <QDir>
#include <QDebug>

Directory_watcher::Directory_watcher(QObject *parent) :
  QObject(parent)
{
  connect(&watcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(directory_changed(QString)));
}

void Directory_watcher::add(QString path) {
  //qDebug() << "Directory_watcher::add" << path;
  if (counter.contains(path)) {
    counter[path]++;
    //qDebug() << "already added";
  } else {
    //qDebug() << "adding";
    counter[path] = 1;
    if (QDir(path).exists()) {
      watcher.addPath(path);
    }
  }
}

void Directory_watcher::remove(QString path) {
  //qDebug() << "Directory_watcher::remove" << path;
  counter[path]--;
  if (counter[path] == 0) {
    //qDebug() << "removing";
    counter.remove(path);
    watcher.removePath(path);
  }

}
