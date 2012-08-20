#include "Hotkey.h"
#include <QAction>

Hotkey::Hotkey(QString _name, QString _text, QString _value, QString _default_value, QAction *_action) :
  name(_name),
  text(_text),
  value(_value),
  default_value(_default_value),
  action(_action)
{
  action->setShortcut(QKeySequence(value));
}

void Hotkey::set_value(QString new_value) {
  value = new_value;
  action->setShortcut(QKeySequence(value));
}

