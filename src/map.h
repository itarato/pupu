#pragma once

#include <cstdio>
#include <ctime>

#include "background.h"
#include "common.h"
#include "raylib.h"

struct Map {
 public:
  void reload_from_file() {
    std::time_t now = std::time(nullptr);
    char* filename{"map.mp"};

    FILE* file = std::fopen(filename, "w");
    if (!file) {
      TraceLog(LOG_ERROR, "Cannot create map file");
      return;
    }

    std::fclose(file);
  }

 private:
  int width{};
  int height{};
};
