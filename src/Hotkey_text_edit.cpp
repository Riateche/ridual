#include "Hotkey_text_edit.h"
#include <QKeyEvent>
#include <QDebug>

Hotkey_text_edit::Hotkey_text_edit(QWidget *parent) :
  QTextEdit(parent)
{
}

void Hotkey_text_edit::keyPressEvent(QKeyEvent *e) {
  qDebug() << "ok" << e->key();
  setText("ok!");
}
