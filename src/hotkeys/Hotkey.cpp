#include "Hotkey.h"

Hotkey::Hotkey(QString p_name, QString p_value, QWidget *parent) :
  name(p_name),
  value(p_value),
  shortcut(QKeySequence(value), parent)
{
}

void Hotkey::set_value(QString new_value) {
  value = new_value;
  shortcut.setKey(QKeySequence(value));
}

QString Hotkey::get_translated_name() const {
  return QObject::tr(name.toAscii().constData());
}

