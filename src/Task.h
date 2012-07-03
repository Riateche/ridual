#ifndef TASK_H
#define TASK_H

#include <QMutex>
#include <QObject>

class Task: public QObject {
  Q_OBJECT
public:
  Task();
  virtual ~Task() {}
  virtual void exec() = 0;

};


#endif // TASK_H
