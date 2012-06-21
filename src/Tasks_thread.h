#ifndef TASKS_THREAD_H
#define TASKS_THREAD_H

#include <QThread>
#include <QList>
#include "Task.h"

class Tasks_thread : public QThread {
  Q_OBJECT
public:
  explicit Tasks_thread(QObject *parent = 0);
  void interrupt();
  
signals:
  void ready(QVariant data);
  
public slots:
  void add_task(Task task);

private:
  QList<Task> queue;
  void run();
  bool stopping;
  
};

#endif // TASKS_THREAD_H
