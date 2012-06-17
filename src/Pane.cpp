#include "Pane.h"
#include "ui_Pane.h"
#include "Read_directory_thread.h"
#include <QKeyEvent>
#include "Main_window.h"

Pane::Pane(QWidget *parent) : QWidget(parent), ui(new Ui::Pane)
{
  ui->setupUi(this);
  ui->list->setModel(&file_list_model);
  ui->list->installEventFilter(this);
  ready = true;
  main_window = 0;
  connect(ui->address, SIGNAL(returnPressed()), this, SLOT(on_go_clicked()));
  connect(ui->list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(open_current()));
}

Pane::~Pane() {
  delete ui;
}

void Pane::set_main_window(Main_window *p_main_window) {
  main_window = p_main_window;
  connect(main_window, SIGNAL(active_pane_changed()), this, SLOT(active_pane_changed()));
}

void Pane::set_directory(QString dir) {
  directory = dir;
  ui->address->setText(dir);
  ready = false;
  Read_directory_thread* t = new Read_directory_thread(dir);
  connect(t, SIGNAL(ready(QFileInfoList)), this, SLOT(directory_readed(QFileInfoList)));
  t->start();
}

bool Pane::eventFilter(QObject *object, QEvent *event) {
  if (object == ui->list) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(event);
      Q_ASSERT(key_event != 0);
      if (!key_event) return false;
      if (key_event->key() == Qt::Key_Down ||
          key_event->key() == Qt::Key_Up ||
          key_event->key() == Qt::Key_Space) {
        if (key_event->modifiers() == Qt::NoModifier) {
          key_event->setModifiers(Qt::ControlModifier);
        }
      }

      /*if (key_event->key() == Qt::Key_Tab && key_event->modifiers() == Qt::NoModifier) {
        if (main_window) main_window->switch_active_pane();
        return true;
      }*/
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
  QFileInfo info = file_list_model.info(ui->list->currentIndex());
  if (info.isDir()) {
    set_directory(info.absoluteFilePath());
  }
}

void Pane::on_go_clicked() {
  set_directory(ui->address->text());
}

void Pane::directory_readed(QFileInfoList files) {
  file_list_model.set_data(files);
}

void Pane::active_pane_changed() {
  QFont font = ui->address->font();
  font.setBold(is_active());
  ui->address->setFont(font);
  if (is_active()) {
    ui->list->setFocus();
  }
}

void Pane::focusInEvent(QFocusEvent *) {
  if (main_window) main_window->set_active_pane(this);
}

