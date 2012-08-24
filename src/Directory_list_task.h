#ifndef DIRECTORY_LIST_TASK_H
#define DIRECTORY_LIST_TASK_H

#include "File_info.h"
#include <QRunnable>

class Directory_list_task : public QObject, public QRunnable {
  Q_OBJECT
public:
  explicit Directory_list_task(QString p_path, QString p_uri);
  ~Directory_list_task();
  void run();
  
signals:
  /*!
    This signal is emitted when task is completed successfully.
    \param files List of files in the directory.
  */
  void ready(File_info_list files);

  /*!
    This signal is emitted when task execution fails.
    \param message Displayed message.
  */
  void error(QString message);

  
public slots:

private:
  QString path;
  QString uri;

  
};

#endif // DIRECTORY_LIST_TASK_H
