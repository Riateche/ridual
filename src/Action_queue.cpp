#include "Action_queue.h"
#include "Action.h"
#include <QTimer>
#include <QDebug>

Action_queue::Action_queue(int p_id) {
  id = p_id;
  connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void Action_queue::run() {
  while(!actions.isEmpty()) {
    QMutexLocker locker(&access_mutex);
    Action* a = actions.dequeue();
    a->run();
    delete a;
  }
}

void Action_queue::add_action(Action *t) {
  QMutexLocker locker(&access_mutex);
  actions.enqueue(t);
  t->set_queue(this);
  t->moveToThread(this);
  //connect(t, SIGNAL(finished()), this, SLOT(launch_action()));
  //emit task_added(t);
  if (!isRunning()) {
    start();
  }
}

QQueue<Action *> Action_queue::get_actions() {
  return actions;
}

Action_queue::~Action_queue() {
  qDebug() << "~Action_queue";
}
