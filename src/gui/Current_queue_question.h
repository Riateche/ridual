#ifndef CURRENT_QUEUE_QUESTION_H
#define CURRENT_QUEUE_QUESTION_H

#include "Question_widget.h"

/*! This widget asks the user about a queue that should be used for adding new actions.
  When user selects a queue, the widget sets the corresponding queue as the current.


  */
class Current_queue_question : public Question_widget {
  Q_OBJECT
public:
  explicit Current_queue_question(Main_window *mw);
  
private:
  void answered(QVariant data);
  
};

#endif // CURRENT_QUEUE_QUESTION_H
