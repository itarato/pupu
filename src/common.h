#pragma once

#include <cstdio>
#include <cstdlib>
#include <functional>

#include "raylib.h"
#include "raymath.h"

#define BAIL                                                             \
  {                                                                      \
    fprintf(stderr, "\x1b[94mBAIL\x1b[0m in %s:%d", __FILE__, __LINE__); \
    exit(EXIT_FAILURE);                                                  \
  }

#define BAILF(...)                                                        \
  {                                                                       \
    log("\x1b[94mBAIL\x1b[0m in %s:%d", __FILE__, __LINE__, __VA_ARGS__); \
    exit(EXIT_FAILURE);                                                   \
  }

constexpr int const REFERENCE_FPS{144};
constexpr int const DEFAULT_PIXEL_SIZE{2};

constexpr int const TILE_SIZE{16};

constexpr Vector2 const vector_zero{0.f, 0.f};

constexpr int const COLLISION_TYPE_NOTHING{0b0};
constexpr int const COLLISION_TYPE_TOP{0b0001};
constexpr int const COLLISION_TYPE_BOTTOM{0b0010};
constexpr int const COLLISION_TYPE_LEFT{0b0100};
constexpr int const COLLISION_TYPE_RIGHT{0b1000};
constexpr int const COLLISION_TYPE_ALL{0b1111};

constexpr Rectangle const DEFAULT_TILE_HITBOX{0.f, 0.f, TILE_SIZE, TILE_SIZE};
constexpr Rectangle const BOX_HITBOX{5.f, 5.f, 22.f, 22.f};
constexpr Rectangle const ENEMY1_HITBOX{14.f, 26.f, 22.f, 22.f};

constexpr int tileset_tile_collision_map[16 * 11]{
    // clang-format off
    1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
    // clang-format on
};

void log_va_list(const char* level, const char* fileName, int lineNo, const char* s, va_list args) {
  printf("[%s][\x1b[93m%s\x1b[39m:\x1b[96m%d\x1b[0m] \x1b[94m", level, fileName, lineNo);
  vprintf(s, args);
  printf("\x1b[0m\n");
}

void log(const char* level, const char* fileName, int lineNo, const char* s, ...) {
  va_list args;
  va_start(args, s);
  log_va_list(level, fileName, lineNo, s, args);
  va_end(args);
}

struct IntVec2 {
  int x{0};
  int y{0};

  bool operator==(const IntVec2& other) const {
    return x == other.x && y == other.y;
  }

  Vector2 to_vector2() const {
    return Vector2{static_cast<float>(x), static_cast<float>(y)};
  }

  void write(FILE* file) const {
    int pack[2]{x, y};
    fwrite(pack, sizeof(int), 2, file);
  }

  IntVec2 scale(int const scale) const {
    return IntVec2{x * scale, y * scale};
  }

  void set_from_vector2(Vector2 const& v) {
    x = static_cast<float>(v.x);
    y = static_cast<float>(v.y);
  }
};

IntVec2 vector2_to_intvec2(Vector2 const v) {
  return IntVec2{static_cast<int>(v.x), static_cast<int>(v.y)};
}

constexpr IntVec2 const intvec2_0_0{0, 0};
constexpr IntVec2 const intvec2_4_4{4, 4};

IntVec2 intvec2_from_file(FILE* file) {
  IntVec2 out{};

  if (std::fread(&out.x, sizeof(int), 1, file) != 1) BAIL;
  if (std::fread(&out.y, sizeof(int), 1, file) != 1) BAIL;

  return out;
}

IntVec2 tile_coord_from_absolute(Vector2 const v, int const pixel_size) {
  return IntVec2{static_cast<int>(v.x / (TILE_SIZE * pixel_size)), static_cast<int>(v.y / (TILE_SIZE * pixel_size))};
}

namespace std {
template <>
struct hash<IntVec2> {
  std::size_t operator()(const IntVec2& v) const noexcept {
    std::size_t h1 = std::hash<int>{}(v.x);
    std::size_t h2 = std::hash<int>{}(v.y);
    return h1 ^ (h2 << 1);
  }
};
}  // namespace std

struct Stepper {
 public:
  Stepper() {
  }
  Stepper(unsigned int threshold) : threshold(threshold) {
  }

  void set_threshold(unsigned int new_threshold) {
    threshold = new_threshold;
    reset();
  }

  void reset() {
    counter = 0;
  }

  bool update() {
    if (counter >= threshold) {
      counter = 0;
      return true;
    } else {
      counter++;
      return false;
    }
  }

 private:
  unsigned int counter{0};
  unsigned int threshold{};
};

constexpr Rectangle upscale(Rectangle const rect, float const scale) {
  return Rectangle{rect.x * scale, rect.y * scale, rect.width * scale, rect.height * scale};
}

constexpr Rectangle move(Rectangle const rect, IntVec2 const v) {
  return Rectangle{rect.x + v.x, rect.y + v.y, rect.width, rect.height};
}

constexpr Rectangle move(Rectangle const rect, Vector2 const v) {
  return Rectangle{rect.x + v.x, rect.y + v.y, rect.width, rect.height};
}

struct Timeout {
 public:
  Timeout() {
  }

  void update() {
    if (timeout == 0.0) return;

    if (timeout <= GetTime()) {
      on_timeout();
      timeout = 0.0;
    }
  }

  void set_on_timeout(std::function<void()> cb, double timeout_seconds) {
    on_timeout = std::move(cb);
    timeout = GetTime() + timeout_seconds;
  }

  void cancel() {
    timeout = 0.0f;
  }

 private:
  std::function<void()> on_timeout{};
  double timeout{0.0};
};

struct RepeatTimer {
 public:
  RepeatTimer(double interval) : interval(interval) {
    next_tick = GetTime() + interval;
  }

  bool update() {
    if (next_tick <= GetTime()) {
      next_tick = GetTime() + interval;
      return true;
    } else {
      return false;
    }
  }

 private:
  double interval;
  double next_tick;
};

enum class TileSource {
  Gui,
  Tileset,
  Box1,
  Box2,
  Box3,
  Enemy1,
  Enemy2,
  Enemy3,
  Enemy4,
};

Rectangle const tile_source_hitbox(TileSource tile_source, IntVec2 const pos) {
  switch (tile_source) {
    case TileSource::Gui:
    case TileSource::Tileset:
      return move(DEFAULT_TILE_HITBOX, pos);
    case TileSource::Box1:
    case TileSource::Box2:
    case TileSource::Box3:
      return move(BOX_HITBOX, pos);
    case TileSource::Enemy1:
    case TileSource::Enemy2:
    case TileSource::Enemy3:
    case TileSource::Enemy4:
      return move(ENEMY1_HITBOX, pos);
    default:
      BAIL;
  }
}

constexpr IntVec2 const TILESIZE_DEFAULT{TILE_SIZE, TILE_SIZE};
constexpr IntVec2 const TILESIZE_BOX{32, 32};
constexpr IntVec2 const TILESIZE_ENEMY1{48, 48};

struct TileSelection {
  TileSource source{};
  IntVec2 tile_coord{};

  void draw(Vector2 const pos, int const pixel_size) const {
    std::shared_ptr<Texture2D> texture;
    if (source == TileSource::Gui) {
      texture = asset_manager.textures[TextureNames::GuiTiles];
    } else if (source == TileSource::Tileset) {
      texture = asset_manager.textures[TextureNames::TilesetTiles];
    } else if (source == TileSource::Box1) {
      texture = asset_manager.textures[TextureNames::Box1__Idle];
    } else if (source == TileSource::Box2) {
      texture = asset_manager.textures[TextureNames::Box2__Idle];
    } else if (source == TileSource::Box3) {
      texture = asset_manager.textures[TextureNames::Box3__Idle];
    } else if (source == TileSource::Enemy1) {
      texture = asset_manager.textures[TextureNames::Enemy1__Example];
    } else if (source == TileSource::Enemy2) {
      texture = asset_manager.textures[TextureNames::Enemy2__Jump];
    } else if (source == TileSource::Enemy3) {
      texture = asset_manager.textures[TextureNames::Enemy3__Example];
    } else if (source == TileSource::Enemy4) {
      texture = asset_manager.textures[TextureNames::Enemy4__Example];
    } else {
      BAIL;
    }

    IntVec2 _tile_size{tile_size()};
    DrawTexturePro(
        *texture,
        {static_cast<float>(tile_coord.x * TILE_SIZE), static_cast<float>(tile_coord.y * TILE_SIZE),
         static_cast<float>(_tile_size.x), static_cast<float>(_tile_size.y)},
        {pos.x, pos.y, static_cast<float>(_tile_size.x * pixel_size), static_cast<float>(_tile_size.y * pixel_size)},
        Vector2Zero(), 0.f, WHITE);
  }

  void write(FILE* file) const {
    fwrite(&source, sizeof(int), 1, file);
    tile_coord.write(file);
  }

  bool collide_from(int direction) const {
    if (source == TileSource::Gui) {
      return true;
    } else if (source == TileSource::Tileset) {
      return (tileset_tile_collision_map[tile_coord.y * 16 + tile_coord.x] & direction) > 0;
    } else {
      BAIL;
    }
  }

  IntVec2 const tile_size() const {
    switch (source) {
      case TileSource::Gui:
      case TileSource::Tileset:
        return TILESIZE_DEFAULT;
      case TileSource::Box1:
      case TileSource::Box2:
      case TileSource::Box3:
        return TILESIZE_BOX;
      case TileSource::Enemy1:
      case TileSource::Enemy2:
      case TileSource::Enemy3:
      case TileSource::Enemy4:
        return TILESIZE_ENEMY1;
      default:
        BAIL;
    }
  }

  int const snap() const {
    switch (source) {
      case TileSource::Gui:
      case TileSource::Tileset:
        return 16;
      case TileSource::Box1:
      case TileSource::Box2:
      case TileSource::Box3:
      case TileSource::Enemy1:
      case TileSource::Enemy2:
      case TileSource::Enemy3:
      case TileSource::Enemy4:
        return 1;
      default:
        BAIL;
    }
  }

  Rectangle const hitbox(IntVec2 const pos) const {
    return tile_source_hitbox(source, pos);
  }
};

TileSelection tile_selection_from_file(FILE* file) {
  int tile_source_raw{};
  if (fread(&tile_source_raw, sizeof(int), 1, file) != 1) BAIL;
  IntVec2 pos = intvec2_from_file(file);

  TileSource source{};
  switch (tile_source_raw) {
    case 0:
      source = TileSource::Gui;
      break;
    case 1:
      source = TileSource::Tileset;
      break;
    case 2:
      source = TileSource::Box1;
      break;
    case 3:
      source = TileSource::Box2;
      break;
    case 4:
      source = TileSource::Box3;
      break;
    case 5:
      source = TileSource::Enemy1;
      break;
    case 6:
      source = TileSource::Enemy2;
      break;
    case 7:
      source = TileSource::Enemy3;
      break;
    case 8:
      source = TileSource::Enemy4;
      break;
    default:
      BAILF("Invalid: %d", tile_source_raw);
  }

  return TileSelection{source, pos};
}

inline int mod_reduced(const int v, const int mod) {
  return v - (v % mod);
}

IntVec2 relative_frame_pos(Rectangle const frame, int const tile_size, int const pixel_size) {
  Vector2 mouse_pos = GetMousePosition();
  return IntVec2{mod_reduced(mouse_pos.x - frame.x, tile_size * pixel_size) / pixel_size,
                 mod_reduced(mouse_pos.y - frame.y, tile_size * pixel_size) / pixel_size};
}

float fps_independent_multiplier() {
  return static_cast<float>(REFERENCE_FPS) / static_cast<float>(GetFPS());
}

void fps_independent_multiply(float* v, float mul) {
  *v *= powf(mul, fps_independent_multiplier());
}

bool is_horizontal_overlap(Rectangle const& rect, int const abs_minx, int const abs_maxx) {
  if (rect.x > abs_maxx || (rect.x + rect.width - 1) < abs_minx) return false;
  return true;
}

bool is_vertical_overlap(Rectangle const& rect, int const abs_miny, int const abs_maxy) {
  if (rect.y > abs_maxy || (rect.y + rect.height - 1) < abs_miny) return false;
  return true;
}

bool is_vertical_overlap(Rectangle const& rect_lhs, Rectangle const& rect_rhs) {
  if (rect_lhs.y > (rect_rhs.y + rect_rhs.height - 1.f) || (rect_lhs.y + rect_lhs.height - 1.f) < rect_rhs.y)
    return false;
  return true;
}

void debug(Vector2 v, const char* msg) {
  TraceLog(LOG_DEBUG, "%s :: Vector2 { %.2f, %.2f }", msg, v.x, v.y);
}

void debug(Rectangle r, const char* msg) {
  TraceLog(LOG_DEBUG, "%s :: Rectangle { %.2f, %.2f, %.2f, %.2f }", msg, r.x, r.y, r.width, r.height);
}

float randf() {
  return static_cast<float>(rand() % 1001) / 1000.f;
}

bool can_charge_character(int west_wall, int east_wall, Rectangle const& self_hitbox,
                          Rectangle const& character_hitbox) {
  if (is_vertical_overlap(self_hitbox, character_hitbox)) {
    if (west_wall <= character_hitbox.x + character_hitbox.width && character_hitbox.x <= self_hitbox.x) {
      return true;
    }
    if (self_hitbox.x + self_hitbox.width <= character_hitbox.x + character_hitbox.width &&
        character_hitbox.x <= east_wall) {
      return true;
    }
  }

  return false;
}
