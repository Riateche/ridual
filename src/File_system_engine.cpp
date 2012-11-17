#include "File_system_engine.h"
#include <errno.h>

File_system_engine::File_system_engine()
{
}

File_system_engine::error_cause File_system_engine::get_cause_from_errno() {
  switch(errno) {
    case EACCES:
    case EPERM:
      return permission_denied;
      break;
    case EEXIST:
      return already_exists;
      break;
    case EMLINK:
      return too_many_entries;
      break;
    case ENOSPC:
      return filesystem_full;
      break;
    case EROFS:
      return readonly_filesystem;
      break;
    case EBUSY:
      return busy;
      break;
    case EINVAL:
      return invalid_path;
      break;
    case ENOENT:
      return not_found;
      break;
    case ENOTDIR:
      return not_directory;
      break;
    case ENAMETOOLONG:
      return path_too_long;
      break;
    case ELOOP:
      return symbolic_links_loop;
      break;
    case EIO:
      return io_error;
      break;
    default:
      return unknown_cause;
  }
}

File_system_engine::Exception::Exception(File_system_engine::error_type p_type, error_cause p_cause)
  : type(p_type), cause(p_cause) {

}

