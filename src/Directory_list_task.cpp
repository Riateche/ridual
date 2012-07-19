#include "Directory_list_task.h"
#include <QDir>
#include <QDebug>

#include "qt_gtk.h"


Directory_list_task::Directory_list_task(QString p_path) :
  path(p_path)
{
}

Directory_list_task::~Directory_list_task() {
}

void Directory_list_task::exec() {
  QDir dir(path);
  if (!dir.exists()) {
    emit error(tr("Directory %1 does not exist.").arg(dir.absolutePath()));
    deleteLater();
    return;
  }
  if (!dir.isReadable()) {
    emit error(tr("Directory %1 can not be read.").arg(dir.absolutePath()));
    deleteLater();
    return;
  }
  QFileInfoList list = dir.entryInfoList(QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot);
  GError* gerror = 0;
  File_info_list r;
  foreach (QFileInfo info, list) {
    File_info item;
    item.name = info.fileName();
    item.full_path = info.absoluteFilePath();
    item.is_file = info.isFile();
    item.owner = info.owner();
    item.group = info.group();
    item.permissions = info.permissions();
    item.name = info.baseName();
    item.extension = info.suffix();
    item.full_name = info.fileName();
    item.parent_folder = info.dir().path();
    item.date_accessed = info.lastRead();
    item.date_modified = info.lastModified();
    item.date_created = info.created();
    item.is_executable = item.is_file && info.isExecutable();

    //qDebug() << "path for GFile: " << info.absoluteFilePath();

    GFile *file = g_file_new_for_path (info.absoluteFilePath().toLocal8Bit());
    //qDebug() << "GFile* file = " << file;
    GFileInfo* gfileinfo = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE, GFileQueryInfoFlags(), 0, &gerror);
    if (gerror) {
      emit error(QString::fromLocal8Bit(gerror->message));
      g_error_free(gerror);
      gerror = 0;
      return;
    }
    if (gfileinfo) {
      //qDebug() << "GFileInfo* gfileinfo = " << gfileinfo;
      const char* content_type = g_file_info_get_content_type(gfileinfo);
      item.mime_type = QString::fromLocal8Bit(content_type);
      g_object_unref(gfileinfo);
    }

    g_object_unref(file);


    /*
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
  emit ready(r);
  deleteLater();
}
