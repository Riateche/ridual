#ifndef QUESTION_WIDGET_H
#define QUESTION_WIDGET_H

#include <QPushButton>
#include <QVariant>

namespace Ui {
  class Question_widget;
}

class Button_settings {
public:
  Button_settings(int _number, QString _caption, QVariant _data = QVariant()) :
    number(_number),
    caption(_caption),
    data(_data)
  {}
  int number;
  QString caption;
  QVariant data;
  //bool enabled;
};

class Main_window;

class Question_widget : public QWidget {
  Q_OBJECT  
public:
  explicit Question_widget(Main_window *mw);
  ~Question_widget();
  void set_message(const QString& message);
  void set_buttons(const QList<Button_settings>& _buttons);

  void start_editor();
  
private:
  Ui::Question_widget *ui;
  QList<Button_settings> buttons;
  QList<QPushButton*> buttons_widgets;
  void update_buttons();
  virtual bool eventFilter(QObject *object, QEvent *event);
  virtual void resizeEvent(QResizeEvent *);

protected:
  Main_window* main_window;
  virtual void answered(QVariant data) = 0;

private slots:
  void on_answer_editor_textEdited(const QString &arg1);
  void slot_button_clicked();
};

#endif // QUESTION_WIDGET_H
