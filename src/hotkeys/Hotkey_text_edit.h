#ifndef HOTKEY_TEXT_EDIT_H
#define HOTKEY_TEXT_EDIT_H

#include <QTextEdit>

/*! Hotkey editing widget. It's a text box. Each key event happened
  with the editor is catched by it, and string representation of
  pressed key combinations replaces editor contents.
  */
class Hotkey_text_edit : public QTextEdit {
  Q_OBJECT
public:
  explicit Hotkey_text_edit(QWidget *parent);
  
private:
  void keyPressEvent(QKeyEvent *e);
};

#endif // HOTKEY_TEXT_EDIT_H
