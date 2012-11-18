#include "Actions_manager.h"
#include "Action_queue.h"
#include "Action.h"
#include "Mount_manager.h"
#include "gio/Gio_file_system_engine.h"


Actions_manager::Actions_manager(QObject *parent) :
  QObject(parent),
  mount_manager(0)
{
}

Action_queue *Actions_manager::create_queue() {
  int id = 1;
  while(queues.contains(id)) id++;
  Action_queue* q = new Action_queue(id);
  connect(q, SIGNAL(destroyed(QObject*)), this, SLOT(queue_destroyed(QObject*)));
  connect(q, SIGNAL(action_added(Action*)), this, SLOT(slot_action_added(Action*)));
  queues[id] = q;
  return q;
}

void Actions_manager::queue_destroyed(QObject *obj) {
  queues.remove(queues.key(reinterpret_cast<Action_queue*>(obj)));
  emit queue_destroyed(reinterpret_cast<Action_queue*>(obj));
}

void Actions_manager::action_destroyed(QObject *obj) {
  actions.removeOne(reinterpret_cast<Action*>(obj));
  emit action_destroyed(reinterpret_cast<Action*>(obj));
}

void Actions_manager::slot_action_added(Action *a) {
  if (mount_manager) {
    //a->set_mounts(mount_manager->get_mounts());

    //todo: better to use core->get_new_file_system_engine() here
    a->set_fs_engine(new Gio_file_system_engine(mount_manager));
  }
  actions << a;
  connect(a, SIGNAL(destroyed(QObject*)), this, SLOT(action_destroyed(QObject*)));
  emit action_added(a);
}
