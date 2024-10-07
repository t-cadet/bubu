#include "bubu.h"

#define NO_MAIN
#include "add/build.h"
#include "sub/build.h"

int main() {
  Target math = Target();
  math.name = "math";
  math.sources = { "math.cpp" };
  math.dependencies = { add, sub };

  Compiler cc = {};
  cc.build(math);
}
