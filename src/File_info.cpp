#include "File_info.h"
#include "utils.h"

QString File_info::mime_type() {
  return get_mime_type(i.absoluteFilePath());
}
