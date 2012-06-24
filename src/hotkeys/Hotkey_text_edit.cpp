#include "Hotkey_text_edit.h"
#include <QKeyEvent>
#include <QDebug>

Hotkey_text_edit::Hotkey_text_edit(QWidget *parent) :
  QTextEdit(parent)
{
}

void Hotkey_text_edit::keyPressEvent(QKeyEvent *e) {
  //converting QKeyEvent to QKeySequence is not trivial; we use solution from StackOverflow
  int keyInt = e->key();
  Qt::Key key = static_cast<Qt::Key>(keyInt);
  if(key == Qt::Key_unknown){
      qDebug() << "Unknown key from a macro probably";
      return;
  }
  // the user have clicked just and only the special keys Ctrl, Shift, Alt, Meta.
  if(key == Qt::Key_Control ||
      key == Qt::Key_Shift ||
      key == Qt::Key_Alt ||
      key == Qt::Key_Meta)
  {
      qDebug() << "Single click of special key: Ctrl, Shift, Alt or Meta";
      qDebug() << "New KeySequence:" << QKeySequence(keyInt).toString(QKeySequence::NativeText);
      return;
  }

  // check for a combination of user clicks
  Qt::KeyboardModifiers modifiers = e->modifiers();
  QString keyText = e->text();
  // if the keyText is empty than it's a special key like F1, F5, ...
  qDebug() << "Pressed Key:" << keyText;

  QList<Qt::Key> modifiersList;
  if(modifiers & Qt::ShiftModifier)
      keyInt += Qt::SHIFT;
  if(modifiers & Qt::ControlModifier)
      keyInt += Qt::CTRL;
  if(modifiers & Qt::AltModifier)
      keyInt += Qt::ALT;
  if(modifiers & Qt::MetaModifier)
      keyInt += Qt::META;

  setText(QKeySequence(keyInt).toString(QKeySequence::NativeText));
}


