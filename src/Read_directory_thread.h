#ifndef READ_DIRECTORY_THREAD_H
#define READ_DIRECTORY_THREAD_H

#include <QThread>
#include <QDir>


class Read_directory_thread : public QThread {
  Q_OBJECT
public:
  explicit Read_directory_thread(QString directory);
  
signals:
  void ready(QFileInfoList files);

private:
  QString directory;

  virtual void run();

  
};

#endif // READ_DIRECTORY_THREAD_H
