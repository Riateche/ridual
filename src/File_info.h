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

  /*! For special locations - displayed name;
    for regular files - basename (filename without path and extension). */
  QString name;
  QString uri; //! The uri displaying in the address bar
  QString full_path; //! Real path in file system
  QIcon icon;
  bool is_file;
  bool is_folder() { return !is_file; }

  QString extension, full_name, parent_folder;
  QDateTime date_created, date_modified, date_accessed;
  QString mime_type; //, mime_description;
  QString owner, group;
  QFile::Permissions permissions;
};

class File_info_list: public QList<File_info> {
public:
  File_info_list(): custom_columns_mode(false) {}
  bool custom_columns_mode;
  Columns columns;
};

//Q_DECLARE_METATYPE(File_info)
Q_DECLARE_METATYPE(File_info_list)

#endif // FILE_INFO_H
