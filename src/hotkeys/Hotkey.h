#ifndef HOTKEY_H
#define HOTKEY_H

#include <QString>
#include <QShortcut>

class Hotkey {
public:
  Hotkey(QString p_name, QString p_value, QWidget* parent);
  inline QString get_name() const { return name; }
  inline QString get_value() const { return value; }
  inline QShortcut* get_shortcut() { return &shortcut; }
  void set_value(QString new_value);
  QString get_translated_name() const;

  QString editor_value;

private:
  QString name;
  QString value;
  QShortcut shortcut;
};

#endif // HOTKEY_H
