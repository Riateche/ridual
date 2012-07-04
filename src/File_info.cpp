#include "File_info.h"
#include "utils.h"

File_info::File_info():
  is_file(false),
  permissions(-1)
{
}

QStringList File_info_list::paths() {
  QStringList r;
  foreach(File_info i, *this) {
    r << i.full_path;
  }
  return r;
}


