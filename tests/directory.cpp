#include "gtest/gtest.h"
#include "Directory.h"
#include <QTest>
#include <QSignalSpy>
#include <QDir>
#include "Main_window.h"
#include "Core.h"
#include <QDebug>
#include "debug_utils.h"
#include "File_info.h"
#include <QProcess>
#include <QElapsedTimer>
#include "Mount_manager.h"

void test_dir_refresh(Directory* dir, File_info_list* result, bool do_refresh, bool expect_error, int max_time = 3000) {
  QSignalSpy spy1(dir, SIGNAL(ready(File_info_list)));
  QSignalSpy spy2(dir, SIGNAL(error(QString)));
  if (do_refresh) dir->refresh();

  QElapsedTimer t;
  t.start();
  while(spy1.isEmpty() && spy2.isEmpty()) {
    if (t.elapsed() > max_time) break;
    QTest::qWait(50);
  }

  if (spy2.count() > 0) qDebug() << "error signal: " << spy2[0][0].toString();
  ASSERT_EQ(expect_error? 0: 1, spy1.count());
  ASSERT_EQ(expect_error? 1: 0, spy2.count());
  if (spy1.count() > 0 && spy1[0].count() > 0) {
    *result = spy1[0][0].value<File_info_list>();
  }
}

TEST(Directory, files_list) {
  Core c;
  QString uri = env_dir("directory").absoluteFilePath("dir1");
  Directory d(&c, uri);
  EXPECT_EQ(uri, d.get_uri());
  EXPECT_FALSE(uri.endsWith("/"));

  File_info_list list;
  test_dir_refresh(&d, &list, true, false);
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
    EXPECT_FALSE(i.icon.isNull()) << "Icon is missing";
    //EXPECT_EQ(i.uri, i.full_path) << "uri and full_path must be equal here";
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

  //now we will check watching
  QDir dir(TEST_ENV_PATH);
  ASSERT_TRUE(dir.cd("directory/dir1"));
  QFile file(dir.absoluteFilePath("new_file.txt"));
  file.open(QFile::WriteOnly);
  file.write("test");
  file.close();

  test_dir_refresh(&d, &list, false, false);
  qSort(list.begin(), list.end(), uri_less_than);
  ASSERT_EQ(5, list.count());
  bool found = false;
  foreach(File_info fi, list) {
    if (fi.file_name() == "new_file.txt") {
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

void umount(Core* core, QString uri) {
  QString unmount_cmd = QString("gvfs-mount -u \"%1\"").arg(uri);
  QProcess process;
  process.start(unmount_cmd);
  process.waitForFinished();
  bool mounted = true;
  while(mounted) {
    QTest::qWait(500);
    mounted = false;
    foreach(Gio_mount m, core->get_mount_manager()->get_mounts()) {
      if (m.uri == uri) mounted = true;
    }
  }
}


TEST(Directory, ftp_list) {  
  Core c;
  QString uri = "ftp://anonymous@ftp.ubuntu.com/";
  umount(&c, uri);
  Directory dir1(&c, uri);
  File_info_list list;
  test_dir_refresh(&dir1, &list, true, false, 10000);
  ASSERT_EQ(1, list.count());

  bool found = false;
  foreach(Gio_mount m, c.get_mount_manager()->get_mounts()) {
    if (m.uri == uri) { found = true; }
  }
  EXPECT_TRUE(found) << "Testing location must be mounted here.";

  Directory dir2(&c, uri);
  test_dir_refresh(&dir2, &list, true, false, 10000);
  ASSERT_EQ(1, list.count()) << "Fetching dir from already mounted location failed.";

}


TEST(Directory, mount_interrupt) {
  Core c;
  QString uri = "ftp://anonymous@ftp.mozilla.org/";
  umount(&c, uri);
  Directory* dir1 = new Directory(&c, uri);
  dir1->refresh();
  delete dir1;
  QTest::qWait(5000);
}

TEST(Directory, error_reporting) {
  Core c;
  Directory dir1(&c, "/non-existent-dir");
  File_info_list list;
  test_dir_refresh(&dir1, &list, true, true);
}

void test_normalization(Core* core, QString uri, QString expected_result) {
  Directory d(core, uri);
  EXPECT_EQ(expected_result, d.get_uri());
}

TEST(Directory, uri_normalization) {
  Core c;
  QStringList tests;
  tests << "/" << "/";
  tests << "~" << QDir::homePath();
  tests << "~/something/" << QDir::home().absoluteFilePath("something");
  tests << "/some/path/" << "/some/path";
  tests << "/some/path" << "/some/path";
  tests << "/some/~/not_home" << "/some/~/not_home";
  tests << "/some//path" << "/some/path";
  tests << "/some/path//" << "/some/path";
  tests << "ftp://user@host.com/test" << "ftp://user@host.com/test";
  tests << "ftp://user@host.com/test/" << "ftp://user@host.com/test";
  tests << "ftp://user@host.com////test/" << "ftp://user@host.com/test";
  tests << "ftp://user@host.com/" << "ftp://user@host.com/";
  tests << "ftp://user@host.com" << "ftp://user@host.com/";

  for(int i = 0; i < tests.count(); i+=2) {
    test_normalization(&c, tests[i], tests[i+1]);
  }

}

void test_parent_uri(Core* core, QString uri, QString expected_result) {
  Directory d(core, uri);
  EXPECT_EQ(expected_result, d.get_parent_uri());
}

TEST(Directory, get_parent_uri) {
  Core c;
  QStringList tests;
  tests << "/home/user/" << "/home";
  tests << "/home" << "/";
  tests << "/" << "places";
  tests << "places" << "places";
  tests << "places/mounts" << "places";
  tests << "ftp://user@host.com" << "places/mounts";
  tests << "ftp://user@host.com/something" << "ftp://user@host.com/";
  tests << "ftp://user@host.com/something/else" << "ftp://user@host.com/something";

  for(int i = 0; i < tests.count(); i+=2) {
    test_parent_uri(&c, tests[i], tests[i+1]);
  }

}
