#pragma once

#include "asset_manager.h"
#include "common.h"
#include "raylib.h"
#include "sprite.h"

struct InteractiveObject {
 public:
  virtual ~InteractiveObject() = default;

  virtual void draw() const = 0;
  virtual void update(Rectangle const& character_hitbox) = 0;
  virtual Rectangle const hitbox() const = 0;
  virtual int collision_directions() const = 0;
};

enum class DisappearingPlankState {
  Solid,
  WaitForCrumbling,
  Crumbling,
  Gone,
};

struct DisappearingPlank : InteractiveObject {
 public:
  DisappearingPlank(int const pixel_size, Vector2 const pos) : pixel_size(pixel_size), pos(pos), sprite(pixel_size) {
    unsigned int sprite_frame_length = static_cast<unsigned int>(GameFPS / 24);
    sprite.init_texture(asset_manager.textures[TextureNames::Trap5], SIMPLE_WALK_NPC_SIZE, 7, sprite_frame_length);
    sprite.stop();
  }

  ~DisappearingPlank() = default;

  void draw() const override {
    if (state != DisappearingPlankState::Gone) sprite.draw(pos);
    // DrawRectangleLinesEx(hitbox(), pixel_size, RED);
  }

  void update(Rectangle const& character_hitbox) override {
    if (sprite.update() == 0) {
      state = DisappearingPlankState::Gone;
      timer.reset();
      sprite.stop();
    }

    if (state == DisappearingPlankState::Solid) {
      if (CheckCollisionRecs(character_hitbox, hitbox_upper_surface())) {
        timer.reset(1.0);
        state = DisappearingPlankState::WaitForCrumbling;
      }
    } else if (state == DisappearingPlankState::WaitForCrumbling) {
      if (timer.update()) {
        sprite.play();
        timer.reset(4.0);
        state = DisappearingPlankState::Crumbling;
      }
    } else if (state == DisappearingPlankState::Crumbling) {
      // Noop.
    } else if (state == DisappearingPlankState::Gone) {
      if (timer.update()) {
        state = DisappearingPlankState::Solid;
      }
    } else {
      BAIL;
    }
  }

  Rectangle const hitbox() const override {
    if (state == DisappearingPlankState::Solid || state == DisappearingPlankState::WaitForCrumbling) {
      return move(upscale(tile_source_hitbox(TileSource::Trap5), pixel_size), pos);
    } else {
      return OutsideRectangle;
    }
  }

  int collision_directions() const override {
    return COLLISION_TYPE_TOP;
  }

 private:
  int const pixel_size;
  Vector2 const pos;
  Sprite sprite;
  DisappearingPlankState state{DisappearingPlankState::Solid};
  RepeatTimer timer{0.f};

  Rectangle const hitbox_upper_surface() const {
    return move(upscale(Trap5Hitbox__UpperSurface, pixel_size), pos);
  }
};
