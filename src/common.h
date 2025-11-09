#pragma once

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <utility>

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
// Set after window initialization.
extern int GAME_FPS;
static float FPS_MULTIPLIER{};

constexpr int const DEFAULT_PIXEL_SIZE{2};

constexpr int const TILE_SIZE{16};

constexpr Vector2 const VECTOR2_ZERO{0.f, 0.f};

constexpr int const COLLISION_TYPE_NOTHING{0b0000};
constexpr int const COLLISION_TYPE_NORTH{0b0001};
constexpr int const COLLISION_TYPE_SOUTH{0b0010};
constexpr int const COLLISION_TYPE_WEST{0b0100};
constexpr int const COLLISION_TYPE_EAST{0b1000};
constexpr int const COLLISION_TYPE_ALL{0b1111};

constexpr Rectangle const CHARACTER_HITBOX{8.f, 6.f, 16.f, 26.f};
constexpr Rectangle const DEFAULT_TILE_HITBOX{0.f, 0.f, TILE_SIZE, TILE_SIZE};
constexpr Rectangle const BOX_HITBOX{5.f, 5.f, 22.f, 22.f};
constexpr Rectangle const ENEMY1_HITBOX{14.f, 20.f, 22.f, 28.f};
constexpr Rectangle const ENEMY2_HITBOX{16.f, 19.f, 16.f, 29.f};
constexpr Rectangle const ENEMY3_HITBOX{14.f, 19.f, 19.f, 29.f};
constexpr Rectangle const ENEMY4_HITBOX{12.f, 23.f, 24.f, 25.f};
constexpr Rectangle const ENEMY5_HITBOX{10.f, 2.f, 26.f, 30.f};

constexpr Rectangle const TRAP1_HITBOX{8.f, 34.f, 32.f, 10.f};
constexpr Rectangle const TRAP2_HITBOX{10.f, 10.f, 28.f, 28.f};
constexpr Rectangle const TRAP4_HITBOX{16.f, 38.f, 16.f, 10.f};
constexpr Rectangle const TRAP5_HITBOX{8.f, 20.f, 32.f, 8.f};
constexpr Rectangle const TRAP5_HITBOX_UPPER_SURFACE{8.f, 18.f, 32.f, 8.f};
constexpr Rectangle const TRAP6_HITBOX{16.f, 26.f, 16.f, 22.f};
constexpr Rectangle const GEM_HITBOX{1.f, 1.f, 14.f, 14.f};
constexpr Rectangle const CHECKPOINT_HITBOX{19.f, 2.f, 10.f, 46.f};
constexpr Rectangle const POINTER_HITBOX{19.f, 46.f, 10.f, 2.f};

constexpr Vector2 const LARGE_48x48_TILE_SIZE{48.f, 48.f};
constexpr Vector2 const MEDIUM_32x32_TILE_SIZE{32.f, 32.f};
constexpr Vector2 const SMALL_16x16_TILE_SIZE{16.f, 16.f};

constexpr Rectangle const OUTSIDE_RECTANGLE{-100.f, -100.f, 0.f, 0.f};

constexpr int tileset_tile_collision_map[16 * 11]{
    // clang-format off
    15, 15, 15, 0, 0, 0, 15, 15, 15, 0, 0, 0, 15, 15, 15, 15,
    15, 15, 15, 0, 0, 0, 15, 15, 15, 0, 0, 0, 15, 15, 15, 15,
    15, 15, 15, 0, 0, 0, 15, 15, 15, 0, 0, 0, 0, 15, 15, 15,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 0, 15, 15, 15, 15, 15, 0, 15, 15, 15, 15,
    1, 15, 1, 1, 1, 0, 15, 15, 15, 15, 15, 0, 15, 15, 15, 15,
    1, 1, 1, 0, 0, 0, 0, 15, 15, 15, 0, 0, 0, 15, 15, 15,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    15, 15, 15, 15, 15, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0,
    15, 15, 15, 15, 15, 0, 1, 15, 1, 1, 1, 0, 1, 1, 1, 0,
    15, 15, 15, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
    // clang-format on
};

const char* const MAP_FILENAMES[]{"assets/maps/leni_1.mp", "assets/maps/leni_2.mp", "assets/maps/leni_3.mp",
                                  "assets/maps/leni_4.mp", "assets/maps/leni_5.mp", "assets/maps/leni_6.mp",
                                  "assets/maps/map_0.mp",  "assets/maps/map_1.mp",  "assets/maps/map_2.mp"};

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

float world_rate() {
  return GetFrameTime() * (static_cast<float>(DEFAULT_PIXEL_SIZE) / 2.f);
}

float leftx(Rectangle const& rect) {
  return rect.x;
}

float rightx(Rectangle const& rect) {
  return rect.x + rect.width - 1.f;
}

float topy(Rectangle const& rect) {
  return rect.y;
}

float bottomy(Rectangle const& rect) {
  return rect.y + rect.height - 1.f;
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

  bool is_between(IntVec2 const upperleft, IntVec2 const bottomright) const {
    return x >= upperleft.x && x <= bottomright.x && y >= upperleft.y && y <= bottomright.y;
  }
};

constexpr IntVec2 vector2_to_intvec2(Vector2 const v) {
  return IntVec2{static_cast<int>(v.x), static_cast<int>(v.y)};
}

constexpr std::pair<Vector2, Vector2> vec2_minmax(Vector2 const lhs, Vector2 const rhs) {
  return {{std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y)}, {std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y)}};
}

constexpr std::pair<Vector2, Vector2> vec2_minmax_x(Vector2 const lhs, Vector2 const rhs) {
  return {{std::min(lhs.x, rhs.x), lhs.y}, {std::max(lhs.x, rhs.x), rhs.y}};
}

constexpr std::pair<Vector2, Vector2> vec2_minmax_y(Vector2 const lhs, Vector2 const rhs) {
  return {{lhs.x, std::min(lhs.y, rhs.y)}, {rhs.x, std::max(lhs.y, rhs.y)}};
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

constexpr Rectangle movex(Rectangle const rect, float const offset) {
  return Rectangle{rect.x + offset, rect.y, rect.width, rect.height};
}

constexpr Rectangle movey(Rectangle const rect, float const offset) {
  return Rectangle{rect.x, rect.y + offset, rect.width, rect.height};
}

constexpr Rectangle shrink(Rectangle const rect, float const dist) {
  return Rectangle{rect.x + dist, rect.y + dist, rect.width - (dist * 2.f), rect.height - (dist * 2.f)};
}

constexpr Rectangle const rec_from_edges(Vector2 const lhs, Vector2 const rhs) {
  return {
      std::min(lhs.x, rhs.x),
      std::min(lhs.y, rhs.y),
      fabs(lhs.x - rhs.x),
      fabs(lhs.y - rhs.y),
  };
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

struct Timer {
 public:
  Timer() {
  }

  Timer(double interval) {
    next_tick = GetTime() + interval;
    is_timeout = false;
  }

  bool update() {
    if (is_timeout) return false;

    if (next_tick <= GetTime()) {
      is_timeout = true;
      return true;
    } else {
      return false;
    }
  }

  void reset(double new_interval) {
    next_tick = GetTime() + new_interval;
    is_timeout = false;
  }

  bool is_passed() const {
    return is_timeout;
  }

 private:
  double next_tick;
  bool is_timeout{true};
};

struct RepeatTimer {
 public:
  RepeatTimer(double interval) : interval(interval) {
    reset();
  }

  bool update() {
    if (next_tick <= GetTime()) {
      reset();
      return true;
    } else {
      return false;
    }
  }

  void reset() {
    next_tick = GetTime() + interval;
  }

  void reset(double new_interval) {
    interval = new_interval;
    next_tick = GetTime() + new_interval;
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
  Enemy5,
  Trap1,
  Trap2,
  Trap4,
  Trap5,
  Trap6,
  Gem1,
  Gem2,
  Gem3,
  Gem4,
  Gem5,
  Gem6,
  Checkpoint,
  Pointer,
  // Four bound checks.
  EndMarker,
};

constexpr Rectangle const tile_source_hitbox(TileSource tile_source) {
  switch (tile_source) {
    case TileSource::Gui:
    case TileSource::Tileset:
      return DEFAULT_TILE_HITBOX;
    case TileSource::Box1:
    case TileSource::Box2:
    case TileSource::Box3:
      return BOX_HITBOX;
    case TileSource::Enemy1:
      return ENEMY1_HITBOX;
    case TileSource::Enemy2:
      return ENEMY2_HITBOX;
    case TileSource::Enemy3:
      return ENEMY3_HITBOX;
    case TileSource::Enemy4:
      return ENEMY4_HITBOX;
    case TileSource::Enemy5:
      return ENEMY5_HITBOX;
    case TileSource::Trap1:
      return TRAP1_HITBOX;
    case TileSource::Trap2:
      return TRAP2_HITBOX;
    case TileSource::Trap4:
      return TRAP4_HITBOX;
    case TileSource::Trap5:
      return TRAP5_HITBOX;
    case TileSource::Trap6:
      return TRAP6_HITBOX;
    case TileSource::Gem1:
    case TileSource::Gem2:
    case TileSource::Gem3:
    case TileSource::Gem4:
    case TileSource::Gem5:
    case TileSource::Gem6:
      return GEM_HITBOX;
    case TileSource::Checkpoint:
      return CHECKPOINT_HITBOX;
    case TileSource::Pointer:
      return POINTER_HITBOX;
    default:
      BAIL;
  }
}

Rectangle const tile_source_hitbox(TileSource tile_source, IntVec2 const pos) {
  return move(tile_source_hitbox(tile_source), pos);
}

constexpr IntVec2 const TILESIZE_DEFAULT{TILE_SIZE, TILE_SIZE};
constexpr IntVec2 const TILESIZE_BOX{32, 32};
constexpr IntVec2 const TILESIZE_ENEMY1{48, 48};

struct TileSelection {
  TileSource source{};
  IntVec2 tile_coord{};

  void draw(Vector2 const pos, int const pixel_size, Color const color = WHITE) const {
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
    } else if (source == TileSource::Enemy5) {
      texture = asset_manager.textures[TextureNames::Enemy5__Example];
    } else if (source == TileSource::Trap1) {
      texture = asset_manager.textures[TextureNames::Trap1__Example];
    } else if (source == TileSource::Trap2) {
      texture = asset_manager.textures[TextureNames::Trap2__Example];
    } else if (source == TileSource::Trap4) {
      texture = asset_manager.textures[TextureNames::Trap4__Example];
    } else if (source == TileSource::Trap5) {
      texture = asset_manager.textures[TextureNames::Trap5__Example];
    } else if (source == TileSource::Trap6) {
      texture = asset_manager.textures[TextureNames::Trap6__Example];
    } else if (source == TileSource::Gem1) {
      texture = asset_manager.textures[TextureNames::Gem1__Example];
    } else if (source == TileSource::Gem2) {
      texture = asset_manager.textures[TextureNames::Gem2__Example];
    } else if (source == TileSource::Gem3) {
      texture = asset_manager.textures[TextureNames::Gem3__Example];
    } else if (source == TileSource::Gem4) {
      texture = asset_manager.textures[TextureNames::Gem4__Example];
    } else if (source == TileSource::Gem5) {
      texture = asset_manager.textures[TextureNames::Gem5__Example];
    } else if (source == TileSource::Gem6) {
      texture = asset_manager.textures[TextureNames::Gem6__Example];
    } else if (source == TileSource::Checkpoint) {
      texture = asset_manager.textures[TextureNames::Checkpoint__Init];
    } else if (source == TileSource::Pointer) {
      texture = asset_manager.textures[TextureNames::Pointer__Static];
    } else {
      BAIL;
    }

    IntVec2 _tile_size{tile_size()};
    DrawTexturePro(
        *texture,
        {static_cast<float>(tile_coord.x * TILE_SIZE), static_cast<float>(tile_coord.y * TILE_SIZE),
         static_cast<float>(_tile_size.x), static_cast<float>(_tile_size.y)},
        {pos.x, pos.y, static_cast<float>(_tile_size.x * pixel_size), static_cast<float>(_tile_size.y * pixel_size)},
        Vector2Zero(), 0.f, color);
  }

  void draw(IntVec2 const coord, int const pixel_size) const {
    draw(Vector2{1.f * coord.x * TILE_SIZE * pixel_size, 1.f * coord.y * TILE_SIZE * pixel_size}, pixel_size);
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

  bool is_solid() const {
    if (source == TileSource::Gui) {
      return true;
    } else if (source == TileSource::Tileset) {
      return tileset_tile_collision_map[tile_coord.y * 16 + tile_coord.x] == COLLISION_TYPE_ALL;
    } else {
      BAIL;
    }
  }

  IntVec2 const tile_size() const {
    switch (source) {
      case TileSource::Gui:
      case TileSource::Tileset:
      case TileSource::Gem1:
      case TileSource::Gem2:
      case TileSource::Gem3:
      case TileSource::Gem4:
      case TileSource::Gem5:
      case TileSource::Gem6:
        return TILESIZE_DEFAULT;
      case TileSource::Box1:
      case TileSource::Box2:
      case TileSource::Box3:
        return TILESIZE_BOX;
      case TileSource::Enemy1:
      case TileSource::Enemy2:
      case TileSource::Enemy3:
      case TileSource::Enemy4:
      case TileSource::Enemy5:
      case TileSource::Trap1:
      case TileSource::Trap2:
      case TileSource::Trap4:
      case TileSource::Trap5:
      case TileSource::Trap6:
      case TileSource::Checkpoint:
      case TileSource::Pointer:
        return TILESIZE_ENEMY1;
      default:
        BAIL;
    }
  }

  int const snap() const {
    switch (source) {
      case TileSource::Gui:
      case TileSource::Tileset:
      case TileSource::Gem1:
      case TileSource::Gem2:
      case TileSource::Gem3:
      case TileSource::Gem4:
      case TileSource::Gem5:
      case TileSource::Gem6:
      case TileSource::Checkpoint:
      case TileSource::Pointer:
        return TILE_SIZE;
      case TileSource::Box1:
      case TileSource::Box2:
      case TileSource::Box3:
      case TileSource::Enemy1:
      case TileSource::Enemy2:
      case TileSource::Enemy3:
      case TileSource::Enemy4:
      case TileSource::Enemy5:
      case TileSource::Trap1:
      case TileSource::Trap2:
      case TileSource::Trap4:
      case TileSource::Trap5:
      case TileSource::Trap6:
        return 1;
      default:
        BAIL;
    }
  }

  Rectangle const hitbox(IntVec2 const pos) const {
    return tile_source_hitbox(source, pos);
  }

  Rectangle const hitbox(IntVec2 const pos, int const pixel_size) const {
    return upscale(tile_source_hitbox(source, pos), pixel_size);
  }
};

TileSelection tile_selection_from_file(FILE* file) {
  int tile_source_raw{};
  if (fread(&tile_source_raw, sizeof(int), 1, file) != 1) BAIL;
  IntVec2 pos = intvec2_from_file(file);

  if (tile_source_raw < 0 || tile_source_raw >= static_cast<int>(TileSource::EndMarker))
    BAILF("Invalid: %d", tile_source_raw);
  ;

  TileSource source = static_cast<TileSource>(tile_source_raw);

  return TileSelection{source, pos};
}

inline constexpr int mod_reduced(const int v, const int mod) {
  return v - (v % mod);
}

inline Vector2 mod_reduced(const Vector2 v, const int mod) {
  int x = static_cast<int>(v.x);
  int y = static_cast<int>(v.y);
  return Vector2{static_cast<float>(mod_reduced(x, mod)), static_cast<float>(mod_reduced(y, mod))};
}

IntVec2 relative_frame_pos(Rectangle const frame, int const tile_size, int const pixel_size) {
  Vector2 mouse_pos = GetMousePosition();
  return IntVec2{mod_reduced(mouse_pos.x - frame.x, tile_size * pixel_size) / pixel_size,
                 mod_reduced(mouse_pos.y - frame.y, tile_size * pixel_size) / pixel_size};
}

void fps_independent_multiply(float* v, float mul) {
  *v *= powf(mul, FPS_MULTIPLIER);
}

bool is_horizontal_overlap(Rectangle const& rect_lhs, Rectangle const& rect_rhs) {
  return leftx(rect_lhs) <= rightx(rect_rhs) && rightx(rect_lhs) >= leftx(rect_rhs);
}

bool is_vertical_overlap(Rectangle const& rect_lhs, Rectangle const& rect_rhs) {
  return topy(rect_lhs) <= bottomy(rect_rhs) && bottomy(rect_lhs) >= topy(rect_rhs);
}

void debug(bool v, const char* msg = "Debug") {
  TraceLog(LOG_DEBUG, "%s :: bool { %s }", msg, v ? "true" : "false");
}

void debug(int v, const char* msg = "Debug") {
  TraceLog(LOG_DEBUG, "%s :: int { %d }", msg, v);
}

void debug(float v, const char* msg = "Debug") {
  TraceLog(LOG_DEBUG, "%s :: float { %.2f }", msg, v);
}

void debug(const char* v, const char* msg = "Debug") {
  TraceLog(LOG_DEBUG, "%s :: string { %s }", msg, v);
}

void debug(Vector2 v, const char* msg = "Debug") {
  TraceLog(LOG_DEBUG, "%s :: Vector2 { %.2f, %.2f }", msg, v.x, v.y);
}

void debug(Rectangle r, const char* msg = "Debug") {
  TraceLog(LOG_DEBUG, "%s :: Rectangle { %.2f, %.2f, %.2f, %.2f }", msg, r.x, r.y, r.width, r.height);
}

float randf() {
  return static_cast<float>(rand() % 1001) / 1000.f;
}

float randd() {
  return static_cast<double>(rand() % 1001) / 1000.f;
}

bool can_charge_character_horizontal(int west_wall, int east_wall, Rectangle const& self_hitbox,
                                     Rectangle const& character_hitbox) {
  if (is_vertical_overlap(self_hitbox, character_hitbox)) {
    if (west_wall <= rightx(character_hitbox) && leftx(character_hitbox) <= leftx(self_hitbox)) {
      return true;
    }
    if (rightx(self_hitbox) <= rightx(character_hitbox) && leftx(character_hitbox) <= east_wall) {
      return true;
    }
  }

  return false;
}

// Note: Only downward check.
bool can_charge_character_vertical(int south_wall, Rectangle const& self_hitbox, Rectangle const& character_hitbox) {
  if (is_horizontal_overlap(self_hitbox, character_hitbox)) {
    return south_wall >= bottomy(character_hitbox) && character_hitbox.y >= bottomy(self_hitbox);
  }

  return false;
}

struct CollisionResult {
  int wall{};
  float wall_horizontal_speed{0};
  float wall_vertical_speed{0};

  CollisionResult() {
  }

  CollisionResult(int wall) : wall(wall) {
  }
};

struct HitAndDragMap {
  int north{};
  int east{};
  int west{};
  CollisionResult south{};
};

struct BehaviourAdjustableObject {
 public:
  virtual void adjust_pos_x(float x) = 0;
  virtual void adjust_pos_y(float y) = 0;
  virtual ~BehaviourAdjustableObject() = default;
};

struct BackAndForther {
 public:
  BackAndForther(std::pair<float, float> const range, float current, float(speed))
      : range(range), current(current), speed(speed) {
  }

  void update() {
    current += speed * world_rate();

    if (speed > 0.f) {
      if (current > range.second) {
        current = range.second;
        speed *= -1.f;
      }
    } else if (speed < 0.f) {
      if (current < range.first) {
        current = range.first;
        speed *= -1.f;
      }
    }
  }

  float const get_current() const {
    return current;
  }

 private:
  std::pair<float, float> const range;
  float current;
  float speed;
};

struct WallGrabJumpCounter {
 public:
  void at_west() {
    side = 1;
    if (counter > 0) counter = 0;
  }

  void at_east() {
    side = 2;
    if (counter < 0) counter = 0;
  }

  void count_jump() {
    if (side == 1) {
      counter--;
    } else if (side == 2) {
      counter++;
    }
  }

  bool can_jump() const {
    return !counter;
  }

 private:
  uint8_t side{0};  // 0=no -1=west +1=east
  int counter;
};
