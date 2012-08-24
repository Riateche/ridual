#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <QFileInfo>
#include <QString>
#include <QDateTime>
#include <QIcon>
#include <QVariant>
#include "Columns.h"

class File_info {
public:
  File_info();

  QString name; //! Displayed name (for special locations only)
  QString uri; //! item uri (required)
  //QString full_path; //! Real path in file system
  QIcon icon;
  bool is_file;
  bool is_executable;
  bool is_folder() { return !is_file; }
  qint64 file_size; // in bytes; 0 if folder or unknown

  QString file_name() const;
  QString extension() const;
  QString basename() const;

  //QString extension, full_name;
  QDateTime date_created, date_modified, date_accessed;
  QString mime_type; //, mime_description;
  QString owner, group;
  QFile::Permissions permissions;
};

class File_info_list: public QList<File_info> {
public:
  File_info_list() {}
  Columns columns;
  //QStringList paths();
};

Q_DECLARE_METATYPE(File_info)
Q_DECLARE_METATYPE(File_info_list)

#endif // FILE_INFO_H
