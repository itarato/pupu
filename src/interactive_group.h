#pragma once

#include <cstdlib>
#include <unordered_map>
#include <vector>

#include "common.h"

enum class ObjectBehaviourType {
  HorizontalMovement,
  VerticalMovement,
};

struct ObjectBehaviour {
  ObjectBehaviourType type;
  union {
    int movement_range;
  };

  void write(FILE* file) const {
    if (type == ObjectBehaviourType::HorizontalMovement || type == ObjectBehaviourType::VerticalMovement) {
      int values[2] = {static_cast<int>(type), movement_range};
      fwrite(values, sizeof(int), 2, file);
    } else {
      BAIL;
    }
  }
};

ObjectBehaviour make_object_behaviour__vertical_movement(int const movement_range = 0) {
  return ObjectBehaviour{ObjectBehaviourType::VerticalMovement, movement_range};
}

ObjectBehaviour make_object_behaviour__horizontal_movement(int const movement_range = 0) {
  return ObjectBehaviour{ObjectBehaviourType::HorizontalMovement, movement_range};
}

ObjectBehaviour object_behaviour_from_file(FILE* file) {
  int type{};
  if (std::fread(&type, sizeof(int), 1, file) != 1) BAIL;

  if (type == 0) {
    int movement_range{};
    if (std::fread(&movement_range, sizeof(int), 1, file) != 1) BAIL;
    return make_object_behaviour__horizontal_movement(movement_range);
  } else if (type == 1) {
    int movement_range{};
    if (std::fread(&movement_range, sizeof(int), 1, file) != 1) BAIL;
    return make_object_behaviour__vertical_movement(movement_range);
  } else {
    BAIL;
  }
}

struct InteractiveGroup {
 public:
  InteractiveGroup() {
  }

  InteractiveGroup(std::unordered_map<IntVec2, TileSelection>&& tiles, std::vector<ObjectBehaviour>&& behaviours)
      : tiles(std::move(tiles)), behaviours(std::move(behaviours)) {
  }

  void add_tile(IntVec2 const coord, TileSelection const tile_selection) {
    tiles[coord] = tile_selection;
  }

  std::unordered_map<IntVec2, TileSelection> const& get_tiles() const {
    return tiles;
  }

  std::vector<ObjectBehaviour>& get_behaviours_mut() {
    return behaviours;
  }

  std::vector<ObjectBehaviour> const& get_behaviours() const {
    return behaviours;
  }

  void add_behaviour(ObjectBehaviourType type) {
    switch (type) {
      case ObjectBehaviourType::HorizontalMovement:
        behaviours.push_back(make_object_behaviour__horizontal_movement());
        break;
      case ObjectBehaviourType::VerticalMovement:
        behaviours.push_back(make_object_behaviour__vertical_movement());
        break;
      default:
        BAIL;
        break;
    }
  }

  void write(FILE* file) const {
    int values[2] = {static_cast<int>(tiles.size()), static_cast<int>(behaviours.size())};
    fwrite(values, sizeof(int), 2, file);

    for (auto const& [k, v] : tiles) {
      k.write(file);
      v.write(file);
    }

    for (auto const& behaviour : behaviours) {
      behaviour.write(file);
    }
  }

 private:
  std::unordered_map<IntVec2, TileSelection> tiles{};
  std::vector<ObjectBehaviour> behaviours{};
};

InteractiveGroup interactive_group_from_file(FILE* file) {
  int tile_count{};
  int behaviour_count{};
  if (std::fread(&tile_count, sizeof(int), 1, file) != 1) BAIL;
  if (std::fread(&behaviour_count, sizeof(int), 1, file) != 1) BAIL;

  std::unordered_map<IntVec2, TileSelection> tiles{};
  for (int i = 0; i < tile_count; i++) {
    IntVec2 tile_pos = intvec2_from_file(file);
    TileSelection tile_selection = tile_selection_from_file(file);
    tiles[tile_pos] = tile_selection;
  }

  std::vector<ObjectBehaviour> behaviours{};
  for (int i = 0; i < behaviour_count; i++) {
    behaviours.push_back(object_behaviour_from_file(file));
  }

  return InteractiveGroup{std::move(tiles), std::move(behaviours)};
}
