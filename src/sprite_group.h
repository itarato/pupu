#pragma once

#include <vector>

#include "raylib.h"
#include "sprite.h"

struct SpriteGroup {
 public:
  void reset() {
    horizontal_reset();
    set_current_sprite(0);
  }

  void set_current_sprite(size_t new_current_sprite_index) {
    if (new_current_sprite_index >= sprites.size()) {
      TraceLog(LOG_ERROR, "Invalid sprite index");
      return;
    }
    current_sprite_index = new_current_sprite_index;
  }

  void horizontal_flip() {
    for (auto& sprite : sprites) sprite.horizontal_flip();
  }

  void horizontal_reset() {
    for (auto& sprite : sprites) sprite.horizontal_reset();
  }

  int update() {
    return current_sprite_mut().update();
  }

  void draw(Vector2 const& pos) const {
    current_sprite().draw(pos);
  }

  void push_sprite(Sprite&& sprite) {
    sprites.push_back(std::move(sprite));
  }

  void restart() {
    current_sprite_mut().restart();
  }

 private:
  std::vector<Sprite> sprites{};
  size_t current_sprite_index{0};

  Sprite& current_sprite_mut() {
    return sprites[current_sprite_index];
  }

  Sprite const& current_sprite() const {
    return sprites[current_sprite_index];
  }
};
