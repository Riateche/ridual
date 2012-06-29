#ifndef DIRECTORY_WATCH_TASK_H
#define DIRECTORY_WATCH_TASK_H

#include "Task.h"
#include <QFileSystemWatcher>

class Directory_watch_task : public Task {
  Q_OBJECT
public:
  explicit Directory_watch_task(QObject *parent, QString path);
  //~Directory_watch_task();
  void exec();

signals:
  void changed();


private:
  QFileSystemWatcher watcher;
  QString path;

private slots:
  //void clean();

  //void test();

  
};

#endif // DIRECTORY_WATCH_TASK_H
