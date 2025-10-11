#pragma once

#include "raylib.h"

struct MovingObject {
 public:
  void update() {
    pos.y += speed.y * GetFrameTime();

    if (pos.y < 300.f) {
      speed.y *= -1.f;
      pos.y = 300.f;
    }
    if (pos.y > 600.f) {
      speed.y *= -1.f;
      pos.y = 600.f;
    }
  }

  void draw() const {
    DrawRectangleRec(frame(), ORANGE);
  }

  Rectangle frame() const {
    return Rectangle{pos.x, pos.y, 100.f, 20.f};
  }

 private:
  Vector2 pos{400.f, 500.f};
  Vector2 speed{0.f, -100.f};
};
