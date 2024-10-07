#pragma once

#include "bubu.h"

Target subInit() {
  Target sub = Target();
  sub.name = "sub";
  sub.kind = StaticLibrary;
  sub.sources = { "sub.cpp" };
  sub.includePaths = { "include" };
  return sub;
}

inline const Target sub = subInit();

#ifndef NO_MAIN
int main() {
  Compiler cc = {};
  // cc.flags = { "-Wall", "-Wextra" };
  return cc.build(sub);
}
#endif
