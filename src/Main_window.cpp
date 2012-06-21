#include "Main_window.h"
#include "ui_Main_window.h"
#include <QKeyEvent>
#include <QSystemLocale>
#include <QDebug>
#include <QShortcut>
#include "File_info.h"
#include "gio/Gio_main.h"
#include "Tasks_thread.h"
#include <QToolButton>
#include <QLabel>
#include "Path_button.h"

Main_window::Main_window(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Main_window),
  hotkeys(this)
{
  tasks_thread = new Tasks_thread(this);
  connect(this, SIGNAL(signal_add_task(Task)), tasks_thread, SLOT(add_task(Task)));
  tasks_thread->start();
  qRegisterMetaType< QList<File_info> >("QList<File_info>");
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

  restoreState(s.value("main_window/state").toByteArray());
  restoreGeometry(s.value("main_window/geometry").toByteArray());

  save_settings_timer.setInterval(10000); //ms
  connect(&save_settings_timer, SIGNAL(timeout()), this, SLOT(save_settings()));
  save_settings_timer.start();

  set_active_pane(ui->left_pane);

  hotkeys.add("Switch between panes", "Tab",      this, SLOT(switch_active_pane()));
  hotkeys.add("Parent directory",     "Alt+Up",   this, SLOT(go_parent()));
  hotkeys.add("Focus address bar",    "Ctrl+L",   this, SLOT(focus_address_line()));


  Gio_main* gio = new Gio_main();
  connect(gio,    SIGNAL(list_changed(QList<gio::Volume>,QList<gio::Mount>)),
          this, SLOT(gio_list_changed(QList<gio::Volume>,QList<gio::Mount>)));
  gio->start();

  connect(ui->left_pane,  SIGNAL(uri_changed()), this, SLOT(refresh_path_toolbar()));
  connect(ui->right_pane, SIGNAL(uri_changed()), this, SLOT(refresh_path_toolbar()));
  connect(this, SIGNAL(active_pane_changed()),   this, SLOT(refresh_path_toolbar()));
  refresh_path_toolbar();

  //qDebug() << "styles" << QStyleFactory::keys();
}

Main_window::~Main_window() {
  save_settings();
  tasks_thread->interrupt();
  delete tasks_thread;
  delete ui;
}

void Main_window::set_active_pane(Pane *pane) {
  if (active_pane == pane) return;
  active_pane = pane;
  emit active_pane_changed();
}

void Main_window::add_task(Task task) {
  emit signal_add_task(task);
}

QList<gio::Mount> Main_window::get_gio_mounts() {
  return mounts;
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
  s.setValue("main_window/state", saveState());
  s.setValue("main_window/geometry", saveGeometry());
}


void Main_window::on_action_hotkeys_triggered() {
  hotkeys.open_editor();
/*  QList<Hotkey> hotkeys;
  hotkeys << Hotkey("Parent directory", "Backspace") <<
             Hotkey("Switch between panes", "Tab");
  Hotkey_editor* e = new Hotkey_editor(hotkeys);
  e->show(); */
}

void Main_window::go_parent() {
  active_pane->go_parent();
}

void Main_window::open_current() {
  active_pane->open_current();
}

void Main_window::focus_address_line() {
  active_pane->focus_address_line();
}

void Main_window::gio_list_changed(QList<gio::Volume> p_volumes, QList<gio::Mount> p_mounts) {
  volumes = p_volumes;
  mounts = p_mounts;
  emit gio_mounts_changed();
}



void Main_window::refresh_path_toolbar() {
  ui->path_toolbar->clear();
  QString path = active_pane->get_uri();
  if (path.startsWith("/")) {
    QStringList parts = path.split("/");
    if (parts.last().isEmpty()) parts.removeLast(); //happens for path="/"
    parts[0] = "/"; //parts[0] was empty before this
    for(int i = 0; i < parts.count(); i++) {
      QString caption = parts[i];
      if (i > 0 && i < parts.count() - 1) {
        caption += "/";
      }
      QString uri = QString("/") + QStringList(parts.mid(1, i)).join("/");
      Path_button* b = new Path_button(this, caption, uri);
      ui->path_toolbar->addWidget(b);
    }
  }
}
