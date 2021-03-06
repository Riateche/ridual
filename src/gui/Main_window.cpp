#include "Main_window.h"
#include "ui_Main_window.h"
#include <QInputDialog>
#include "gio/Gio_file_system_engine.h"
#include "File_system_engine.h"
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

  foreach(Action_queue* q, core->get_actions_manager()->get_queues()) {
    q->cancel_pending_actions();
  }

  foreach(Action_state_widget* w, action_state_widgets) {
    w->abort();
    delete w;
  }

  while(!core->get_actions_manager()->get_queues().isEmpty()) {
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  }

  delete ui;
}


void Main_window::init() {
  setAttribute(Qt::WA_DeleteOnClose);

#if QT_VERSION < 0x050000
  QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
#endif

  QThreadPool::globalInstance()->setMaxThreadCount(5);

  ui->setupUi(this);

  foreach(QAction* a, QList<QAction*>() << ui->action_recursive_fetch_auto
          << ui->action_recursive_fetch_off << ui->action_recursive_fetch_on) {
    connect(a, SIGNAL(triggered()), this, SLOT(actions_recursive_fetch_triggered()));
  }

  ui->current_queue_notice->hide();

  QVariant v = settings.value("columns");
  columns = v.isValid()? Columns::deserialize(v): Columns::get_default();
  emit columns_changed(columns);

  settings.beginGroup("left_pane");
  ui->left_pane->load_state(&settings);
  settings.endGroup();
  settings.beginGroup("right_pane");
  ui->right_pane->load_state(&settings);
  settings.endGroup();


  int option = settings.value("recursive_fetch_option", static_cast<int>(recursive_fetch_auto)).toInt();
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

  connect(ui->left_pane,  SIGNAL(selection_changed()),   this, SIGNAL(selection_changed()));
  connect(ui->right_pane, SIGNAL(selection_changed()),   this, SIGNAL(selection_changed()));
  connect(this,           SIGNAL(active_pane_changed()), this, SIGNAL(selection_changed()));
  connect(this, SIGNAL(selection_changed()), this, SLOT(slot_selection_changed()));

  if (settings.value("main_window/state").isValid()) {
    restoreState(settings.value("main_window/state").toByteArray());
    restoreGeometry(settings.value("main_window/geometry").toByteArray());
    ui->panes_splitter->restoreState(settings.value("panes_spliter_state").toByteArray());
    show();
  } else {
    show();
    setWindowState(Qt::WindowMaximized);
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
  App_info_list r;
  GList* list = g_app_info_get_all_for_type(mime_type.toLocal8Bit());
  if (!list) return r;
  for(; list; list = list->next) {
    r << App_info(new App_info_data(this, static_cast<GAppInfo*>(list->data)));
  }

  QStringList was;
  for(int i = 0; i < r.count(); i++) {

    QString command = r[i]->command();
    if (was.contains(command)) {
      r.removeAt(i);
      i--;
    } else {
      was << command;
    }
  }

  GAppInfo* default_app = g_app_info_get_default_for_type(mime_type.toLocal8Bit(), 0);
  r.default_app = App_info(new App_info_data(this, default_app));
  return r;
}

App_info Main_window::get_default_app(const QString &mime_type) {
  GAppInfo* default_app = g_app_info_get_default_for_type(mime_type.toLocal8Bit(), 0);
  return App_info(new App_info_data(this, default_app));
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
  if (data.type != Action_type::remove && data.type != Action_type::trash) {
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
  q->create_action(data);
  if (current_queue) {
    show_message(tr("Task added to the queue."), Icon::info);
  }
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
  ui->messages_layout->insertWidget(0, question);
}

void Main_window::switch_focus_question(Question_widget *target, int direction) {
  int i = question_widgets.indexOf(target) + direction;
  if (i >= 0 && i < question_widgets.count()) {
    question_widgets[i]->start_editor();
  }
}




void Main_window::view_or_edit_selected(bool edit) {
  File_info_list list = active_pane->get_selected_files();
  view_or_edit_files(list, edit);
}

void Main_window::view_or_edit_files(const File_info_list &list, bool edit) {
  QString settings_key = edit? "edit_command": "view_command";
  QString command_pattern = settings.value(settings_key, "gedit %U").toString();
  foreach(File_info f, list) {
    QProcess* p = new QProcess();
    QString real_filename = core->get_file_system_engine()->get_real_file_name(f.uri);
    p->setWorkingDirectory(Directory::get_parent_uri(real_filename));
    QString command = command_pattern;
    command = command.replace("%U", QString("\"%1\"").arg(f.uri));
    command = command.replace("%F", QString("\"%1\"").arg(real_filename));
    //todo: correct shell escaping
    connect(p, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(process_error(QProcess::ProcessError)));
    connect(p, SIGNAL(error(QProcess::ProcessError)),
            p, SLOT(deleteLater()));
    connect(p, SIGNAL(finished(int)),
            p, SLOT(deleteLater()));
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
    for(int i = 0; i < ui->messages_layout->count(); i++) {
      QWidget* w = ui->messages_layout->itemAt(i)->widget();
      if (dynamic_cast<Message_widget*>(w)) {
        delete w;
        i--;
      }
    }
  }
}

void Main_window::closeEvent(QCloseEvent *event) {
  if (!action_state_widgets.isEmpty()) {
    if (QMessageBox::question(0, "", tr("Some tasks are still running. Do you want to interrupt them and quit?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
      event->ignore();
    }
  }
}

Action_data Main_window::get_auto_target_and_destination(Action_type::Enum action_type) {
  Action_data data;
  data.type = action_type;
  data.targets = active_pane->get_selected_files();

  if (data.type != Action_type::remove &&
      data.type != Action_type::trash) {
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
  QString text = tr("Are you sure you want to delete selected files?") + "\n\n";
  Action_data data = get_auto_target_and_destination(Action_type::remove);
  int count = 0;
  foreach(File_info fi, data.targets) {
    if (count > 3) {
      text += "...";
      break;
    }
    text += fi.uri + "\n";
    count++;
  }

  if (QMessageBox::question(this, tr("Delete files?"),
                            text,
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
    Action_data data = get_auto_target_and_destination(Action_type::remove);
    create_action(data);
  }
}

void Main_window::on_action_move_triggered() {
  Action_data data = get_auto_target_and_destination(Action_type::move);
  create_action(data);
}

void Main_window::switch_active_pane() {
  set_active_pane(ui->left_pane == active_pane? ui->right_pane: ui->left_pane);
}

void Main_window::show_message(QString message, Icon::Enum icon) {
  ui->messages_layout->insertWidget(0, new Message_widget(message, icon));
}

void Main_window::show_error(QString message) {
  show_message(message, Icon::error);
}

void Main_window::save_settings() {
  settings.beginGroup("left_pane");
  ui->left_pane->save_state(&settings);
  settings.endGroup();
  settings.beginGroup("right_pane");
  ui->right_pane->save_state(&settings);
  settings.endGroup();
  settings.setValue("main_window/state", saveState());
  settings.setValue("main_window/geometry", saveGeometry());
  settings.setValue("recursive_fetch_option", static_cast<int>(get_recursive_fetch_option()));
  settings.setValue("panes_spliter_state", ui->panes_splitter->saveState());
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
      types[i.mime_type] << core->get_file_system_engine()->get_real_file_name(i.uri);
    }
  }
  foreach (QString mime_type, types.keys()) {
    get_default_app(mime_type)->launch(types[mime_type]);
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
  active_pane->refresh();
}

void Main_window::on_action_execute_triggered() {
  File_info_list list = active_pane->get_selected_files();
  foreach(File_info f, list) {
    QProcess* p = new QProcess(this);
    p->setWorkingDirectory(Directory::get_parent_uri(core->get_file_system_engine()->get_real_file_name(f.uri)));
    p->start(core->get_file_system_engine()->get_real_file_name(f.uri));
    //todo: catch errors; run in tasks thread
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
  Action_answerer* aa = new Action_answerer(this, data);
  aa->start_editor();
}

void Main_window::slot_focus_question() {
  if (!question_widgets.isEmpty()) {
    question_widgets.first()->start_editor();
  }
  /*
  for(int i = 0; i < ui->messages_layout->count(); i++) {
    QWidget* w = ui->messages_layout->itemAt(i)->widget();
    if (w != 0) {
      Question_widget* qw = dynamic_cast<Question_widget*>(w);
      if (qw) {
        qw->start_editor();
        return;
      }
    }
  }*/
}

void Main_window::action_started() {
  Action_state_widget* w = dynamic_cast<Action_state_widget*>(sender());
  ui->messages_layout->insertWidget(0, w);

}

void Main_window::question_widget_destroyed(QObject *object) {
  Question_widget* w = static_cast<Question_widget*>(object);
  int i = question_widgets.indexOf(w);
  question_widgets.removeAll(w);
  if (i >= 0 && i < question_widgets.count()) {
    question_widgets[i]->start_editor();
  }
}

void Main_window::action_state_widget_destroyed(QObject *object) {
  action_state_widgets.removeAll(static_cast<Action_state_widget*>(object));
  ui->left_pane->refresh();
  ui->right_pane->refresh();
}

void Main_window::copy_or_cut_files_to_clipboard(bool cut) {
  QStringList list;
  foreach(File_info fi, active_pane->get_selected_files()) {
    QString uri = fi.uri;
    if (uri.startsWith("/")) {
      uri = "file://" + uri;
    }
    list << QString::fromLatin1(QUrl::toPercentEncoding(uri, ":/"));
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
  hotkeys.add("execute",        ui->action_execute);
  hotkeys.add("view",           ui->action_view);
  hotkeys.add("edit",           ui->action_edit);
  hotkeys.add("copy",           ui->action_copy);
  hotkeys.add("move",           ui->action_move);
  hotkeys.add("rename",         ui->action_rename);
  hotkeys.add("remove",         ui->action_remove);
  hotkeys.add("move_to_trash",  ui->action_move_to_trash);
  hotkeys.add("create_folder",  ui->action_create_folder);
  hotkeys.add("edit_new_file",  ui->action_edit_new_file);

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
  File_info_list normal_list;
  foreach(QString s, list) {
    File_info info;
    info.uri = Directory::canonize(QUrl::fromPercentEncoding(s.toLatin1()));
    //todo: request info from fs and fill `info` variable properly
    normal_list << info;
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
  connect(a, SIGNAL(error(QString)), this, SLOT(show_error(QString)));
  connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(action_state_widget_destroyed(QObject*)));
  action_state_widgets << w;
}

void Main_window::on_action_move_to_trash_triggered() {
  Action_data data = get_auto_target_and_destination(Action_type::trash);
  create_action(data);
}

void Main_window::on_action_move_from_trash_triggered() {
  foreach(File_info fi, active_pane->get_selected_files()) {
    Action_data data;
    data.type = Action_type::move;
    data.targets << fi;
    data.destination = Gio_file_system_engine::get_trash_original_path(fi.uri);
    data.destination_includes_filename = true;
    create_action(data);
  }
}

void Main_window::on_action_create_folder_triggered() {
  bool ok = false;
  QString default_name = get_free_file_name(tr("New folder"));
  QString name = QInputDialog::getText(this, tr("Create folder"),
                                       tr("Enter new folder name:"), QLineEdit::Normal,
                                       default_name, &ok);
  if (ok && !name.isEmpty()) {
    Action_data data;
    data.type = Action_type::make_directory;
    File_info info;
    info.uri = active_pane->get_uri() + "/" + name;
    data.targets << info;
    create_action(data);
  }
}

QString Main_window::get_free_file_name(QString prefix) {
  QString default_name = prefix;
  File_info_list existing_files = active_pane->get_all_files();
  int attempts_count = 10000;
  if (!existing_files.isEmpty()) {
    attempts_count /= existing_files.count();
  }
  for(int i = 1; i < attempts_count; i++) {
    QString candidate = i == 1 ? prefix : tr("%1 %2").arg(prefix).arg(i);
    bool found = false;
    foreach(File_info fi, existing_files) {
      if (fi.file_name() == candidate) {
        found = true;
        break;
      }
    }
    if (!found) {
      default_name = candidate;
      break;
    }
  }
  return default_name;
}

void Main_window::on_action_edit_new_file_triggered() {
  bool ok = false;
  QString default_name = get_free_file_name(tr("New file"));
  QString name = QInputDialog::getText(this, tr("Edit new file"),
                                       tr("Enter new file name:"), QLineEdit::Normal,
                                       default_name, &ok);
  if (ok && !name.isEmpty()) {
    File_info fi;
    fi.uri = active_pane->get_uri() + "/" + name;
    File_info_list file_list;
    file_list << fi;
    view_or_edit_files(file_list, true);

  }
}

void Main_window::process_error(QProcess::ProcessError error) {
  if (error == QProcess::FailedToStart) {
    show_message(tr("Failed to start process."), Icon::error);
  } else if (error == QProcess::Crashed) {
    show_message(tr("Started process has crashed."), Icon::error);
  } else {
    show_message(tr("Unknown process error."), Icon::error);
  }
}

void Main_window::on_action_rename_triggered() {
  active_pane->start_renaming();
}
