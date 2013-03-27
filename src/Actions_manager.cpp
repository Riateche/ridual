#include "Actions_manager.h"
#include "Action_queue.h"
#include "Action.h"
#include "Mount_manager.h"
#include "gio/Gio_file_system_engine.h"
#include "Core.h"

Actions_manager::Actions_manager(Core* c) :
  Core_ally(c)
{
}

Action_queue *Actions_manager::create_queue() {
  int id = 1;
  while(queues.contains(id)) id++;
  Action_queue* q = new Action_queue(core, id);
  connect(q, SIGNAL(destroyed(QObject*)), this, SLOT(queue_destroyed(QObject*)));
  connect(q, SIGNAL(action_added(Action*)), this, SLOT(slot_action_added(Action*)));
  queues[id] = q;
  return q;
}

void Actions_manager::queue_destroyed(QObject *obj) {
  queues.remove(queues.key(reinterpret_cast<Action_queue*>(obj)));
  emit queue_destroyed(reinterpret_cast<Action_queue*>(obj));
}

void Actions_manager::slot_action_destroyed(QObject *obj) {
  actions.removeOne(reinterpret_cast<Action*>(obj));
  emit action_destroyed(reinterpret_cast<Action*>(obj));
}

void Actions_manager::slot_action_added(Action *a) {

  actions << a;
  connect(a, SIGNAL(destroyed(QObject*)), this, SLOT(slot_action_destroyed(QObject*)));
  emit action_added(a);
}
