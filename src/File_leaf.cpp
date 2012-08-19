#include "File_leaf.h"
#include <QStringList>

File_leaf::File_leaf(const QString &_name, bool is_folder, const QString &_parent_path) :
  name(_name), parent_path(_parent_path), parent(0), _is_folder(is_folder)
{
}

File_leaf::File_leaf(const QString &_name, bool is_folder, const File_leaf *_parent) :
  name(_name), parent(_parent), _is_folder(is_folder)
{
}



QString File_leaf::get_path(bool absolute) const {
  QStringList list;
  const File_leaf* item = this;
  while(item) {
    list.prepend(item->name);
    if (!item->parent_path.isEmpty()) {
      if (absolute) list.prepend(item->parent_path);
      break;
    }
    item = item->parent;
  }
  return list.join("/");
}
