#ifndef TASK_H
#define TASK_H

#include <QMutex>
#include <QObject>

class Task: public QObject {
  Q_OBJECT
public:
  Task(QObject* parent);
  virtual ~Task() {}
  virtual void exec() = 0;

protected:
  QMutex access_mutex;
};


#endif // TASK_H
