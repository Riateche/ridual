#include "Read_directory_thread.h"

Read_directory_thread::Read_directory_thread(QString p_directory) : directory(p_directory) {
}

void Read_directory_thread::run() {
  QDir dir(directory);
  QFileInfoList list = dir.entryInfoList(QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot);
  emit ready(list);
  deleteLater();
}
