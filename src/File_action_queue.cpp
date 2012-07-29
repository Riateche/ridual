#include "File_action_queue.h"
#include "File_action_task.h"

Action_queue::Action_queue(int p_id) {
  id = p_id;
  connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void Action_queue::add_action(Action *t) {
  QMutexLocker locker(&access_mutex);
  actions << t;
  emit task_added(t);
  if (!isRunning()) start();
}

void Action_queue::run() {
  while(true) {
    Action* task = 0;
    {
      QMutexLocker locker(&access_mutex);
      if (actions.isEmpty()) return;
      task = actions.first();
      actions.removeFirst();
    }
    task->run(this);
  }
}
