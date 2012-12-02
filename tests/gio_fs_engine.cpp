#include "gtest/gtest.h"
#include <QString>
#include <QStringList>
#include <QDebug>
#include "gio/Gio_file_system_engine.h"
#include "Directory.h"
#include "Core.h"
#include "debug_utils.h"


TEST(Gio_fs_engine, list) {
  Core core;
  Gio_file_system_engine engine(core.get_mount_manager());
  QString uri = env_dir("gio_fs_engine").absoluteFilePath("dir1");
  File_system_engine::Iterator* iterator = engine.list(uri);
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


TEST(Gio_fs_engine, unreadable_list) {
  Core core;
  Gio_file_system_engine engine(core.get_mount_manager());
  QString uri = env_dir("gio_fs_engine").absoluteFilePath("unreadable_dir");
  EXPECT_TRUE(QDir().mkdir(uri));
  EXPECT_TRUE(QFile(uri).setPermissions(0));
  try {
    engine.list(uri);
  } catch (File_system_engine::Exception e) {
    EXPECT_EQ(File_system_engine::directory_list_failed, e.get_type());
    EXPECT_EQ(File_system_engine::unknown_cause, e.get_cause());
    return;
  }
  EXPECT_TRUE(false) << "Exception was not thrown";
}

TEST(Gio_fs_engine, copy) {
  Core core;
  Gio_file_system_engine engine(core.get_mount_manager());
  QDir dir = env_dir("gio_fs_engine");
  File_system_engine::Operation* o =
      engine.copy(dir.absoluteFilePath("source.txt"), dir.absoluteFilePath("dest.txt"));
  while(!o->is_finished()) {
    o->run_iteration();
  }
  delete o;
  QFile f(dir.absoluteFilePath("dest.txt"));
  EXPECT_TRUE(f.exists());
  EXPECT_TRUE(f.open(QFile::ReadOnly));
  EXPECT_EQ("Source file contents", f.readAll());

  QFile old_f(dir.absoluteFilePath("source.txt"));
  EXPECT_TRUE(old_f.exists());
  EXPECT_TRUE(old_f.open(QFile::ReadOnly));
  EXPECT_EQ("Source file contents", old_f.readAll());
}

TEST(Gio_fs_engine, move) {
  Core core;
  Gio_file_system_engine engine(core.get_mount_manager());
  QDir dir = env_dir("gio_fs_engine");
  File_system_engine::Operation* o =
      engine.move(dir.absoluteFilePath("source.txt"), dir.absoluteFilePath("dest2.txt"));
  while(!o->is_finished()) {
    o->run_iteration();
  }
  delete o;
  QFile f(dir.absoluteFilePath("dest2.txt"));
  EXPECT_TRUE(f.exists());
  EXPECT_TRUE(f.open(QFile::ReadOnly));
  EXPECT_EQ("Source file contents", f.readAll());

  QFile old_f(dir.absoluteFilePath("source.txt"));
  EXPECT_FALSE(old_f.exists());
}

TEST(Gio_fs_engine, move_fail) {
  Core core;
  Gio_file_system_engine engine(core.get_mount_manager());
  QDir dir = env_dir("gio_fs_engine");

  try {
    File_system_engine::Operation* o =
        engine.move(dir.absoluteFilePath("no-such-file.txt"), dir.absoluteFilePath("dest3.txt"));
    while(!o->is_finished()) {
      o->run_iteration();
    }
    delete o;
  } catch (File_system_engine::Exception e) {
    EXPECT_EQ(File_system_engine::move_failed, e.get_type());
    EXPECT_EQ(File_system_engine::not_found, e.get_cause());
    return;
  }
  EXPECT_TRUE(false) << "Exception was not thrown";
}

TEST(Gio_fs_engine, remove) {
  Core core;
  Gio_file_system_engine engine(core.get_mount_manager());
  QDir dir = env_dir("gio_fs_engine");
  QString filename = dir.absoluteFilePath("file1.ini");
  EXPECT_TRUE(QFile(filename).exists());
  engine.remove(filename);
  EXPECT_FALSE(QFile(filename).exists());
}
