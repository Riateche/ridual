#include "Hotkey.h"
#include <QAction>

Hotkey::Hotkey(QString p_name, QString p_value, QString p_default_value, QWidget *parent) :
  name(p_name),
  value(p_value),
  default_value(p_default_value),
  shortcut(new QShortcut(QKeySequence(value), parent)),
  action(0)
{
}

Hotkey::Hotkey(QString p_name, QString p_value, QString p_default_value, QAction *p_action) :
  name(p_name),
  value(p_value),
  default_value(p_default_value),
  shortcut(0),
  action(p_action)
{
  action->setShortcut(QKeySequence(value));
}

void Hotkey::set_value(QString new_value) {
  //qDebug() << "Hotkey::set_value" << new_value;
  value = new_value;
  if (action) {
    action->setShortcut(QKeySequence(value));
  } else {
    shortcut->setKey(QKeySequence(value));
  }
}

QString Hotkey::get_translated_name() const {
  return QObject::tr(name.toAscii().constData());
}

