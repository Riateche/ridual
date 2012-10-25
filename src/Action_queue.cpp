#include "Action_queue.h"
#include "Action.h"
#include <QTimer>
#include <QDebug>

Action_queue::Action_queue(int p_id) {
  id = p_id;
  connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void Action_queue::run() {
  while(true) {
    Action* a;
    {
      QMutexLocker locker(&access_mutex);
      if (actions.isEmpty()) return;
      a = actions.dequeue();
    }
    a->run();
    delete a;
  }
}

void Action_queue::add_action(Action *a) {
  QMutexLocker locker(&access_mutex);
  actions.enqueue(a);
  a->set_queue(this);
  emit action_added(a); //must be before moveToThread
  a->moveToThread(this);
  if (!isRunning()) {
    start();
  }
}

QQueue<Action *> Action_queue::get_actions() {
  QMutexLocker locker(&access_mutex);
  return actions;
}

Action_queue::~Action_queue() {
  qDebug() << "~Action_queue";
}
