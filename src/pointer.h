#pragma once

#include "asset_manager.h"
#include "common.h"
#include "raylib.h"
#include "sprite.h"
#include "sprite_group.h"

struct Pointer {
 public:
  Pointer(float const pixel_size, Vector2 const pos)
      : pos(pos), _hitbox(move(upscale(tile_source_hitbox(TileSource::Checkpoint), pixel_size), pos)) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GameFPS / 24);
    sprite_group.push_sprite(Sprite{
        pixel_size,
        asset_manager.textures[TextureNames::Pointer__Idle],
        LARGE_48x48_TILE_SIZE,
        7,
        sprite_frame_length,
    });
    sprite_group.push_sprite(Sprite{
        pixel_size,
        asset_manager.textures[TextureNames::Pointer__Static],
        LARGE_48x48_TILE_SIZE,
        1,
        0,
    });
  }

  void update() {
    sprite_group.update();
  }

  void draw() const {
    sprite_group.draw(pos);
  }

  Rectangle const hitbox() const {
    return _hitbox;
  }

 private:
  Vector2 const pos;
  Rectangle _hitbox;
  SpriteGroup sprite_group{};
};
