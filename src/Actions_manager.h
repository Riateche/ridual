#ifndef ACTIONS_MANAGER_H
#define ACTIONS_MANAGER_H

#include <QObject>
#include <QHash>

class Action_queue;
class Action;
class Mount_manager;


class Actions_manager : public QObject {
  Q_OBJECT
public:
  explicit Actions_manager(QObject *parent = 0);
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

  /*! Set mount manager used to initialize Action objects.
    Must be called once just after constructing this object.
    */
  void set_mount_manager(Mount_manager* m) { mount_manager = m; }

signals:
  /*! Emitted when an action is added to any queue but not launched yet.
    */
  void action_added(Action* action);

  void action_destroyed(Action* action);
  void queue_destroyed(Action_queue* queue);
    
private slots:
  void queue_destroyed(QObject *obj);
  void action_destroyed(QObject *obj);
  void slot_action_added(Action* a);

private:
  QHash<int, Action_queue*> queues;
  QList<Action*> actions;
  Mount_manager* mount_manager;
  
};

#endif // ACTIONS_MANAGER_H
