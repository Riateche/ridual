#ifndef SPECIAL_URI_H
#define SPECIAL_URI_H

#include <QString>

/*!
 * Class for handling ridual-specific URIs:
 *
 * - "places":
 *   Root pseudo-URI, listing of all available common places.
 * - "places/mounts/42":
 *   Requesting this URI means an attempt to mount N-th volume of the
 *   Mount_manager list. When the volume was successfully mounted,
 *   Pane is redirected to the volume's mount location without displaying
 *   this pseudo-URI.
 *
 *
 */
class Special_uri {
public:
  enum Name {
    _invalid,
    places,
    mounts,
  };

  inline Name name() { return _name; }

  Special_uri(Name name);
  QString uri();
  QString caption();
  Special_uri(QString uri);
  operator bool() const;

private:
  Name _name;
};

#endif // SPECIAL_URI_H
