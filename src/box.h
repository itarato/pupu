#pragma once

#include "asset_manager.h"
#include "common.h"
#include "raylib.h"
#include "sprite.h"
#include "sprite_group.h"

constexpr int const BOX_SPRITE_IDLE{0};
constexpr int const BOX_SPRITE_BREAK{1};
constexpr int const BOX_SPRITE_HIT{2};

enum class BoxState {
  Idle,
  Broken,
  Hurting,
};

struct Box {
 public:
  Box(float const pixel_size, Vector2 const pos, TileSource const tile_source)
      : pos(pos), _hitbox(move(upscale(tile_source_hitbox(TileSource::Box1), pixel_size), pos)) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GAME_FPS / 24);

    if (tile_source == TileSource::Box1) {
      sprite_group.push_sprite(
          Sprite{pixel_size, asset_manager.textures[TextureNames::Box1__Idle], MEDIUM_32x32_TILE_SIZE, 1, 0});
      sprite_group.push_sprite(
          Sprite{pixel_size, asset_manager.textures[TextureNames::Box1__Break], MEDIUM_32x32_TILE_SIZE, 1, 0});
      sprite_group.push_sprite(Sprite{pixel_size, asset_manager.textures[TextureNames::Box1__Hit],
                                      MEDIUM_32x32_TILE_SIZE, 3, sprite_frame_length});
    } else if (tile_source == TileSource::Box2) {
      sprite_group.push_sprite(
          Sprite{pixel_size, asset_manager.textures[TextureNames::Box2__Idle], MEDIUM_32x32_TILE_SIZE, 1, 0});
      sprite_group.push_sprite(
          Sprite{pixel_size, asset_manager.textures[TextureNames::Box2__Break], MEDIUM_32x32_TILE_SIZE, 1, 0});
      sprite_group.push_sprite(Sprite{pixel_size, asset_manager.textures[TextureNames::Box2__Hit],
                                      MEDIUM_32x32_TILE_SIZE, 3, sprite_frame_length});
    } else if (tile_source == TileSource::Box3) {
      sprite_group.push_sprite(
          Sprite{pixel_size, asset_manager.textures[TextureNames::Box3__Idle], MEDIUM_32x32_TILE_SIZE, 1, 0});
      sprite_group.push_sprite(
          Sprite{pixel_size, asset_manager.textures[TextureNames::Box3__Break], MEDIUM_32x32_TILE_SIZE, 1, 0});
      sprite_group.push_sprite(Sprite{pixel_size, asset_manager.textures[TextureNames::Box3__Hit],
                                      MEDIUM_32x32_TILE_SIZE, 3, sprite_frame_length});
    } else {
      BAIL;
    }

    sprite_group.set_current_sprite(BOX_SPRITE_IDLE);
  }

  void update() {
    sprite_group.update();

    if (hurting_timer.update()) {
      sprite_group.set_current_sprite(BOX_SPRITE_BREAK);
      state = BoxState::Broken;
    }
  }

  void draw() const {
    sprite_group.draw(pos);
  }

  Rectangle const hitbox() const {
    if (state == BoxState::Idle) {
      return _hitbox;
    } else {
      return OUTSIDE_RECTANGLE;
    }
  }

  void hit() {
    if (state == BoxState::Idle) {
      state = BoxState::Hurting;
      sprite_group.set_current_sprite(BOX_SPRITE_HIT);
      hurting_timer.reset(2.0);
    }
  }

 private:
  Vector2 const pos;
  SpriteGroup sprite_group;
  Rectangle const _hitbox;
  Timer hurting_timer{};
  BoxState state{BoxState::Idle};
};
