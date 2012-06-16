#ifndef HOTKEY_EDITOR_H
#define HOTKEY_EDITOR_H

#include <QDialog>
#include "Hotkey_editor_model.h"
#include <QItemEditorFactory>
#include <QItemDelegate>

namespace Ui {
  class Hotkey_editor;
}





class Hotkey_editor : public QDialog, public QItemEditorFactory {
  Q_OBJECT
  
public:
  explicit Hotkey_editor(QList<Hotkey> hotkeys);
  ~Hotkey_editor();
  static QKeySequence get_hotkey(QString name);

private slots:
  void on_buttonBox_accepted();

private:
  Ui::Hotkey_editor *ui;
  Hotkey_editor_model model;
  static QString settings_group() { return "hotkeys"; }


};

#endif // HOTKEY_EDITOR_H
