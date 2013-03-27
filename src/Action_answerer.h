#ifndef ACTION_ANSWERED_H
#define ACTION_ANSWERED_H

#include <QObject>
#include "Action.h"
#include "Question_widget.h"

class Main_window;

/*!
  This widget displays a question sent by Action and buttons with available options.
  When the user clicks a button, the widget sends corresponding signal to the Action.

  This widget is created by Main_window and is embedded into it.
  */
class Action_answerer : public Question_widget {
  Q_OBJECT
public:
  explicit Action_answerer(Main_window* mw, Question_data data);
  
signals:
  void question_answered(Error_reaction::Enum);

private:
  void answered(QVariant data);
  
};

#endif // ACTION_ANSWERED_H
