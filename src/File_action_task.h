#ifndef FILE_ACTION_TASK_H
#define FILE_ACTION_TASK_H

#include <QObject>
#include <QStringList>

enum File_action_type {
  file_action_copy,
  file_action_move,
  file_action_delete
};

enum Recursive_fetch_option {
  recursive_fetch_on = 1,
  recursive_fetch_off = 2,
  recursive_fetch_auto = 3
};

class File_action_task : public QObject {
  Q_OBJECT
public:
  explicit File_action_task(File_action_type p_action,
                            QStringList p_target,
                            QString p_destination,
                            Recursive_fetch_option p_recursive_fetch_option);

  void run();
  
signals:
  void error(QString message);
  void status(QString message, double progress);
  
private:
  File_action_type action;
  QStringList target;
  QString destination;
  Recursive_fetch_option recursive_fetch_option;
};

#endif // FILE_ACTION_TASK_H
