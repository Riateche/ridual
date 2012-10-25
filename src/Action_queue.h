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

  Use Actions_manager to create new queues.
  */
class Action_queue : public QThread {
  Q_OBJECT
public:
  virtual ~Action_queue();
  void add_action(Action* a);

  /*! Get the id of queue. Id is an unique positive number.
    This function must be called only from the thread
    represented by this object.
    */
  inline int get_id() { return id; }
  QQueue<Action*> get_actions();

  // public morozov pattern
  void msleep(unsigned long time) { QThread::msleep(time); }

  
signals:
  /*! Emitted when action is added to the queue but not launched yet.
    */
  void action_added(Action* action);
  
private:
  explicit Action_queue(int p_id);
  friend class Actions_manager;
  QQueue<Action*> actions;
  QMutex access_mutex;
  int id;
  void run();

};

#endif // FILE_ACTION_QUEUE_H
