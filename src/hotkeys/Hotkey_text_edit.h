#ifndef HOTKEY_TEXT_EDIT_H
#define HOTKEY_TEXT_EDIT_H

#include <QTextEdit>

class Hotkey_text_edit : public QTextEdit {
  Q_OBJECT
public:
  explicit Hotkey_text_edit(QWidget *parent);
  
private:
  void keyPressEvent(QKeyEvent *e);
  bool eventFilter(QObject *, QEvent *);
};

#endif // HOTKEY_TEXT_EDIT_H
