#include "Directory_watch_task.h"
#include <QThread>
#include <QDir>
#include <QDebug>

Directory_watch_task::Directory_watch_task(QObject *parent, QString p_path) :
  Task(parent),
  watcher(0),
  path(p_path)
{
}

Directory_watch_task::~Directory_watch_task() {
  access_mutex.lock();
}


void Directory_watch_task::exec() {
  QMutexLocker locker(&access_mutex);
  watcher.addPath(path);
  if (QDir(path).exists()) {
    connect(&watcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(changed()));
    qDebug() << "create watcher for" << path;
  }
}

