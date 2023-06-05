//
// Created by henry on 4/12/23.
//

#ifndef HGAMETEMPLATE_GAME_H
#define HGAMETEMPLATE_GAME_H

#include <hagame/core/game.h>
#include <hagame/graphics/camera.h>
#include <hagame/utils/spatialMap.h>
#include <hagame/graphics/primitives/quad.h>
#include <hagame/graphics/primitives/line.h>
#include <hagame/graphics/mesh.h>
#include <hagame/utils/pathfinding.h>
#include "constants.h"

const hg::Vec2 TILE_SIZE(25, 25);

class Pathfinding : public hg::Game {
public:

#if HEADLESS
    Game(std::string name):
        hg::Game(name)
    {}
#else
    Pathfinding(std::string name, hg::Vec2i size):
        m_size(size),
        hg::Game(name)
    {}
#endif

protected:

    void onInit() override;
    void onBeforeUpdate() override;
    void onUpdate(double dt) override;
    void onAfterUpdate() override;
    void onDestroy() override;

private:

#if !HEADLESS

    enum class ModeType {
        Obstacle,
        Start,
        Goal
    };

    struct Mode {
        ModeType type;
        std::string name;
    };

    struct Tile {

        Tile(hg::Vec2i _pos, ModeType _type, float _weight):
            pos(_pos),
            type(_type),
            weight(_weight)
        {}

        hg::Vec2i pos;
        ModeType type;
        float weight = 1.0;
    };

    const std::vector<Mode> MODES {
        Mode{ModeType::Obstacle, "Obstacle"},
        Mode{ModeType::Start, "Start"},
        Mode{ModeType::Goal, "Goal"},
    };

    void renderTile(Tile tile);

    bool m_guiActive = false;

    ModeType m_mode;

    float m_cameraPanSpeed = 500;
    float m_cameraZoomSpeed = 100;

    hg::graphics::OrthographicCamera m_camera;
    hg::graphics::Window* m_window;
    hg::Vec2i m_size;
    hg::Vec2 m_rawMousePos;
    hg::Vec2i m_mousePos;

    long long m_lastTick;

    int m_ticksPerSecond = 60;

    bool m_running = false;

    bool m_hasError = false;
    std::string m_error;

    std::unique_ptr<hg::graphics::primitives::Quad> m_quad;
    std::unique_ptr<hg::graphics::MeshInstance> m_mesh;

    std::unique_ptr<hg::graphics::primitives::Line> m_line;
    std::unique_ptr<hg::graphics::MeshInstance> m_pathMesh;

    std::unique_ptr<hg::utils::PathFinding<std::vector<Tile>>> m_pathfinding;

    std::unique_ptr<Tile> m_start;
    std::unique_ptr<Tile> m_end;

    float m_weight = 1.0;

    hg::utils::SpatialMap2D<Tile, int> m_tiles;
#endif

};

#endif //HGAMETEMPLATE_GAME_H
