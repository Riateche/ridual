#include "Read_directory_thread.h"
#include "utils.h"

Read_directory_thread::Read_directory_thread(QString p_directory) : directory(p_directory) {
}

void Read_directory_thread::run() {
  QDir dir(directory);
  QFileInfoList list = dir.entryInfoList(QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot);
  QList<File_info> result;
  foreach (QFileInfo info, list) {
    File_info item;
    item.type = File_info::type_file;
    item.i = info;
    result << item;
  }
  emit ready(result);
  deleteLater();
}
