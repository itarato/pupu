#pragma once

#include <utility>

#include "asset_manager.h"
#include "common.h"
#include "raylib.h"
#include "sprite.h"

constexpr const float DYNAMIC_OBJECT_MOVE_SPEED{80.f};

struct InteractiveObject {
 public:
  virtual ~InteractiveObject() = default;

  virtual void draw() const = 0;
  virtual void update(Rectangle const& character_hitbox) = 0;
  //                                                          v Hitbox   v Hori v Vert movement
  virtual void hitbox_check(int direction, std::function<void(Rectangle, float, float)> check_hitbox_fn) const = 0;
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
    unsigned int sprite_frame_length = static_cast<unsigned int>(GAME_FPS / 24);
    sprite.init_texture(asset_manager.textures[TextureNames::Trap5], LARGE_48x48_TILE_SIZE, 7, sprite_frame_length);
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

  void hitbox_check(int direction, std::function<void(Rectangle, float, float)> check_hitbox_fn) const override {
    if ((COLLISION_TYPE_TOP & direction) == 0) return;
    check_hitbox_fn(hitbox(), 0.f, 0.f);
  }

 private:
  int const pixel_size;
  Vector2 const pos;
  Sprite sprite;
  DisappearingPlankState state{DisappearingPlankState::Solid};
  RepeatTimer timer{0.f};

  Rectangle const hitbox_upper_surface() const {
    return move(upscale(TRAP5_HITBOX_UPPER_SURFACE, pixel_size), pos);
  }

  Rectangle const hitbox() const {
    if (state == DisappearingPlankState::Solid || state == DisappearingPlankState::WaitForCrumbling) {
      return move(upscale(tile_source_hitbox(TileSource::Trap5), pixel_size), pos);
    } else {
      return OUTSIDE_RECTANGLE;
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
  HorizontalMoveBehaviourHandler(float const min, float const max)
      : back_and_forther(std::pair<float, float>(min, max), 0.f, DYNAMIC_OBJECT_MOVE_SPEED) {
  }

  virtual void update(BehaviourAdjustableObject& target) override {
    back_and_forther.update();
    target.adjust_pos_x(back_and_forther.get_current());
  }

  ~HorizontalMoveBehaviourHandler() override = default;

 private:
  BackAndForther back_and_forther;
};

struct VerticalMoveBehaviourHandler : BehaviourHandler {
 public:
  VerticalMoveBehaviourHandler(float const min, float const max)
      : back_and_forther(std::pair<float, float>(min, max), 0.f, DYNAMIC_OBJECT_MOVE_SPEED) {
  }

  virtual void update(BehaviourAdjustableObject& target) override {
    back_and_forther.update();
    target.adjust_pos_y(back_and_forther.get_current());
  }

  ~VerticalMoveBehaviourHandler() override = default;

 private:
  BackAndForther back_and_forther;
};

struct DynamicBehaviourObject : InteractiveObject, BehaviourAdjustableObject {
 public:
  DynamicBehaviourObject(int const pixel_size, std::unordered_map<IntVec2, TileSelection> tiles,
                         std::vector<ObjectBehaviour> behaviours)
      : pixel_size(pixel_size), tiles(tiles) {
    for (auto const& behaviour : behaviours) {
      switch (behaviour.type) {
        case ObjectBehaviourType::HorizontalMovement:
          behaviour_handlers.push_back(
              std::make_shared<HorizontalMoveBehaviourHandler>(0, behaviour.movement_range * pixel_size));
          break;
        case ObjectBehaviourType::VerticalMovement:
          behaviour_handlers.push_back(
              std::make_shared<VerticalMoveBehaviourHandler>(0, behaviour.movement_range * pixel_size));
          break;
        default:
          BAIL;
      }
    }
  }

  ~DynamicBehaviourObject() = default;

  void draw() const override {
    for (auto const& [tile_pos, tile_selection] : tiles) {
      tile_selection.draw(Vector2Add(tile_pos.scale(pixel_size).to_vector2(), offset), pixel_size);
    }
  }

  void update(Rectangle const& character_hitbox) override {
    for (auto& behaviour_handler : behaviour_handlers) {
      float pre_x = offset.x;
      float pre_y = offset.y;
      behaviour_handler->update(*this);
      current_frame_xdelta = offset.x - pre_x;
      current_frame_ydelta = offset.y - pre_y;
    }
  }

  void hitbox_check(int direction, std::function<void(Rectangle, float, float)> check_hitbox_fn) const override {
    for (auto const& [tile_pos, tile_selection] : tiles) {
      Rectangle hitbox = move(tile_selection.hitbox(tile_pos, pixel_size), offset);
      check_hitbox_fn(hitbox, current_frame_xdelta, current_frame_ydelta);
    }
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
  float current_frame_xdelta{0.f};
  float current_frame_ydelta{0.f};
};
