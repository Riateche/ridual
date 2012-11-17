#include "gtest/gtest.h"
#include <QString>
#include <QStringList>
#include <QDebug>
#include "Real_file_system_engine.h"
#include "Directory.h"

bool uri_less_than(const File_info &v1, const File_info &v2) {
  return v1.uri < v2.uri;
}

TEST(Real_fs_engine, list) {
  Real_file_system_engine engine;
  QString uri = QDir(TEST_ENV_PATH).absoluteFilePath("dir1");
  Real_file_system_engine::Iterator* iterator = engine.list(uri);
  File_info_list list;
  while(iterator->has_next()) {
    list << iterator->get_next();
  }
  delete iterator;

  qSort(list.begin(), list.end(), uri_less_than);

  ASSERT_EQ(4, list.count());
  EXPECT_EQ("file1.txt", list[0].file_name());
  EXPECT_EQ("file2.png", list[1].file_name());
  EXPECT_EQ("file3.txt", list[2].file_name());
  EXPECT_EQ("folder1", list[3].file_name());

  EXPECT_EQ("file1", list[0].basename());
  EXPECT_EQ("file2", list[1].basename());
  EXPECT_EQ("file3", list[2].basename());
  EXPECT_EQ("folder1", list[3].basename());

  EXPECT_EQ(uri + "/file1.txt", list[0].uri);
  EXPECT_EQ(uri + "/file2.png", list[1].uri);
  EXPECT_EQ(uri + "/file3.txt", list[2].uri);
  EXPECT_EQ(uri + "/folder1", list[3].uri);

  foreach(File_info i, list) {
    EXPECT_EQ(uri, Directory::get_parent_uri(i.uri));
    EXPECT_TRUE(i.date_accessed.isValid());
    EXPECT_TRUE(i.date_created.isValid());
    EXPECT_TRUE(i.date_modified.isValid());

    EXPECT_FALSE(i.owner.isEmpty());
    EXPECT_FALSE(i.group.isEmpty());
    EXPECT_FALSE(i.permissions == -1);

  }

  EXPECT_TRUE(list[0].is_file());
  EXPECT_TRUE(list[1].is_file());
  EXPECT_TRUE(list[2].is_file());
  EXPECT_TRUE(list[3].is_folder);

  EXPECT_EQ("txt", list[0].extension());
  EXPECT_EQ("png", list[1].extension());
  EXPECT_EQ("txt", list[2].extension());
  EXPECT_EQ("", list[3].extension());

  EXPECT_EQ("text/plain", list[0].mime_type);
  EXPECT_EQ("image/png", list[1].mime_type);
  EXPECT_EQ("text/plain", list[2].mime_type);
  EXPECT_EQ("inode/directory", list[3].mime_type);

}


TEST(Real_fs_engine, unreadable_list) {
  Real_file_system_engine engine;
  QString uri = QDir(TEST_ENV_PATH).absoluteFilePath("unreadable_dir");
  QDir().mkdir(uri);
  QFile(uri).setPermissions(0);
  try {
    engine.list(uri);
  } catch (Real_file_system_engine::Exception e) {
    EXPECT_EQ(Real_file_system_engine::directory_list_failed, e.get_type());
    EXPECT_EQ(Real_file_system_engine::unknown_cause, e.get_cause());
    return;
  }
  EXPECT_TRUE(false) << "Exception was not thrown";
}

TEST(Real_fs_engine, copy_test) {
  Real_file_system_engine engine;
  QDir dir(TEST_ENV_PATH);
  EXPECT_TRUE(dir.cd("dir3"));
  Real_file_system_engine::Operation* o =
      engine.copy(dir.absoluteFilePath("source.txt"), dir.absoluteFilePath("dest.txt"));
  while(!o->is_finished()) {
    o->run_iteration();
  }
  delete o;
  QFile f(dir.absoluteFilePath("dest.txt"));
  EXPECT_TRUE(f.exists());
  EXPECT_TRUE(f.open(QFile::ReadOnly));
  EXPECT_EQ("Source file contents", f.readAll());
}
