#include "gtest/gtest.h"
#include "Main_window.h"
#include "ui_Main_window.h"
#include <QSignalSpy>
#include "Core.h"

TEST(Main_window, active_pane) {
  Core core(true);
  Main_window* mw = core.get_main_window();
  EXPECT_TRUE(mw->get_active_pane() == mw->get_ui()->left_pane ||
              mw->get_active_pane() == mw->get_ui()->right_pane);
  mw->set_active_pane(mw->get_ui()->left_pane);
  EXPECT_TRUE(mw->get_active_pane() == mw->get_ui()->left_pane);

  QSignalSpy spy(mw, SIGNAL(active_pane_changed()));

  mw->set_active_pane(mw->get_ui()->right_pane);
  EXPECT_TRUE(mw->get_active_pane() == mw->get_ui()->right_pane);
  EXPECT_EQ(1, spy.count());

  mw->set_active_pane(0);
  EXPECT_TRUE(mw->get_active_pane() == mw->get_ui()->right_pane);
  EXPECT_EQ(1, spy.count());

  mw->set_active_pane(mw->get_ui()->right_pane);
  EXPECT_TRUE(mw->get_active_pane() == mw->get_ui()->right_pane);
  EXPECT_EQ(1, spy.count());

  mw->set_active_pane(mw->get_ui()->left_pane);
  EXPECT_TRUE(mw->get_active_pane() == mw->get_ui()->left_pane);
  EXPECT_EQ(2, spy.count());

}
