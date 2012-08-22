#ifndef CURRENT_QUEUE_QUESTION_H
#define CURRENT_QUEUE_QUESTION_H

#include "Question_widget.h"

class Current_queue_question : public Question_widget {
  Q_OBJECT
public:
  explicit Current_queue_question(Main_window *mw);
  
private:
  void answered(QVariant data);
  
};

#endif // CURRENT_QUEUE_QUESTION_H
