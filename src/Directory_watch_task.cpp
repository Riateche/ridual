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


void Directory_watch_task::exec() {
  watcher.addPath(path);
  if (QDir(path).exists()) {
    connect(&watcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(changed()));
    qDebug() << "create watcher for" << path;
  }
}

