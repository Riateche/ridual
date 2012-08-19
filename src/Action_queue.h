#ifndef FILE_ACTION_QUEUE_H
#define FILE_ACTION_QUEUE_H

#include <QThread>
#include <QMutex>
#include <QQueue>

class Action;

/*!
  All operations with Action_queue must be executed from GUI thread.
  Accessing from other threads is forbidden. Use signal-slot system
  to interact with queues.
  */
class Action_queue : public QThread {
  Q_OBJECT
public:
  virtual ~Action_queue();
  void add_action(Action* t);

  /*! Get the id of queue. Id is an unique positive number.
    This function must be called only from the thread
    represented by this object.
    */
  inline int get_id() { return id; }
  QQueue<Action*> get_actions();

  // public morozov pattern
  void msleep(unsigned long time) { QThread::msleep(time); }

  
signals:
  void task_added(Action* task);
  
private:
  explicit Action_queue(int p_id);
  friend class Main_window;
  QQueue<Action*> actions;
  QMutex access_mutex;
  int id;

private slots:
  void launch_action();

};

#endif // FILE_ACTION_QUEUE_H
