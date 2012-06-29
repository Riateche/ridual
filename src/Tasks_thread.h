#ifndef TASKS_THREAD_H
#define TASKS_THREAD_H

#include <QThread>
#include <QList>
#include "Task.h"
#include <QMutex>

class Tasks_thread : public QThread {
  Q_OBJECT
public:
  explicit Tasks_thread(QObject *parent = 0);
  void interrupt();
  void add_task(Task* task);

signals:
  void about_to_terminate();
  
public slots:

private:
  QList<Task*> queue;
  void run();
  bool stopping;

  QMutex queue_mutex;

  
};

#endif // TASKS_THREAD_H
