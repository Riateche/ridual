#include "Action_queue.h"
#include "Action.h"
#include <QTimer>
#include <QDebug>

Action_queue::Action_queue(Core* c, int p_id): Core_ally(c), id(p_id) {
}

void Action_queue::action_finished() {
  Action* a = static_cast<Action*>(sender());
  if (a != actions.first()) {
    qWarning("unknown sender");
    return;
  }
  a->deleteLater();
  actions.removeFirst();
  if (actions.isEmpty()) {
    exit();
    wait();
    deleteLater();
  } else {
    QTimer::singleShot(0, actions.first(), SLOT(run()));
  }

}

/*
QList<Action *> Action_queue::get_actions() {
  QMutexLocker locker(&access_mutex);
  return actions;
}*/

Action_queue::~Action_queue() {
  qDebug() << "~Action_queue";
}

void Action_queue::create_action(Action_data data) {
  Action* a = new Action(this, data);
  connect(a, SIGNAL(finished()), this, SLOT(action_finished()));
  QMutexLocker locker(&access_mutex);
  actions << a;
  emit action_added(a);
  if (!isRunning()) {
    QTimer::singleShot(0, a, SLOT(run()));
    start();
  }
}

