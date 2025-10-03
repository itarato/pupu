#pragma once

#include "asset_manager.h"
#include "raylib.h"
#include "sprite_group.h"

struct Character {
 public:
  void init(Vector2 new_pos) {
    pos = new_pos;
    sprite_group.push_sprite(Sprite{3.f, asset_manager.textures[TextureNames::Character1__Run], {32.f, 32.f}, 12, 6});
    sprite_group.push_sprite(Sprite{3.f, asset_manager.textures[TextureNames::Character1__Idle], {32.f, 32.f}, 11, 6});
    sprite_group.push_sprite(Sprite{3.f, asset_manager.textures[TextureNames::Character1__Hit], {32.f, 32.f}, 7, 6});
  }

  void update() {
    // if (IsKeyPressed(KEY_ZERO)) sprite_group.set_current_sprite(0);
    // if (IsKeyPressed(KEY_ONE)) sprite_group.set_current_sprite(1);
    // if (IsKeyPressed(KEY_TWO)) sprite_group.set_current_sprite(2);

    if (IsKeyDown(KEY_LEFT)) {
      sprite_group.horizontal_flip();
      sprite_group.set_current_sprite(0);
      pos.x -= 3;
    } else if (IsKeyDown(KEY_RIGHT)) {
      sprite_group.horizontal_reset();
      sprite_group.set_current_sprite(0);
      pos.x += 3;
    } else {
      sprite_group.set_current_sprite(1);
    }

    sprite_group.update();
  }

  void draw() const {
    sprite_group.draw(pos);
  }

 private:
  SpriteGroup sprite_group{};
  Vector2 pos{};
};
