#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <QFileInfo>
#include <QString>

class File_info {
public:
  enum Type { type_file, type_mount };
  Type type;
  QFileInfo i;
  QString mime_type();

  QString mount_name;
  bool mount_ready;
};

#endif // FILE_INFO_H
