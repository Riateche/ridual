#ifndef DIRECTORY_TREE_ITEM_H
#define DIRECTORY_TREE_ITEM_H

#include <QString>
#include <QList>
#include <QDirIterator>
#include "Error_reaction.h"

class Directory_tree_item {
public:
  Directory_tree_item();
  ~Directory_tree_item();
  bool is_folder;
  bool is_folder_read;
  bool is_processed;
  Error_type error_type;
  QDirIterator* iterator;
  Directory_tree_item* parent;

  Directory_tree_item* find_next();

  QString name;

  QString parent_path; //for root item only!
  inline QString get_relative_path() { return get_path(false); }
  inline QString get_absolute_path() { return get_path(true); }

  QList<Directory_tree_item*> children;

private:
  QString get_path(bool absolute);
  Directory_tree_item* read_next_child();
  Directory_tree_item* get_next_for(Directory_tree_item* item);
};

#endif // DIRECTORY_TREE_ITEM_H
