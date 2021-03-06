#include "Pane.h"
#include "ui_Pane.h"
#include <QKeyEvent>
#include "Main_window.h"
#include "ui_Main_window.h"
#include <QMovie>
#include <QDebug>
#include "Directory.h"
#include <QMessageBox>
#include <QMenu>
#include <QScrollBar>
#include "Special_uri.h"
#include <QCompleter>
#include "Core.h"
#include "File_list_model.h"
#include "Mount_manager.h"

Pane::Pane(QWidget *parent) :
  QWidget(parent),
  Core_ally(dynamic_cast<Main_window*>(parent->window())->get_core()),
  ui(new Ui::Pane)
{

  file_list_model = new File_list_model(core);
  connect(file_list_model, SIGNAL(action_requested(Action_data)),
          core->get_main_window(), SLOT(create_action(Action_data)));
  directory = 0;
  pending_directory = 0;
  completion_directory = 0;
  ui->setupUi(this);
  ui->list->setModel(file_list_model);
  ui->list->installEventFilter(this);
  ui->list->viewport()->installEventFilter(this);

  QFontMetrics font_metrics(ui->list->font());
  ui->list->verticalHeader()->setFixedWidth(10 + font_metrics.width(">"));

  ui->address->installEventFilter(this);
  ready = true;
  connect(ui->address, SIGNAL(returnPressed()), this, SLOT(on_go_clicked()));

  ui->loading_indicator->hide();
  QMovie* loading_movie = new QMovie("://resources/images/loading.gif", QByteArray(), ui->loading_indicator);
  ui->loading_indicator->setMovie(loading_movie);
  loading_movie->start();

  connect(ui->list->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(current_index_changed(QModelIndex,QModelIndex)));
  connect(ui->list->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SIGNAL(selection_changed()));
  connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SIGNAL(selection_changed()));


  QCompleter* completer = new QCompleter();
  completer->setCompletionMode(QCompleter::PopupCompletion);
  completer->setModel(&uri_completion_model);
  completer->setCompletionRole(Qt::UserRole);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  ui->address->setCompleter(completer);


  connect(core->get_main_window(), SIGNAL(active_pane_changed()),
          this, SLOT(active_pane_changed()));
  connect(ui->list, SIGNAL(doubleClicked(QModelIndex)),
          core->get_main_window(), SLOT(open_current()));
  connect(core->get_main_window(), SIGNAL(columns_changed(Columns)),
          file_list_model, SLOT(set_columns(Columns)));

  connect(ui->list->verticalScrollBar(), SIGNAL(valueChanged(int)),
          this, SLOT(vertical_scroll_bar_moved()));

  refresh_path_toolbar();
}

Pane::~Pane() {
  delete ui;
  if (directory) delete directory;
}


void Pane::set_uri(QString new_directory) {
  //refresh_path_toolbar();  maybe it's important
  if (directory && new_directory == directory->get_uri()) {
    refresh();
    return;
  }
  if (pending_directory) delete pending_directory;
  if (directory != 0 &&
      Directory::is_relative(new_directory)) {
    //it's relative path
    new_directory = directory->get_uri() + "/" + new_directory;
  }
  pending_directory = new Directory(core, new_directory);
  connect(pending_directory, SIGNAL(ready(File_info_list)),
          this, SLOT(directory_ready(File_info_list)));
  connect(pending_directory, SIGNAL(error(QString)),
          this, SLOT(directory_error(QString)));
  ui->loaded_indicator->hide();
  ui->loading_indicator->show();
  pending_directory->refresh();

  ready = false;  
}

bool Pane::eventFilter(QObject *object, QEvent *event) {
  //by default using navigation keys sets selection to current row;
  //we need to change active row (dotted border usually) using navigation keys
  //without changing selection. It's the hormal behaviour for ctrl+nav.keys,
  //so we're going to emulate Ctrl pressing.
  QList<int> nav_keys;
  nav_keys << Qt::Key_Down << Qt::Key_Up << Qt::Key_Left << Qt::Key_Right <<
              Qt::Key_Space <<
              Qt::Key_PageDown << Qt::Key_PageUp << Qt::Key_Home << Qt::Key_End;
  if (object == ui->list) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(event);
      Q_ASSERT(key_event != 0);
      /*if (key_event->key() == Qt::Key_Left && key_event->modifiers() == Qt::NoModifier) {
        ui->list->horizontalScrollBar()->setValue(ui->list->horizontalScrollBar()->value() -
                                                  ui->list->horizontalScrollBar()->singleStep());
        return true;
      }
      if (key_event->key() == Qt::Key_Right && key_event->modifiers() == Qt::NoModifier) {
        ui->list->horizontalScrollBar()->setValue(ui->list->horizontalScrollBar()->value() +
                                                  ui->list->horizontalScrollBar()->singleStep());
        return true;
      }*/
      if (!key_event) return false;
      if (nav_keys.contains(key_event->key()) && key_event->modifiers() == Qt::NoModifier) {
        key_event->setModifiers(Qt::ControlModifier);
      }
      if (key_event->key() == Qt::Key_Return && key_event->modifiers() == Qt::NoModifier) {
        core->get_main_window()->open_current();
        return true;
      }
    } else if (event->type() == QEvent::FocusIn) {
      core->get_main_window()->set_active_pane(this);
      update_model_current_index();
    } else if (event->type() == QEvent::FocusOut) {
      update_model_current_index();
    }
  }
  if (object == ui->address) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(event);
      Q_ASSERT(key_event != 0);
      if (!key_event) return false;
      if (key_event->key() == Qt::Key_Escape && key_event->modifiers() == Qt::NoModifier) {
        ui->list->setFocus();
        ui->address->setText(directory ? directory->get_uri() : QString());
        return true;
      }
    }
  }
  return false;
}

void Pane::resizeEvent(QResizeEvent *) {
  ui->path_widget->refresh();
}


void Pane::load_state(QSettings *s) {
  QString uri = s->value("path", "").toString();
  if (uri.isEmpty()) {
    uri = Special_uri(Special_uri::places).uri();
  }
  ui->list->sortByColumn(s->value("sort_column").toInt(),
                         static_cast<Qt::SortOrder>(s->value("sort_order").toInt()));
  ui->list->horizontalHeader()->restoreState(s->value("header_state").toByteArray());

#if QT_VERSION >= 0x050000
  ui->list->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->list->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#else
  ui->list->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  ui->list->verticalHeader()->setResizeMode(QHeaderView::Fixed);
#endif

  set_uri(uri);
}

void Pane::save_state(QSettings *s) {
  s->setValue("path", get_uri());
  s->setValue("sort_column", file_list_model->get_sort_column());
  s->setValue("sort_order", file_list_model->get_sort_order());
  s->setValue("header_state", ui->list->horizontalHeader()->saveState());
}

bool Pane::is_active() const {
  return core->get_main_window()->get_active_pane() == this;
}

QString Pane::get_uri() {
  return directory? directory->get_uri(): QString();
}



File_info_list Pane::get_selected_files(bool fallback_to_current) {
  QModelIndexList indexes = ui->list->selectionModel()->selection().indexes();
  if (indexes.isEmpty()) {
    if (!fallback_to_current) {
      return File_info_list();
    }
    File_info info = file_list_model->get_file_info(ui->list->currentIndex());
    if (info.uri.isEmpty()) {
      return File_info_list();
    } else {
      File_info_list list;
      list << info;
      return list;
    }
  } else {
    File_info_list list;
    foreach (QModelIndex i, indexes) {
      if (i.column() == 0) {
        list << file_list_model->get_file_info(i);
      }
    }
    return list;
  }
}

File_info Pane::get_current_file() {
  return file_list_model->get_file_info(ui->list->currentIndex());
}

File_info_list Pane::get_all_files() {
  return file_list_model->get_files();
}

void Pane::setFocus() {
  ui->list->setFocus();
}

void Pane::start_renaming() {
  int row = ui->list->currentIndex().row();
  if (row < 0) { return; }
  Columns columns = file_list_model->get_current_columns();
  for(int column = 0; column < columns.count(); column++) {
    if (columns[column] == Column::file_name) {
      ui->list->edit(file_list_model->index(row, column));
      break;
    }
  }
  //todo: allow editing of columns: basename, full_path, uri (and maybe others)
}

void Pane::go_parent() {
  if (!directory) { return; }
  set_uri(directory->get_parent_uri());
}


void Pane::focus_address_line() {
  ui->address->setFocus();
  ui->address->selectAll();
}

void Pane::refresh() {
  if (!directory) { return; }
  ui->loaded_indicator->hide();
  ui->loading_indicator->show();
  directory->refresh();
}

void Pane::on_go_clicked() {
  ui->list->setFocus();
  set_uri(ui->address->text());
}


void Pane::active_pane_changed() {
  QFont font = ui->address->font();
  font.setBold(is_active());
  ui->address->setFont(font);
  if (is_active()) {
    ui->list->setFocus();
  }
  update_model_current_index();
}

void Pane::show_loading_indicator() {
  sender()->deleteLater(); // delete QTimer
  if (pending_directory != 0) {
    ui->loading_indicator->show();
  }
}

void Pane::directory_ready(File_info_list files) {
  bool old_state_stored = false;
  QString new_current_uri;
  QString old_current_uri;
  QStringList old_selected_uris;
  QPoint old_scroll_pos;

  if (sender() == pending_directory) {
    if (pending_directory && directory &&
        pending_directory->get_uri() == directory->get_parent_uri()) {
      new_current_uri = directory->get_uri();
    }
    if (directory) delete directory;
    directory = pending_directory;
    emit uri_changed();
    pending_directory = 0;
    ui->address->setText(directory->get_uri());
    refresh_path_toolbar();
  } else if (sender() == directory) {
    //it's a refresh, we need to store selection state
    old_state_stored = true;
    old_current_uri = get_current_file().uri;
    File_info_list list = get_selected_files(false);
    foreach(File_info i, list) old_selected_uris << i.uri;
    old_scroll_pos.setX(ui->list->horizontalScrollBar()->value());
    old_scroll_pos.setY(ui->list->verticalScrollBar()->value());
  } else {
    qWarning("Unknown sender");
    return;
  }

  //QByteArray header_state = ui->list->horizontalHeader()->saveState();
  file_list_model->set_files(files);
  if (file_list_model->rowCount() > 0) {
    ui->list->setCurrentIndex(file_list_model->index(0, 0));
  }
  ui->list->clearSelection();
  ui->loading_indicator->hide();
  ui->loaded_indicator->show();

  if (old_state_stored) {
    QModelIndex index = file_list_model->index_for_uri(old_current_uri);
    if (index.isValid()) {
      ui->list->setCurrentIndex(index);
    }
    ui->list->selectionModel()->clearSelection();
    foreach(QString uri, old_selected_uris) {
      QModelIndex i = file_list_model->index_for_uri(uri);
      if (i.isValid()) {
        ui->list->selectionModel()->select(i, QItemSelectionModel::Select);
      }
    }

    ui->list->horizontalScrollBar()->setValue(old_scroll_pos.x());
    ui->list->verticalScrollBar()->setValue(old_scroll_pos.y());
  }
  if (!new_current_uri.isEmpty()) {
    ui->list->selectionModel()->setCurrentIndex(file_list_model->index_for_uri(new_current_uri),
                              QItemSelectionModel::NoUpdate);
  }

  ui->list->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  emit selection_changed();
  //ui->list->horizontalHeader()->restoreState(header_state);
}

void Pane::directory_error(QString message) {
  if (sender() == pending_directory) {
    pending_directory->deleteLater();
    pending_directory = 0;
    ui->loading_indicator->hide();
    ui->loaded_indicator->show();

  } else if (sender() == directory) {

  } else {
    qWarning("Unknown sender");
    return;
  }
  core->get_main_window()->show_message(tr("Failed to get file list: %1").arg(message), Icon::error);
}

void Pane::current_index_changed(QModelIndex current, QModelIndex previous) {
  //todo: model must not know about current index
  if (is_active()) {
    file_list_model->set_current_index(current);
  } else {
    file_list_model->set_current_index(file_list_model->index(-1 , -1));
  }
  Q_UNUSED(previous);
}

void Pane::completion_directory_ready(File_info_list files) {
  if (completion_directory == 0 || completion_directory != sender()) {
    qWarning("invalid sender");
    return;
  }
  QString uri = ui->address->text();
  bool parent_mode = !uri.endsWith("/");
  if (parent_mode) {
    uri = Directory::get_parent_uri(uri);
  }
  if (!uri.isEmpty() && !uri.endsWith("/")) uri += "/";

  uri_completion_model.clear();
  foreach(File_info fi, files) {
    if (fi.is_folder) {
      QString filename = fi.name.isEmpty() ? fi.file_name() : fi.name;
      QStandardItem* item = new QStandardItem(filename);
      item->setData(uri + fi.file_name(), Qt::UserRole);
      uri_completion_model.appendRow(item);
    }
  }
  if (ui->address->hasFocus()) {
    ui->address->completer()->complete();
  }
}



void Pane::on_list_customContextMenuRequested(const QPoint &pos) {
  File_info file = file_list_model->get_file_info(ui->list->indexAt(pos));
  App_info_list apps = core->get_main_window()->get_apps(file.mime_type);
  QMenu* menu = new QMenu(this);
  if (file.is_folder) {
    menu->addAction(tr("Browse"), core->get_main_window(), SLOT(open_current()));
  } else {
    menu->addAction(core->get_main_window()->get_ui()->action_view);
    menu->addAction(core->get_main_window()->get_ui()->action_edit);
    if (file.is_executable) {
      menu->addAction(core->get_main_window()->get_ui()->action_execute);
    }
  }
  if (file.uri.startsWith("trash:")) {
    menu->addAction(core->get_main_window()->get_ui()->action_move_from_trash);
  }

  if (menu->actions().count() > 0) menu->addSeparator();

  foreach(App_info app, apps) {
    QAction* a = menu->addAction(tr("Open with %1 (%2)").arg(app->name()).arg(app->command()), this, SLOT(action_launch_triggered()));
    QVariantList data;
    data << QVariant::fromValue(app) << QVariant::fromValue(file);
    a->setData(data);
    if (app == apps.default_app) {
      QFont f = a->font();
      f.setBold(true);
      a->setFont(f);
    }
  }
  menu->exec(ui->list->viewport()->mapToGlobal(pos) + QPoint(5, 5));
}

void Pane::action_launch_triggered() {
  QAction* a = dynamic_cast<QAction*>(sender());
  if (!a) return;
  QVariantList data = a->data().toList();
  if (data.count() != 2 || !data[0].canConvert<App_info>() || !data[1].canConvert<File_info>()) {
    qWarning("wrong data attached");
    return;
  }
  App_info app = data[0].value<App_info>();
  File_info file = data[1].value<File_info>();
  app->launch(core->get_file_system_engine()->get_real_file_name(file.uri));
}

void Pane::update_model_current_index() {
  if (ui->list->hasFocus()) {
    file_list_model->set_current_index(ui->list->selectionModel()->currentIndex());
  } else {
    file_list_model->set_current_index(file_list_model->index(-1 , -1));
  }
}

void Pane::on_address_textEdited(const QString&) {
  QString uri = ui->address->text();
  bool parent_mode = !uri.endsWith("/");
  if (Directory::is_relative(uri)) {
    if (!directory) { return; }
    uri = directory->get_uri() + "/" + uri;
  }
  if (parent_mode) {
    uri = Directory::get_parent_uri(uri);
  }
  if (last_completion_uri == uri) {
    qDebug() << "keep old completion_directory";
  } else {
    delete completion_directory;
    completion_directory = 0;
    uri_completion_model.clear();
    if (!uri.startsWith(Special_uri(Special_uri::places).uri())) {
      qDebug() << "create new completion_directory";
      last_completion_uri = uri;
      completion_directory = new Directory(core, uri);
      connect(completion_directory, SIGNAL(ready(File_info_list)), this, SLOT(completion_directory_ready(File_info_list)));
      completion_directory->refresh();
    }
  }
}

void Pane::vertical_scroll_bar_moved() {
  ui->list->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}


void Pane::refresh_path_toolbar() {
  File_info_list path_items;
  QString real_path = get_uri();
  QString headless_path;
  bool root_found = false;

  bool samba_roots_supported = true;

  foreach(Gio_mount mount, core->get_mount_manager()->get_mounts()) {
    QString uri_prefix = mount.uri;
    if (!uri_prefix.isEmpty() && real_path.startsWith(uri_prefix)) {
      File_info file_info;
      file_info.uri = uri_prefix;
      file_info.name = mount.name;
      bool ok = false;
      if (samba_roots_supported && file_info.uri.startsWith("smb://")) {
        int schema_length = QString("smb://").length();
        int domain_end_index = file_info.uri.indexOf("/", schema_length);
        QStringList uri_parts = file_info.uri.split("/");
        if (domain_end_index > 0 && uri_parts.count() > 3) {
          ok = true;
          file_info.name = file_info.uri.split("/")[3];
          File_info root_file_info;
          root_file_info.uri = file_info.uri.left(domain_end_index + 1); // including last '/'
          root_file_info.name = file_info.uri.split("/")[2];
          path_items << root_file_info << file_info;
        }
      }
      if (!ok) {
        path_items << file_info;
      }
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
  if (!root_found && real_path.startsWith("trash:")) {
    File_info file_info;
    file_info.uri = "trash:///";
    file_info.name = tr("Trash");
    path_items << file_info;
    headless_path = real_path.mid(6);
    while(headless_path.startsWith("/")) {
      headless_path = headless_path.mid(1);
    }
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
    Path_button* b = new Path_button(core, caption, path_items[i].uri);
    b->setChecked(path_items[i].uri == real_path);
    connect(b, SIGNAL(go_to(QString)), this, SLOT(set_uri(QString)));
    buttons << b;
  }
  ui->path_widget->set_buttons(buttons);
}

