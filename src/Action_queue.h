#ifndef FILE_ACTION_QUEUE_H
#define FILE_ACTION_QUEUE_H

#include <QThread>
#include <QMutex>
#include <QQueue>
#include "Core_ally.h"
#include "types.h"

class Action;

/*!
  All operations with Action_queue must be executed from GUI thread.
  Use Actions_manager to create new queues.

  Action_queue manages a queue of Action objects and executes them one by one.
  Finished Action objects are deleted here.

  Action_queue object is deleted when the last action is finished.

  */
class Action_queue : public QThread, public Core_ally {
  Q_OBJECT
public:
  virtual ~Action_queue();
  //void add_action(Action* a);

  void create_action(Action_data data);

  /*! Get the id of queue. Id is an unique positive number.
    This function must be called only from the thread
    represented by this object.
    */
  inline int get_id() { return id; }
  
signals:
  /*! Emitted when action is added to the queue but not launched yet.
    */
  void action_added(Action* action);
  
private:
  explicit Action_queue(Core* c, int p_id);
  friend class Actions_manager;
  QList<Action*> actions;
  QMutex access_mutex;
  int id;

private slots:
  void action_finished();

};

#endif // FILE_ACTION_QUEUE_H
