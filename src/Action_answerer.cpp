#include "Action_answerer.h"
#include "Main_window.h"

Action_answerer::Action_answerer(Main_window *mw, Question_data data) :
  Question_widget(mw)
{
  qRegisterMetaType<Error_reaction::Enum>("Error_reaction::Enum");
  connect(this, SIGNAL(question_answered(Error_reaction::Enum)), data.action, SLOT(question_answered(Error_reaction::Enum)));

  QList<Button_settings> buttons;

  foreach(Error_reaction::Enum reaction, get_error_reactions(data, true)) {
    buttons << Button_settings(1, error_reaction_to_string(reaction),
                               static_cast<int>(reaction));
  }

  set_message(data.get_message());
  set_buttons(buttons);
}

void Action_answerer::answered(QVariant data) {
  emit question_answered(static_cast<Error_reaction::Enum>(data.toInt()));
}

