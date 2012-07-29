#ifndef FILE_ACTION_QUEUE_H
#define FILE_ACTION_QUEUE_H

#include <QThread>
#include <QMutex>

class Action;

class Action_queue : public QThread {
  Q_OBJECT
public:
  void add_action(Action* t);
  inline int get_id() { return id; }

  friend class Main_window;
  
signals:
  void task_added(Action* task);
  
private:
  explicit Action_queue(int p_id);
  void run();
  QList<Action*> actions;
  QMutex access_mutex;
  int id; // id must be accessed only from gui thread
};

#endif // FILE_ACTION_QUEUE_H
