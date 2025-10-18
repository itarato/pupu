#pragma once

#include <algorithm>

#include "../asset_manager.h"
#include "../background.h"
#include "../common.h"
#include "common.h"
#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"

constexpr const int fixed_pixel_size{2};

struct Editor {
 public:
  Editor() {
    background.preload(0, tile_width, tile_height, pixel_size);
  }

  void load_from_file() {
    reset();

    FILE* file = std::fopen("assets/maps/map.mp", "r");
    if (!file) BAILF("Cannot open map file");

    int background_index{};
    int tiles_count{};
    if (std::fread(&tile_width, sizeof(int), 1, file) != 1) BAIL;
    if (std::fread(&tile_height, sizeof(int), 1, file) != 1) BAIL;
    if (std::fread(&background_index, sizeof(int), 1, file) != 1) BAIL;
    if (std::fread(&tiles_count, sizeof(int), 1, file) != 1) BAIL;

    character_position = intvec2_from_file(file);

    background.preload(background_index, tile_width, tile_height, pixel_size);

    for (int i = 0; i < tiles_count; i++) {
      IntVec2 tile_pos = intvec2_from_file(file);
      TileSelection tile_selection{tile_selection_from_file(file)};

      tiles[tile_pos] = tile_selection;
    }

    std::fclose(file);
  }

  void update() {
    Vector2 mouse_pos = GetMousePosition();

    if (IsMouseButtonDown(0)) {
      if (CheckCollisionPointRec(mouse_pos, game_area())) {
        // Draw tile.
        IntVec2 int_coord{mod_reduced(mouse_pos.x, tile_selection.snap() * pixel_size) / pixel_size,
                          mod_reduced(mouse_pos.y, tile_selection.snap() * pixel_size) / pixel_size};
        tiles[int_coord] = tile_selection;
      }
    }

    if (IsMouseButtonDown(1)) {
      // Erase tile.
      if (CheckCollisionPointRec(mouse_pos, game_area())) {
        std::erase_if(tiles, [&](const auto& p) {
          IntVec2 const& pos = p.first;
          TileSelection const& selection = p.second;

          const Rectangle hitbox = upscale(selection.hitbox(pos), pixel_size);
          return CheckCollisionPointRec(mouse_pos, hitbox);
        });
      }
    }

    if (IsMouseButtonDown(2)) {
      character_position.set_from_vector2(GetMousePosition());
    }
  }

  void draw() {
    // Background.
    background.draw({0.f, 0.f}, pixel_size);

    // Tiles.
    for (auto const& [k, v] : tiles) v.draw(k.scale(pixel_size).to_vector2(), pixel_size);
    DrawTexturePro(*asset_manager.textures[TextureNames::Character1__Example],
                   {0.f, 0.f, static_cast<float>(asset_manager.textures[TextureNames::Character1__Example]->width),
                    static_cast<float>(asset_manager.textures[TextureNames::Character1__Example]->height)},
                   {static_cast<float>(character_position.x), static_cast<float>(character_position.y),
                    static_cast<float>(asset_manager.textures[TextureNames::Character1__Example]->width) * pixel_size,
                    static_cast<float>(asset_manager.textures[TextureNames::Character1__Example]->height) * pixel_size},
                   vector_zero, 0.f, WHITE);

    Vector2 mouse_pos = GetMousePosition();

    if (CheckCollisionPointRec(mouse_pos, game_area())) {
      tile_selection.draw({static_cast<float>(mod_reduced(mouse_pos.x, tile_selection.snap() * pixel_size)),
                           static_cast<float>(mod_reduced(mouse_pos.y, tile_selection.snap() * pixel_size))},
                          pixel_size);
    }

    draw_gui();
  }

  void unload() {
    background.unload();
  }

 private:
  Background background{};
  TileSelection tile_selection{TileSource::Gui, {0, 0}};
  std::unordered_map<IntVec2, TileSelection> tiles{};
  int tile_width{32};
  int tile_height{20};
  int pixel_size{DEFAULT_PIXEL_SIZE};
  const Rectangle background_frame{0.f, 0.f, static_cast<float>(TILE_SIZE* tile_width),
                                   static_cast<float>(TILE_SIZE* tile_height)};
  const Rectangle gui_tile_frame{0.f, background_frame.height, 240.f, 112.f};
  const Rectangle tileset_tile_frame{gui_tile_frame.width, background_frame.height, 256.f, 176.f};
  const Rectangle gui_tile_frame_px = upscale(gui_tile_frame, pixel_size);
  const Rectangle tileset_tile_frame_px = upscale(tileset_tile_frame, pixel_size);
  IntVec2 character_position{};

  void reset() {
    tiles.clear();
  }

  void export_to_file() {
    const char* filename{"assets/maps/map.mp"};

    FILE* file = std::fopen(filename, "w");
    if (!file) {
      TraceLog(LOG_ERROR, "Cannot create map file");
      return;
    }

    int values[4] = {tile_width, tile_height, background.get_current_index(), static_cast<int>(tiles.size())};
    fwrite(values, sizeof(int), 4, file);

    character_position.write(file);

    for (auto const& [k, v] : tiles) {
      k.write(file);
      v.write(file);
    }

    std::fclose(file);
  }

  void draw_gui() {
    static const int toolbar_width{600};

    rlImGuiBegin();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - toolbar_width, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(toolbar_width, viewport->WorkSize.y));
    ImGui::Begin("Toolbar", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    draw_gui_pane_core();
    draw_gui_pane_walls();
    draw_gui_pane_boxes();
    draw_gui_pane_enemies();

    ImGui::End();

    rlImGuiEnd();
  }

  void draw_gui_pane_core() {
    bool bgr_need_redraw{false};
    static int new_background_tile_index{0};

    if (ImGui::CollapsingHeader("Core")) {
      bgr_need_redraw |= ImGui::SliderInt("Pixel size", &pixel_size, 1, 12);
      bgr_need_redraw |= ImGui::SliderInt("Tile width", &tile_width, 16, 64);
      bgr_need_redraw |= ImGui::SliderInt("Tile height", &tile_height, 16, 64);
      bgr_need_redraw |= ImGui::SliderInt("Background tile", &new_background_tile_index, 0, 5);

      if (bgr_need_redraw) background.preload(new_background_tile_index, tile_width, tile_height, pixel_size);

      if (ImGui::Button("Reset editor")) reset();
      if (ImGui::Button("Save")) export_to_file();
    }
  }

  void draw_gui_pane_walls() {
    if (ImGui::CollapsingHeader("Walls")) {
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
        TraceLog(LOG_INFO, "Tileset: %d:%d", static_cast<int>(relativePos.x) / (TILE_SIZE * fixed_pixel_size),
                 static_cast<int>(relativePos.y) / (TILE_SIZE * fixed_pixel_size));
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

      if (rlImGuiImageButtonSize(
              "Box2", &*asset_manager.textures[TextureNames::Box2__Idle],
              {static_cast<float>(asset_manager.textures[TextureNames::Box2__Idle]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Box2__Idle]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Box2, {0, 0}};
      }

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

      if (rlImGuiImageButtonSize(
              "Enemy2", &*asset_manager.textures[TextureNames::Enemy2__Fall],
              {static_cast<float>(asset_manager.textures[TextureNames::Enemy2__Fall]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Enemy2__Fall]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Enemy2, {0, 0}};
      }

      if (rlImGuiImageButtonSize(
              "Enemy3", &*asset_manager.textures[TextureNames::Enemy3__Example],
              {static_cast<float>(asset_manager.textures[TextureNames::Enemy3__Example]->width * fixed_pixel_size),
               static_cast<float>(asset_manager.textures[TextureNames::Enemy3__Example]->height * fixed_pixel_size)})) {
        tile_selection = TileSelection{TileSource::Enemy3, {0, 0}};
      }
    }
  }

  Rectangle const game_area() const {
    return upscale(background_frame, pixel_size);
  }
};
