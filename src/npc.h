#pragma once

#include "common.h"
#include "raylib.h"
#include "sprite.h"
#include "sprite_group.h"

struct Npc {
 public:
  virtual void draw() const = 0;
  virtual void update() = 0;
};

struct Enemy1Npc : Npc {
 public:
  Enemy1Npc(IntVec2 const pos) : pos(pos.to_vector2()) {
    // sprite_group.
  }

  void draw() const override {
  }

  void update() override {
  }

 private:
  Vector2 pos;
  SpriteGroup sprite_group{};
};
