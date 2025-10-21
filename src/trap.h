#pragma once

#include "character.h"
#include "raylib.h"

struct Trap {
 public:
  virtual void draw() const = 0;
  virtual void update(Map const& map, Character& character) = 0;
  virtual Rectangle hitbox() const = 0;

  virtual ~Trap() = default;
};