//
// Created by henry on 4/12/23.
//
#include "Pathfinding.h"

#include <hagame/graphics/windows.h>
#include <hagame/core/assets.h>

#if USE_IMGUI
#include "imgui.h"
#include "imgui_demo.cpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#endif

using namespace hg;
using namespace hg::graphics;

void Pathfinding::onInit() {
#if !HEADLESS
    m_window = Windows::Create(GAME_NAME, m_size);

    Windows::Events.subscribe(WindowEvents::Close, [&](Window* window) {
        running(false);
    });

    Windows::Events.subscribe(WindowEvents::Resize, [&](Window* window) {

    });

    m_quad = std::make_unique<primitives::Quad>(TILE_SIZE, TILE_SIZE * 0.5);
    m_mesh = std::make_unique<MeshInstance>(m_quad.get());

    m_line = std::make_unique<primitives::Line>();
    m_pathMesh = std::make_unique<MeshInstance>(m_line.get());

    m_camera = OrthographicCamera();
    m_camera.centered = true;
    m_camera.size = m_size;

    m_pathfinding = std::make_unique<hg::utils::PathFinding>([&](hg::utils::PathFinding::Node node) {
        std::vector<hg::utils::PathFinding::Node> neighbors;
        for (const auto& rawNeighbor : m_tiles.getNeighbors(node.position)) {
            hg::utils::PathFinding::Node neighbor;
            neighbor.position = rawNeighbor.index;

            if (rawNeighbor.value.size() == 0) {

                neighbor.cost = 0;
            } else {

                if (rawNeighbor.value[0].type == ModeType::Wall) {
                    continue;
                }
                neighbor.cost = rawNeighbor.value[0].weight;
            }

            neighbors.push_back(neighbor);
        }
        return neighbors;
    });

    hg::loadShaders({
        "shaders/color",
        "shaders/sprite",
    });

    hg::loadTextures({
        "textures/flag.png",
        "textures/start.png",
    });

#endif

#if USE_IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(m_window->window(), true);
    ImGui_ImplOpenGL3_Init("#version 300 es");
#endif
}

void Pathfinding::onBeforeUpdate() {
#if !HEADLESS
    m_window->setVSync(false);
    m_window->clear();
#endif

#if USE_IMGUI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
#endif
}

void Pathfinding::onAfterUpdate() {
#if USE_IMGUI
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

#if !HEADLESS

    auto colorShader = getShader("color");
    auto spriteShader = getShader("sprite");

    colorShader->use();
    colorShader->setMat4("model", Mat4::Identity());
    colorShader->setMat4("view", m_camera.view());
    colorShader->setMat4("projection", m_camera.projection());

    spriteShader->use();
    spriteShader->setMat4("model", Mat4::Identity());
    spriteShader->setMat4("view", m_camera.view());
    spriteShader->setMat4("projection", m_camera.projection());

    colorShader->use();

    for (const auto& tile : m_pathfinding->m_closedList) {
        colorShader->setVec4("color", Color::green());
        colorShader->setMat4("model", Mat4::Translation(tile->position.cast<float>().prod(TILE_SIZE).cast<float>().resize<3>()));
        m_mesh->render();
    }

    for (const auto& tile : m_pathfinding->m_openList) {
        colorShader->setVec4("color", Color::blue());
        colorShader->setMat4("model", Mat4::Translation(tile->position.cast<float>().prod(TILE_SIZE).cast<float>().resize<3>()));
        m_mesh->render();
    }

    auto rawMousePos = m_window->input.keyboardMouse.mouse.position;
    rawMousePos[1] = m_window->size()[1] - rawMousePos[1];
    m_rawMousePos = m_camera.getGamePos(rawMousePos);
    m_mousePos = m_rawMousePos.div(TILE_SIZE).floor().cast<int>();

    m_tiles.forEach([&](Vec2i pos, Tile tile) {
        renderTile(tile);
    });

    if (m_pathfinding->finished() && m_pathfinding->m_foundPath) {
        colorShader->setVec4("color", Color::red());
        colorShader->setMat4("model", Mat4::Identity());
        m_pathMesh->render();
    }

    if (m_start != nullptr) {
        renderTile(*m_start);
    }

    if (m_end != nullptr) {
        renderTile(*m_end);
    }

    if (!m_guiActive) {
        renderTile(Tile{m_mousePos, m_mode, m_weight});
    }

    m_mesh->render();

    m_window->render();
#endif
}

void Pathfinding::onDestroy() {
    // CLEAN UP
}

void Pathfinding::onUpdate(double dt) {

    m_camera.transform.position += m_window->input.keyboardMouse.lAxis.resize<3>() * m_cameraPanSpeed * dt;
    m_camera.zoom += m_window->input.keyboardMouse.mouse.wheel * m_cameraZoomSpeed * dt;

    ImGui::Begin("Demo Window");

    ImGui::Text(("Hovered? " + std::to_string(ImGui::IsWindowHovered())).c_str());
    ImGui::Text(("Focused? " + std::to_string(ImGui::IsAnyItemActive())).c_str());

    m_guiActive = ImGui::IsWindowHovered() || ImGui::IsAnyItemActive();

    if (!m_guiActive) {
        if (m_window->input.keyboardMouse.mouse.left) {
            if (m_mode == ModeType::Obstacle || m_mode == ModeType::Wall) {
                if (m_tiles.get(m_mousePos).value.size() == 0) {
                    m_tiles.insert(m_mousePos, Tile(m_mousePos, m_mode, m_weight));
                }
            } else if (m_mode == ModeType::Start) {
                m_start = std::make_unique<Tile>(m_mousePos, m_mode, m_weight);
            } else if (m_mode == ModeType::Goal) {
                m_end = std::make_unique<Tile>(m_mousePos, m_mode, m_weight);
            };
        }

        if (m_window->input.keyboardMouse.mouse.right) {
            m_tiles.clear(m_mousePos);
        }
    }

    // FILL ME IN!


    ImGui::Text(("DT: " + std::to_string(dt)).c_str());
    ImGui::Text(("Mouse Pos: " + (std::string) m_mousePos).c_str());
    ImGui::DragFloat("Weight", &m_weight, 0.1f, 0.0, MAX_WEIGHT);
    ImGui::DragInt("Ticks Per Second", &m_ticksPerSecond, 1, 1, 1000);

    for (const auto& mode : MODES) {
        if (ImGui::RadioButton(mode.name.c_str(), mode.type == m_mode)) {
            m_mode = mode.type;
        }
    }

    if (!m_running) {
        if (ImGui::Button("Start!")) {
            if (m_start == nullptr || m_end == nullptr) {
                m_hasError = true;
                m_error = "Start and Goal must be placed to run pathfinding";
            } else {
                m_running = true;
                m_hasError = false;

                m_pathfinding->start(m_start->pos, m_end->pos);

                m_lastTick = utils::Clock::Now();
            }
        }
    } else {
        if (m_pathfinding->finished()) {
            m_running = false;

            if (m_pathfinding->m_foundPath) {
                m_line->clearPoints();
                auto path = m_pathfinding->constructPath(m_pathfinding->m_current.get());
                for (auto& point : path) {
                    m_line->addPoint((point.cast<float>().prod(TILE_SIZE) + TILE_SIZE * 0.5).resize<3>());
                }
                m_pathMesh->update(m_line.get());
            }

        } else {

            float tickRate = 1.0f / m_ticksPerSecond;
            double timeSinceLastTick = utils::Clock::ToSeconds(utils::Clock::Now() - m_lastTick);

            if (timeSinceLastTick >= tickRate) {
                m_pathfinding->tick();
                m_lastTick = utils::Clock::Now();
            }

        }
    }

    if (m_hasError) {
        ImGui::Text(("Error: " + m_error).c_str());
    }

    ImGui::End();
}

void Pathfinding::renderTile(Tile tile) {

    ShaderProgram* shader;

    if (tile.type == ModeType::Wall) {
        shader = getShader("color");
        shader->use();
        shader->setVec4("color", Color::white());
    } else if (tile.type == ModeType::Obstacle) {
        shader = getShader("color");
        shader->use();
        shader->setVec4("color", Color(tile.weight / MAX_WEIGHT, tile.weight / MAX_WEIGHT, tile.weight / MAX_WEIGHT, 1.0f));
    } else if (tile.type == ModeType::Goal) {
        shader = getShader("sprite");
        shader->use();
        getTexture("flag")->bind();
    } else if (tile.type == ModeType::Start) {
        shader = getShader("sprite");
        shader->use();
        getTexture("start")->bind();
    }

    shader->setMat4("view", m_camera.view());
    shader->setMat4("projection", m_camera.projection());
    shader->setMat4("model", Mat4::Translation(tile.pos.cast<float>().prod(TILE_SIZE).cast<float>().resize<3>()));

    m_mesh->render();
}
