#pragma once

#include <utility>

#include "asset_manager.h"
#include "common.h"
#include "raylib.h"
#include "sprite.h"

struct InteractiveObject {
 public:
  virtual ~InteractiveObject() = default;

  virtual void draw() const = 0;
  virtual void update(Rectangle const& character_hitbox) = 0;
  virtual void hitbox_check(int direction, std::function<void(Rectangle)> check_hitbox_fn) const = 0;
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

  void hitbox_check(int direction, std::function<void(Rectangle)> check_hitbox_fn) const override {
    if ((COLLISION_TYPE_TOP & direction) == 0) return;
    check_hitbox_fn(hitbox());
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

  Rectangle const hitbox() const {
    if (state == DisappearingPlankState::Solid || state == DisappearingPlankState::WaitForCrumbling) {
      return move(upscale(tile_source_hitbox(TileSource::Trap5), pixel_size), pos);
    } else {
      return OutsideRectangle;
    }
  }
};

struct BehaviourHandler {
 public:
  virtual void update(BehaviourAdjustableObject& target) = 0;
  virtual ~BehaviourHandler() = default;
};

struct HorizontalMoveBehaviourHandler : BehaviourHandler {
 public:
  HorizontalMoveBehaviourHandler(float const min, float const max, float current) : range(min, max), current(current) {
  }

  virtual void update(BehaviourAdjustableObject& target) override {
    target.adjust_pos_x(current);
  }

  ~HorizontalMoveBehaviourHandler() override = default;

 private:
  std::pair<float, float> const range;
  float current;
};

struct VerticalMoveBehaviourHandler : BehaviourHandler {
 public:
  VerticalMoveBehaviourHandler(float const min, float const max, float current) : range(min, max) {
  }

  virtual void update(BehaviourAdjustableObject& target) override {
    target.adjust_pos_y(current);
  }

  ~VerticalMoveBehaviourHandler() override = default;

 private:
  std::pair<float, float> const range;
  float current{0.f};
};

struct DynamicBehaviourObject : InteractiveObject, BehaviourAdjustableObject {
 public:
  DynamicBehaviourObject(int const pixel_size, std::unordered_map<IntVec2, TileSelection> tiles,
                         std::vector<ObjectBehaviour> behaviours)
      : pixel_size(pixel_size), tiles(tiles) {
    for (auto const& behaviour : behaviours) {
      switch (behaviour.type) {
        case ObjectBehaviourType::HorizontalMovement:
          behaviour_handlers.push_back(std::make_shared<HorizontalMoveBehaviourHandler>(0, behaviour.movement_range));
          break;
        case ObjectBehaviourType::VerticalMovement:
          behaviour_handlers.push_back(std::make_shared<VerticalMoveBehaviourHandler>(0, behaviour.movement_range));
          break;
        default:
          BAIL;
      }
    }
  }

  ~DynamicBehaviourObject() = default;

  void draw() const override {
    for (auto const& [tile_pos, tile_selection] : tiles) {
      tile_selection.draw(tile_pos.scale(pixel_size).to_vector2(), pixel_size);
    }
  }

  void update(Rectangle const& character_hitbox) override {
    for (auto& behaviour_handler : behaviour_handlers) {
      behaviour_handler->update(*this);
    }
  }

  void hitbox_check(int direction, std::function<void(Rectangle)> check_hitbox_fn) const override {
  }

  void adjust_pos_x(float x) override {
    offset.x = x;
  }

  void adjust_pos_y(float y) override {
    offset.y = y;
  }

 private:
  int const pixel_size;
  std::unordered_map<IntVec2, TileSelection> tiles{};
  std::vector<std::shared_ptr<BehaviourHandler>> behaviour_handlers{};
  Vector2 offset{};
};
