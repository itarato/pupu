#include <cstdlib>
#include <ctime>

#include "app.h"

int main(int argc, char* argv[]) {
  srand(time(nullptr));

  App app{};
  app.init(argc > 1 ? argv[1] : nullptr);
  app.run();
}
