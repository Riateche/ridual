#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <QFileInfo>
#include <QString>
#include <QDateTime>
#include <QIcon>
#include <QVariant>
#include "Columns.h"

/*!
 * General representation of a file entry.
 *
 * Objects of this class are passed by value.
 */
class File_info {
public:
  File_info();

  QString name; //! Displayed name (for special locations only)
  QString uri; //! item uri (required)
  QIcon icon;
  bool is_folder;
  bool is_executable;
  bool is_file() const { return !is_folder; }  
  qint64 file_size; // in bytes; -1 if folder or unknown

  QString file_name() const;
  QString extension() const;
  QString basename() const;

  QDateTime date_created, date_modified, date_accessed;
  QString mime_type; //, mime_description;
  QString owner, group;
  QFile::Permissions permissions;

  bool is_header_entry; // header entry for "places" listing
};

/*!
 * List of files with representation hints.
 *
 * Objects of this class are passed by value.
 */
class File_info_list: public QList<File_info> {
public:
  File_info_list() : disable_sort(false) {}
  //! If not empty, instructs that these columns should be displayed.
  Columns columns;
  //! If true, items should not be sorted for display.
  bool disable_sort;
};

Q_DECLARE_METATYPE(File_info)
Q_DECLARE_METATYPE(File_info_list)

#endif // FILE_INFO_H
