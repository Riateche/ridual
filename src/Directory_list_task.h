#ifndef DIRECTORY_LIST_TASK_H
#define DIRECTORY_LIST_TASK_H

#include "Task.h"
#include "File_info.h"


class Directory_list_task : public Task {
  Q_OBJECT
public:
  explicit Directory_list_task(QObject *parent, QString p_path);
  ~Directory_list_task();
  void exec();
  
signals:
  /*!
    This signal is emitted when task is completed successfully.
    \param files List of files in the directory.
  */
  void ready(QList<File_info> files);

  /*!
    This signal is emitted when task execution fails.
    \param message Displayed message.
  */
  void error(QString message);

  
public slots:

private:
  QString path;

  
};

#endif // DIRECTORY_LIST_TASK_H
