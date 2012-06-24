#include "Main_window.h"
#include "ui_Main_window.h"
#include <QKeyEvent>
#include <QSystemLocale>
#include <QDebug>
#include <QShortcut>
#include "File_info.h"
#include "Tasks_thread.h"
#include <QToolButton>
#include <QLabel>
#include "Path_button.h"

#include "qt_gtk.h"
#include "gio/gio.h"


Main_window::Main_window(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Main_window),
  hotkeys(this)
{
  init_gio_connects();
  fetch_gio_mounts();

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

  hotkeys.add("Switch between panes",  "Tab",       this, SLOT(switch_active_pane()));
  hotkeys.add("Parent directory",      "Backspace", ui->action_go_parent_directory);
  hotkeys.add("Focus address bar",     "Ctrl+L",    this, SLOT(focus_address_line()));
  hotkeys.add("Go to filesystem root", "Ctrl+2",    ui->action_go_root);
  hotkeys.add("Go to places",          "Ctrl+1",    ui->action_go_places);

  connect(ui->action_go_parent_directory, SIGNAL(triggered()),
          this, SLOT(go_parent()));


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

QList<gio::Mount *> Main_window::get_gio_mounts() {
  return mounts;
}

void Main_window::init_gio_connects() {
  //qRegisterMetaType< QList<gio::Volume> >("QList<gio::Volume>");
  //qRegisterMetaType< QList<gio::Mount> >("QList<gio::Mount>");

  int argc = QApplication::argc();
  char** argv = QApplication::argv();
  gtk_init(&argc, &argv);
  volume_monitor = g_volume_monitor_get();
  g_signal_connect(volume_monitor, "volume-added",
                   G_CALLBACK(gio_mount_changed), this);
  g_signal_connect(volume_monitor, "volume-changed",
                   G_CALLBACK(gio_mount_changed), this);
  g_signal_connect(volume_monitor, "volume-removed",
                   G_CALLBACK(gio_mount_changed), this);
  g_signal_connect(volume_monitor, "mount-added",
                   G_CALLBACK(gio_mount_changed), this);
  g_signal_connect(volume_monitor, "mount-changed",
                   G_CALLBACK(gio_mount_changed), this);
  g_signal_connect(volume_monitor, "mount-removed",
                   G_CALLBACK(gio_mount_changed), this);
}

void Main_window::fetch_gio_mounts() {
  foreach (gio::Mount* m, mounts) delete m;
  foreach (gio::Volume* m, volumes) delete m;
  volumes.clear();
  mounts.clear();

  GList* list = g_volume_monitor_get_volumes(volume_monitor);
  for(; list; list = list->next) {
    GVolume* volume = static_cast<GVolume*>(list->data);
    volumes << new gio::Volume(volume);
    g_object_unref(volume);
  }
  g_free(list);

  list = g_volume_monitor_get_mounts(volume_monitor);
  for(; list; list = list->next) {
    GMount* mount = static_cast<GMount*>(list->data);
    mounts << new gio::Mount(mount);
    g_object_unref(mount);
  }
  g_free(list);
  emit gio_mounts_changed();
}

void Main_window::gio_mount_changed(GVolumeMonitor*, GDrive*, Main_window* _this) {
  _this->fetch_gio_mounts();
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




void Main_window::refresh_path_toolbar() {
  ui->path_toolbar->clear();
  QList<File_info> path_items;
  QString real_path = active_pane->get_uri();
  if (real_path.endsWith("/")) real_path = real_path.left(real_path.length() - 1);
  if (real_path == "places") {
    File_info file_info;
    file_info.uri = "places";
    file_info.caption = tr("Places");
    path_items << file_info;
  } else {
    QString headless_path;
    bool root_found = false;
    foreach(gio::Mount* mount, mounts) {
      QString uri_prefix = mount->uri;
      if (!uri_prefix.isEmpty() && real_path.startsWith(uri_prefix)) {
        File_info file_info;
        file_info.uri = uri_prefix;
        file_info.caption = mount->name;
        path_items << file_info;
        if (!uri_prefix.endsWith("/")) {
          uri_prefix += "/";
        }
        headless_path = real_path.mid(uri_prefix.count());
        root_found = true;
      }
    }
    if (!root_found && real_path.startsWith("/")) {
      File_info file_info;
      file_info.uri = "/";
      file_info.caption = tr("Root");
      path_items << file_info;
      headless_path = real_path.mid(1);
      root_found = true;
    }
    if (!root_found) {
      return;
    }
    if (!headless_path.isEmpty()) {
      QStringList parts = headless_path.split("/");
      for(int i = 0; i < parts.count(); i++) {
        File_info file_info;
        file_info.caption = parts[i];
        file_info.uri = path_items.last().uri;
        if (!file_info.uri.endsWith("/")) {
          file_info.uri += "/";
        }
        file_info.uri += parts[i];
        path_items << file_info;
      }
    } /*else {
      File_info file_info;
      file_info.caption = tr("Invalid location");
      file_info.uri = "places";
      path_items << file_info;
    }*/

    for(int i = 0; i < old_path_items.count(); i++) {
      if (i < path_items.count() &&
          old_path_items[i].uri != path_items[i].uri) break;
      if (i >= path_items.count()) {
        path_items << old_path_items[i];
      }
    }
    old_path_items = path_items;
  }

  for(int i = 0; i < path_items.count(); i++) {
    QString caption = path_items[i].caption;
    if (i < path_items.count() - 1) {
      caption += tr(" ‣");
    }
    Path_button* b = new Path_button(this, caption, path_items[i].uri);
    if (i == 0) {
      b->set_go_parent_visible(true);
    }
    b->setChecked(path_items[i].uri == real_path);
    connect(b, SIGNAL(go_to(QString)), this, SLOT(go_to(QString)));
    ui->path_toolbar->addWidget(b);
  }
}

void Main_window::go_to(QString uri) {
  active_pane->set_uri(uri);
}

void Main_window::on_action_go_places_triggered() {
  go_to("places");
}

void Main_window::on_action_go_root_triggered() {
  go_to("/");
}
