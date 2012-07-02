#include "Hotkey.h"
#include <QAction>

Hotkey::Hotkey(QString p_name, QString p_text, QString p_value, QString p_default_value, QAction *p_action) :
  name(p_name),
  text(p_text),
  value(p_value),
  default_value(p_default_value),
  action(p_action)
{
  action->setShortcut(QKeySequence(value));
}

void Hotkey::set_value(QString new_value) {
  value = new_value;
  action->setShortcut(QKeySequence(value));
}

