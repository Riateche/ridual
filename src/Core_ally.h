#ifndef CORE_ALLY_H
#define CORE_ALLY_H

class Core;

/*! This is a helper class that should be inherited by classes that keeps
  a pointer to Core object.
  */
class Core_ally {
public:
  Core_ally(Core* _core): core(_core) {}
  inline Core* get_core() const { return core; }
protected:
  Core* core;
};

#endif // CORE_ALLY_H
