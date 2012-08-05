#include "gtest/gtest.h"
#include "Directory_tree_item.h"
#include <QDebug>

TEST(Directory_tree_item, test1) {
  Directory_tree_item item;
  item.is_folder = true;
  item.parent_path = TEST_ENV_PATH;
  item.name = "dir2";
  Directory_tree_item* current = 0;
  while(current = current? current->find_next(): &item) {
    qDebug() << current->get_absolute_path();
  }
  qDebug() << "and again";
  current = 0;
  while(current = current? current->find_next(): &item) {
    qDebug() << current->get_relative_path();
  }
  //todo: test on single file, on single folder without files

}
