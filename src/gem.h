#pragma once

#include "asset_manager.h"
#include "common.h"
#include "raylib.h"
#include "sprite.h"

struct Gem {
 public:
  Gem(int const pixel_size, Vector2 const pos, TileSource const tile_source)
      : pos(pos), sprite(pixel_size), _hitbox(move(upscale(tile_source_hitbox(tile_source), pixel_size), pos)) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GameFPS / 24);
    switch (tile_source) {
      case TileSource::Gem1:
        sprite.init_texture(asset_manager.textures[TextureNames::Gem1], SMALL_16x16_TILE_SIZE, 7, sprite_frame_length);
        break;
      case TileSource::Gem2:
        sprite.init_texture(asset_manager.textures[TextureNames::Gem2], SMALL_16x16_TILE_SIZE, 7, sprite_frame_length);
        break;
      case TileSource::Gem3:
        sprite.init_texture(asset_manager.textures[TextureNames::Gem3], SMALL_16x16_TILE_SIZE, 7, sprite_frame_length);
        break;
      case TileSource::Gem4:
        sprite.init_texture(asset_manager.textures[TextureNames::Gem4], SMALL_16x16_TILE_SIZE, 7, sprite_frame_length);
        break;
      case TileSource::Gem5:
        sprite.init_texture(asset_manager.textures[TextureNames::Gem5], SMALL_16x16_TILE_SIZE, 7, sprite_frame_length);
        break;
      case TileSource::Gem6:
        sprite.init_texture(asset_manager.textures[TextureNames::Gem6], SMALL_16x16_TILE_SIZE, 7, sprite_frame_length);
        break;
      default:
        BAIL;
    }
  }

  void update() {
    sprite.update();
  }

  void draw() const {
    sprite.draw(pos);
  }

  Rectangle const hitbox() const {
    return _hitbox;
  }

  void consume() {
    _is_consumed = true;
  }

  bool const is_consumed() const {
    return _is_consumed;
  }

 private:
  Vector2 const pos;
  Sprite sprite;
  Rectangle const _hitbox;
  bool _is_consumed{false};
};
