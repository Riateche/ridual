#include "Directory_tree_item.h"
#include <QDebug>

Directory_tree_item::Directory_tree_item() :
  is_folder(false),
  is_folder_read(false),
  is_processed(false),
  error(no_error),
  iterator(0),
  parent(0)
{
  //qDebug() << "   Directory_tree_item constructor";
}

Directory_tree_item::~Directory_tree_item() {
  //qDebug() << "   Directory_tree_item destructor";
  foreach(Directory_tree_item* c, children) delete c;
}

Directory_tree_item* Directory_tree_item::find_next() {
  if (is_folder) {
    if (!children.isEmpty()) return children.first();
    Directory_tree_item* r = read_next_child();
    if (r) return r;
  }
  Directory_tree_item* p = this;
  while(p->parent) {
    Directory_tree_item* item = p->parent->get_next_for(p);
    if (item) return item;
    p = p->parent;
  }
  return 0;
}

QString Directory_tree_item::get_path(bool absolute) {
  QStringList list;
  Directory_tree_item* item = this;
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

Directory_tree_item *Directory_tree_item::read_next_child() {
//  qDebug() << "read_next_child" << get_path(false);
  if (is_folder_read) {
//    qDebug() << "r1";
    return 0;
  }
  if (!iterator) {
    iterator = new QDirIterator(get_path(true), QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
//    qDebug() << "create iterator for " << get_path(true) << iterator->hasNext();
  }
  if (!iterator->hasNext()) {
    is_folder_read = true;
//    qDebug() << "r2";
    return 0;
  }
  iterator->next();
  Directory_tree_item* new_item = new Directory_tree_item();
  QFileInfo fi = iterator->fileInfo();
  new_item->name = fi.fileName();
  new_item->is_folder = fi.isDir();
  if (!fi.isReadable()) {
    new_item->error = error_cannot_read;
  }
  new_item->parent = this;
  children << new_item;
  return new_item;
}

Directory_tree_item* Directory_tree_item::get_next_for(Directory_tree_item *item) {
  if (!children.contains(item)) {
    qWarning("Directory_tree_item::get_next_for: error: unknown child");
    return 0;
  }
  //children is definitely not empty here
  if (children.last() != item) {
    return children[children.indexOf(item) + 1];
  }
  return read_next_child(); // can be 0
}
