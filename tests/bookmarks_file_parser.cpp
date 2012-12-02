#include "gtest/gtest.h"
#include "Bookmarks_file_parser.h"
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QTest>
#include "debug_utils.h"




TEST(Bookmarks_file_parser, gtk_bookmarks) {
  Bookmarks_file_parser p(env_dir("bookmarks_file_parser").absoluteFilePath(".gtk-bookmarks"),
                          Bookmarks_file_parser::format_gtk);
  File_info_list list = p.get_all();
  ASSERT_EQ(8, list.count());
  EXPECT_EQ("/home/user/Documents", list[0].uri);
  EXPECT_EQ("/home/user/Music", list[1].uri);
  EXPECT_EQ("/d", list[2].uri);
  EXPECT_EQ("/s", list[3].uri);
  EXPECT_EQ(QString::fromUtf8("/tmp/тест"), list[4].uri);
  EXPECT_EQ(QString::fromUtf8("/home/тест"), list[5].uri);
  EXPECT_EQ(QString::fromUtf8("sftp://user@something.org/"), list[6].uri);
  EXPECT_EQ(QString::fromUtf8("smb://user@something.org/"), list[7].uri);

  EXPECT_EQ("Documents", list[0].name);
  EXPECT_EQ("Music", list[1].name);
  EXPECT_EQ("dname", list[2].name);
  EXPECT_EQ(QString::fromUtf8("тест"), list[3].name);
  EXPECT_EQ(QString::fromUtf8("тест"), list[4].name);
  EXPECT_EQ(QString::fromUtf8("тест2"), list[5].name);
  EXPECT_EQ(QString::fromUtf8("Some name"), list[6].name);
  EXPECT_EQ(QString::fromUtf8("smb://user@something.org/"), list[7].uri);

}


TEST(Bookmarks_file_parser, xdg_bookmarks) {
  Bookmarks_file_parser p(env_dir("bookmarks_file_parser").absoluteFilePath("user-dirs.dirs"),
                          Bookmarks_file_parser::format_xdg);

  File_info_list list = p.get_all();

  QString home = QDir::homePath();
  EXPECT_FALSE(home.endsWith("/"));

  ASSERT_EQ(7, list.count());

  EXPECT_EQ(home + "/Desktop", list[0].uri);
  EXPECT_EQ(home + "/Downloads", list[1].uri);
  EXPECT_EQ("/some/templates", list[2].uri);
  EXPECT_EQ(home, list[3].uri);
  //EXPECT_EQ(home + "/\"Music\"", list[4].uri);
  EXPECT_EQ(home + "/Pictures", list[5].uri);
  EXPECT_EQ(home, list[6].uri);

  EXPECT_EQ(QObject::tr("Desktop"), list[0].name);
  EXPECT_EQ(QObject::tr("Downloads"), list[1].name);
  EXPECT_EQ(QObject::tr("Templates"), list[2].name);
  EXPECT_EQ(QObject::tr("Public"), list[3].name);
  EXPECT_EQ(QObject::tr("Music"), list[4].name);
  EXPECT_EQ(QObject::tr("Pictures"), list[5].name);
  EXPECT_EQ(QObject::tr("Videos"), list[6].name);

}



TEST(Bookmarks_file_parser, auto_update) {
  QDir dir = env_dir("bookmarks_file_parser");
  Bookmarks_file_parser p(dir.absoluteFilePath(".gtk-bookmarks"),
                          Bookmarks_file_parser::format_gtk);
  File_info_list list = p.get_all();
  ASSERT_EQ(8, list.count());
  QSignalSpy spy(&p, SIGNAL(changed()));
  QFile f(dir.absoluteFilePath(".gtk-bookmarks"));
  ASSERT_TRUE(f.open(QFile::Append));
  f.write("file:///new/uri new name\n");
  f.close();
  ASSERT_TRUE(wait_for_signal(&spy));
  list = p.get_all();
  ASSERT_EQ(9, list.count()) << "Bookmarks wasn't updated";
  EXPECT_EQ("/new/uri", list[8].uri);
  EXPECT_EQ("new name", list[8].name);

}
