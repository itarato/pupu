#pragma once

#include "character.h"
#include "common.h"
#include "raylib.h"

struct Trap {
 public:
  virtual void draw() const = 0;
  virtual void update(Map const& map, Character& character) = 0;
  virtual Rectangle hitbox() const = 0;

  virtual ~Trap() = default;
};

struct BouncingTrap : Trap {
 public:
  BouncingTrap(Vector2 pos, int const pixel_size) : pos(pos), pixel_size(pixel_size), sprite(pixel_size) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GameFPS / 24);
    sprite.init_texture(asset_manager.textures[TextureNames::Trap1], SIMPLE_WALK_NPC_SIZE, 7, sprite_frame_length);
    sprite.stop();
  }

  void draw() const override {
    sprite.draw(pos);
    // DrawRectangleLinesEx(hitbox(), pixel_size, RED);
  }

  virtual void update(Map const& map, Character& character) override {
    if (sprite.update() == 0) sprite.stop();

    if (character.is_falling() && CheckCollisionRecs(hitbox(), character.hitbox())) {
      character.bouncing_trap_interact();
      sprite.reset();
      sprite.play();
    }
  }

  virtual Rectangle hitbox() const override {
    return move(upscale(tile_source_hitbox(TileSource::Trap1), pixel_size), pos);
  }

  virtual ~BouncingTrap() = default;

 private:
  Vector2 pos;
  int const pixel_size;
  Sprite sprite;
};

struct CircleSawTrap : Trap {
 public:
  CircleSawTrap(Vector2 pos, int const pixel_size) : pos(pos), pixel_size(pixel_size), sprite(pixel_size) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GameFPS / 24);
    sprite.init_texture(asset_manager.textures[TextureNames::Trap2], SIMPLE_WALK_NPC_SIZE, 7, sprite_frame_length);
  }

  void draw() const override {
    sprite.draw(pos);
    // DrawRectangleLinesEx(hitbox(), pixel_size, RED);
  }

  virtual void update(Map const& map, Character& character) override {
    sprite.update();

    if (CheckCollisionRecs(hitbox(), character.hitbox())) {
      character.injure(true);
    }
  }

  virtual Rectangle hitbox() const override {
    return move(upscale(tile_source_hitbox(TileSource::Trap2), pixel_size), pos);
  }

  virtual ~CircleSawTrap() = default;

 private:
  Vector2 pos;
  int const pixel_size;
  Sprite sprite;
};

struct SpikeTrap : Trap {
 public:
  SpikeTrap(Vector2 pos, int const pixel_size) : pos(pos), pixel_size(pixel_size), sprite(pixel_size) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GameFPS / 24);
    sprite.init_texture(asset_manager.textures[TextureNames::Trap4], SIMPLE_WALK_NPC_SIZE, 7, sprite_frame_length);
  }

  void draw() const override {
    if (!is_hidden) sprite.draw(pos);
    // DrawRectangleLinesEx(hitbox(), pixel_size, RED);
  }

  virtual void update(Map const& map, Character& character) override {
    if (!is_hidden && sprite.update() == 0) {
      sprite.stop();
      timer.reset();
      is_hidden = true;
    }

    if (is_hidden && timer.update()) {
      sprite.play();
      is_hidden = false;
    }

    if (CheckCollisionRecs(hitbox(), character.hitbox())) {
      character.injure(true);
    }
  }

  virtual Rectangle hitbox() const override {
    if (is_hidden) {
      return OutsideRectangle;
    } else {
      return move(upscale(tile_source_hitbox(TileSource::Trap4), pixel_size), pos);
    }
  }

  virtual ~SpikeTrap() = default;

 private:
  Vector2 pos;
  int const pixel_size;
  Sprite sprite;
  RepeatTimer timer{1.0};
  bool is_hidden{false};
};
