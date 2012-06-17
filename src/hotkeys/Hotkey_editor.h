#ifndef HOTKEY_EDITOR_H
#define HOTKEY_EDITOR_H

#include <QDialog>

namespace Ui {
  class Hotkey_editor;
}

class Hotkeys;

class Hotkey_editor : public QDialog {
  Q_OBJECT
  
public:
  explicit Hotkey_editor(Hotkeys* parent);
  ~Hotkey_editor();
  static QKeySequence get_hotkey(QString name);

private slots:
  void on_buttonBox_accepted();

private:
  Ui::Hotkey_editor *ui;
  static QString settings_group() { return "hotkeys"; }
  Hotkeys* hotkeys;

};

#endif // HOTKEY_EDITOR_H
