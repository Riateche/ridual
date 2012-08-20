#include "Action_answerer.h"
#include "Main_window.h"

Action_answerer::Action_answerer(Main_window *mw) :
  QObject(mw),
  main_window(mw)
{
  qRegisterMetaType<Error_reaction::Enum>("Error_reaction::Enum");
  connect(main_window, SIGNAL(action_added(Action*)), this, SLOT(action_added(Action*)));
}

void Action_answerer::action_added(Action *a) {
  connect(a, SIGNAL(question(Question_data)), this, SLOT(question(Question_data)));
}

void Action_answerer::question(Question_data data) {
  queue.enqueue(data);
  if (queue.count() == 1) {
    display_question();
  }
}

void Action_answerer::question_answered(QVariant data) {
  Error_reaction::Enum r = static_cast<Error_reaction::Enum>(data.toInt());
  Question_data d = queue.dequeue();
  QMetaObject::invokeMethod(d.action, "question_answered", Q_ARG(Error_reaction::Enum, r));
}

void Action_answerer::display_question() {
  Question_data d = queue.head();
  QList<Button_settings> buttons;
  //todo: request real options
  buttons << Button_settings(1, tr("Skip"),  static_cast<int>(Error_reaction::skip));
  if (d.error_type != Error_type::destination_inside_source) {
    buttons << Button_settings(2, tr("Retry"), static_cast<int>(Error_reaction::retry));
  }
  buttons << Button_settings(3, tr("Abort"), static_cast<int>(Error_reaction::abort));
  main_window->show_question(d.message, buttons, this, "question_answered");
}
