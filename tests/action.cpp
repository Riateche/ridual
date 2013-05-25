#include "gtest/gtest.h"
#include "Action.h"
#include <QDir>
#include "gio/Gio_file_system_engine.h"
#include "Mount_manager.h"
#include <QProcess>
#include "debug_utils.h"
#include <QSignalSpy>


class ActionTest : public ::testing::Test {
 protected:
  // Per-test-case set-up.
  // Called before the first test in this test case.
  // Can be omitted if not needed.
  static void SetUpTestCase() {
    dir = env_dir("action");
    for(int i = 1; i <= 3; i++) {
      File_info file_info;
      file_info.uri = dir.absoluteFilePath(QString("move_source/%1").arg(i));
      files_list1 << file_info;
    }
    mount_manager = new Mount_manager;
    fs_engine = new Gio_file_system_engine(mount_manager);
  }

  static void TearDownTestCase() {
    delete fs_engine;
    delete mount_manager;
  }

  static QDir dir;
  static Action_data data;
  static File_info_list files_list1;
  static Mount_manager* mount_manager;
  static Gio_file_system_engine* fs_engine;
};

QDir ActionTest::dir;
Action_data ActionTest::data;
Mount_manager* ActionTest::mount_manager;
Gio_file_system_engine* ActionTest::fs_engine;
File_info_list ActionTest::files_list1;

TEST_F(ActionTest, copy) {
  File_info file_info;
  file_info.uri = dir.absoluteFilePath("dir");
  file_info.is_folder = true;
  data.targets << file_info;
  EXPECT_TRUE(dir.mkdir("destination"));
  data.destination = dir.absoluteFilePath("destination");
  data.type = Action_type::copy;
  Action a(0, fs_engine, data);
  QSignalSpy spy1(&a, SIGNAL(question(Question_data)));
  a.run();
  EXPECT_EQ(0, spy1.count());

  QProcess p;
  p.setWorkingDirectory(dir.absolutePath());
  p.start("diff dir destination/dir");
  p.waitForFinished();
  EXPECT_EQ(0, p.exitCode());
}

TEST_F(ActionTest, copy_skip) {
  data.targets = files_list1;
  data.type = Action_type::copy;
  data.destination = dir.absoluteFilePath("move_dest1");
  Action a(0, fs_engine, data);
  QSignalSpy spy1(&a, SIGNAL(question(Question_data)));
  a.run();
  EXPECT_EQ(1, spy1.count());
  Question_data question = spy1[0][0].value<Question_data>();
  EXPECT_EQ(Error_type::file_system_error, question.error_type);
  EXPECT_EQ(&a, question.action);
  EXPECT_EQ(File_system_engine::file_already_exists, question.fs_exception.get_cause());
  EXPECT_EQ(File_system_engine::copy_failed, question.fs_exception.get_type());
  QDir dir1 = dir;
  EXPECT_TRUE(dir1.cd("move_dest1"));
  EXPECT_TRUE(dir1.exists("1"));
  EXPECT_TRUE(dir1.exists("2"));
  EXPECT_FALSE(dir1.exists("3"));
  a.question_answered(Error_reaction::skip);
  EXPECT_TRUE(dir1.exists("3"));
  check_file_contents(dir1.absoluteFilePath("1"), "new1");
  check_file_contents(dir1.absoluteFilePath("2"), "old2");
  check_file_contents(dir1.absoluteFilePath("3"), "new3");
}


TEST_F(ActionTest, copy_abort) {
  data.targets = files_list1;
  data.type = Action_type::copy;
  data.destination = dir.absoluteFilePath("move_dest2");
  Action a(0, fs_engine, data);
  QSignalSpy spy1(&a, SIGNAL(question(Question_data)));
  a.run();
  EXPECT_EQ(1, spy1.count());

  QDir dir1 = dir;
  EXPECT_TRUE(dir1.cd("move_dest2"));
  a.question_answered(Error_reaction::abort);
  EXPECT_FALSE(dir1.exists("3"));
  check_file_contents(dir1.absoluteFilePath("2"), "old2");
  check_file_contents(dir1.absoluteFilePath("1"), "new1");
}


TEST_F(ActionTest, copy_overwrite) {
  data.targets = files_list1;
  data.type = Action_type::copy;
  data.destination = dir.absoluteFilePath("move_dest3");
  Action a(0, fs_engine, data);
  QSignalSpy spy1(&a, SIGNAL(question(Question_data)));
  a.run();
  EXPECT_EQ(1, spy1.count());

  QDir dir1 = dir;
  EXPECT_TRUE(dir1.cd("move_dest3"));
  a.question_answered(Error_reaction::overwrite);
  EXPECT_TRUE(dir1.exists("3"));
  check_file_contents(dir1.absoluteFilePath("1"), "new1");
  check_file_contents(dir1.absoluteFilePath("2"), "new2");
  check_file_contents(dir1.absoluteFilePath("3"), "new3");
}

