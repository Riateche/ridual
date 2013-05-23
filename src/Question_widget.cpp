#include "Question_widget.h"
#include "ui_Question_widget.h"
#include "Main_window.h"
#include "Pane.h"
#include <QKeyEvent>
#include <QDebug>
#include <QTextDocument>

Question_widget::Question_widget(Main_window *mw) :
  ui(new Ui::Question_widget)
, main_window(mw)
{
  ui->setupUi(this);
  ui->answer_editor->installEventFilter(this);
  //ui->answer_frame->hide();
  main_window->add_question(this);
  //setFocusPolicy(Qt::StrongFocus);
  ui->answer_frame->hide();
}

Question_widget::~Question_widget() {
  delete ui;
}

void Question_widget::set_message(const QString &message) {
  QString s = Qt::escape(message).replace("/", "/&#8203;");
  ui->message->setText(s);
}

void Question_widget::set_buttons(const QList<Button_settings> &_buttons) {
  buttons = _buttons;
  //update_buttons();
}

void Question_widget::start_editor() {
  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  //ui->answer_frame->show();
  //ui->answer_editor->setFocus();
  if (!buttons_widgets.isEmpty()) {
    buttons_widgets.first()->setFocus();
  }
}

void Question_widget::update_buttons() {
  if (ui->frame->width() < 1) return; //can't align buttons without having widget size
  foreach(QPushButton* b, buttons_widgets) {
    b->deleteLater();
  }
  buttons_widgets.clear();
  int button_width = 0;
  for(int i = 0; i < buttons.count(); i++) {
    Button_settings s = buttons[i];
    QPushButton* b = new QPushButton(tr("%1) %2").arg(s.number < 0? i+1 : s.number).arg(s.caption), this);
    //b->setEnabled(s.enabled);
    b->setStyleSheet("text-align: left; padding: 3px");
    int w = b->sizeHint().width();
    if (w > button_width) button_width = w;
    connect(b, SIGNAL(clicked()), this, SLOT(slot_button_clicked()));
    b->installEventFilter(this);
    buttons_widgets << b;
  }
  int margin_left, margin_right;
  ui->frame_layout->getContentsMargins(&margin_left, 0, &margin_right, 0);
  int columns = (ui->frame->width() - margin_left - margin_right + ui->buttons_layout->horizontalSpacing())
      / (button_width + ui->buttons_layout->horizontalSpacing());
  qDebug() << ui->frame->width() << button_width << columns;
  if (columns < 1) columns = 1;
  for(int i = 0; i < buttons_widgets.count(); i++) {
    QPushButton* b = buttons_widgets[i];
    b->setFixedWidth(button_width);
    ui->buttons_layout->addWidget(b, i / columns, i % columns);
  }
  for(int i = 0; i < columns; i++) {
    ui->buttons_layout->setColumnStretch(i, 0);
  }
  ui->buttons_layout->setColumnStretch(columns, 1);
}

bool Question_widget::eventFilter(QObject *object, QEvent *event) {
  //if (object == ui->answer_editor) {
  if (buttons_widgets.contains(static_cast<QPushButton*>(object))) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* e = static_cast<QKeyEvent*>(event);
      if (e->key() == Qt::Key_Escape) {
        //ui->answer_frame->hide();
        main_window->get_active_pane()->setFocus();
        return true;
      } else if (e->key() == Qt::Key_Down) {
        main_window->switch_focus_question(this, 1);
        return true;
      } else if (e->key() == Qt::Key_Up) {
        main_window->switch_focus_question(this, -1);
        return true;
      } else if (e->key() == Qt::Key_Left && buttons_widgets.first() == object) {
        buttons_widgets.last()->setFocus();
        return true;
      } else if (e->key() == Qt::Key_Right && buttons_widgets.last() == object) {
        buttons_widgets.first()->setFocus();
        return true;
      }
      bool ok;
      int number = e->text().toInt(&ok);
      if (ok) {
        for(int i = 0; i < buttons.count(); i++) {
          if (buttons[i].number == number) {
            answered(buttons[i].data);
            main_window->get_active_pane()->setFocus();
            deleteLater();
            return true;
          }
        }
      }
    } else if (event->type() == QEvent::FocusOut) {
      //ui->answer_frame->hide();
    }
  }
  return false;
}

void Question_widget::resizeEvent(QResizeEvent* e) {
  if (e->oldSize().width() != width()) {
    update_buttons();
  }
}

void Question_widget::focusInEvent(QFocusEvent *event) {
  //ui->frame->setFrameShape(QFrame::Panel);
 // ui->frame->setLineWidth(2);
}

void Question_widget::focusOutEvent(QFocusEvent *event) {
  //ui->frame->setFrameShape(QFrame::Panel);
  //ui->frame->setLineWidth(1);
//  ui->answer_frame->hide();
}

void Question_widget::on_answer_editor_textEdited(const QString &text) {
  if (text.isEmpty()) return;
  int number = text.toInt();
  for(int i = 0; i < buttons.count(); i++) {
    if (buttons[i].number == number) {
      answered(buttons[i].data);
      main_window->get_active_pane()->setFocus();
      deleteLater();
      return;
    }
  }
  ui->answer_editor->clear();
}

void Question_widget::slot_button_clicked() {
  QPushButton* b = dynamic_cast<QPushButton*>(sender());
  int index = buttons_widgets.indexOf(b);
  answered(buttons[index].data);
  main_window->get_active_pane()->setFocus();
  deleteLater();
}
