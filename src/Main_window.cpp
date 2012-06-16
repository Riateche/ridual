#include "Main_window.h"
#include "ui_Main_window.h"
#include <QKeyEvent>
#include <QSystemLocale>
#include <QDebug>
#include "Hotkey_editor.h"
#include <QShortcut>


Main_window::Main_window(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Main_window)
{
  qRegisterMetaType<QFileInfoList>("QFileInfoList");
  ui->setupUi(this);
  ui->left_pane->set_main_window(this);
  ui->right_pane->set_main_window(this);

  //QLocale::Language language = QLocale::system().language();
  //qDebug() << "language: " << QLocale::languageToString(language);

  QApplication::setApplicationName("ridual");
  QApplication::setOrganizationName("riateche");

  QSettings s;
  s.beginGroup("left_pane");
  ui->left_pane->load_state(&s);
  s.endGroup();
  s.beginGroup("right_pane");
  ui->right_pane->load_state(&s);
  s.endGroup();

  save_settings_timer.setInterval(10000); //ms
  connect(&save_settings_timer, SIGNAL(timeout()), this, SLOT(save_settings()));
  save_settings_timer.start();

  set_active_pane(ui->left_pane);

  QShortcut* shortcut = new QShortcut(QKeySequence(Hotkey_editor::get_hotkey("Switch between panes")), this);
  shortcut->setContext(Qt::ApplicationShortcut);
  connect(shortcut, SIGNAL(activated()), this, SLOT(switch_active_pane()));

}

Main_window::~Main_window() {
  save_settings();
  delete ui;
}

void Main_window::set_active_pane(Pane *pane) {
  if (active_pane == pane) return;
  active_pane = pane;
  emit active_pane_changed();
}

void Main_window::switch_active_pane() {
  set_active_pane(ui->left_pane == active_pane? ui->right_pane: ui->left_pane);

}

void Main_window::save_settings() {
  QSettings s;
  s.beginGroup("left_pane");
  ui->left_pane->save_state(&s);
  s.endGroup();
  s.beginGroup("right_pane");
  ui->right_pane->save_state(&s);
  s.endGroup();
}


void Main_window::on_action_hotkeys_triggered() {
  QList<Hotkey> hotkeys;
  hotkeys << Hotkey("Parent directory", "Backspace") <<
             Hotkey("Switch between panes", "Tab");
  Hotkey_editor* e = new Hotkey_editor(hotkeys);
  e->show();
}
