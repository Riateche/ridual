#include "File_action_queue.h"
#include "File_action_task.h"

File_action_queue::File_action_queue(File_action_task *first_task) {
  tasks << first_task;
}

void File_action_queue::add_task(File_action_task *t) {
  QMutexLocker locker(&access_mutex);
  tasks << t;
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
