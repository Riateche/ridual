#ifndef CORE_ALLY_H
#define CORE_ALLY_H

class Core;

class Core_ally {
public:
  Core_ally(Core* _core): core(_core) {}
protected:
  Core* core;
};

#endif // CORE_ALLY_H
