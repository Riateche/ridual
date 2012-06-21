#include "Tasks_thread.h"
#include "File_info.h"
#include <QDir>

Tasks_thread::Tasks_thread(QObject *parent) :
  QThread(parent)
{
  stopping = false;
}

void Tasks_thread::interrupt() {
  stopping = true;
  wait();
}

void Tasks_thread::add_task(Task task) {
  queue.push_back(task);
}

void Tasks_thread::run() {
  while(!stopping) {
    if (queue.isEmpty()) {
      msleep(50);
      continue;
    }
    Task task = queue.last();
    queue.pop_back();
    connect(this, SIGNAL(ready(QVariant)), task.receiver, task.slot);
    QVariant result;
    switch(task.type) {
      case task_directory_list: {
        QDir dir(task.arguments.first().toString());
        if (!dir.exists()) {
          result = QVariant::fromValue<Task_error>(Task_error(tr("Directory %1 does not exist.").arg(dir.absolutePath())));
        } else if (!dir.isReadable()){
          result = QVariant::fromValue<Task_error>(Task_error(tr("Directory %1 can not be read.").arg(dir.absolutePath())));
        } else {
          QFileInfoList list = dir.entryInfoList(QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot);
          QList<File_info> r;
          foreach (QFileInfo info, list) {
            File_info item;
            item.caption = info.fileName();
            item.file_path = info.absoluteFilePath();
            item.is_file = info.isFile();
            r << item;
          }
          result = QVariant::fromValue< QList<File_info> >(r);
        }
      }
    }
    emit ready(result);
    disconnect(this, SIGNAL(ready(QVariant)), task.receiver, task.slot);
  }
}
