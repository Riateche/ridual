#include "Directory_list_task.h"
#include <QDir>
#include <QDebug>

#include "qt_gtk.h"


Directory_list_task::Directory_list_task(QString p_path, QString p_uri) :
  path(p_path),
  uri(p_uri)
{
}

Directory_list_task::~Directory_list_task() {
}

void Directory_list_task::run() {
  QDir dir(path);
  if (!dir.exists()) {
    emit error(tr("Directory '%1' does not exist.").arg(dir.absolutePath()));
    deleteLater();
    return;
  }
  if (!dir.isReadable()) {
    emit error(tr("Directory '%1' can not be read.").arg(dir.absolutePath()));
    deleteLater();
    return;
  }
  QFileInfoList list = dir.entryInfoList(QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot);
  GError* gerror = 0;
  File_info_list r;
  if (!uri.endsWith("/")) uri += "/";
  foreach (QFileInfo info, list) {
    File_info item;
    //item.name = info.fileName();
    //item.full_path = info.absoluteFilePath();
    item.uri = uri + info.fileName();
    item.is_file = info.isFile();
    if (item.is_file) {
      item.file_size = info.size();
    } else {
      item.file_size = 0;
    }
    item.owner = info.owner();
    item.group = info.group();
    item.permissions = info.permissions();
    //item.name = info.baseName();
    //item.extension = info.suffix();
    //item.full_name = info.fileName();
    //item.parent_folder = info.dir().path();
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
    r << item;
  }
  emit ready(r);
}
