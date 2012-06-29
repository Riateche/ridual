#include "Tasks_thread.h"
#include "File_info.h"
#include <QDir>
#include <QDebug>

#include "qt_gtk.h"
#include "gio/gio.h"

//#define ICON_BUFFER_SIZE 64*64*16
// max. size 64x64, 16 will be enough for all ;)


Tasks_thread::Tasks_thread(QObject *parent) :
  QThread(parent)
{
  stopping = false;
}

void Tasks_thread::interrupt() {
  stopping = true;
  wait();
}

void Tasks_thread::add_task(Task *task) {
  QMutexLocker locker(&queue_mutex);
  queue.push_back(task);
}

void Tasks_thread::run() {
  while(!stopping) {
    Task* task = 0;
    {
      QMutexLocker locker(&queue_mutex);
      if (!queue.isEmpty()) {
        task = queue.last();
      }
    }
    if (!task) {
      msleep(50);
      continue;
    }
    queue.pop_back();
    task->exec();
  }
}
