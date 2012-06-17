#include "Pane.h"
#include "ui_Pane.h"
#include "Read_directory_thread.h"
#include <QKeyEvent>
#include "Main_window.h"
#include <QMovie>
#include <QDebug>

Pane::Pane(QWidget *parent) : QWidget(parent), ui(new Ui::Pane)
{
  filesystem_watcher = 0;
  last_launched_thread = 0;
  old_state_stored = false;
  //  setFocusPolicy(Qt::StrongFocus);
  ui->setupUi(this);
  ui->list->setModel(&file_list_model);
  ui->list->installEventFilter(this);
  ui->address->installEventFilter(this);
  ready = true;
  main_window = 0;
  connect(ui->address, SIGNAL(returnPressed()), this, SLOT(on_go_clicked()));
  connect(ui->list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(open_current()));

  ui->loading_indicator->hide();
  QMovie* loading_movie = new QMovie(":/loading.gif", QByteArray(), ui->loading_indicator);
  ui->loading_indicator->setMovie(loading_movie);
  loading_movie->start();
}

Pane::~Pane() {
  delete ui;
  if (filesystem_watcher) delete filesystem_watcher;
}

void Pane::set_main_window(Main_window *p_main_window) {
  main_window = p_main_window;
  connect(main_window, SIGNAL(active_pane_changed()), this, SLOT(active_pane_changed()));
}

void Pane::set_directory(QString new_directory) {
  if (new_directory.startsWith("~")) {
    new_directory = QDir::homePath() + new_directory.mid(1);
  }
  directory = new_directory;
  ui->address->setText(directory);
  if (directory == "gio://") {
    directory_ready(main_window->get_gio_mounts());
    return;
  }

  ready = false;
  Read_directory_thread* t = new Read_directory_thread(directory);
  connect(t, SIGNAL(ready(QList<File_info>)), this, SLOT(read_thread_finished(QList<File_info>)));
  last_launched_thread = t;
  t->start();

  QTimer* timer = new QTimer();
  timer->singleShot(300, this, SLOT(show_loading_indicator()));

}

bool Pane::eventFilter(QObject *object, QEvent *event) {
  //by default using navigation keys sets selection to current row;
  //we need to change active row (dotted border usually) using navigation keys
  //without changing selection. It's the hormal behaviour for ctrl+nav.keys,
  //so we're going to emulate Ctrl pressing.
  QList<int> nav_keys;
  nav_keys << Qt::Key_Down << Qt::Key_Up << Qt::Key_Space <<
              Qt::Key_PageDown << Qt::Key_PageUp << Qt::Key_Home << Qt::Key_End;
  if (object == ui->list) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(event);
      Q_ASSERT(key_event != 0);
      if (!key_event) return false;
      if (nav_keys.contains(key_event->key()) && key_event->modifiers() == Qt::NoModifier) {
        key_event->setModifiers(Qt::ControlModifier);
      }
      if (key_event->key() == Qt::Key_Return && key_event->modifiers() == Qt::NoModifier) {
        open_current();
        return true;
      }
    } else if (event->type() == QEvent::FocusIn) {
      if (main_window) main_window->set_active_pane(this);
    }
  }
  if (object == ui->address) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(event);
      Q_ASSERT(key_event != 0);
      if (!key_event) return false;
      if (key_event->key() == Qt::Key_Escape && key_event->modifiers() == Qt::NoModifier) {
        ui->list->setFocus();
        return true;
      }
    }
  }
  return false;
}

void Pane::load_state(QSettings *s) {
  set_directory(s->value("path", QDir::current().absolutePath()).toString());
}

void Pane::save_state(QSettings *s) {
  s->setValue("path", directory);
}

bool Pane::is_active() const {
  if (!main_window) return false;
  return main_window->get_active_pane() == this;
}

void Pane::go_parent() {
  QDir dir(directory);
  dir.cdUp(); //fixme: this can cause a freeze
  set_directory(dir.absolutePath());
}

void Pane::open_current() {
  File_info info = file_list_model.info(ui->list->currentIndex());
  if (info.i.isDir()) {
    set_directory(info.i.absoluteFilePath());
  }
}

void Pane::focus_address_line() {
  ui->address->setFocus();
  ui->address->selectAll();
}

void Pane::refresh() {
  old_state_stored = true;
  old_current_index = ui->list->currentIndex();
  old_selection = ui->list->selectionModel()->selection();
  set_directory(directory);
}

void Pane::on_go_clicked() {
  set_directory(ui->address->text());
}

void Pane::directory_ready(QList<File_info> files) {
  file_list_model.set_data(files);
  ui->list->setFocus();
  if (file_list_model.rowCount() > 0) {
    ui->list->setCurrentIndex(file_list_model.index(0, 0));
  }
  ui->list->clearSelection();
  ui->loading_indicator->hide();

  //fixme: this can cause freeze
  if (QDir(directory).exists()) {
    if (filesystem_watcher) delete filesystem_watcher;
    filesystem_watcher = new QFileSystemWatcher(); //remove old bindings
    filesystem_watcher->addPath(directory);
    connect(filesystem_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(refresh()));
  }
  if (old_state_stored) {
    old_state_stored = false;
    ui->list->setCurrentIndex(old_current_index);
    ui->list->selectionModel()->select(old_selection, QItemSelectionModel::SelectCurrent);
  }
}

void Pane::active_pane_changed() {
  QFont font = ui->address->font();
  font.setBold(is_active());
  ui->address->setFont(font);
  if (is_active()) {
    ui->list->setFocus();
  }
}

void Pane::show_loading_indicator() {
  sender()->deleteLater();
  if (last_launched_thread) {
    ui->loading_indicator->show();
  }
}

void Pane::read_thread_finished(QList<File_info> files) {
  if (sender() != last_launched_thread) {
    qDebug() << "Pane::read_thread_finished: old thread finished, ignoring";
    return;
  } else {
    last_launched_thread = 0;
  }
  directory_ready(files);
}


