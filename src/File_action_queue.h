#ifndef FILE_ACTION_QUEUE_H
#define FILE_ACTION_QUEUE_H

#include <QThread>
#include <QMutex>

class File_action_task;

class File_action_queue : public QThread {
  Q_OBJECT
public:
  explicit File_action_queue(File_action_task* first_task);
  void add_task(File_action_task* t);
  
signals:
  
private:
  void run();
  QList<File_action_task*> tasks;
  QMutex access_mutex;
};

#endif // FILE_ACTION_QUEUE_H
