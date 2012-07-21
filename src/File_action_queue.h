#ifndef FILE_ACTION_QUEUE_H
#define FILE_ACTION_QUEUE_H

#include <QThread>
#include <QMutex>

class File_action_task;

class File_action_queue : public QThread {
  Q_OBJECT
public:
  void add_task(File_action_task* t);
  inline int get_id() { return id; }

  friend class Main_window;
  
signals:
  void task_added(File_action_task* task);
  
private:
  explicit File_action_queue(int p_id);
  void run();
  QList<File_action_task*> tasks;
  QMutex access_mutex;
  int id; // id must be accessed only from gui thread
};

#endif // FILE_ACTION_QUEUE_H
