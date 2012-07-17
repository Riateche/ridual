#include "gtest/gtest.h"
#include "Bookmarks_file_parser.h"
#include <QDir>

TEST(Bookmarks_file_parser, gtk_bookmarks) {
  Bookmarks_file_parser p(QDir(TEST_ENV_PATH).absoluteFilePath(".gtk-bookmarks"),
                          Bookmarks_file_parser::format_gtk);
  File_info_list list = p.get_all();
  ASSERT_EQ(8, list.count());
}
