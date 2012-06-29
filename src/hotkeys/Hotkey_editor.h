#ifndef HOTKEY_EDITOR_H
#define HOTKEY_EDITOR_H

#include <QDialog>
#include <QModelIndex>

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

private slots:
  void on_table_customContextMenuRequested(const QPoint &pos);
  void set_default_value();
  void disable_shortcut();

private:
  Ui::Hotkey_editor *ui;
  Hotkeys* hotkeys;
  QModelIndex menu_index;

};

#endif // HOTKEY_EDITOR_H
