#include "add/add.h"
#include "sub.h"

#include <iostream>

int main() {
  std::cout << sub(add(1, 2), 1) << "\n";
}
