#ifndef HOTKEY_EDITOR_H
#define HOTKEY_EDITOR_H

#include <QDialog>

namespace Ui {
  class Hotkey_editor;
}

class Hotkeys;

/*! Widget for hotkey editing. It should be constructed using
  Hotkeys::open_editor.

  */
class Hotkey_editor : public QDialog {
  Q_OBJECT
  
public:
  explicit Hotkey_editor(Hotkeys* parent);
  ~Hotkey_editor();

private:
  Ui::Hotkey_editor *ui;
  Hotkeys* hotkeys;

};

#endif // HOTKEY_EDITOR_H
