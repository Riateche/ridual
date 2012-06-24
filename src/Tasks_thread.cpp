#include "Tasks_thread.h"
#include "File_info.h"
#include <QDir>
#include <QDebug>

#include "qt_gtk.h"
#include "gio/gio.h"

#define ICON_BUFFER_SIZE 64*64*16
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

            /*
            GFile *file = g_file_new_for_path (info.absoluteFilePath().toLocal8Bit());
            //todo: check for null pointers; add gerror for debug output
            GFileInfo *file_info = g_file_query_info (file, "standard::*", (GFileQueryInfoFlags) 0, 0, 0);
            GIcon *icon = g_file_info_get_icon (file_info);
            qDebug() << "is_loadable" << G_IS_LOADABLE_ICON(icon);
            qDebug() << "is_themed" << G_IS_THEMED_ICON(icon);
            qDebug() << "is_emblem" << G_IS_EMBLEM(icon);
            qDebug() << "is_emblemed_icon" << G_IS_EMBLEMED_ICON(icon);
            qDebug() << "is_file_icon" << G_IS_FILE_ICON(icon);

            GError* error = 0;
            GInputStream* icon_stream = g_loadable_icon_load( (GLoadableIcon*) icon, 16, 0, 0, &error);
            if (error) qDebug() << error->message;
            char buffer[ICON_BUFFER_SIZE];
            gsize bytes_read;
            error = 0;
            gboolean b = g_input_stream_read_all(icon_stream, buffer, ICON_BUFFER_SIZE, &bytes_read, 0, &error);
            if (b) {
              //QString s = QString::fromAscii(buffer, bytes_read);
              qDebug() << "g_input_stream_read_all bytes read: " << bytes_read;
            } else {
              qDebug() << "g_input_stream_read_all failed";
            }
*/



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
