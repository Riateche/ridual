#include "File_action_queue.h"
#include "File_action_task.h"

File_action_queue::File_action_queue(int p_id) {
  id = p_id;
}

void File_action_queue::add_task(File_action_task *t) {
  QMutexLocker locker(&access_mutex);
  tasks << t;
  emit task_added(t);
  if (!isRunning()) start();
}

void File_action_queue::run() {
  while(true) {
    File_action_task* task = 0;
    {
      QMutexLocker locker(&access_mutex);
      if (tasks.isEmpty()) return;
      task = tasks.first();
      tasks.removeFirst();
    }
    task->run();
  }
}
