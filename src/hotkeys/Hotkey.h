#ifndef HOTKEY_H
#define HOTKEY_H

#include <QString>
#include <QShortcut>

/*! This class represents a hotkey. It's used internally in Hotkeys.
  An object of this class holds QAction and
  updates its state to hotkey value.

  \sa Hotkeys (main class for this system)
  */
class Hotkey {
public:
  /*! Constructs a hotkey based on QAction.
    \param _name The name as latin untranslatable string  used for storing settings.
    \param _text The text displayed in hotkeys editor.
    \param _value String representation of current shortcut as returned from QKeySequence::toString
    \param _default_value String representation of default shortcut value.
    \param _action Accosiated action. The passed _value will be assigned as shortcut for the action.
  */
  Hotkey(QString _name, QString _text, QString _value,
         QString _default_value, QAction* _action);

  inline QString get_name() const { return name; }
  inline QString get_text() const { return text; }
  inline QString get_value() const { return value; }
  inline QString get_default_value() const { return default_value; }


  /*! Set new value of shortcut (as returned from QKeySequence::toString)
      and update QAction accordingly.
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
