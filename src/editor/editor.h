#pragma once

#include <algorithm>
#include <list>

#include "../asset_manager.h"
#include "../background.h"
#include "../common.h"
#include "../interactive_group.h"
#include "common.h"
#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"

constexpr const int fixed_pixel_size{2};
constexpr const int MAP_FILE_WRITE_VERSION{10};
constexpr const int COLOR_LIST_SIZE{6};
constexpr const Color COLOR_LIST[COLOR_LIST_SIZE] = {
    RED, ORANGE, BLUE, GREEN, MAGENTA, PINK,
};

constexpr const int grid_sizes[6]{-1, 8, 16, 24, 32, 48};
constexpr const char* grid_size_names[6]{"No grid", "8px", "16px", "24px", "32px", "48px"};

constexpr const int TOOLBAR_WIDTH{600};

enum class SpecialOperation {
  Nothing,
  GroupElemSelect,
};

struct Editor {
 public:
  Editor() {
    background.preload(0, tile_width, tile_height, pixel_size);
    refresh_map_filenames_without_free_memory();
    update_map_filename();
  }

  void load_from_file() {
    reset();

    FILE* file = std::fopen(filename, "r");
    if (!file) {
      TraceLog(LOG_ERROR, "Cannot open map file");
      return;
    }

    int version{};
    if (std::fread(&version, sizeof(int), 1, file) != 1) {
      TraceLog(LOG_ERROR, "Cannot find version. Skipping reading map file.");
      std::fclose(file);
      return;
    }

    if (version != 10) {
      TraceLog(LOG_ERROR, "Version mismatch. Skipping reading map file.");
      std::fclose(file);
      return;
    }

    if (std::fread(&tile_width, sizeof(int), 1, file) != 1) BAIL;
    if (std::fread(&tile_height, sizeof(int), 1, file) != 1) BAIL;
    int background_index{};
    if (std::fread(&background_index, sizeof(int), 1, file) != 1) BAIL;
    int tiles_count{};
    if (std::fread(&tiles_count, sizeof(int), 1, file) != 1) BAIL;
    int interactive_group_count{};
    if (std::fread(&interactive_group_count, sizeof(int), 1, file) != 1) BAIL;

    TraceLog(LOG_INFO, "Loading map. Version=%d W=%d H=%d B=%d Tiles=%d Interactives=%d", version, tile_width,
             tile_height, background_index, tiles_count, interactive_group_count);

    character_position = intvec2_from_file(file);

    background.preload(background_index, tile_width, tile_height, pixel_size);

    for (int i = 0; i < tiles_count; i++) {
      IntVec2 tile_pos = intvec2_from_file(file);
      TileSelection tile_selection{tile_selection_from_file(file)};

      tiles[tile_pos] = tile_selection;
    }

    for (int i = 0; i < interactive_group_count; i++) {
      InteractiveGroup interactive_group = interactive_group_from_file(file);
      interactive_groups.push_back(interactive_group);
    }

    std::fclose(file);
  }

  void update() {
    Vector2 mouse_pos = GetMousePosition();

    if (CheckCollisionPointRec(mouse_pos, game_area()) && mouse_pos.x < GetScreenWidth() - TOOLBAR_WIDTH) {
      if (special_operation == SpecialOperation::Nothing) {
        if (cluster_mode) {
          if (IsMouseButtonPressed(0)) {
            cluster_start = mouse_pos;
          }
          if (IsMouseButtonReleased(0)) {
            // Unimplemented.
          }
        } else {
          if (IsMouseButtonDown(0)) {
            // Draw tile.
            IntVec2 int_coord{mod_reduced(mouse_pos.x, tile_selection.snap() * pixel_size) / pixel_size,
                              mod_reduced(mouse_pos.y, tile_selection.snap() * pixel_size) / pixel_size};
            tiles[int_coord] = tile_selection;
          }
        }
      } else if (special_operation == SpecialOperation::GroupElemSelect) {
        if (IsMouseButtonReleased(0)) {
          bool has_match{false};
          IntVec2 match_tile_pos;
          for (auto const& [tile_pos, tile_selection] : tiles) {
            if (CheckCollisionPointRec(mouse_pos, tile_selection.hitbox(tile_pos, pixel_size))) {
              match_tile_pos = tile_pos;
              interactive_groups[active_interactive_group].add_tile(tile_pos, tile_selection);
              has_match = true;
              break;
            }
          }

          if (has_match) tiles.erase(match_tile_pos);
          special_operation = SpecialOperation::Nothing;
        }
      }
    }

    if (IsMouseButtonDown(1)) {
      // Erase tile.
      if (CheckCollisionPointRec(mouse_pos, game_area()) && mouse_pos.x < GetScreenWidth() - TOOLBAR_WIDTH) {
        std::erase_if(tiles, [&](auto const& p) {
          IntVec2 const& pos = p.first;
          TileSelection const& selection = p.second;

          const Rectangle hitbox = selection.hitbox(pos, pixel_size);
          return CheckCollisionPointRec(mouse_pos, hitbox);
        });
      }
    }

    if (IsMouseButtonDown(2)) {
      character_position.x = mouse_pos.x / pixel_size;
      character_position.y = mouse_pos.y / pixel_size;
    }
  }

  void draw() {
    // Background.
    background.draw({0.f, 0.f}, pixel_size);

    // Tiles.
    for (auto const& [k, v] : tiles) v.draw(k.scale(pixel_size).to_vector2(), pixel_size);
    DrawTexturePro(
        *asset_manager.textures[TextureNames::Character1__Example],
        {0.f, 0.f, static_cast<float>(asset_manager.textures[TextureNames::Character1__Example]->width),
         static_cast<float>(asset_manager.textures[TextureNames::Character1__Example]->height)},
        {static_cast<float>(character_position.x * pixel_size), static_cast<float>(character_position.y * pixel_size),
         static_cast<float>(asset_manager.textures[TextureNames::Character1__Example]->width) * pixel_size,
         static_cast<float>(asset_manager.textures[TextureNames::Character1__Example]->height) * pixel_size},
        VECTOR2_ZERO, 0.f, WHITE);

    int group_index = 0;
    for (auto const& interactive_group : interactive_groups) {
      for (auto const& [tile_pos, tile_selection] : interactive_group.get_tiles()) {
        tile_selection.draw(tile_pos.scale(pixel_size).to_vector2(), pixel_size);
        DrawRectangleLinesEx(tile_selection.hitbox(tile_pos, pixel_size), pixel_size,
                             COLOR_LIST[group_index & COLOR_LIST_SIZE]);
      }

      for (auto const& behaviour : interactive_group.get_behaviours()) {
        for (auto const& [tile_pos, tile_selection] : interactive_group.get_tiles()) {
          Rectangle const elem_hitbox = tile_selection.hitbox(tile_pos, pixel_size);

          switch (behaviour.type) {
            case ObjectBehaviourType::HorizontalMovement:
              DrawLineEx({elem_hitbox.x, elem_hitbox.y},
                         {elem_hitbox.x + behaviour.movement_range * pixel_size, elem_hitbox.y}, pixel_size, RED);
              break;
            case ObjectBehaviourType::VerticalMovement:
              DrawLineEx({elem_hitbox.x, elem_hitbox.y},
                         {elem_hitbox.x, elem_hitbox.y + behaviour.movement_range * pixel_size}, pixel_size, RED);
              break;
            default:
              BAIL;
          }
        }
      }

      group_index++;
    }

    draw_grid();

    Vector2 mouse_pos = GetMousePosition();

    if (CheckCollisionPointRec(mouse_pos, game_area()) && mouse_pos.x < GetScreenWidth() - TOOLBAR_WIDTH) {
      if (special_operation == SpecialOperation::Nothing) {
        if (cluster_mode && IsMouseButtonDown(0)) {
          draw_cluster();
        } else {
          tile_selection.draw(Vector2{static_cast<float>(mod_reduced(mouse_pos.x, tile_selection.snap() * pixel_size)),
                                      static_cast<float>(mod_reduced(mouse_pos.y, tile_selection.snap() * pixel_size))},
                              pixel_size);
        }
      }
    }

    draw_gui();
  }

  void unload() {
    background.unload();

    const char** group_list_names_raw = group_list_names.data();
    for (int i = 0; i < static_cast<int>(group_list_names.size()); i++) {
      char* word = const_cast<char*>(group_list_names_raw[i]);
      free(word);
    }

    UnloadDirectoryFiles(map_files);
  }

 private:
  Background background{};
  TileSelection tile_selection{TileSource::Gui, {0, 0}};
  std::unordered_map<IntVec2, TileSelection> tiles{};
  int tile_width{32};
  int tile_height{20};
  int pixel_size{DEFAULT_PIXEL_SIZE};
  IntVec2 character_position{};
  std::vector<InteractiveGroup> interactive_groups{};
  int active_interactive_group{-1};
  SpecialOperation special_operation{SpecialOperation::Nothing};
  char filename[128]{""};
  int filename_index{0};
  std::vector<const char*> group_list_names{};
  FilePathList map_files;
  int grid_size_index{0};
  bool cluster_mode{false};
  Vector2 cluster_start{};

  void reset() {
    tiles.clear();
    interactive_groups.clear();
    special_operation = SpecialOperation::Nothing;
    active_interactive_group = -1;
  }

  void export_to_file() {
    FILE* file = std::fopen(filename, "w");
    if (!file) {
      TraceLog(LOG_ERROR, "Cannot create map file");
      return;
    }

    int values[6] = {MAP_FILE_WRITE_VERSION,
                     tile_width,
                     tile_height,
                     background.get_current_index(),
                     static_cast<int>(tiles.size()),
                     static_cast<int>(interactive_groups.size())};
    fwrite(values, sizeof(int), 6, file);

    character_position.write(file);

    for (auto const& [k, v] : tiles) {
      k.write(file);
      v.write(file);
    }

    for (auto const& interactive_group : interactive_groups) {
      interactive_group.write(file);
    }

    std::fclose(file);
  }

  void draw_cluster() {
    yield_cluster([this](TileSelection const tile_selection, IntVec2 const tile_pos) {
      tile_selection.draw(tile_pos, pixel_size);
    });
  }

  void yield_cluster(std::function<void(TileSelection const, IntVec2 const)> callback) {
    if (tile_selection.source == TileSource::Gui) {
      if (tile_selection.tile_coord.is_between({0, 0}, {2, 2})) {
        yield_cluster_3x3(tile_selection.source, {0, 0}, callback);
      }
      if (tile_selection.tile_coord.is_between({6, 0}, {8, 2})) {
        yield_cluster_3x3(tile_selection.source, {6, 0}, callback);
      }
      if (tile_selection.tile_coord.is_between({0, 4}, {2, 6})) {
        yield_cluster_3x3(tile_selection.source, {0, 4}, callback);
      }
      if (tile_selection.tile_coord.is_between({6, 4}, {8, 6})) {
        yield_cluster_3x3(tile_selection.source, {6, 4}, callback);
      }

      if (tile_selection.tile_coord.is_between({12, 0}, {14, 0})) {
        yield_cluster_3x1(tile_selection.source, {12, 0}, callback);
      }
      if (tile_selection.tile_coord.is_between({12, 2}, {14, 2})) {
        yield_cluster_3x1(tile_selection.source, {12, 2}, callback);
      }
      if (tile_selection.tile_coord.is_between({12, 4}, {14, 4})) {
        yield_cluster_3x1(tile_selection.source, {12, 4}, callback);
      }
      if (tile_selection.tile_coord.is_between({12, 6}, {14, 6})) {
        yield_cluster_3x1(tile_selection.source, {12, 6}, callback);
      }
    } else if (tile_selection.source == TileSource::Tileset) {
      if (tile_selection.tile_coord.is_between({0, 0}, {2, 2})) {
        yield_cluster_3x3(tile_selection.source, {0, 0}, callback);
      }
      if (tile_selection.tile_coord.is_between({6, 0}, {8, 2})) {
        yield_cluster_3x3(tile_selection.source, {6, 0}, callback);
      }
      if (tile_selection.tile_coord.is_between({0, 4}, {2, 6})) {
        yield_cluster_3x3(tile_selection.source, {0, 4}, callback);
      }
      if (tile_selection.tile_coord.is_between({6, 4}, {8, 6})) {
        yield_cluster_3x3(tile_selection.source, {6, 4}, callback);
      }
      if (tile_selection.tile_coord.is_between({0, 8}, {2, 10})) {
        yield_cluster_3x3(tile_selection.source, {0, 8}, callback);
      }
      if (tile_selection.tile_coord.is_between({6, 8}, {8, 10})) {
        yield_cluster_3x3(tile_selection.source, {6, 8}, callback);
      }

      if (tile_selection.tile_coord.is_between({12, 0}, {14, 0})) {
        yield_cluster_3x1(tile_selection.source, {12, 0}, callback);
      }
      if (tile_selection.tile_coord.is_between({12, 4}, {14, 4})) {
        yield_cluster_3x1(tile_selection.source, {12, 4}, callback);
      }
      if (tile_selection.tile_coord.is_between({12, 8}, {14, 8})) {
        yield_cluster_3x1(tile_selection.source, {12, 8}, callback);
      }
      if (tile_selection.tile_coord.is_between({12, 9}, {14, 9})) {
        yield_cluster_3x1(tile_selection.source, {12, 9}, callback);
      }
      if (tile_selection.tile_coord.is_between({12, 10}, {14, 10})) {
        yield_cluster_3x1(tile_selection.source, {12, 10}, callback);
      }

      if (tile_selection.tile_coord.is_between({15, 0}, {15, 2})) {
        yield_cluster_1x3(tile_selection.source, {15, 0}, callback);
      }
      if (tile_selection.tile_coord.is_between({15, 4}, {15, 6})) {
        yield_cluster_1x3(tile_selection.source, {15, 4}, callback);
      }
    }
  }

  void yield_cluster_3x3(TileSource const tile_source, IntVec2 const cluster_pos,
                         std::function<void(TileSelection const, IntVec2 const)> callback) {
    auto const endpoints = vec2_minmax(cluster_start, GetMousePosition());
    IntVec2 start_coord{tile_coord_from_absolute(endpoints.first, pixel_size)};
    IntVec2 end_coord{tile_coord_from_absolute(endpoints.second, pixel_size)};

    TileSelection ts_middle{tile_source, {cluster_pos.x + 1, cluster_pos.y + 1}};
    for (int y = start_coord.y + 1; y <= end_coord.y - 1; y++) {
      for (int x = start_coord.x + 1; x <= end_coord.x - 1; x++) {
        callback(ts_middle, {x, y});
      }
    }

    TileSelection ts_top{tile_source, {cluster_pos.x + 1, cluster_pos.y + 0}};
    for (int x = start_coord.x + 1; x <= end_coord.x - 1; x++) {
      callback(ts_top, {x, start_coord.y});
    }

    TileSelection ts_bottom{tile_source, {cluster_pos.x + 1, cluster_pos.y + 2}};
    for (int x = start_coord.x + 1; x <= end_coord.x - 1; x++) {
      callback(ts_bottom, {x, end_coord.y});
    }

    TileSelection ts_left{tile_source, {cluster_pos.x + 0, cluster_pos.y + 1}};
    for (int y = start_coord.y + 1; y <= end_coord.y - 1; y++) {
      callback(ts_left, {start_coord.x, y});
    }

    TileSelection ts_right{tile_source, {cluster_pos.x + 2, cluster_pos.y + 1}};
    for (int y = start_coord.y + 1; y <= end_coord.y - 1; y++) {
      callback(ts_right, {end_coord.x, y});
    }

    callback({tile_source, {cluster_pos.x + 0, cluster_pos.y + 0}}, {start_coord.x, start_coord.y});
    callback({tile_source, {cluster_pos.x + 2, cluster_pos.y + 0}}, {end_coord.x, start_coord.y});
    callback({tile_source, {cluster_pos.x + 0, cluster_pos.y + 2}}, {start_coord.x, end_coord.y});
    callback({tile_source, {cluster_pos.x + 2, cluster_pos.y + 2}}, {end_coord.x, end_coord.y});
  }

  void yield_cluster_3x1(TileSource const tile_source, IntVec2 const cluster_pos,
                         std::function<void(TileSelection const, IntVec2 const)> callback) {
    auto const endpoints = vec2_minmax_x(cluster_start, GetMousePosition());
    IntVec2 start_coord{tile_coord_from_absolute(endpoints.first, pixel_size)};
    IntVec2 end_coord{tile_coord_from_absolute(endpoints.second, pixel_size)};

    TileSelection ts_top{tile_source, {cluster_pos.x + 1, cluster_pos.y + 0}};
    for (int x = start_coord.x + 1; x <= end_coord.x - 1; x++) {
      callback(ts_top, {x, end_coord.y});
    }

    callback({tile_source, {cluster_pos.x + 0, cluster_pos.y + 0}}, {start_coord.x, end_coord.y});
    callback({tile_source, {cluster_pos.x + 2, cluster_pos.y + 0}}, {end_coord.x, end_coord.y});
  }

  void yield_cluster_1x3(TileSource const tile_source, IntVec2 const cluster_pos,
                         std::function<void(TileSelection const, IntVec2 const)> callback) {
    auto const endpoints = vec2_minmax_y(cluster_start, GetMousePosition());
    IntVec2 start_coord{tile_coord_from_absolute(endpoints.first, pixel_size)};
    IntVec2 end_coord{tile_coord_from_absolute(endpoints.second, pixel_size)};

    TileSelection ts_top{tile_source, {cluster_pos.x, cluster_pos.y + 1}};
    for (int y = start_coord.y + 1; y <= end_coord.y - 1; y++) {
      callback(ts_top, {end_coord.x, y});
    }

    callback({tile_source, {cluster_pos.x + 0, cluster_pos.y + 0}}, {end_coord.x, start_coord.y});
    callback({tile_source, {cluster_pos.x + 0, cluster_pos.y + 2}}, {end_coord.x, end_coord.y});
  }

  void draw_gui() {
    static const int toolbar_width{TOOLBAR_WIDTH};

    rlImGuiBegin();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - toolbar_width, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(toolbar_width, viewport->WorkSize.y));
    ImGui::Begin("Toolbar", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    draw_gui_pane_core();
    draw_gui_pane_walls();
    draw_gui_pane_boxes();
    draw_gui_pane_enemies();
    draw_gui_pane_traps();
    draw_gui_pane_gems();
    draw_gui_pane_checkpoint();
    draw_gui_pane_groups();

    ImGui::End();

    rlImGuiEnd();
  }

  void draw_gui_pane_core() {
    bool bgr_need_redraw{false};
    static int new_background_tile_index{0};

    if (ImGui::CollapsingHeader("Core", ImGuiTreeNodeFlags_DefaultOpen)) {
      bgr_need_redraw |= ImGui::SliderInt("Pixel size", &pixel_size, 1, 12);
      bgr_need_redraw |= ImGui::SliderInt("Tile width", &tile_width, 16, 64);
      bgr_need_redraw |= ImGui::SliderInt("Tile height", &tile_height, 16, 64);
      bgr_need_redraw |= ImGui::SliderInt("Background tile", &new_background_tile_index, 0, 5);
      ImGui::Combo("Grid", &grid_size_index, grid_size_names, sizeof(grid_size_names) / sizeof(grid_size_names[0]));

      if (bgr_need_redraw) background.preload(new_background_tile_index, tile_width, tile_height, pixel_size);

      ImGui::Separator();

      if (ImGui::Combo("Maps", &filename_index, map_files.paths, map_files.count)) {
        update_map_filename();
      }

      ImGui::InputText("filename", filename, IM_ARRAYSIZE(filename));

      if (ImGui::Button("Load")) load_from_file();

      ImGui::SameLine();

      if (ImGui::Button("Reset editor")) reset();

      ImGui::SameLine();

      if (ImGui::Button("Save")) {
        export_to_file();
        refresh_map_filenames();
      }
    }
  }

  void draw_gui_pane_walls() {
    if (ImGui::CollapsingHeader("Walls")) {
      ImGui::Checkbox("Cluster mode", &cluster_mode);

      rlImGuiImageSize(&*asset_manager.textures[TextureNames::GuiTiles],
                       asset_manager.textures[TextureNames::GuiTiles]->width * fixed_pixel_size,
                       asset_manager.textures[TextureNames::GuiTiles]->height * fixed_pixel_size);

      // Detect mouse click on the image
      if (ImGui::IsItemClicked()) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 itemRectMin = ImGui::GetItemRectMin();
        ImVec2 relativePos = {mousePos.x - itemRectMin.x, mousePos.y - itemRectMin.y};

        // TraceLog(LOG_INFO, "%.2f - %.2f", relativePos.x, relativePos.y);
        tile_selection = TileSelection{TileSource::Gui,
                                       {static_cast<int>(relativePos.x) / (TILE_SIZE * fixed_pixel_size),
                                        static_cast<int>(relativePos.y) / (TILE_SIZE * fixed_pixel_size)}};
      }

      ImGui::Separator();

      rlImGuiImageSize(&*asset_manager.textures[TextureNames::TilesetTiles],
                       asset_manager.textures[TextureNames::TilesetTiles]->width * fixed_pixel_size,
                       asset_manager.textures[TextureNames::TilesetTiles]->height * fixed_pixel_size);

      // Detect mouse click on the image
      if (ImGui::IsItemClicked()) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 itemRectMin = ImGui::GetItemRectMin();
        ImVec2 relativePos = {mousePos.x - itemRectMin.x, mousePos.y - itemRectMin.y};

        tile_selection = TileSelection{TileSource::Tileset,
                                       {static_cast<int>(relativePos.x) / (TILE_SIZE * fixed_pixel_size),
                                        static_cast<int>(relativePos.y) / (TILE_SIZE * fixed_pixel_size)}};
        // TraceLog(LOG_INFO, "Tileset: %d:%d", static_cast<int>(relativePos.x) / (TILE_SIZE * fixed_pixel_size),
        //          static_cast<int>(relativePos.y) / (TILE_SIZE * fixed_pixel_size));
      }
    }
  }

  void draw_gui_pane_boxes() {
    if (ImGui::CollapsingHeader("Boxes")) {
      if (rlImGuiImageButtonSize(
              "Box1", &*asset_manager.textures[TextureNames::Box1__Idle],
              {static_cast<float>(asset_manager.textures[TextureNames::Box1__Idle]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Box1__Idle]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Box1, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Box2", &*asset_manager.textures[TextureNames::Box2__Idle],
              {static_cast<float>(asset_manager.textures[TextureNames::Box2__Idle]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Box2__Idle]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Box2, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Box3", &*asset_manager.textures[TextureNames::Box3__Idle],
              {static_cast<float>(asset_manager.textures[TextureNames::Box3__Idle]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Box3__Idle]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Box3, {0, 0}};
      }
    }
  }

  void draw_gui_pane_enemies() {
    if (ImGui::CollapsingHeader("Enemies")) {
      if (rlImGuiImageButtonSize(
              "Enemy1", &*asset_manager.textures[TextureNames::Enemy1__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Enemy1__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Enemy1__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Enemy1, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Enemy2", &*asset_manager.textures[TextureNames::Enemy2__Fall],
              {static_cast<float>(asset_manager.textures[TextureNames::Enemy2__Fall]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Enemy2__Fall]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Enemy2, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Enemy3", &*asset_manager.textures[TextureNames::Enemy3__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Enemy3__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Enemy3__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Enemy3, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Enemy4", &*asset_manager.textures[TextureNames::Enemy4__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Enemy4__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Enemy4__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Enemy4, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Enemy5", &*asset_manager.textures[TextureNames::Enemy5__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Enemy5__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Enemy5__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Enemy5, {0, 0}};
      }
    }
  }

  void draw_gui_pane_traps() {
    if (ImGui::CollapsingHeader("Traps")) {
      if (rlImGuiImageButtonSize(
              "Trap1", &*asset_manager.textures[TextureNames::Trap1__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Trap1__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Trap1__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Trap1, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Trap2", &*asset_manager.textures[TextureNames::Trap2__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Trap2__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Trap2__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Trap2, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Trap4", &*asset_manager.textures[TextureNames::Trap4__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Trap4__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Trap4__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Trap4, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Trap5", &*asset_manager.textures[TextureNames::Trap5__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Trap5__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Trap5__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Trap5, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Trap6", &*asset_manager.textures[TextureNames::Trap6__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Trap6__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Trap6__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Trap6, {0, 0}};
      }
    }
  }

  void draw_gui_pane_groups() {
    char group_name_buf[16]{};

    if (ImGui::CollapsingHeader("Group Management")) {
      if (ImGui::Button("+ New group")) interactive_groups.emplace_back();
      while (interactive_groups.size() > group_list_names.size()) {
        sprintf(group_name_buf, "Group %lu", group_list_names.size());
        char* new_group_name = strdup(group_name_buf);
        group_list_names.push_back(new_group_name);
      }

      ImGui::Separator();

      if (interactive_groups.size() == 0) {
        ImGui::Text("No groups");
        return;
      }

      const char** items = group_list_names.data();
      ImGui::Combo("Groups", &active_interactive_group, items, group_list_names.size());

      if (active_interactive_group < 0 || active_interactive_group >= static_cast<int>(interactive_groups.size())) {
        ImGui::Text("No group selected");
        return;
      }

      if (ImGui::Button("Add elem")) {
        special_operation = SpecialOperation::GroupElemSelect;
      }

      ImGui::Separator();

      const char* behaviour_names[] = {"Horizontal movement", "Vertical movement"};
      static int selected_behaviour{0};
      ImGui::Combo("Behaviour", &selected_behaviour, behaviour_names, IM_ARRAYSIZE(behaviour_names));
      if (ImGui::Button("Add behaviour")) {
        interactive_groups[active_interactive_group].add_behaviour(
            static_cast<ObjectBehaviourType>(selected_behaviour));
      }

      for (auto& behaviour : interactive_groups[active_interactive_group].get_behaviours_mut()) {
        switch (behaviour.type) {
          case ObjectBehaviourType::HorizontalMovement:
            ImGui::Text("Behaviour: horizontal movement");
            ImGui::SliderInt("Right movement", &behaviour.movement_range, 0, tile_width * TILE_SIZE);
            break;
          case ObjectBehaviourType::VerticalMovement:
            ImGui::Text("Behaviour: vertical movement");
            ImGui::SliderInt("Down movement", &behaviour.movement_range, 0, tile_height * TILE_SIZE);
            break;
          default:
            BAIL;
        }
      }
    }
  }

  void draw_gui_pane_gems() {
    if (ImGui::CollapsingHeader("Gems")) {
      if (rlImGuiImageButtonSize(
              "Gem1", &*asset_manager.textures[TextureNames::Gem1__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Gem1__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Gem1__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Gem1, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Gem2", &*asset_manager.textures[TextureNames::Gem2__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Gem2__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Gem2__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Gem2, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Gem3", &*asset_manager.textures[TextureNames::Gem3__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Gem3__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Gem3__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Gem3, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Gem4", &*asset_manager.textures[TextureNames::Gem4__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Gem4__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Gem4__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Gem4, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Gem5", &*asset_manager.textures[TextureNames::Gem5__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Gem5__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Gem5__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Gem5, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Gem6", &*asset_manager.textures[TextureNames::Gem6__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Gem6__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Gem6__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Gem6, {0, 0}};
      }
    }
  }

  void draw_gui_pane_checkpoint() {
    if (ImGui::CollapsingHeader("Checkpoints")) {
      if (rlImGuiImageButtonSize(
              "Checkpoint", &*asset_manager.textures[TextureNames::Checkpoint__Init],
              {static_cast<float>(asset_manager.textures[TextureNames::Checkpoint__Init]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Checkpoint__Init]->height *
                                  fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Checkpoint, {0, 0}};
      }

      ImGui::SameLine();

      if (rlImGuiImageButtonSize(
              "Pointer", &*asset_manager.textures[TextureNames::Pointer__Static],
              {static_cast<float>(asset_manager.textures[TextureNames::Pointer__Static]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Pointer__Static]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Pointer, {0, 0}};
      }
    }
  }

  Rectangle const game_area() const {
    return {0.f, 0.f, static_cast<float>(TILE_SIZE * tile_width * pixel_size),
            static_cast<float>(TILE_SIZE * tile_height * pixel_size)};
  }

  void update_map_filename() {
    strncpy(filename, map_files.paths[filename_index],
            std::min(127, static_cast<int>(strlen(map_files.paths[filename_index]))));
  }

  void refresh_map_filenames() {
    UnloadDirectoryFiles(map_files);
    refresh_map_filenames_without_free_memory();
  }

  void refresh_map_filenames_without_free_memory() {
    map_files = LoadDirectoryFilesEx("assets/maps", "mp", false);
  }

  void draw_grid() {
    if (grid_size_index == 0) return;

    const int grid_size = grid_sizes[grid_size_index];
    const int iter_x = (tile_width * TILE_SIZE) / grid_size;
    const int iter_y = (tile_height * TILE_SIZE) / grid_size;

    for (int i = 0; i <= iter_x; i++) {
      DrawLineEx({1.f * i * grid_size * pixel_size, 0.f},
                 {1.f * i * grid_size * pixel_size, 1.f * tile_height * TILE_SIZE * pixel_size}, 2.f, GRAY);
    }

    for (int i = 0; i <= iter_y; i++) {
      DrawLineEx({0.f, 1.f * i * grid_size * pixel_size},
                 {1.f * tile_width * TILE_SIZE * pixel_size, 1.f * i * grid_size * pixel_size}, 2.f, GRAY);
    }
  }
};
