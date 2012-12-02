#include "gtest/gtest.h"
#include "Action.h"
#include <QDir>
#include "gio/Gio_file_system_engine.h"
#include "Mount_manager.h"
#include <QProcess>
#include "debug_utils.h"

TEST(Action, copy) {
  QDir dir = env_dir("action");
  Action_data data;
  data.targets << dir.absoluteFilePath("dir");
  EXPECT_TRUE(dir.mkdir("destination"));
  data.destination = dir.absoluteFilePath("destination");
  data.type = Action_type::copy;
  Action a(data);
  Mount_manager mount_manager;
  Gio_file_system_engine fs_engine(&mount_manager);
  a.set_fs_engine(&fs_engine);
  a.run();

  QProcess p;
  p.setWorkingDirectory(dir.absolutePath());
  p.start("diff dir destination/dir");
  p.waitForFinished();
  EXPECT_EQ(0, p.exitCode());

}
