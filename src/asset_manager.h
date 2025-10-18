#pragma once

#include <memory>
#include <unordered_map>

#include "raylib.h"

constexpr int BACKGROUND_COUNT{6};

enum TextureNames {
  Character1__Run,
  Character1__Idle,
  Character1__Hit,
  Character1__Jump,
  Character1__Fall,
  Character1__Double_Jump,
  Character1__Wall_Jump,
  Character1__Example,
  Character__Appear,
  Background__0,
  Background__1,
  Background__2,
  Background__3,
  Background__4,
  Background__5,
  GuiTiles,
  TilesetTiles,
  Box1__Idle,
  Box2__Idle,
  Box3__Idle,
  Enemy1__Example,
  Enemy1__Fall,
  Enemy1__Hit,
  Enemy1__Idle,
  Enemy1__Jump,
  Enemy1__Run,
  Enemy2__Fall,
  Enemy2__Hit,
  Enemy2__Idle,
  Enemy2__Jump,
  Enemy2__Run,
  Enemy3__Example,
  Enemy3__Charge,
  Enemy3__Hit,
  Enemy3__Idle,
  Enemy3__Stun,
  Enemy3__Walk,
};

struct AssetManager {
 public:
  std::unordered_map<int, std::shared_ptr<Texture2D>> textures{};

  // Must be the last thing called.
  void unload_assets() {
    TraceLog(LOG_INFO, "Unload all textures");

    for (auto [k, v] : textures) UnloadTexture(*v);
  }

  void preload() {
    textures[TextureNames::Character1__Run] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Run.png"));
    textures[TextureNames::Character1__Idle] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Idle.png"));
    textures[TextureNames::Character1__Hit] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Hit.png"));
    textures[TextureNames::Character1__Jump] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Jump.png"));
    textures[TextureNames::Character1__Fall] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Fall.png"));
    textures[TextureNames::Character1__Double_Jump] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Double_Jump.png"));
    textures[TextureNames::Character1__Wall_Jump] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Wall_Jump.png"));
    textures[TextureNames::Character1__Example] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/1/Example.png"));

    textures[TextureNames::Character__Appear] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/1 Main Characters/Appearing.png"));

    textures[TextureNames::Background__0] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Backgrounds/1.png"));
    textures[TextureNames::Background__1] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Backgrounds/2.png"));
    textures[TextureNames::Background__2] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Backgrounds/3.png"));
    textures[TextureNames::Background__3] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Backgrounds/4.png"));
    textures[TextureNames::Background__4] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Backgrounds/5.png"));
    textures[TextureNames::Background__5] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Backgrounds/6.png"));

    textures[TextureNames::GuiTiles] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/GUI.png"));
    textures[TextureNames::TilesetTiles] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/7 Levels/Tiled/Tileset.png"));

    textures[TextureNames::Box1__Idle] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/3 Objects/Boxes/1_Idle.png"));
    textures[TextureNames::Box2__Idle] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/3 Objects/Boxes/2_Idle.png"));
    textures[TextureNames::Box3__Idle] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/3 Objects/Boxes/3_Idle.png"));

    textures[TextureNames::Enemy1__Example] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/1/Example.png"));
    textures[TextureNames::Enemy1__Fall] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/1/Fall.png"));
    textures[TextureNames::Enemy1__Hit] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/1/Hit.png"));
    textures[TextureNames::Enemy1__Idle] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/1/Idle.png"));
    textures[TextureNames::Enemy1__Jump] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/1/Jump.png"));
    textures[TextureNames::Enemy1__Run] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/1/Run.png"));

    textures[TextureNames::Enemy2__Fall] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/2/Fall.png"));
    textures[TextureNames::Enemy2__Hit] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/2/Hit.png"));
    textures[TextureNames::Enemy2__Idle] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/2/Idle.png"));
    textures[TextureNames::Enemy2__Jump] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/2/Jump.png"));
    textures[TextureNames::Enemy2__Run] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/2/Run.png"));

    textures[TextureNames::Enemy3__Example] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/3/Example.png"));
    textures[TextureNames::Enemy3__Charge] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/3/Charge.png"));
    textures[TextureNames::Enemy3__Hit] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/3/Hit.png"));
    textures[TextureNames::Enemy3__Idle] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/3/Idle.png"));
    textures[TextureNames::Enemy3__Stun] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/3/Stun.png"));
    textures[TextureNames::Enemy3__Walk] =
        std::make_shared<Texture2D>(LoadTexture("assets/craftpixnet/4 Enemies/3/Walk.png"));
  }

 private:
};

static AssetManager asset_manager{};
