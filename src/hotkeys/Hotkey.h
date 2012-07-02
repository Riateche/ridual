#ifndef HOTKEY_H
#define HOTKEY_H

#include <QString>
#include <QShortcut>

/*! This class represents a hotkey. It's used internally in Hotkeys.
  An object of this class holds QAction and
  updates its state to hotkey value.

  \sa Hotkeys

  */
class Hotkey {
public:
  /*! Constructs a hotkey based on QAction.
    \param p_name Displayed name of hotkey
    \param p_value String representation of shortcut as returned from QKeySequence::toString
    \param action Action to set up.
  */
  Hotkey(QString p_name, QString p_text, QString p_value, QString p_default_value, QAction* p_action);
  inline QString get_name() const { return name; }
  inline QString get_text() const { return text; }
  inline QString get_value() const { return value; }
  inline QString get_default_value() const { return default_value; }


  /*! Set new value and update QAction accordingly.
    */
  void set_value(QString new_value);


  /*! This property holds current state of hotkey editor widget.
    */
  QString editor_value;

private:
  QString name;
  QString text;
  QString value;
  QString default_value;
  QAction* action;
};

#endif // HOTKEY_H
