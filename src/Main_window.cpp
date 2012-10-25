#include "Main_window.h"
#include "ui_Main_window.h"

#include "Actions_manager.h"
#include <QClipboard>
#include <QMimeData>
#include <QResource>
#include <QTextBrowser>
#include "Directory.h"
#include "Action_state_widget.h"
#include "Message_widget.h"
#include "Mount_manager.h"
#include "Core.h"
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

#include <QTextCodec>
#include "Action_queue.h"
#include <QMessageBox>
#include "Action_answerer.h"

Main_window::Main_window(Core* c) :
  QMainWindow(0)
, Core_ally(c)
, ui(new Ui::Main_window)
, active_pane(0)
, hotkeys(this)
, current_queue(0)
{

}

Main_window::~Main_window() {

  save_settings();
  //tasks_thread->interrupt();
  //delete tasks_thread;
  delete ui;
}


void Main_window::init() {
  setAttribute(Qt::WA_DeleteOnClose);

  QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));

  QThreadPool::globalInstance()->setMaxThreadCount(5);

  ui->setupUi(this);

  foreach(QAction* a, QList<QAction*>() << ui->action_recursive_fetch_auto
          << ui->action_recursive_fetch_off << ui->action_recursive_fetch_on) {
    connect(a, SIGNAL(triggered()), this, SLOT(actions_recursive_fetch_triggered()));
  }

  ui->current_queue_notice->hide();

  //QLocale::Language language = QLocale::system().language();
  //qDebug() << "language: " << QLocale::languageToString(language);

  QApplication::setApplicationName("ridual");
  QApplication::setOrganizationName("ridual");

  QSettings s;

  QVariant v = s.value("columns");
  columns = v.isValid()? Columns::deserialize(v): Columns::get_default();
  emit columns_changed(columns);

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



  save_settings_timer.setInterval(10000); //ms
  connect(&save_settings_timer, SIGNAL(timeout()), this, SLOT(save_settings()));
  save_settings_timer.start();

  set_active_pane(ui->left_pane);

  init_hotkeys();

  connect(ui->action_go_parent_directory, SIGNAL(triggered()),
          this, SLOT(go_parent()));

  //connect(ui->left_pane,  SIGNAL(uri_changed()), this, SLOT(refresh_path_toolbar()));
  //connect(ui->right_pane, SIGNAL(uri_changed()), this, SLOT(refresh_path_toolbar()));
  //connect(this, SIGNAL(active_pane_changed()),   this, SLOT(refresh_path_toolbar()));
  //refresh_path_toolbar();

  connect(ui->left_pane,  SIGNAL(selection_changed()),   this, SIGNAL(selection_changed()));
  connect(ui->right_pane, SIGNAL(selection_changed()),   this, SIGNAL(selection_changed()));
  connect(this,           SIGNAL(active_pane_changed()), this, SIGNAL(selection_changed()));
  connect(this, SIGNAL(selection_changed()), this, SLOT(slot_selection_changed()));


  if (s.value("main_window/state").isValid()) {
    restoreState(s.value("main_window/state").toByteArray());
    restoreGeometry(s.value("main_window/geometry").toByteArray());
    ui->panes_splitter->restoreState(s.value("panes_spliter_state").toByteArray());
    show();
  } else {
    show();
    setWindowState(Qt::WindowMaximized);
    qDebug() << width() / 2;
    ui->panes_splitter->setSizes(QList<int>() << width() / 2 << width() / 2);
  }

  connect(core->get_actions_manager(), SIGNAL(queue_destroyed(Action_queue*)), this, SLOT(queue_destroyed(Action_queue*)));
  connect(core->get_actions_manager(), SIGNAL(action_added(Action*)), this, SLOT(action_added(Action*)));

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

void Main_window::set_columns(Columns v) {
  columns = v;
  QSettings s;
  s.setValue("columns", v.serialize());
  emit columns_changed(v);
}

QString Main_window::get_version() {
  QResource r(":/VERSION");
  return QString::fromUtf8(reinterpret_cast<const char*>(r.data()));
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
  if (active_pane->get_uri() == Special_uri(Special_uri::places).uri()) {
    show_message(tr("Invalid target for this operation."), Icon::error);
    return;
  }
  if (data.type != Action_type::remove) {
    if (data.destination == Special_uri(Special_uri::places).uri()) {
      show_message(tr("Can't use 'Places' special location as destination."), Icon::error);
      return;
    }
  }

  Action_queue* q;
  if (current_queue) {
    q = current_queue;
  } else {
    q = core->get_actions_manager()->create_queue();
  }
  q->add_action(new Action(data));
}

Recursive_fetch_option Main_window::get_recursive_fetch_option() {
  if (ui->action_recursive_fetch_auto->isChecked()) return recursive_fetch_auto;
  if (ui->action_recursive_fetch_on->isChecked()) return recursive_fetch_on;
  if (ui->action_recursive_fetch_off->isChecked()) return recursive_fetch_off;
  qWarning("Main_window::get_recursive_fetch_option failed");
  return recursive_fetch_auto;
}

void Main_window::add_question(Question_widget *question) {
  question_widgets.prepend(question);
  connect(question, SIGNAL(destroyed(QObject*)), this, SLOT(question_widget_destroyed(QObject*)));
  ui->questions_layout->insertWidget(0, question);
}

void Main_window::switch_focus_question(Question_widget *target, int direction) {
  int i = question_widgets.indexOf(target) + direction;
  if (i >= 0 && i < question_widgets.count()) {
    question_widgets[i]->start_editor();
  }
}



void Main_window::view_or_edit_selected(bool edit) {
  File_info_list list = active_pane->get_selected_files();
  foreach(File_info f, list) {
    QProcess* p = new QProcess(this);
    p->setWorkingDirectory(Directory::find_real_path(Directory::get_parent_uri(f.uri), core));
    QString command = QSettings().value(edit? "edit_command": "view_command", "gedit %U").toString();
    command = command.replace("%U", QString("\"%1\"").arg(f.uri));
    command = command.replace("%F", QString("\"%1\"").arg(f.path));
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

void Main_window::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    for(int i = 0; i < ui->questions_layout->count(); i++) {
      QWidget* w = ui->questions_layout->itemAt(i)->widget();
      if (dynamic_cast<Message_widget*>(w)) {
        delete w;
        i--;
      }
    }
  }
}

Action_data Main_window::get_auto_target_and_destination(Action_type::Enum action_type) {
  Action_data data;
  data.type = action_type;
  foreach(File_info fi, active_pane->get_selected_files()) {
    data.targets << fi.uri;
  }
  if (data.type != Action_type::remove) {
    data.destination = get_destination_pane()->get_uri();
  }
  return data;
}




void Main_window::on_action_about_triggered() {
  QTextBrowser* b = new QTextBrowser();
  b->setWindowTitle(tr("About Ridual"));
  b->setOpenExternalLinks(true);
  QResource r(":/resources/about.html");
  QString s = QString::fromUtf8(reinterpret_cast<const char*>(r.data()));
  s = s.replace("%v", get_version());
  b->setHtml(s);
  b->show();
}

void Main_window::on_action_remove_triggered() {
  Action_data data = get_auto_target_and_destination(Action_type::remove);
  create_action(data);
}

void Main_window::on_action_move_triggered() {
  Action_data data = get_auto_target_and_destination(Action_type::move);
  create_action(data);
}

void Main_window::switch_active_pane() {
  set_active_pane(ui->left_pane == active_pane? ui->right_pane: ui->left_pane);
}

void Main_window::show_message(QString message, Icon::Enum icon) {
  ui->questions_layout->insertWidget(0, new Message_widget(message, icon));
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
  s.setValue("panes_spliter_state", ui->panes_splitter->saveState());
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
  //if (info.uri.isEmpty()) return;
  if (info.is_folder) {
    active_pane->set_uri(info.uri);
    return;
  }
  File_info_list files = active_pane->get_selected_files();
  QMap<QString, QStringList> types;
  foreach (File_info i, files) {
    if (i.is_file() && !i.uri.isEmpty() && !i.mime_type.isEmpty()) {
//      types[i.mime_type] << i.uri;
      types[i.mime_type] << i.path;
    }
  }
  foreach (QString mime_type, types.keys()) {
    get_default_app(mime_type).launch(types[mime_type]);
  }
}

void Main_window::focus_address_line() {
  active_pane->focus_address_line();
}






void Main_window::go_to(QString uri) {
  //refresh_path_toolbar();
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
    if (f.is_file()) {
      can_edit = true;
    }
  }
  ui->action_execute->setEnabled(can_execute);
  ui->action_view->setEnabled(can_edit);
  ui->action_edit->setEnabled(can_edit);
}

void Main_window::actions_recursive_fetch_triggered() {
  foreach(QAction* a, QList<QAction*>() << ui->action_recursive_fetch_auto
          << ui->action_recursive_fetch_off << ui->action_recursive_fetch_on) {
    if (a == sender()) {
      a->setChecked(true);
    } else {
      a->setChecked(false);
    }
  }
}



void Main_window::on_action_queue_choose_triggered() {
  Current_queue_question* q = new Current_queue_question(this);
  q->start_editor();
}


void Main_window::on_action_go_places_triggered() {
  /*show_message("test info", Icon::info);
  show_message("test success", Icon::success);
  show_message("test error", Icon::error);
  show_message("test warning", Icon::warning); */
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
    p->setWorkingDirectory(Directory::find_real_path(Directory::get_parent_uri(f.uri), core));
    p->start(f.path);
    //todo: catch errors
    //todo: run in tasks thread
  }
}

void Main_window::on_action_general_settings_triggered() {
  (new Settings_dialog(core))->show();
}

void Main_window::on_action_view_triggered() {
  view_or_edit_selected(false);
}

void Main_window::on_action_edit_triggered() {
  view_or_edit_selected(true);

}

void Main_window::on_action_copy_triggered() {
  Action_data data = get_auto_target_and_destination(Action_type::copy);
  create_action(data);
}

void Main_window::queue_destroyed(Action_queue* object) {
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
      Question_widget* qw = dynamic_cast<Question_widget*>(w);
      if (qw) {
        qw->start_editor();
        return;
      }
    }
  }
}

void Main_window::action_started() {
  Action_state_widget* w = dynamic_cast<Action_state_widget*>(sender());
  ui->questions_layout->insertWidget(0, w);

}

void Main_window::question_widget_destroyed(QObject *object) {
  Question_widget* w = reinterpret_cast<Question_widget*>(object);
  int i = question_widgets.indexOf(w);
  question_widgets.removeAll(w);
  if (i >= 0 && i < question_widgets.count()) {
    question_widgets[i]->start_editor();
  }
}

void Main_window::copy_or_cut_files_to_clipboard(bool cut) {
  QStringList list;
  foreach(File_info fi, active_pane->get_selected_files()) {
    QString uri = fi.uri;
    if (uri.startsWith("/")) {
      uri = "file://" + uri;
    }
    list << QString::fromAscii(QUrl::toPercentEncoding(uri, ":/"));
  }
  QClipboard* clipboard = QApplication::clipboard();
  QMimeData* d = new QMimeData();
  d->setData("x-special/gnome-copied-files", QString("%1\n%2")
             .arg(cut? "cut": "copy")
             .arg(list.join("\n"))
             .toUtf8());
  d->setText(list.join("\n").toUtf8());
  clipboard->setMimeData(d);
}

void Main_window::init_hotkeys() {
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
  hotkeys.add("remove",     ui->action_remove);
  hotkeys.add("choose_queue", tr("Choose queue"), ui->action_queue_choose);
  hotkeys.add("clipboard_copy",     tr("Copy files to clipboard"),  ui->action_clipboard_copy);
  hotkeys.add("clipboard_cut",      tr("Cut files to clipboard"),  ui->action_cut);
  hotkeys.add("clipboard_paste",    tr("Paste files from clipboard"),  ui->action_paste);
  hotkeys.add("abort_last_created_task",     ui->action_abort_last_created_task);

}

void Main_window::on_action_clipboard_copy_triggered() {
  copy_or_cut_files_to_clipboard(false);
}

void Main_window::on_action_cut_triggered() {
  copy_or_cut_files_to_clipboard(true);
}

void Main_window::on_action_paste_triggered() {
  QClipboard* clipboard = QApplication::clipboard();
  const QMimeData* d = clipboard->mimeData();
  foreach(QString s, d->formats()) {
    qDebug() << s << d->data(s);
  }

  if (!d->hasFormat("x-special/gnome-copied-files")) {
    show_message(tr("Failed to paste from clipboard."), Icon::error);
    return;
  }
  QStringList list = QString::fromUtf8(d->data("x-special/gnome-copied-files")).split("\n");
  QString mode = list.first().trimmed();
  list.removeFirst();
  QStringList normal_list;
  foreach(QString s, list) {
    normal_list << Directory::canonize(QUrl::fromPercentEncoding(s.toAscii()));
  }
  Action_data data;
  data.targets = normal_list;
  data.destination = active_pane->get_uri();
  if (mode == "copy") {
    data.type = Action_type::copy;
  } else if (mode == "cut") {
    data.type = Action_type::move;
  } else {
    show_message(tr("Failed to paste from clipboard."), Icon::error);
    return;
  }
  create_action(data);
}

void Main_window::on_action_abort_last_created_task_triggered() {
  QList<Action*> actions = core->get_actions_manager()->get_actions();
  if (actions.isEmpty()) {
    show_message(tr("No actions exist"), Icon::error);
    return;
  }
  Action* a = actions.last();
  QMetaObject::invokeMethod(a, "abort");
}

void Main_window::action_added(Action* a) {
  connect(a, SIGNAL(question(Question_data)), this, SLOT(slot_action_question(Question_data)));
  Action_state_widget* w = new Action_state_widget(a);
  connect(w, SIGNAL(show_requested()), this, SLOT(action_started()));
}
