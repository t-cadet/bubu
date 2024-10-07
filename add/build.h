#pragma once

#include "bubu.h"

Target addInit() {
  Target add = Target();
  add.name = "add";
  add.kind = StaticLibrary;
  add.sources = { "add.cpp" };
  return add;
}

inline const Target add = addInit();

#ifndef NO_MAIN
int main() {
  Compiler cc = {};
  return cc.build(add);
}
#endif
