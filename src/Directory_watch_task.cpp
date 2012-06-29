#include "Directory_watch_task.h"
#include <QThread>
#include <QDir>
#include <QDebug>

Directory_watch_task::Directory_watch_task(QObject *parent, QString p_path) :
  Task(parent),
  watcher(0),
  path(p_path)
{
  //watcher = new QFileSystemWatcher();
  //exec();
}

//Directory_watch_task::~Directory_watch_task() {
//  clean();
//}


void Directory_watch_task::exec() {
  watcher.addPath(path);
  if (QDir(path).exists()) {
    connect(&watcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(changed()));
    //connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(test()));
    qDebug() << "create watcher for" << path;

    //assuming this will be called from Tasks_thread
    //connect(QThread::currentThread(), SIGNAL(about_to_terminate()), this, SLOT(clean()), Qt::DirectConnection);
  }
}

/*void Directory_watch_task::clean() {
  qDebug() << "remove watcher for" << path;
  if (watcher) {
    delete watcher;
    watcher = 0;
  }
}*/

//void Directory_watch_task::test() {
//  qDebug() << "watcher fired";
//}
