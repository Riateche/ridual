#ifndef HOTKEY_TEXT_EDIT_H
#define HOTKEY_TEXT_EDIT_H

#include <QTextEdit>

/*! Hotkey editing widget. It's a text box. if you press a hotkey
  inside this text box, its text will represent pressed hotkey
  as returned by QKeySequence::toString.
  */
class Hotkey_text_edit : public QTextEdit {
  Q_OBJECT
public:
  explicit Hotkey_text_edit(QWidget *parent);
  
private:
  void keyPressEvent(QKeyEvent *e);
};

#endif // HOTKEY_TEXT_EDIT_H
