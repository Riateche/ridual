#include "gtest/gtest.h"
#include <QDebug>
#include "File_list_model.h"
#include "debug_output.h"
#include <QListView>

TEST(File_list_model, get_mime_description) {
  EXPECT_FALSE(File_list_model::get_mime_description("text/plain").isEmpty());
}

TEST(File_list_model, format_octal_permissions) {
  QFile::Permissions p = 0;
  EXPECT_EQ("000", File_list_model::format_octal_permissions(p));

  p = QFile::ReadOwner | QFile::WriteGroup | QFile::ExeOther;
  EXPECT_EQ("421", File_list_model::format_octal_permissions(p));

  p = QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner;
  EXPECT_EQ("700", File_list_model::format_octal_permissions(p));

}

TEST(File_list_model, columns) {
  File_list_model model;
  Columns columns1;
  columns1 << Column::uri << Column::mime_type;
  model.set_columns(columns1);
  File_info_list list;
  model.set_data(list);
  EXPECT_EQ(columns1, model.get_current_columns());

  list.columns << Column::name << Column::type_description;
  list.disable_sort = true;
  model.set_data(list);
  EXPECT_NE(columns1, model.get_current_columns());
  EXPECT_EQ(list.columns, model.get_current_columns());

  list.columns.clear();
  model.set_data(list);
  EXPECT_EQ(columns1, model.get_current_columns());

}


TEST(File_list_model, cell_count) {
  File_list_model model;
  File_info_list list;
  list << File_info() << File_info() << File_info();
  model.set_data(list);
  EXPECT_EQ(3, model.rowCount());
  EXPECT_EQ(model.get_current_columns().count(), model.columnCount());

  list.columns << Column::name << Column::extension;
  list.disable_sort = true;
  model.set_data(list);
  EXPECT_EQ(3, model.rowCount());
  EXPECT_EQ(2, model.columnCount());

  list.clear();
  model.set_data(list);
  EXPECT_EQ(1, model.rowCount());
  EXPECT_EQ(1, model.columnCount());

}


TEST(File_list_model, index_for_uri) {
  File_list_model model;
  File_info_list list;
  File_info fi;
  fi.uri = "/usr";
  list << fi;
  fi.uri = "/bin";
  list << fi;
  fi.uri = "/etc";
  list << fi;
  fi.uri = "/var";
  list << fi;
  fi.uri = "/boot";
  list << fi;
  model.set_data(list);

  //qDebug() << model.index(0, 0);
  EXPECT_EQ(0, model.index(0, 0).row()) << "Failed to create QModelIndex.";

  EXPECT_EQ(2, model.index_for_uri("/etc").row()) << "Wrong result of index_for_uri.";
  EXPECT_EQ(0, model.index_for_uri("/etc").column()) << "Wrong result of index_for_uri.";

  EXPECT_TRUE(model.get_file_info(model.index(2, 0)).uri == list[2].uri)
      << "Failed to get file info by index.";
}


TEST(File_list_model, current_line_header) {
  File_list_model model;
  File_info_list list;
  list << File_info() << File_info() << File_info();
  model.set_data(list);

  model.set_current_index(model.index(1, 0));
  EXPECT_TRUE(model.headerData(0, Qt::Vertical, Qt::DisplayRole) != ">");
  EXPECT_TRUE(model.headerData(1, Qt::Vertical, Qt::DisplayRole) == ">");
  EXPECT_TRUE(model.headerData(2, Qt::Vertical, Qt::DisplayRole) != ">");

}
