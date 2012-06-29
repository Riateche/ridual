#ifndef HOTKEY_H
#define HOTKEY_H

#include <QString>
#include <QShortcut>

/*! This class represents a hotkey. It's used internally in Hotkeys.
  An object of this class holds either QShortcut or QAction and
  updates its state to hotkey value.

  \sa Hotkeys

  */
class Hotkey {
public:
  /*! Constructs a hotkey based on QShortcut.
    \param p_name Displayed name of hotkey
    \param p_value String representation of shortcut as returned from QKeySequence::toString
    \param parent QShortcut's parent
  */
  Hotkey(QString p_name, QString p_value, QString default_value, QWidget* parent);

  /*! Constructs a hotkey based on QAction.
    \param p_name Displayed name of hotkey
    \param p_value String representation of shortcut as returned from QKeySequence::toString
    \param action Action to set up.
  */
  Hotkey(QString p_name, QString p_value, QString default_value, QAction* action);
  inline QString get_name() const { return name; }
  inline QString get_value() const { return value; }
  inline QString get_default_value() const { return default_value; }

  /*! Returns pointer to QShortcut or 0 if hotkey is based on QAction.
    */
  inline QShortcut* get_shortcut() { return shortcut; }

  /*! Set new value and update QShortcut or QAction accordingly.
    */
  void set_value(QString new_value);

  /*! Passes name to QObject::tr function and returns its result.
    */
  QString get_translated_name() const;

  /*! This property holds current state of hotkey editor widget.
    */
  QString editor_value;

private:
  QString name;
  QString value;
  QString default_value;
  QShortcut* shortcut;
  QAction* action;
};

#endif // HOTKEY_H
