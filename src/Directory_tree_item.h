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

  Directory_tree_item* parent;
  QString name;

  bool is_folder;
  bool is_prepared;
  bool is_processed;
  Error_type error_type;
  Error_reaction error_reaction;

  Directory_tree_item* find_next();

  QString parent_path; //for root item only!
  inline QString get_relative_path() { return get_path(false); }
  inline QString get_absolute_path() { return get_path(true); }

  void set_children(const QList<Directory_tree_item*>& v);



private:
  QString get_path(bool absolute);
  Directory_tree_item* read_next_child();
  Directory_tree_item* get_next_for(Directory_tree_item* item);

  bool is_folder_read;
  QList<Directory_tree_item*> children;
  QDirIterator* iterator;

};

#endif // DIRECTORY_TREE_ITEM_H
