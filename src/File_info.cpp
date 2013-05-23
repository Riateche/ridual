#include "File_info.h"
#include "utils.h"

File_info::File_info():
  is_folder(false),
  is_executable(false),
  file_size(-1),
  permissions(-1)
{
}

QString File_info::file_name() const {
  if (uri.isEmpty()) return "";
  if (uri == "/") return "/";
  return uri.split("/", QString::SkipEmptyParts).last();
}

QString File_info::extension() const {
  return QFileInfo(file_name()).suffix();
}

QString File_info::basename() const {
  return QFileInfo(file_name()).completeBaseName();
}

/*QStringList File_info_list::paths() {
  QStringList r;
  foreach(File_info i, *this) {
    r << i.full_path;
  }
  return r;
}*/


