#include "Main_window.h"
#include "ui_Main_window.h"

#include "Current_queue_question.h"
#include <QUrl>
#include <QDesktopServices>
#include <QThreadPool>
#include <QKeyEvent>
#include <QSystemLocale>
#include <QDebug>
#include <QShortcut>
#include "File_info.h"
#include <QToolButton>
#include <QLabel>
#include "Path_button.h"
#include <QSpacerItem>
#include "Special_uri.h"
#include <QProcess>
#include "Settings_dialog.h"
#include "qt_gtk.h"
#include <QTextCodec>
#include "Action_queue.h"
#include "Tasks_model.h"
#include <QMessageBox>
#include "Directory_watcher.h"
#include "Action_answerer.h"

Main_window::Main_window(QWidget *parent) :
  QMainWindow(parent),
  bookmarks(QDir::home().absoluteFilePath(".gtk-bookmarks"), Bookmarks_file_parser::format_gtk),
  user_dirs(QDir::home().absoluteFilePath(".config/user-dirs.dirs"), Bookmarks_file_parser::format_xdg),
  ui(new Ui::Main_window),
  hotkeys(this),
  current_queue(0)
{
  QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));

  watcher = new Directory_watcher();
  watcher_thread = new QThread();
  watcher_thread->start();
  watcher->moveToThread(watcher_thread);

  init_gio_connects();
  fetch_gio_mounts();


  QThreadPool::globalInstance()->setMaxThreadCount(5);

  qRegisterMetaType<File_info_list>("File_info_list");
  qRegisterMetaType<Error_type::Enum>("Error_type::Enum");
  ui->setupUi(this);
  //ui->tasks_table->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  //ui->tasks_table->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  ui->left_pane->set_main_window(this);
  ui->right_pane->set_main_window(this);

  tasks_model = new Tasks_model(this);
  ui->tasks_table->hide();
  ui->tasks_table->setModel(tasks_model);
  connect(tasks_model, SIGNAL(layoutChanged()), this, SLOT(resize_tasks_table()));
  foreach(QAction* a, QList<QAction*>() << ui->action_recursive_fetch_auto
          << ui->action_recursive_fetch_off << ui->action_recursive_fetch_on) {
    connect(a, SIGNAL(triggered()), this, SLOT(slot_actions_recursive_fetch_triggered()));
  }

  ui->current_queue_notice->hide();

  //QLocale::Language language = QLocale::system().language();
  //qDebug() << "language: " << QLocale::languageToString(language);

  QApplication::setApplicationName("ridual");
  QApplication::setOrganizationName("ridual");

  QSettings s;
  s.beginGroup("left_pane");
  ui->left_pane->load_state(&s);
  s.endGroup();
  s.beginGroup("right_pane");
  ui->right_pane->load_state(&s);
  s.endGroup();

  int option = s.value("recursive_fetch_option", static_cast<int>(recursive_fetch_auto)).toInt();
  switch(static_cast<Recursive_fetch_option>(option)) {
    case recursive_fetch_on: ui->action_recursive_fetch_on->setChecked(true); break;
    case recursive_fetch_off: ui->action_recursive_fetch_off->setChecked(true); break;
    case recursive_fetch_auto: ui->action_recursive_fetch_auto->setChecked(true); break;
  }



  //todo: load columns
  Columns columns = Columns::get_default();
  ui->left_pane->set_columns(columns);
  ui->right_pane->set_columns(columns);

  restoreState(s.value("main_window/state").toByteArray());
  restoreGeometry(s.value("main_window/geometry").toByteArray());

  save_settings_timer.setInterval(10000); //ms
  connect(&save_settings_timer, SIGNAL(timeout()), this, SLOT(save_settings()));
  save_settings_timer.start();

  set_active_pane(ui->left_pane);

  hotkeys.add("switch_active_pane",
              tr("Switch between panes"),
              "Tab",
              this, SLOT(switch_active_pane())
              );

  hotkeys.add("focus_question",
              tr("Focus question dialog"),
              "Ctrl+E",
              this, SLOT(slot_focus_question())
              );


  //ui->menu_file->addAction(test);

  hotkeys.add("Focus address bar",
              tr("Focus address bar"),
              "Ctrl+L",
              this, SLOT(focus_address_line())
              );

  hotkeys.add("go_parent_directory", tr("Go to parent directory"), ui->action_go_parent_directory);
  hotkeys.add("go_root",             tr("Go to filesystem root"),  ui->action_go_root);
  hotkeys.add("go_places",           tr("Go to places"),           ui->action_go_places);
  hotkeys.add("execute",  ui->action_execute);
  hotkeys.add("view",     ui->action_view);
  hotkeys.add("edit",     ui->action_edit);
  hotkeys.add("copy",     ui->action_copy);
  hotkeys.add("move",     ui->action_move);
  hotkeys.add("choose_queue", tr("Choose queue"), ui->action_queue_choose);

  connect(ui->action_go_parent_directory, SIGNAL(triggered()),
          this, SLOT(go_parent()));


  connect(ui->left_pane,  SIGNAL(uri_changed()), this, SLOT(refresh_path_toolbar()));
  connect(ui->right_pane, SIGNAL(uri_changed()), this, SLOT(refresh_path_toolbar()));
  connect(this, SIGNAL(active_pane_changed()),   this, SLOT(refresh_path_toolbar()));
  refresh_path_toolbar();

  connect(ui->left_pane,  SIGNAL(selection_changed()),   this, SIGNAL(selection_changed()));
  connect(ui->right_pane, SIGNAL(selection_changed()),   this, SIGNAL(selection_changed()));
  connect(this,           SIGNAL(active_pane_changed()), this, SIGNAL(selection_changed()));
  connect(this, SIGNAL(selection_changed()), this, SLOT(slot_selection_changed()));


}

Main_window::~Main_window() {
  foreach(gulong id, gio_connects) {
    g_signal_handler_disconnect (volume_monitor, id);
  }
  gio_connects.clear();

  save_settings();
  //tasks_thread->interrupt();
  watcher_thread->quit();
  watcher_thread->wait();
  //delete tasks_thread;
  delete ui;
}

void Main_window::set_active_pane(Pane *pane) {
  if (active_pane == pane) return;
  if (pane != ui->left_pane && pane != ui->right_pane) {
    qWarning("Main_window::set_active_pane called with wrong argument");
    return;
  }
  active_pane = pane;
  emit active_pane_changed();
}

QList<Gio_mount> Main_window::get_gio_mounts() {
  static QMutex mutex;
  QMutexLocker locker(&mutex);
  return mounts;
}

App_info_list Main_window::get_apps(const QString &mime_type) {
  //qDebug() << "Main_window::get_apps";
  App_info_list r;
  GList* list = g_app_info_get_all_for_type(mime_type.toLocal8Bit());
  if (!list) return r;
  for(; list; list = list->next) {
    r << App_info(static_cast<GAppInfo*>(list->data));
    //qDebug() << "found app:" << list->data;
  }

  QStringList was;
  for(int i = 0; i < r.count(); i++) {

    QString command = r[i].command();
    if (was.contains(command)) {
      r.removeAt(i);
      i--;
    } else {
      was << command;
    }
  }

  GAppInfo* default_app = g_app_info_get_default_for_type(mime_type.toLocal8Bit(), 0);
  r.default_app = App_info(default_app);
  return r;
}

App_info Main_window::get_default_app(const QString &mime_type) {
  GAppInfo* default_app = g_app_info_get_default_for_type(mime_type.toLocal8Bit(), 0);
  return App_info(default_app);
}

Pane *Main_window::get_destination_pane() {
  return ui->left_pane == active_pane? ui->right_pane: ui->left_pane;
}

Action_queue *Main_window::create_queue() {
  QSet<int> ids;
  foreach(Action_queue* q, get_queues()) ids << q->get_id();
  int id = 1;
  while(ids.contains(id)) id++;
  Action_queue* q = new Action_queue(id);
  q->setParent(this);
  connect(q, SIGNAL(destroyed(QObject*)), this, SLOT(slot_queue_destroyed(QObject*)));
  return q;
}

QList<Action_queue*> Main_window::get_queues() {
  return findChildren<Action_queue*>();
}

Action_queue *Main_window::get_current_queue() {
  if (current_queue) return current_queue;
  return create_queue();
}

void Main_window::set_current_queue(Action_queue* queue) {
  current_queue = queue;
  ui->current_queue_notice->setVisible(current_queue != 0);
  if (current_queue) {
    ui->current_queue_id->setText(QString("%1").arg(current_queue->get_id()));
  }
}

void Main_window::create_action(Action_data data) {
  data.recursive_fetch_option = get_recursive_fetch_option();
  data.targets = active_pane->get_selected_files();
  data.destination = get_destination_pane()->get_uri();
  Action* a = new Action(this, data);
  Action_queue* q = get_current_queue();
  q->add_action(a);
  connect(a, SIGNAL(question(Question_data)), this, SLOT(slot_action_question(Question_data)));
  emit action_added(a);
}

Recursive_fetch_option Main_window::get_recursive_fetch_option() {
  if (ui->action_recursive_fetch_auto->isChecked()) return recursive_fetch_auto;
  if (ui->action_recursive_fetch_on->isChecked()) return recursive_fetch_on;
  if (ui->action_recursive_fetch_off->isChecked()) return recursive_fetch_off;
  qWarning("Main_window::get_recursive_fetch_option failed");
  return recursive_fetch_auto;
}

void Main_window::add_question(Question_widget *question) {
  ui->questions_layout->insertWidget(0, question);
}

void Main_window::switch_focus_question(Question_widget *target, int direction) {
  for(int i = ui->questions_layout->indexOf(target) + direction;
      i >= 0 && i < ui->questions_layout->count();
      i += direction) {
    if (ui->questions_layout->itemAt(i)->widget() != 0) {
      dynamic_cast<Question_widget*>(ui->questions_layout->itemAt(i)->widget())->start_editor();
      return;
    }
  }
}



void Main_window::init_gio_connects() {
  int argc = QApplication::argc();
  char** argv = QApplication::argv();
  gtk_init(&argc, &argv);
  volume_monitor = g_volume_monitor_get();
  gio_connects << g_signal_connect(volume_monitor, "volume-added",
                   G_CALLBACK(gio_mount_changed), this);
  gio_connects << g_signal_connect(volume_monitor, "volume-changed",
                   G_CALLBACK(gio_mount_changed), this);
  gio_connects << g_signal_connect(volume_monitor, "volume-removed",
                   G_CALLBACK(gio_mount_changed), this);
  gio_connects << g_signal_connect(volume_monitor, "mount-added",
                   G_CALLBACK(gio_mount_changed), this);
  gio_connects << g_signal_connect(volume_monitor, "mount-changed",
                   G_CALLBACK(gio_mount_changed), this);
  gio_connects << g_signal_connect(volume_monitor, "mount-removed",
                   G_CALLBACK(gio_mount_changed), this);
}

void Main_window::fetch_gio_mounts() {
  //foreach (gio::Mount* m, mounts) delete m;
  foreach (Gio_volume* m, volumes) delete m;
  volumes.clear();
  mounts.clear();

  GList* list = g_volume_monitor_get_volumes(volume_monitor);
  for(; list; list = list->next) {
    GVolume* volume = static_cast<GVolume*>(list->data);
    volumes << new Gio_volume(volume);
    g_object_unref(volume);
  }
  g_free(list);

  list = g_volume_monitor_get_mounts(volume_monitor);
  for(; list; list = list->next) {
    GMount* mount = static_cast<GMount*>(list->data);
    mounts << Gio_mount(mount);
    g_object_unref(mount);
  }
  g_free(list);
  emit gio_mounts_changed();
}

void Main_window::gio_mount_changed(GVolumeMonitor*, GDrive*, Main_window* _this) {
  _this->fetch_gio_mounts();
}

void Main_window::resizeEvent(QResizeEvent *) {
  ui->path_widget->refresh();
}

void Main_window::view_or_edit_selected(bool edit) {
  File_info_list list = active_pane->get_selected_files();
  foreach(File_info f, list) {
    QProcess* p = new QProcess(this);
    p->setWorkingDirectory(QFileInfo(f.full_path).dir().absolutePath());
    QString command = QSettings().value(edit? "edit_command": "view_command", "gedit %U").toString();
    command = command.replace("%U", QString("\"%1\"").arg(f.uri));
    command = command.replace("%F", QString("\"%1\"").arg(f.full_path));
    //todo: correct shell escaping
    p->start(command);
    //todo: catch errors
    //todo: run in tasks thread
  }

}


bool Main_window::eventFilter(QObject *object, QEvent *event) {
  return false;
  Q_UNUSED(object);
  Q_UNUSED(event);
}


void Main_window::on_action_about_triggered() {
  QDesktopServices::openUrl(QUrl("https://github.com/Riateche/ridual"));
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
  s.setValue("recursive_fetch_option", static_cast<int>(get_recursive_fetch_option()));
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
  File_info info = active_pane->get_current_file();
  if (info.is_folder()) {
    active_pane->set_uri(info.uri);
    return;
  }
  File_info_list files = active_pane->get_selected_files();
  QMap<QString, QStringList> types;
  foreach (File_info i, files) {
    if (i.is_file && !i.uri.isEmpty() && !i.mime_type.isEmpty()) {
//      types[i.mime_type] << i.uri;
      types[i.mime_type] << i.full_path;
    }
  }
  foreach (QString mime_type, types.keys()) {
    get_default_app(mime_type).launch(types[mime_type]);
  }
}

void Main_window::focus_address_line() {
  active_pane->focus_address_line();
}




void Main_window::refresh_path_toolbar() {
  File_info_list path_items;
  QString real_path = active_pane->get_uri();
  QString headless_path;
  bool root_found = false;
  QList<Special_uri> special_uris;
  special_uris << Special_uri(Special_uri::mounts) <<
                  Special_uri(Special_uri::bookmarks) <<
                  Special_uri(Special_uri::userdirs);
  foreach(Special_uri u, special_uris) {
    if (real_path.startsWith(u.uri())) {
      File_info f;
      f.uri = u.uri();
      f.name = u.caption();
      path_items << f;
      break;
    }
  }




  foreach(Gio_mount mount, mounts) {
    QString uri_prefix = mount.uri;
    if (!uri_prefix.isEmpty() && real_path.startsWith(uri_prefix)) {
      File_info file_info;
      file_info.uri = uri_prefix;
      file_info.name = mount.name;
      path_items << file_info;
      if (!uri_prefix.endsWith("/")) {
        uri_prefix += "/";
      }
      headless_path = real_path.mid(uri_prefix.count());
      root_found = true;
    }
  }
  QString home = QDir::homePath();
  if (!root_found && real_path.startsWith(home)) {
    File_info file_info;
    file_info.uri = home;
    file_info.name = tr("Home");
    path_items << file_info;
    if (!home.endsWith("/")) home += "/";
    headless_path = real_path.mid(home.length());
    root_found = true;
  }
  if (!root_found && real_path.startsWith("/")) {
    File_info file_info;
    file_info.uri = "/";
    file_info.name = tr("Root");
    path_items << file_info;
    headless_path = real_path.mid(1);
    root_found = true;
  }
  if (root_found && !headless_path.isEmpty()) {
    QStringList parts = headless_path.split("/");
    for(int i = 0; i < parts.count(); i++) {
      File_info file_info;
      file_info.name = parts[i];
      file_info.uri = path_items.last().uri;
      if (!file_info.uri.endsWith("/")) {
        file_info.uri += "/";
      }
      file_info.uri += parts[i];
      path_items << file_info;
    }
  }

  for(int i = 0; i < old_path_items.count(); i++) {
    if (i < path_items.count() &&
        old_path_items[i].uri != path_items[i].uri) break;
    if (i >= path_items.count()) {
      path_items << old_path_items[i];
    }
  }
  old_path_items = path_items;

  File_info places;
  places.name = Special_uri(Special_uri::places).caption();
  places.uri = Special_uri(Special_uri::places).uri();
  path_items.prepend(places);
  QList<Path_button*> buttons;
  for(int i = 0; i < path_items.count(); i++) {
    QString caption = path_items[i].name;
    if (i < path_items.count() - 1) {
      caption += tr(" ‣");
    }
    Path_button* b = new Path_button(this, caption, path_items[i].uri);
    b->setChecked(path_items[i].uri == real_path);
    connect(b, SIGNAL(go_to(QString)), this, SLOT(go_to(QString)));
    buttons << b;
  }
  ui->path_widget->set_buttons(buttons);
}

void Main_window::go_to(QString uri) {
  refresh_path_toolbar();
  active_pane->set_uri(uri);
}

void Main_window::slot_selection_changed() {
  File_info_list list = active_pane->get_selected_files();
  bool can_execute = false;
  bool can_edit = false;
  foreach(File_info f, list) {
    if (f.is_executable) {
      can_execute = true;
    }
    if (f.is_file) {
      can_edit = true;
    }
  }
  ui->action_execute->setEnabled(can_execute);
  ui->action_view->setEnabled(can_edit);
  ui->action_edit->setEnabled(can_edit);
}

void Main_window::slot_actions_recursive_fetch_triggered() {
  foreach(QAction* a, QList<QAction*>() << ui->action_recursive_fetch_auto
          << ui->action_recursive_fetch_off << ui->action_recursive_fetch_on) {
    if (a == sender()) {
      a->setChecked(true);
    } else {
      a->setChecked(false);
    }
  }
}

void Main_window::fatal_error(QString message) {
  QMessageBox::critical(0, "", message);
}

void Main_window::resize_tasks_table() {
  ui->tasks_table->setVisible(tasks_model->rowCount() > 0);
  if (tasks_model->rowCount() > 0) {
    int h = ui->tasks_table->horizontalHeader()->height() + 3 +
        ui->tasks_table->rowHeight(0) * qMin(5, tasks_model->rowCount());
    ui->tasks_table->setFixedHeight(h);
  }
}

void Main_window::on_action_queue_choose_triggered() {
  Current_queue_question* q = new Current_queue_question(this);
  q->start_editor();
}


void Main_window::on_action_go_places_triggered() {
  go_to(Special_uri(Special_uri::places).uri());
}

void Main_window::on_action_go_root_triggered() {
  go_to("/");
}

void Main_window::on_action_refresh_triggered(){
  active_pane->set_uri(active_pane->get_uri());
}

void Main_window::on_action_execute_triggered() {
  File_info_list list = active_pane->get_selected_files();
  foreach(File_info f, list) {
    QProcess* p = new QProcess(this);
    p->setWorkingDirectory(QFileInfo(f.full_path).dir().absolutePath());
    p->start(f.full_path);
    //todo: catch errors
    //todo: run in tasks thread
  }
}

void Main_window::on_action_general_settings_triggered() {
  (new Settings_dialog(this))->show();
}

void Main_window::on_action_view_triggered() {
  view_or_edit_selected(false);
}

void Main_window::on_action_edit_triggered() {
  view_or_edit_selected(true);

}

void Main_window::on_action_copy_triggered() {
  Action_data data;
  data.type = Action_type::copy;
  create_action(data);
}

void Main_window::slot_queue_destroyed(QObject *object) {
  if (object == current_queue) {
    set_current_queue(0);
  }
}

void Main_window::slot_action_question(Question_data data) {
  new Action_answerer(this, data);
}

void Main_window::slot_focus_question() {
  for(int i = 0; i < ui->questions_layout->count(); i++) {
    QWidget* w = ui->questions_layout->itemAt(i)->widget();
    if (w != 0) {
      dynamic_cast<Question_widget*>(w)->start_editor();
      return;
    }
  }
}
