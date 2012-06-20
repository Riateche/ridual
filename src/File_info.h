#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <QFileInfo>
#include <QString>
#include <QDateTime>
#include <QIcon>
#include <QVariant>

class File_info {
public:
  File_info();
  QString caption; //! The string displaying in file list
  QString uri; //! The uri displaying in the address bar
  QString file_path; //! Real path in file system
  QIcon icon;
  bool is_file;
  bool is_folder() { return !is_file; }

  QDateTime created_at, modified_at;
  QString mime_type;
  QString owner, group;
  int permissions;
};

//Q_DECLARE_METATYPE(File_info)
Q_DECLARE_METATYPE(QList<File_info>)

#endif // FILE_INFO_H
