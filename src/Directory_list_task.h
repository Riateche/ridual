#ifndef DIRECTORY_LIST_TASK_H
#define DIRECTORY_LIST_TASK_H

#include "File_info.h"
#include <QRunnable>

class File_system_engine;

/*! This class provides executing of directory listing in the thread pool.
  It's used internally by Directory.

  Don't use this class directly from other threads. Use signal-slot interaction only.
  */
class Directory_list_task : public QObject, public QRunnable {
  Q_OBJECT
public:
  explicit Directory_list_task(QString p_uri, File_system_engine* e);
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

  void location_not_found();

private:
  QString uri;
  File_system_engine* fs_engine;
  
};

#endif // DIRECTORY_LIST_TASK_H
