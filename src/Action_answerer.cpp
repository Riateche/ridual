#include "Action_answerer.h"
#include "Main_window.h"

Action_answerer::Action_answerer(Main_window *mw, Question_data data) :
  Question_widget(mw)
{
  qRegisterMetaType<Error_reaction::Enum>("Error_reaction::Enum");
  connect(this, SIGNAL(question_answered(Error_reaction::Enum)), data.action, SLOT(question_answered(Error_reaction::Enum)));

  QList<Button_settings> buttons;
  //todo: request real options
  buttons << Button_settings(1, tr("Skip"),  static_cast<int>(Error_reaction::skip));
  if (data.error_type != Error_type::destination_inside_source) {
    buttons << Button_settings(2, tr("Retry"), static_cast<int>(Error_reaction::retry));
  }
  buttons << Button_settings(3, tr("Abort"), static_cast<int>(Error_reaction::abort));
  set_message(data.message);
  set_buttons(buttons);
}

void Action_answerer::answered(QVariant data) {
  emit question_answered(static_cast<Error_reaction::Enum>(data.toInt()));
}

