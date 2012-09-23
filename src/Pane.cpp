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
#include "File_list_proxy_model.h"
#include <QCompleter>

Pane::Pane(QWidget *parent) : QWidget(parent), ui(new Ui::Pane) {
  directory = 0;
  pending_directory = 0;
  completion_directory = 0;
  ui->setupUi(this);
  proxy_model = new File_list_proxy_model();
  proxy_model->setSourceModel(&file_list_model);
  proxy_model->setDynamicSortFilter(true);
  proxy_model->setSortRole(File_list_model::sort_role);
  ui->list->setModel(proxy_model);
  ui->list->installEventFilter(this);
  ui->list->viewport()->installEventFilter(this);
  ui->list->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);


  QFontMetrics font_metrics(ui->list->font());
  ui->list->verticalHeader()->setFixedWidth(10 + font_metrics.width(">"));

  ui->address->installEventFilter(this);
  ready = true;
  main_window = 0;
  connect(ui->address, SIGNAL(returnPressed()), this, SLOT(on_go_clicked()));

  ui->loading_indicator->hide();
  QMovie* loading_movie = new QMovie(":/loading.gif", QByteArray(), ui->loading_indicator);
  ui->loading_indicator->setMovie(loading_movie);
  loading_movie->start();

  connect(ui->list->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(current_index_changed(QModelIndex,QModelIndex)));
  connect(ui->list->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SIGNAL(selection_changed()));
  connect(ui->list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SIGNAL(selection_changed()));


  QCompleter* completer = new QCompleter();
  completer->setCompletionMode(QCompleter::PopupCompletion);
  //completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
  completer->setModel(&uri_completion_model);
  completer->setCompletionRole(Qt::UserRole);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  ui->address->setCompleter(completer);
}

Pane::~Pane() {
  delete ui;
  if (directory) delete directory;
}

void Pane::set_main_window(Main_window *p_main_window) {
  main_window = p_main_window;
  connect(main_window, SIGNAL(active_pane_changed()), this, SLOT(active_pane_changed()));
  connect(ui->list, SIGNAL(doubleClicked(QModelIndex)), main_window, SLOT(open_current()));
  connect(main_window, SIGNAL(columns_changed(Columns)),
          &file_list_model, SLOT(set_columns(Columns)));

}

void Pane::set_uri(QString new_directory) {
  if (directory && new_directory == directory->get_uri()) {
    directory->refresh();
    return;
  }
  if (pending_directory) delete pending_directory;
  if (directory != 0 &&
      Directory::is_relative(new_directory)) {
    //it's relative path
    new_directory = directory->get_uri() + "/" + new_directory;
  }
  pending_directory = new Directory(main_window->get_core(), new_directory);
  connect(pending_directory, SIGNAL(ready(File_info_list)),
          this, SLOT(directory_ready(File_info_list)));
  connect(pending_directory, SIGNAL(error(QString)),
          this, SLOT(directory_error(QString)));
  pending_directory->refresh();
  //ui->address->setText(directory);

  ready = false;  
  QTimer* timer = new QTimer();
  timer->singleShot(300, this, SLOT(show_loading_indicator()));

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
  //qDebug() << event;
  if (object == ui->list) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(event);
      Q_ASSERT(key_event != 0);
      if (!key_event) return false;
      if (nav_keys.contains(key_event->key()) && key_event->modifiers() == Qt::NoModifier) {
        key_event->setModifiers(Qt::ControlModifier);
      }
      /*if (key_event->key() == Qt::Key_Left || key_event->key() == Qt::Key_Right) {
        if (key_event->modifiers() == Qt::NoModifier) {
          int dx = key_event->key() == Qt::Key_Left? -50: 50;
          ui->list->horizontalScrollBar()->setValue(ui->list->horizontalScrollBar()->value() + dx);
          return true;
        }
      }*/
      if (key_event->key() == Qt::Key_Return && key_event->modifiers() == Qt::NoModifier) {
        main_window->open_current();
        return true;
      }
    } else if (event->type() == QEvent::FocusIn) {
      if (main_window) main_window->set_active_pane(this);
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
  /*if (object == ui->list->viewport()) {
    QMouseEvent* e = dynamic_cast<QMouseEvent*>(event);
    if (e && e->buttons() == Qt::LeftButton) {
       ui->list->selectionModel()->setCurrentIndex(file_list_model.index(
                                                     ui->list->indexAt(e->pos()).row(), 0),
                                                     QItemSelectionModel::NoUpdate);
       return true;
     }
  }*/
  return false;
}

void Pane::load_state(QSettings *s) {
  QString uri = s->value("path", "").toString();
  if (uri.isEmpty()) {
    uri = Special_uri(Special_uri::places).uri();
  }
  ui->list->sortByColumn(s->value("sort_column").toInt(),
                         static_cast<Qt::SortOrder>(s->value("sort_order").toInt()));
  set_uri(uri);
}

void Pane::save_state(QSettings *s) {
  s->setValue("path", get_uri());
  s->setValue("sort_column", proxy_model->sortColumn());
  s->setValue("sort_order", static_cast<int>(proxy_model->sortOrder()));
}

bool Pane::is_active() const {
  if (!main_window) return false;
  return main_window->get_active_pane() == this;
}

QString Pane::get_uri() {
  return directory? directory->get_uri(): QString();
}

File_info_list Pane::get_selected_files(bool fallback_to_current) {
  QModelIndexList indexes = proxy_model->mapSelectionToSource(ui->list->selectionModel()->selection()).indexes();
  if (indexes.isEmpty()) {
    if (!fallback_to_current) {
      return File_info_list();
    }
    File_info info = file_list_model.get_file_info(proxy_model->mapToSource(ui->list->currentIndex()));
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
        list << file_list_model.get_file_info(i);
      }
    }
    return list;
  }
}

File_info Pane::get_current_file() {
  return file_list_model.get_file_info(proxy_model->mapToSource(ui->list->currentIndex()));
}

void Pane::setFocus() {
  ui->list->setFocus();
}

void Pane::go_parent() {
  set_uri(directory->get_parent_uri());
}


void Pane::focus_address_line() {
  ui->address->setFocus();
  ui->address->selectAll();
}

void Pane::refresh() {
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
    ui->loading_indicator->hide();
    ui->address->setText(directory->get_uri());
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

  file_list_model.set_data(files);
  proxy_model->setSourceModel(&file_list_model); //hack; proxy_model->index(0, 0) gives segfault if one remove this line
  if (proxy_model->rowCount() > 0) {
    ui->list->setCurrentIndex(proxy_model->index(0, 0));
  }
  ui->list->clearSelection();
  ui->loading_indicator->hide();

  ui->list->setSortingEnabled(!files.disable_sort);
  if (files.disable_sort) {
    proxy_model->sort(-1);
  } else {
  }

  if (old_state_stored) {
    QModelIndex index = file_list_model.index_for_uri(old_current_uri);
    if (index.isValid()) {
      ui->list->setCurrentIndex(proxy_model->mapFromSource(index));
    }
    ui->list->selectionModel()->clearSelection();
    foreach(QString uri, old_selected_uris) {
      QModelIndex i = file_list_model.index_for_uri(uri);
      if (i.isValid()) {
        ui->list->selectionModel()->select(proxy_model->mapFromSource(i), QItemSelectionModel::Select);
      }
    }

//    ui->list->setCurrentIndex(proxy_model->index(old_current_row, old_current_column));
//    ui->list->selectionModel()->select(old_selection, QItemSelectionModel::SelectCurrent);
    ui->list->horizontalScrollBar()->setValue(old_scroll_pos.x());
    ui->list->verticalScrollBar()->setValue(old_scroll_pos.y());
  }
  if (!new_current_uri.isEmpty()) {
    ui->list->selectionModel()->setCurrentIndex(proxy_model->mapFromSource(file_list_model.index_for_uri(new_current_uri)),
                              QItemSelectionModel::NoUpdate);
  }

  ui->list->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  emit selection_changed();
}

void Pane::directory_error(QString message) {
  if (sender() == pending_directory) {
    pending_directory->deleteLater();
    pending_directory = 0;
    ui->loading_indicator->hide();

  } else if (sender() == directory) {

  } else {
    qWarning("Unknown sender");
    return;
  }
  main_window->show_message(tr("Failed to get file list: %1").arg(message), Icon::error);
}

void Pane::current_index_changed(QModelIndex current, QModelIndex previous) {
  //todo: model must not know about current index
  if (is_active()) {
    file_list_model.set_current_index(proxy_model->mapToSource(current));
  } else {
    file_list_model.set_current_index(file_list_model.index(-1 , -1));
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
      QString filename = fi.file_name();
/*      QString completion_value = last_completion_uri;
      if ( !completion_value.isEmpty() &&
           !completion_value.endsWith("/")) completion_value += "/";
      completion_value += filename; */
      QStandardItem* item = new QStandardItem(filename);
      item->setData(uri + filename, Qt::UserRole);
      uri_completion_model.appendRow(item);
    }
  }
  ui->address->completer()->complete();
  qDebug() << "popup!";
}



void Pane::on_list_customContextMenuRequested(const QPoint &pos) {
  File_info file = file_list_model.get_file_info(proxy_model->mapToSource(ui->list->indexAt(pos)));
  App_info_list apps = main_window->get_apps(file.mime_type);
  QMenu* menu = new QMenu(this);
  if (file.is_folder) {
    menu->addAction(tr("Browse"))->setEnabled(false);
  }
  if (file.is_file()) {
    menu->addAction(main_window->get_ui()->action_view);
    menu->addAction(main_window->get_ui()->action_edit);
  }
  if (file.is_executable) {
    menu->addAction(main_window->get_ui()->action_execute);
  }
  if (menu->actions().count() > 0) menu->addSeparator();

  foreach(App_info app, apps) {
    QAction* a = menu->addAction(tr("Open with %1 (%2)").arg(app.name()).arg(app.command()), this, SLOT(action_launch_triggered()));
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
  app.launch(Directory::find_real_path(file.uri, main_window->get_core()));
}

void Pane::update_model_current_index() {
  if (ui->list->hasFocus()) {
    file_list_model.set_current_index(proxy_model->mapToSource(ui->list->selectionModel()->currentIndex()));
  } else {
    file_list_model.set_current_index(file_list_model.index(-1 , -1));
  }
}

void Pane::on_address_textEdited(const QString&) {
  QString uri = ui->address->text();
  bool parent_mode = !uri.endsWith("/");
  if (Directory::is_relative(uri)) {
    uri = directory->get_uri() + "/" + uri;
  }
  if (parent_mode) {
    uri = Directory::get_parent_uri(uri);
  }
  if (last_completion_uri == uri) {
//  if (completion_directory && completion_directory->get_uri() == uri) {
    qDebug() << "keep old completion_directory";
  } else {
    qDebug() << "create new completion_directory";
    last_completion_uri = uri;
    uri_completion_model.clear();
    if (completion_directory) delete completion_directory;
    completion_directory = new Directory(main_window->get_core(), uri);
    connect(completion_directory, SIGNAL(ready(File_info_list)), this, SLOT(completion_directory_ready(File_info_list)));
    completion_directory->refresh();
  }
}
