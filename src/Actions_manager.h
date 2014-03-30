#ifndef ACTIONS_MANAGER_H
#define ACTIONS_MANAGER_H

#include <QObject>
#include <QHash>
#include "Core_ally.h"

class Action_queue;
class Action;
class Mount_manager;

/*!
  This class manages action queues. It also provides signals for
  actions' and queues' creating and destroying tracking.

  All operations with Actions_manager must be executed from GUI thread.

  */
class Actions_manager : public QObject, public Core_ally {
  Q_OBJECT
public:
  explicit Actions_manager(Core* c);
  Action_queue* create_queue();

  /*! Get all currently existing queues. Note that any queue is deleted
    when it becomes empty.
    */
  QList<Action_queue*> get_queues() { return queues.values(); }

  /*! Get all currently existing queues. Queue id is used as a hash key.
    Note that any queue is deleted when it becomes empty.
    */
  QHash<int, Action_queue*> get_queues_hash() { return queues; }

  /*! Get all existing actions ordered by descending create time,
    i.e. new actions placed at the end of list.
    Note that completed actions are destroyed and don't appear in the list.
    */
  QList<Action*> get_actions() { return actions; }

signals:
  /*! Emitted when an action is added to any queue but not launched yet.
    */
  void action_added(Action* action);

  void action_destroyed(Action* action);
  void queue_destroyed(Action_queue* queue);
    
private slots:
  void slot_queue_destroyed(QObject *obj);
  void slot_action_destroyed(QObject *obj);
  void slot_action_added(Action* a);

private:
  QHash<int, Action_queue*> queues;
  QList<Action*> actions;
  
};

#endif // ACTIONS_MANAGER_H
