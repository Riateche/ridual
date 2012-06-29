#ifndef SPECIAL_URI_H
#define SPECIAL_URI_H

#include <QString>

class Special_uri {
public:
  enum Name {
    _invalid,
    places,
    mounts,
    bookmarks,
    userdirs
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
