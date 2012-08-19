#ifndef ACTION_ANSWERED_H
#define ACTION_ANSWERED_H

#include <QObject>
#include "Action.h"
#include <QQueue>

class Main_window;

class Action_answerer : public QObject {
  Q_OBJECT
public:
  explicit Action_answerer(Main_window* mw);
  
signals:
  
private slots:
  void action_added(Action* a);
  void question(Question_data data);
  void question_answered(QVariant data);

private:
  Main_window* main_window;
  QQueue<Question_data> queue;
  void display_question();
  
};

#endif // ACTION_ANSWERED_H
