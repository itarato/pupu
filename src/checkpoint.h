#pragma once

#include "asset_manager.h"
#include "common.h"
#include "raylib.h"
#include "sprite.h"
#include "sprite_group.h"

constexpr int const CHECKPOINT_SPRITE_IDLE{0};
constexpr int const CHECKPOINT_SPRITE_OUT{1};
constexpr int const CHECKPOINT_SPRITE_IDLE_VICTORY{2};
constexpr int const CHECKPOINT_SPRITE_OUT_VICTORY{3};
constexpr int const CHECKPOINT_SPRITE_INIT{4};

enum class CheckpointState {
  Closed,
  Opening,
  Opened,
};

struct Checkpoint {
 public:
  Checkpoint(float const pixel_size, Vector2 const pos)
      : pixel_size(pixel_size),
        pos(pos),
        _hitbox(move(upscale(tile_source_hitbox(TileSource::Checkpoint), pixel_size), pos)) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GameFPS / 24);
    sprite_group.push_sprite(Sprite{
        pixel_size,
        asset_manager.textures[TextureNames::Checkpoint1__Idle],
        LARGE_48x48_TILE_SIZE,
        7,
        sprite_frame_length,
    });
    sprite_group.push_sprite(Sprite{
        pixel_size,
        asset_manager.textures[TextureNames::Checkpoint1__Out],
        LARGE_48x48_TILE_SIZE,
        7,
        sprite_frame_length,
    });
    sprite_group.push_sprite(Sprite{
        pixel_size,
        asset_manager.textures[TextureNames::Checkpoint2__Idle],
        LARGE_48x48_TILE_SIZE,
        7,
        sprite_frame_length,
    });
    sprite_group.push_sprite(Sprite{
        pixel_size,
        asset_manager.textures[TextureNames::Checkpoint2__Out],
        LARGE_48x48_TILE_SIZE,
        7,
        sprite_frame_length,
    });
    sprite_group.push_sprite(Sprite{
        pixel_size,
        asset_manager.textures[TextureNames::Checkpoint__Init],
        LARGE_48x48_TILE_SIZE,
        1,
        0,
    });

    sprite_group.set_current_sprite(CHECKPOINT_SPRITE_INIT);
  }

  void draw() const {
    sprite_group.draw(pos);
  }

  void update() {
    if (sprite_group.update() == 0 && state == CheckpointState::Opening) {
      state = CheckpointState::Opened;

      if (is_victory) {
        sprite_group.set_current_sprite(CHECKPOINT_SPRITE_IDLE_VICTORY);
      } else {
        sprite_group.set_current_sprite(CHECKPOINT_SPRITE_IDLE);
      }
    }
  }

  void touch() {
    if (state != CheckpointState::Closed) return;

    state = CheckpointState::Opening;
    if (is_victory) {
      sprite_group.set_current_sprite(CHECKPOINT_SPRITE_OUT_VICTORY);
    } else {
      sprite_group.set_current_sprite(CHECKPOINT_SPRITE_OUT);
    }
  }

  void victory() {
    if (is_victory) return;
    is_victory = true;

    if (state != CheckpointState::Closed) {
      state = CheckpointState::Opening;
      sprite_group.set_current_sprite(CHECKPOINT_SPRITE_OUT_VICTORY);
    }
  }

  Rectangle const hitbox() const {
    return _hitbox;
  }

 private:
  float const pixel_size;
  Vector2 const pos;
  SpriteGroup sprite_group{};
  CheckpointState state{CheckpointState::Closed};
  Rectangle _hitbox;
  bool is_victory{false};
};
