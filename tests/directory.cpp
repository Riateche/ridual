#include "gtest/gtest.h"
#include "Directory.h"
#include <QTest>
#include <QSignalSpy>
#include <QDir>
#include "Main_window.h"
#include <QDebug>
#include "debug_output.h"
#include "File_info.h"

TEST(Directory, files_list) {
  Main_window mw;
  QString uri = QDir(TEST_ENV_PATH).absoluteFilePath("dir1");
  Directory d(&mw, uri);
  EXPECT_EQ(uri, d.get_uri());
  EXPECT_FALSE(uri.endsWith("/"));
  QSignalSpy spy1(&d, SIGNAL(ready(File_info_list)));
  QSignalSpy spy2(&d, SIGNAL(error(QString)));
  d.refresh();
  QTest::qWait(1000);
  EXPECT_EQ(1, spy1.count()) << "Ready signal was not emitted";
  EXPECT_EQ(0, spy2.count()) << "Error signal was emitted";

  File_info_list list = spy1.takeFirst().first().value<File_info_list>();
  qDebug() << "count " << list.count();
  EXPECT_EQ(4, list.count());
  EXPECT_EQ("file1.txt", list[0].full_name);
  EXPECT_EQ("file2.png", list[1].full_name);
  EXPECT_EQ("file3.txt", list[2].full_name);
  EXPECT_EQ("folder1", list[3].full_name);

  EXPECT_EQ("file1", list[0].name);
  EXPECT_EQ("file2", list[1].name);
  EXPECT_EQ("file3", list[2].name);
  EXPECT_EQ("folder1", list[3].full_name);

  EXPECT_EQ(uri + "/file1.txt", list[0].uri);
  EXPECT_EQ(uri + "/file2.png", list[1].uri);
  EXPECT_EQ(uri + "/file3.txt", list[2].uri);
  EXPECT_EQ(uri + "/folder1", list[3].uri);

  foreach(File_info i, list) {
    EXPECT_FALSE(i.icon.isNull()) << "Icon is missing";
    EXPECT_EQ(i.uri, i.full_path) << "uri and full_path must be equal here";
    EXPECT_EQ(uri, i.parent_folder);
    EXPECT_TRUE(i.date_accessed.isValid());
    EXPECT_TRUE(i.date_created.isValid());
    EXPECT_TRUE(i.date_modified.isValid());

    EXPECT_FALSE(i.owner.isEmpty());
    EXPECT_FALSE(i.group.isEmpty());
    EXPECT_FALSE(i.permissions == -1);

  }

  EXPECT_TRUE(list[0].is_file);
  EXPECT_TRUE(list[1].is_file);
  EXPECT_TRUE(list[2].is_file);
  EXPECT_TRUE(list[3].is_folder());

  EXPECT_EQ("txt", list[0].extension);
  EXPECT_EQ("png", list[1].extension);
  EXPECT_EQ("txt", list[2].extension);
  EXPECT_EQ("", list[3].extension);

  EXPECT_EQ("text/plain", list[0].mime_type);
  EXPECT_EQ("image/png", list[1].mime_type);
  EXPECT_EQ("text/plain", list[2].mime_type);
  EXPECT_EQ("inode/directory", list[3].mime_type);





}
