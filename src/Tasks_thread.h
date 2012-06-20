#ifndef TASKS_THREAD_H
#define TASKS_THREAD_H

#include <QThread>
#include <QList>
#include "Task.h"

class Tasks_thread : public QThread {
  Q_OBJECT
public:
  explicit Tasks_thread(QObject *parent = 0);
  
signals:
  void ready(QVariant data);
  
public slots:
  void add_task(Task task);

private:
  QList<Task> queue;
  void run();
  
};

#endif // TASKS_THREAD_H
