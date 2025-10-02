#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <variant>
#include <string>
#include <fstream>
#include <sstream>
#include <random>
#include <iterator>
#include <cfloat>
#include <unordered_map>

// Logging system
enum class LogCategory {
    GAME_FLOW,
    DICE_ALLOCATION,
    MOVEMENT,
    KNOCKBACK,
    CAREER_BONUS,
    CAREER_LANDING,
    CLICK_DEBUG,
    SYSTEM
};

class Logger {
private:
    std::unordered_map<LogCategory, bool> enabledCategories;

public:
    Logger() {
        // Default: all categories disabled except CAREER_LANDING
        enabledCategories[LogCategory::GAME_FLOW] = false;
        enabledCategories[LogCategory::DICE_ALLOCATION] = false;
        enabledCategories[LogCategory::MOVEMENT] = false;
        enabledCategories[LogCategory::KNOCKBACK] = false;
        enabledCategories[LogCategory::CAREER_BONUS] = false;
        enabledCategories[LogCategory::CAREER_LANDING] = true;  // Only tile landings
        enabledCategories[LogCategory::CLICK_DEBUG] = false;
        enabledCategories[LogCategory::SYSTEM] = false;
    }

    void setEnabled(LogCategory category, bool enabled) {
        enabledCategories[category] = enabled;
    }

    bool isEnabled(LogCategory category) const {
        auto it = enabledCategories.find(category);
        return it != enabledCategories.end() && it->second;
    }

    void log(LogCategory category, const std::string& message) {
        if (isEnabled(category)) {
            std::cout << message << std::endl;
        }
    }
};

// Global logger instance
Logger g_logger;

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>

#include <flecs.h>

#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.h"

// ECS Components
struct Position {
    float x, y;
};

// Tags for local/world position
struct Local { };
struct World { };

struct Velocity {
    float dx, dy;
};

struct Bounds {
    float minX, maxX, minY, maxY;
};

struct MousePosition {
    float x, y;
    MousePosition() : x(0.0f), y(0.0f) {}
    MousePosition(float mx, float my) : x(mx), y(my) {}
};

struct UIElementBounds {
    float xmin, ymin, xmax, ymax;

    UIElementBounds() : xmin(FLT_MAX), ymin(FLT_MAX), xmax(-FLT_MAX), ymax(-FLT_MAX) {}
    UIElementBounds(float minX, float minY, float maxX, float maxY)
        : xmin(minX), ymin(minY), xmax(maxX), ymax(maxY) {}
};

struct RenderStatus {
    bool visible;
    RenderStatus() : visible(true) {}
    RenderStatus(bool vis) : visible(vis) {}
};

struct RectRenderable {
    float width, height;
    uint32_t color;
};

struct TextRenderable {
    std::string text;
    std::string fontFace;
    float fontSize;
    uint32_t color;
    int alignment;
};

struct ImageRenderable {
    int imageHandle;
    float width, height;
    float alpha;
};

struct CircleRenderable {
    float radius;
    uint32_t fillColor;
    uint32_t strokeColor;
    float strokeWidth;
};

struct ZIndex {
    int layer;
};

struct CreateSprite {
    std::string imagePath;
    float alpha;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    bool scaleToWindow;
};

struct DieRoll {
    int currentValue;
    DieRoll() : currentValue(1) {}
    DieRoll(int value) : currentValue(value) {}
};

struct BoardPosition {
    int tileIndex;
    BoardPosition() : tileIndex(0) {}
    BoardPosition(int index) : tileIndex(index) {}
};

struct MovementAnimation {
    Position startPos;
    Position targetPos;
    float duration;
    float elapsed;
    bool active;

    MovementAnimation() : startPos({0, 0}), targetPos({0, 0}), duration(0.3f), elapsed(0.0f), active(false) {}
    MovementAnimation(Position start, Position target, float dur = 0.3f)
        : startPos(start), targetPos(target), duration(dur), elapsed(0.0f), active(true) {}
};

struct FloatingText {
    std::string text;
    float duration;
    float elapsed;
    float riseSpeed;
    bool active;

    FloatingText() : text(""), duration(1.0f), elapsed(0.0f), riseSpeed(50.0f), active(false) {}
    FloatingText(const std::string& txt, float dur = 1.0f, float speed = 50.0f)
        : text(txt), duration(dur), elapsed(0.0f), riseSpeed(speed), active(true) {}
};

enum EmployeeState {
    RECRUIT,     // In recruitment area, not on board
    ACTIVE,      // On the board and can move
    ELIMINATED   // Removed from play
};

struct Employee {
    EmployeeState state;
    int allocatedDice;
    int playerIndex;

    Employee() : state(RECRUIT), allocatedDice(0), playerIndex(0) {}
    Employee(int player) : state(RECRUIT), allocatedDice(0), playerIndex(player) {}
};

struct DiceAllocation {
    std::vector<int> dicePerEmployee; // Dice allocated to each employee (indices match employee lists)
    std::vector<int> diceValues; // Individual dice values (hidden until allocation complete)
    int availableDice;
    int currentDieIndex; // Which die is currently being allocated (0 or 1)
    bool allocationPhase;

    DiceAllocation() : availableDice(0), currentDieIndex(0), allocationPhase(false) {}
};

struct Player {
    std::string name;
    int money;
    int tech;
    int recruit;
    Player() : name("Player"), money(10), tech(0), recruit(0) {}
    Player(const std::string& playerName) : name(playerName), money(10), tech(0), recruit(0) {}
    Player(const std::string& playerName, int startMoney, int startTech, int startRecruit)
        : name(playerName), money(startMoney), tech(startTech), recruit(startRecruit) {}
};

struct Clickable {
    c2AABB bounds;
    std::string clickEventType;
    bool wasClicked;
    bool boundsNeedUpdate;

    Clickable() : clickEventType("click"), wasClicked(false), boundsNeedUpdate(true) {
        bounds.min = {0, 0};
        bounds.max = {0, 0};
    }

    Clickable(const std::string& eventType) : clickEventType(eventType), wasClicked(false), boundsNeedUpdate(true) {
        bounds.min = {0, 0};
        bounds.max = {0, 0};
    }
};

struct ClickEvent {
    flecs::entity clickedEntity;
    std::string eventType;
    float mouseX, mouseY;
};

struct DebugRender {
    bool enabled;
    DebugRender() : enabled(false) {}
};

struct GameState {
    int currentPlayerIndex;
    std::vector<flecs::entity> playerEntities;
    std::vector<std::vector<flecs::entity>> allEmployeeEntities; // [playerIndex][employeeIndex]
    DiceAllocation currentAllocation;

    GameState() : currentPlayerIndex(0) {
        allEmployeeEntities.resize(4); // 4 players
        currentAllocation.dicePerEmployee.resize(2); // 2 employees per player initially
        currentAllocation.diceValues.resize(2); // 2 dice per player
    }
};

// Tile type constants (matching Python script)
enum TileType {
    TILE_NORMAL = 0,
    TILE_HANDCUFF = 1,
    TILE_HANDCUFF_PATH = 2,
    TILE_SALES = 3,
    TILE_HR = 4,
    TILE_ENG = 5
};

const char* getTileTypeName(int tileType) {
    switch(tileType) {
        case TILE_NORMAL: return "normal";
        case TILE_HANDCUFF: return "handcuff";
        case TILE_HANDCUFF_PATH: return "handcuff_path";
        case TILE_SALES: return "sales";
        case TILE_HR: return "hr";
        case TILE_ENG: return "eng";
        default: return "unknown";
    }
}

struct BoardGrid {
    std::vector<std::vector<int>> tiles;
    int width;
    int height;
    float tileSize;

    BoardGrid() : width(0), height(0), tileSize(60.0f) {}

    bool loadFromCSV(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open tile map file: " << filename << std::endl;
            return false;
        }

        tiles.clear();
        std::string line;

        while (std::getline(file, line)) {
            std::vector<int> row;
            std::stringstream ss(line);
            std::string cell;

            while (std::getline(ss, cell, ',')) {
                row.push_back(std::stoi(cell));
            }

            if (!row.empty()) {
                tiles.push_back(row);
            }
        }

        if (!tiles.empty()) {
            height = tiles.size();
            width = tiles[0].size();
            g_logger.log(LogCategory::SYSTEM, "Loaded tile map: " + std::to_string(width) + "x" + std::to_string(height) + " tiles");
            return true;
        }

        return false;
    }

    int getTileAt(int col, int row) const {
        if (row >= 0 && row < height && col >= 0 && col < width) {
            return tiles[row][col];
        }
        return -1; // Invalid tile
    }

    // Check if a tile coordinate is within the valid playable area
    bool isValidTile(int col, int row) const {
        return (row >= 0 && row < height && col >= 0 && col < width);
    }

    // Check if a linear index corresponds to a valid playable tile
    bool isValidIndex(int index) const {
        auto [col, row] = indexToTileCoords(index);
        return isValidTile(col, row);
    }

    // Convert mouse position to tile coordinates
    std::pair<int, int> mouseToTile(double mouseX, double mouseY, int windowWidth, int windowHeight) const {
        // Board image dimensions: 769 x 1361
        float boardImageWidth = 769.0f;
        float boardImageHeight = 1361.0f;

        // Tile grid offset within the board image
        float tileGridOffsetX = 0.0f;  // Corrected from 771
        float tileGridOffsetY = 30.0f;

        // Calculate actual tile dimensions based on loaded CSV dimensions
        float tileWidth = boardImageWidth / width;
        float tileHeight = boardImageHeight / height;

        // Convert mouse coordinates to board image space
        float tileGridX = (mouseX - tileGridOffsetX);
        float tileGridY = (mouseY - tileGridOffsetY);

        // Convert to tile coordinates
        int tileCol = static_cast<int>(tileGridX / tileWidth);
        int tileRow = static_cast<int>(tileGridY / tileHeight);

        return {tileCol, tileRow};
    }

    // Convert linear index to tile coordinates using snake pattern
    // Index 0 = bottom-left, snakes right then left on next row up, etc.
    std::pair<int, int> indexToTileCoords(int index) const {
        if (index < 0 || index >= width * height) {
            return {-1, -1}; // Invalid index
        }

        int row = index / width;
        int col;

        // Snake pattern: even rows go left-to-right, odd rows go right-to-left
        if (row % 2 == 0) {
            col = index % width;
        } else {
            col = width - 1 - (index % width);
        }

        // Convert to bottom-up coordinates (row 0 = bottom)
        int bottomUpRow = height - 1 - row;

        return {col, bottomUpRow};
    }

    // Get center position of tile at given index, accounting for window scaling
    Position getTileCenterPosition(int index) const {
        auto [col, row] = indexToTileCoords(index);

        if (col == -1 || row == -1 || !isValidTile(col, row)) {
            return {-1.0f, -1.0f}; // Invalid position
        }

        // Board image dimensions
        float boardImageWidth = 769.0f;
        float boardImageHeight = 1361.0f;
    
        float tileGridOffsetX = 771.0f; 
        float tileGridOffsetY = 30.0f;

        float tileWidth = boardImageWidth / (width+2);
        float tileHeight = boardImageHeight / (height+2);

        // Calculate center position in board image space
        float centerX = tileGridOffsetX + col * tileWidth + tileWidth * 1.5f;
        float centerY = tileGridOffsetY + row * tileHeight + tileHeight * 1.5f;

        return {centerX, centerY};
    }
};

struct Window {
    GLFWwindow* handle;
    int width, height;
};

struct Graphics {
    NVGcontext* vg;
};

enum class RenderType {
    Rectangle,
    Text,
    Image,
    Circle
};

struct RenderCommand {
    Position pos;
    std::variant<RectRenderable, TextRenderable, ImageRenderable, CircleRenderable> renderData;
    RenderType type;
    int zIndex;

    bool operator<(const RenderCommand& other) const {
        return zIndex < other.zIndex;
    }
};

struct RenderQueue {
    std::vector<RenderCommand> commands;

    void clear() {
        commands.clear();
    }

    void addRectCommand(const Position& pos, const RectRenderable& renderable, int zIndex) {
        commands.push_back({pos, renderable, RenderType::Rectangle, zIndex});
    }

    void addTextCommand(const Position& pos, const TextRenderable& renderable, int zIndex) {
        commands.push_back({pos, renderable, RenderType::Text, zIndex});
    }

    void addImageCommand(const Position& pos, const ImageRenderable& renderable, int zIndex) {
        commands.push_back({pos, renderable, RenderType::Image, zIndex});
    }

    void addCircleCommand(const Position& pos, const CircleRenderable& renderable, int zIndex) {
        commands.push_back({pos, renderable, RenderType::Circle, zIndex});
    }

    void sort() {
        std::sort(commands.begin(), commands.end());
    }
};

// Global board grid instance
BoardGrid g_boardGrid;

// Global die entities and game piece entity and random number generator
flecs::entity g_die1Entity;
flecs::entity g_die2Entity;
flecs::entity g_gamePieceEntity;
flecs::entity g_gameStateEntity;
std::random_device g_rd;
std::mt19937 g_gen(g_rd());
std::uniform_int_distribution<> g_dieRoll(1, 6);

// Get closest point that is radius away from src to dest
void get_closest_radius_point(float src_x, float src_y, float dest_x, float dest_y, float radius, float* out_x, float* out_y) {
    // Calculate direction vector
    float dx = dest_x - src_x;
    float dy = dest_y - src_y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist < 0.0001f) {
        // If too close, just return the destination
        *out_x = dest_x;
        *out_y = dest_y;
        return;
    }

    // Normalize direction
    dx /= dist;
    dy /= dist;

    // Calculate the point that is 'radius' distance from dest towards src
    *out_x = dest_x - dx * radius;
    *out_y = dest_y - dy * radius;
}

void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (g_world) {
        // Update mouse position in ECS
        g_world->set<MousePosition>({(float)xpos, (float)ypos});
    }

    // Get window size
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    // Convert mouse position to tile coordinates
    auto [tileCol, tileRow] = g_boardGrid.mouseToTile(xpos, ypos, windowWidth, windowHeight);

    // Get tile type at that position
    int tileType = g_boardGrid.getTileAt(tileCol, tileRow);

    // Commented out to reduce log spam
    // if (tileType >= 0) {
    //     std::cout << "Mouse at tile (" << tileCol << ", " << tileRow << "): "
    //               << getTileTypeName(tileType) << " (type " << tileType << ")" << std::endl;
    // } else {
    //     std::cout << "Mouse position: (" << xpos << ", " << ypos << ") - outside board" << std::endl;
    // }
}

// Global world reference for mouse callback
flecs::world* g_world = nullptr;

// Helper function to check if a tile is occupied by any piece
bool isTileOccupied(int tileIndex) {
    if (!g_world) return false;

    bool occupied = false;
    auto pieceQuery = g_world->query<BoardPosition>();
    pieceQuery.each([&](flecs::entity e, BoardPosition& pos) {
        if (pos.tileIndex == tileIndex) {
            occupied = true;
        }
    });
    return occupied;
}

// Helper function to get all pieces on a specific tile
std::vector<flecs::entity> getPiecesOnTile(int tileIndex) {
    std::vector<flecs::entity> pieces;
    if (!g_world) return pieces;

    auto pieceQuery = g_world->query<BoardPosition>();
    pieceQuery.each([&](flecs::entity e, BoardPosition& pos) {
        if (pos.tileIndex == tileIndex) {
            pieces.push_back(e);
        }
    });
    return pieces;
}

// Helper function to find the tile directly below current tile (accounting for winding pattern)
int findTileBelowInWindingPattern(int currentTileIndex) {
    if (!g_boardGrid.isValidIndex(currentTileIndex)) {
        return 0; // Invalid tile, return to start
    }

    // Get current tile coordinates (bottom-up: row 0 = bottom, higher row = towards top)
    auto [col, row] = g_boardGrid.indexToTileCoords(currentTileIndex);

    // Move towards start (back down towards bottom) - increase row in bottom-up coordinates
    int newRow = row + 1;

    // If already at bottom row, return to start
    if (newRow < 0) {
        return 0;
    }

    // Now we need to convert back to index, accounting for the winding pattern
    // But we need to work with the internal row numbering (not bottom-up)
    int internalNewRow = g_boardGrid.height - 1 - newRow; // Convert back to internal coordinates
    int newIndex;

    // Determine winding pattern for this internal row
    if (internalNewRow % 2 == 0) {
        // Even internal rows: left-to-right
        newIndex = internalNewRow * g_boardGrid.width + col;
    } else {
        // Odd internal rows: right-to-left
        newIndex = internalNewRow * g_boardGrid.width + (g_boardGrid.width - 1 - col);
    }

    // Ensure the calculated index is valid
    if (g_boardGrid.isValidIndex(newIndex)) {
        return newIndex;
    } else {
        return 0; // Fallback to start if calculation failed
    }
}

// Helper function to get recruitment area position for a player and employee slot
Position getRecruitmentPosition(int playerIndex, int employeeSlot) {
    // Position recruitment areas to the left of the board
    float baseX = 700.0f; // Left margin
    float baseY = 1000.0f; // Starting Y position
    float playerSpacing = 100.0f; // Space between players
    float employeeSpacing = -50.0f; // Space between employees for same player

    return {
        baseX + employeeSlot * employeeSpacing,
        baseY + playerIndex * playerSpacing
    };
}

// Helper function to create an employee entity
flecs::entity createEmployee(flecs::world& world, int playerIndex, int employeeIndex, const std::string& playerType) {
    std::string employeeName = playerType + "_employee_" + std::to_string(employeeIndex);
    std::string spritePath = "../assets/piece_" + playerType + "_trim.png";
    std::string spriteShadowPath = "../assets/piece_" + playerType + "_shadow.png";

    Position recruitPos = getRecruitmentPosition(playerIndex, employeeIndex);
    float piece_scale = 0.7f; // Use original scale

    auto employee = world.entity(employeeName.c_str())
        .add<Position, World>()
        .set<Position, Local>(recruitPos)
        .set<Employee>(Employee(playerIndex))
        .set<BoardPosition>(BoardPosition(-1)) // -1 indicates not on board
        .set<UIElementBounds>({});

    auto piece = world.entity((playerType + "_piece_" + std::to_string(employeeIndex)).c_str())
        .add<Position, World>()
        .set<Position, Local>({-26.0f, -88.0f*piece_scale})
        .set<CreateSprite>({spritePath, 1.0f, piece_scale, piece_scale})
        .set<ZIndex>({100})
        .set<Clickable>({"employee_click"})
        .set<UIElementBounds>({})
        .child_of(employee);

    auto shadow = world.entity((playerType + "_shadow_" + std::to_string(employeeIndex)).c_str())
        .set<Position, World>({0.0f, 0.0f})
        .set<Position, Local>({-205.0f*piece_scale, -88.0f*piece_scale})
        .set<CreateSprite>({spriteShadowPath, 0.7f, piece_scale, piece_scale})
        .set<ZIndex>({50})
        .set<UIElementBounds>({})
        .child_of(employee);

    return employee;
}

// Forward declarations
void finalizeDiceAllocation();
void performCascadingKnockback(const std::vector<flecs::entity>& piecesToKnockback, int fromTileIndex, float animationDuration);
void startDiceAllocation();

// Helper function to animate piece movement to a tile
void animatePieceToTile(flecs::entity piece, int targetTileIndex, float duration = 0.3f) {
    if (!piece.is_valid() || !piece.has<Position, Local>()) return;

    Position currentPos = piece.get<Position, Local>();
    Position targetPos = g_boardGrid.getTileCenterPosition(targetTileIndex);

    if (targetPos.x != -1.0f && targetPos.y != -1.0f) {
        // Apply player-specific offset
        if (piece.has<BoardPosition>()) {
            float piece_offset = 0.0f; // Same offset used elsewhere

            // Determine which player this piece belongs to based on name
            std::string pieceName = piece.name() ? piece.name() : "";
            if (pieceName.find("triangle") != std::string::npos) {
                targetPos.x -= piece_offset;
                targetPos.y -= piece_offset;
            } else if (pieceName.find("circle") != std::string::npos) {
                targetPos.x += piece_offset;
                targetPos.y -= piece_offset;
            } else if (pieceName.find("hexagon") != std::string::npos) {
                targetPos.x -= piece_offset;
                targetPos.y += piece_offset;
            } else if (pieceName.find("square") != std::string::npos) {
                targetPos.x += piece_offset;
                targetPos.y += piece_offset;
            }
        }

        piece.set<MovementAnimation>(MovementAnimation(currentPos, targetPos, duration));
    }
}

// Helper function to allocate a die to an employee
void allocateDieToEmployee(int employeeIndex) {
    if (!g_gameStateEntity.is_valid()) return;

    GameState gameState = g_gameStateEntity.get<GameState>();

    if (!gameState.currentAllocation.allocationPhase || gameState.currentAllocation.availableDice <= 0) {
        g_logger.log(LogCategory::DICE_ALLOCATION, "No dice available for allocation");
        return;
    }

    if (employeeIndex < 0 || employeeIndex >= gameState.currentAllocation.dicePerEmployee.size()) {
        g_logger.log(LogCategory::DICE_ALLOCATION, "Invalid employee index");
        return;
    }

    // Allocate one die to this employee
    gameState.currentAllocation.dicePerEmployee[employeeIndex]++;
    gameState.currentAllocation.availableDice--;

    // Update the employee's allocated dice count
    flecs::entity employee = gameState.allEmployeeEntities[gameState.currentPlayerIndex][employeeIndex];
    if (employee.is_valid() && employee.has<Employee>()) {
        Employee emp = employee.get<Employee>();
        emp.allocatedDice = gameState.currentAllocation.dicePerEmployee[employeeIndex];
        employee.set<Employee>(emp);
    }

    // Update die visuals and progress to next die
    flecs::entity currentDie = (gameState.currentAllocation.currentDieIndex == 0) ? g_die1Entity : g_die2Entity;
    if (currentDie.is_valid()) {
        // Mark current die as allocated (change sprite)
        currentDie.set<CreateSprite>({"../assets/die_1.png", 1.0f}); // Show as allocated
        currentDie.remove<Clickable>(); // Make unclickable
    }

    // Move to next die
    gameState.currentAllocation.currentDieIndex++;

    // If there's a next die, make it active
    if (gameState.currentAllocation.currentDieIndex == 1 && g_die2Entity.is_valid()) {
        g_die2Entity.set<CreateSprite>({"../assets/die_allocation.png", 1.0f}); // Make fully visible
        g_die2Entity.set<Clickable>({"die_click"}); // Make clickable
        g_logger.log(LogCategory::DICE_ALLOCATION, "Die 1 allocated to employee " + std::to_string(employeeIndex) + ". Now allocate die 2.");
    }

    g_gameStateEntity.set<GameState>(gameState);

    g_logger.log(LogCategory::DICE_ALLOCATION, "Allocated die " + std::to_string(gameState.currentAllocation.currentDieIndex) + " to employee " + std::to_string(employeeIndex) +
              " (total dice for employee: " + std::to_string(gameState.currentAllocation.dicePerEmployee[employeeIndex]) +
              ", remaining: " + std::to_string(gameState.currentAllocation.availableDice) + ")");

    // If no more dice available, end allocation phase
    if (gameState.currentAllocation.availableDice == 0) {
        finalizeDiceAllocation();
    }
}

// Helper function to start dice allocation phase
void startDiceAllocation() {
    if (!g_gameStateEntity.is_valid()) return;

    GameState gameState = g_gameStateEntity.get<GameState>();

    if (!gameState.currentAllocation.allocationPhase) {
        // Process career bonuses for the current player's employees
        int currentPlayer = gameState.currentPlayerIndex;
        flecs::entity playerEntity = gameState.playerEntities[currentPlayer];
        Player player = playerEntity.get<Player>();

        g_logger.log(LogCategory::CAREER_BONUS, "\n=== Processing career bonuses for " + player.name + " ===");

        // Check each employee for career tile bonuses
        for (auto& employee : gameState.allEmployeeEntities[currentPlayer]) {
            if (employee.is_valid() && employee.has<Employee>() && employee.has<BoardPosition>()) {
                Employee emp = employee.get<Employee>();
                BoardPosition boardPos = employee.get<BoardPosition>();

                // Only active employees on the board get bonuses
                if (emp.state == ACTIVE && boardPos.tileIndex >= 0) {
                    // Get tile coordinates
                    auto [col, row] = g_boardGrid.indexToTileCoords(boardPos.tileIndex);
                    int tileType = g_boardGrid.getTileAt(col, row);

                    // Engineering career: earn +1 tech
                    if (tileType == TILE_ENG) {
                        player.tech += 1;
                        g_logger.log(LogCategory::CAREER_BONUS, "  " + std::string(employee.name()) + " earned +1 tech (on engineering tile)");

                        // Create floating "+1" text particle at employee position
                        if (employee.has<Position, Local>()) {
                            Position empPos = employee.get<Position, Local>();
                            g_world->entity()
                                .add<Position, World>()
                                .set<Position, Local>({empPos.x, empPos.y - 40.0f}) // Start above employee
                                .set<FloatingText>({"+1", 1.5f, 30.0f})
                                .set<TextRenderable>({"+ 1", "CharisSIL-Regular", 24.0f, 0x00FF00FF, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE})
                                .set<ZIndex>({500}); // High z-index to appear on top
                        }
                    }

                    // HR career: earn +1 recruit point
                    if (tileType == TILE_HR) {
                        player.recruit += 1;
                        g_logger.log(LogCategory::CAREER_BONUS, "  " + std::string(employee.name()) + " earned +1 recruit point (on HR tile)");

                        // Create floating "+1" text particle at employee position
                        if (employee.has<Position, Local>()) {
                            Position empPos = employee.get<Position, Local>();
                            g_world->entity()
                                .add<Position, World>()
                                .set<Position, Local>({empPos.x, empPos.y - 40.0f}) // Start above employee
                                .set<FloatingText>({"+1", 1.5f, 30.0f})
                                .set<TextRenderable>({"+ 1", "CharisSIL-Regular", 24.0f, 0xFF00FFFF, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE}) // Magenta for recruit
                                .set<ZIndex>({500}); // High z-index to appear on top
                        }

                        // Check if player has 10 recruit points to spawn a new employee
                        if (player.recruit >= 10) {
                            player.recruit -= 10; // Spend 10 points
                            g_logger.log(LogCategory::CAREER_BONUS, "  " + player.name + " has 10 recruit points! Spawning new employee...");

                            // Find the next available employee slot
                            int nextEmployeeIndex = gameState.allEmployeeEntities[currentPlayer].size();
                            std::string playerType = "";
                            if (player.name.find("triangle") != std::string::npos) playerType = "triangles";
                            else if (player.name.find("circle") != std::string::npos) playerType = "circles";
                            else if (player.name.find("hexagon") != std::string::npos) playerType = "hexagons";
                            else if (player.name.find("square") != std::string::npos) playerType = "squares";

                            if (!playerType.empty()) {
                                flecs::entity newEmployee = createEmployee(*g_world, currentPlayer, nextEmployeeIndex, playerType);
                                gameState.allEmployeeEntities[currentPlayer].push_back(newEmployee);
                                g_logger.log(LogCategory::CAREER_BONUS, "  Spawned new employee: " + std::string(newEmployee.name()));
                            }
                        }
                    }
                }
            }
        }

        // Update player stats
        playerEntity.set<Player>(player);

        // Update game state with potentially new employees
        g_gameStateEntity.set<GameState>(gameState);

        g_logger.log(LogCategory::CAREER_BONUS, "=== Career bonuses complete ===");

        // Start allocation phase (re-get gameState in case it was updated with new employees)
        gameState = g_gameStateEntity.get<GameState>();
        gameState.currentAllocation.allocationPhase = true;
        gameState.currentAllocation.availableDice = 2; // Each player gets 2 dice
        gameState.currentAllocation.currentDieIndex = 0; // Start with first die
        gameState.currentAllocation.dicePerEmployee.assign(gameState.allEmployeeEntities[gameState.currentPlayerIndex].size(), 0);
        gameState.currentAllocation.diceValues.assign(2, 0); // Initialize 2 dice values to 0 (unrolled)
        g_gameStateEntity.set<GameState>(gameState);
        g_logger.log(LogCategory::DICE_ALLOCATION, "Allocation phase started! Click on employees to allocate dice (2 dice available)");
    }
}

// Helper function to finalize dice allocation and execute moves
void finalizeDiceAllocation() {
    if (!g_gameStateEntity.is_valid()) return;

    GameState gameState = g_gameStateEntity.get<GameState>();

    g_logger.log(LogCategory::DICE_ALLOCATION, "Finalizing dice allocation for " + gameState.playerEntities[gameState.currentPlayerIndex].get<Player>().name);

    // Now roll the actual dice values for allocated dice
    g_logger.log(LogCategory::DICE_ALLOCATION, "Rolling dice...");
    int diceIndex = 0;
    for (int i = 0; i < gameState.currentAllocation.dicePerEmployee.size(); ++i) {
        int diceCount = gameState.currentAllocation.dicePerEmployee[i];
        int totalMovement = 0;

        if (diceCount > 0) {
            std::string rollMsg = "Employee " + std::to_string(i) + " rolling " + std::to_string(diceCount) + " dice: ";
            for (int d = 0; d < diceCount; ++d) {
                int rollValue = g_dieRoll(g_gen);
                gameState.currentAllocation.diceValues[diceIndex] = rollValue;
                totalMovement += rollValue;
                rollMsg += std::to_string(rollValue) + " ";
                diceIndex++;
            }
            rollMsg += "(total: " + std::to_string(totalMovement) + ")";
            g_logger.log(LogCategory::DICE_ALLOCATION, rollMsg);

            // Execute move for this employee
            flecs::entity employee = gameState.allEmployeeEntities[gameState.currentPlayerIndex][i];
            if (employee.is_valid() && employee.has<Employee>()) {
                Employee emp = employee.get<Employee>();

                if (emp.state == RECRUIT) {
                    // Recruit enters the board and moves forward by total dice value
                    emp.state = ACTIVE;
                    employee.set<Employee>(emp);

                    int targetTileIndex = totalMovement-1;

                    // Handle collisions at target tile BEFORE setting position
                    std::vector<flecs::entity> piecesOnTarget = getPiecesOnTile(targetTileIndex);
                    // No need to remove this employee since its BoardPosition is still -1

                    Position targetPosition = g_boardGrid.getTileCenterPosition(targetTileIndex);
                    if (targetPosition.x != -1.0f && targetPosition.y != -1.0f) {
                        performCascadingKnockback(piecesOnTarget, targetTileIndex, 0.25f);

                        // Set position AFTER knockback is complete
                        employee.set<BoardPosition>(BoardPosition(targetTileIndex));
                        animatePieceToTile(employee, targetTileIndex, 0.4f);

                        // Log career tile landing
                        auto [col, row] = g_boardGrid.indexToTileCoords(targetTileIndex);
                        int tileType = g_boardGrid.getTileAt(col, row);
                        g_logger.log(LogCategory::CAREER_LANDING, "Employee " + std::to_string(i) + " recruited and landed on tile " + std::to_string(targetTileIndex) + " (" + getTileTypeName(tileType) + ")");
                    } else {
                        g_logger.log(LogCategory::SYSTEM, "Employee " + std::to_string(i) + " recruited but target tile " + std::to_string(targetTileIndex) + " is out of bounds!");
                    }
                } else if (emp.state == ACTIVE) {
                    // Active employee moves based on total dice value
                    BoardPosition boardPos = employee.get<BoardPosition>();
                    int targetTileIndex = boardPos.tileIndex + totalMovement;

                    // Handle collisions and move
                    std::vector<flecs::entity> piecesOnTarget = getPiecesOnTile(targetTileIndex);
                    piecesOnTarget.erase(
                        std::remove(piecesOnTarget.begin(), piecesOnTarget.end(), employee),
                        piecesOnTarget.end()
                    );

                    Position targetPosition = g_boardGrid.getTileCenterPosition(targetTileIndex);
                    if (targetPosition.x != -1.0f && targetPosition.y != -1.0f) {
                        performCascadingKnockback(piecesOnTarget, targetTileIndex, 0.25f);
                        boardPos.tileIndex = targetTileIndex;
                        employee.set<BoardPosition>(boardPos);
                        animatePieceToTile(employee, targetTileIndex, 0.4f);

                        // Log career tile landing
                        auto [col, row] = g_boardGrid.indexToTileCoords(targetTileIndex);
                        int tileType = g_boardGrid.getTileAt(col, row);
                        g_logger.log(LogCategory::CAREER_LANDING, "Employee " + std::to_string(i) + " moved to tile " + std::to_string(targetTileIndex) + " (" + getTileTypeName(tileType) + ")");
                    }
                }
            }
        }
    }

    // Reset dice visuals for next turn
    if (g_die1Entity.is_valid()) {
        g_die1Entity.set<CreateSprite>({"../assets/die_allocation.png", 1.0f});
        g_die1Entity.set<Clickable>({"die_click"});
    }
    if (g_die2Entity.is_valid()) {
        g_die2Entity.set<CreateSprite>({"../assets/die_allocation.png", 0.5f});
        g_die2Entity.remove<Clickable>();
    }

    // Clear allocated dice badges from all employees
    g_logger.log(LogCategory::DICE_ALLOCATION, "Clearing allocated dice badges from all employees...");
    for (int playerIdx = 0; playerIdx < gameState.allEmployeeEntities.size(); ++playerIdx) {
        for (auto& employee : gameState.allEmployeeEntities[playerIdx]) {
            if (employee.is_valid() && employee.has<Employee>()) {
                Employee emp = employee.get<Employee>();
                if (emp.allocatedDice > 0) {
                    g_logger.log(LogCategory::DICE_ALLOCATION, "  Clearing badge from " + std::string(employee.name()) + " (had " + std::to_string(emp.allocatedDice) + " dice)");
                }
                emp.allocatedDice = 0;
                employee.set<Employee>(emp);
            }
        }
    }
    g_logger.log(LogCategory::DICE_ALLOCATION, "Badge clearing complete.");

    // End allocation phase and advance to next player
    gameState.currentAllocation.allocationPhase = false;
    gameState.currentAllocation.currentDieIndex = 0;
    gameState.currentAllocation.dicePerEmployee.assign(gameState.currentAllocation.dicePerEmployee.size(), 0);
    gameState.currentAllocation.diceValues.assign(gameState.currentAllocation.diceValues.size(), 0);
    gameState.currentPlayerIndex = (gameState.currentPlayerIndex + 1) % 4;
    g_gameStateEntity.set<GameState>(gameState);

    g_logger.log(LogCategory::GAME_FLOW, "Turn complete! It's now " + gameState.playerEntities[gameState.currentPlayerIndex].get<Player>().name + "'s turn.");

    // Automatically start dice allocation for the next player
    startDiceAllocation();
}

// Helper function to perform cascading knockback with animations
void performCascadingKnockback(const std::vector<flecs::entity>& piecesToKnockback, int fromTileIndex, float animationDuration = 0.3f) {
    for (auto& piece : piecesToKnockback) {
        if (piece.is_valid() && piece.has<BoardPosition>() && piece.has<Employee>()) {
            BoardPosition piecePos = piece.get<BoardPosition>();
            Employee emp = piece.get<Employee>();

            // Special case: if piece is already on tile 0 (start), send it back to recruitment
            if (piecePos.tileIndex == 0) {
                g_logger.log(LogCategory::CAREER_LANDING, std::string(piece.name()) + " knocked from tile 0 to recruitment area");

                // Change state back to RECRUIT
                emp.state = RECRUIT;
                emp.allocatedDice = 0; // Clear any allocated dice
                piece.set<Employee>(emp);

                // Set board position to -1 (not on board)
                piecePos.tileIndex = -1;
                piece.set<BoardPosition>(piecePos);

                // Find the employee's slot index in the game state
                int employeeSlot = 0;
                if (g_gameStateEntity.is_valid()) {
                    GameState gameState = g_gameStateEntity.get<GameState>();
                    const auto& employees = gameState.allEmployeeEntities[emp.playerIndex];
                    for (int i = 0; i < employees.size(); ++i) {
                        if (employees[i] == piece) {
                            employeeSlot = i;
                            break;
                        }
                    }
                }

                // Animate back to recruitment position
                Position recruitPos = getRecruitmentPosition(emp.playerIndex, employeeSlot);
                if (piece.has<Position, Local>()) {
                    Position currentPos = piece.get<Position, Local>();
                    piece.set<MovementAnimation>(MovementAnimation(currentPos, recruitPos, animationDuration));
                }

                continue; // Skip to next piece
            }

            // Find the knockback target tile
            int knockbackTile = findTileBelowInWindingPattern(piecePos.tileIndex);

            // Special handling for tile 0 (start position)
            if (knockbackTile == 0) {
                // Check if there's already someone on tile 0
                std::vector<flecs::entity> piecesOnTile0 = getPiecesOnTile(0);

                if (!piecesOnTile0.empty()) {
                    // If tile 0 is occupied, send ALL pieces on tile 0 back to recruitment
                    g_logger.log(LogCategory::KNOCKBACK, "  -> Tile 0 is occupied, sending existing pieces back to recruitment");
                    for (auto& existingPiece : piecesOnTile0) {
                        if (existingPiece.is_valid() && existingPiece.has<Employee>() && existingPiece.has<BoardPosition>()) {
                            Employee existingEmp = existingPiece.get<Employee>();
                            BoardPosition existingPos = existingPiece.get<BoardPosition>();

                            // Send to recruitment
                            existingEmp.state = RECRUIT;
                            existingEmp.allocatedDice = 0;
                            existingPiece.set<Employee>(existingEmp);

                            existingPos.tileIndex = -1;
                            existingPiece.set<BoardPosition>(existingPos);

                            // Find slot and animate
                            int employeeSlot = 0;
                            if (g_gameStateEntity.is_valid()) {
                                GameState gs = g_gameStateEntity.get<GameState>();
                                const auto& employees = gs.allEmployeeEntities[existingEmp.playerIndex];
                                for (int i = 0; i < employees.size(); ++i) {
                                    if (employees[i] == existingPiece) {
                                        employeeSlot = i;
                                        break;
                                    }
                                }
                            }

                            Position recruitPos = getRecruitmentPosition(existingEmp.playerIndex, employeeSlot);
                            if (existingPiece.has<Position, Local>()) {
                                Position currentPos = existingPiece.get<Position, Local>();
                                existingPiece.set<MovementAnimation>(MovementAnimation(currentPos, recruitPos, animationDuration));
                            }

                            g_logger.log(LogCategory::KNOCKBACK, "    -> Sent " + std::string(existingPiece.name()) + " to recruitment");
                        }
                    }
                }

                // Now place this piece on tile 0
                piecePos.tileIndex = knockbackTile;
                piece.set<BoardPosition>(piecePos);

                // Animate movement to start position
                animatePieceToTile(piece, knockbackTile, animationDuration);

                // Log knockback to tile 0
                auto [col, row] = g_boardGrid.indexToTileCoords(knockbackTile);
                int tileType = g_boardGrid.getTileAt(col, row);
                g_logger.log(LogCategory::CAREER_LANDING, std::string(piece.name()) + " knocked back to tile " + std::to_string(knockbackTile) + " (" + getTileTypeName(tileType) + ")");
            } else {
                // Check if there are pieces already on the knockback target tile
                std::vector<flecs::entity> piecesOnKnockbackTarget = getPiecesOnTile(knockbackTile);

                // Remove this piece from the collision check to avoid self-collision
                piecesOnKnockbackTarget.erase(
                    std::remove(piecesOnKnockbackTarget.begin(), piecesOnKnockbackTarget.end(), piece),
                    piecesOnKnockbackTarget.end()
                );

                // If there are pieces on the target, cascade the knockback first
                if (!piecesOnKnockbackTarget.empty()) {
                    g_logger.log(LogCategory::KNOCKBACK, "  -> Cascading knockback from tile " + std::to_string(knockbackTile));
                    performCascadingKnockback(piecesOnKnockbackTarget, knockbackTile, animationDuration);
                }

                // Update board position
                piecePos.tileIndex = knockbackTile;
                piece.set<BoardPosition>(piecePos);

                // Animate movement to knockback tile
                animatePieceToTile(piece, knockbackTile, animationDuration);

                // Log knockback
                auto [col, row] = g_boardGrid.indexToTileCoords(knockbackTile);
                int tileType = g_boardGrid.getTileAt(col, row);
                g_logger.log(LogCategory::CAREER_LANDING, std::string(piece.name()) + " knocked back to tile " + std::to_string(knockbackTile) + " (" + getTileTypeName(tileType) + ")");
            }
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS && g_world) {
        // Get mouse position
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        c2v mousePoint = {(float)mouseX, (float)mouseY};
        bool clickHandled = false;

        g_logger.log(LogCategory::CLICK_DEBUG, "\n=== MOUSE CLICK DEBUG ===");
        g_logger.log(LogCategory::CLICK_DEBUG, "Mouse clicked at (" + std::to_string(mouseX) + ", " + std::to_string(mouseY) + ")");

        // Debug: Count all clickable entities first
        auto clickQuery = g_world->query_builder<Position, Clickable>()
            .term_at(0).second<World>()
            .build();
        int clickableCount = 0;
        clickQuery.each([&](flecs::entity e, Position& worldPos, Clickable& clickable) {
            clickableCount++;

            // Check if this entity has a parent (hierarchical positioning)
            bool hasParent = e.parent() != 0;
            Position localPos = {0, 0};
            if (e.has<Position, Local>()) {
                localPos = e.get<Position, Local>();
            }

            // std::cout << "  Clickable entity #" << clickableCount << ": " << e.name()
            //           << " world: (" << worldPos.x << ", " << worldPos.y << ")"
            //           << " local: (" << localPos.x << ", " << localPos.y << ")"
            //           << " hasParent: " << (hasParent ? "yes" : "no")
            //           << " event: " << clickable.clickEventType << std::endl;
            // std::cout << "    Bounds: (" << clickable.bounds.min.x << "," << clickable.bounds.min.y
            //           << ") to (" << clickable.bounds.max.x << "," << clickable.bounds.max.y << ")" << std::endl;

            // Calculate actual clickable area
            c2AABB entityBounds;
            entityBounds.min.x = worldPos.x + clickable.bounds.min.x;
            entityBounds.min.y = worldPos.y + clickable.bounds.min.y;
            entityBounds.max.x = worldPos.x + clickable.bounds.max.x;
            entityBounds.max.y = worldPos.y + clickable.bounds.max.y;

            // std::cout << "    World bounds: (" << entityBounds.min.x << "," << entityBounds.min.y
            //           << ") to (" << entityBounds.max.x << "," << entityBounds.max.y << ")" << std::endl;
        });

        // std::cout << "Total clickable entities: " << clickableCount << std::endl;

        // std::cout << "\nChecking collisions..." << std::endl;

        // Check all clickable entities for collision
        auto collisionQuery = g_world->query_builder<Position, Clickable>()
            .term_at(0).second<World>()
            .build();
        collisionQuery.each([&](flecs::entity e, Position& worldPos, Clickable& clickable) {
            // For child entities, manually compute world position if needed
            Position actualWorldPos = worldPos;
            if (e.parent() != 0 && e.has<Position, Local>()) {
                Position localPos = e.get<Position, Local>();
                flecs::entity parent = e.parent();
                if (parent.has<Position, Local>()) {
                    Position parentPos = parent.get<Position, Local>();
                    actualWorldPos.x = parentPos.x + localPos.x;
                    actualWorldPos.y = parentPos.y + localPos.y;
                    // std::cout << "  Computed world pos for " << e.name()
                    //           << ": parent(" << parentPos.x << "," << parentPos.y
                    //           << ") + local(" << localPos.x << "," << localPos.y
                    //           << ") = (" << actualWorldPos.x << "," << actualWorldPos.y << ")" << std::endl;
                }
            }

            // Create AABB at entity's actual world position
            c2AABB entityBounds;
            entityBounds.min.x = actualWorldPos.x + clickable.bounds.min.x;
            entityBounds.min.y = actualWorldPos.y + clickable.bounds.min.y;
            entityBounds.max.x = actualWorldPos.x + clickable.bounds.max.x;
            entityBounds.max.y = actualWorldPos.y + clickable.bounds.max.y;

            // Check collision using cute_c2
            bool collision = c2AABBtoPoint(entityBounds, mousePoint);
            // std::cout << "  Testing " << e.name() << " at (" << actualWorldPos.x << "," << actualWorldPos.y << "): " << (collision ? "HIT!" : "miss") << std::endl;

            if (collision) {
                clickable.wasClicked = true;

                // Create click event
                ClickEvent clickEvent;
                clickEvent.clickedEntity = e;
                clickEvent.eventType = clickable.clickEventType;
                clickEvent.mouseX = (float)mouseX;
                clickEvent.mouseY = (float)mouseY;

                // Add click event to a temporary entity (will be handled by observer)
                g_world->entity()
                    .set<ClickEvent>(clickEvent);

                // std::cout << "*** CLICKED ON ENTITY: " << e.name() << " (event: " << clickable.clickEventType << ") ***" << std::endl;
                clickHandled = true;
            }
        });

        g_logger.log(LogCategory::CLICK_DEBUG, "Click result: " + std::string(clickHandled ? "HANDLED" : "NOT HANDLED"));
        g_logger.log(LogCategory::CLICK_DEBUG, "=== END CLICK DEBUG ===\n");

        // All game logic now handled through click events
    }
}

// Global debug render entity
flecs::entity g_debugRenderEntity;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Toggle debug render mode with '.' key
    static bool dotKeyPressed = false;
    bool dotKeyDown = glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS;

    if (dotKeyDown && !dotKeyPressed && g_debugRenderEntity.is_valid()) {
        DebugRender debugRender = g_debugRenderEntity.get<DebugRender>();
        debugRender.enabled = !debugRender.enabled;
        g_debugRenderEntity.set<DebugRender>(debugRender);
        g_logger.log(LogCategory::SYSTEM, "Debug render mode: " + std::string(debugRender.enabled ? "ON" : "OFF"));
    }

    dotKeyPressed = dotKeyDown;
}

int main(int, char *[]) {
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_ALPHA_BITS, 8);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Handcuffs and Parachutes", monitor, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glfwSwapInterval(0);
        glViewport(0, 0, mode->width, mode->height);

    // Should add after gladLoadGL():

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Load tile map data
    if (!g_boardGrid.loadFromCSV("tile_map.csv")) {
        std::cerr << "Warning: Could not load tile map. Mouse tile detection will not work." << std::endl;
    }

    auto vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (vg == NULL) {
        std::cerr << "Failed to initialize NanoVG" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Create flecs world
    flecs::world world;
    g_world = &world; // Set global reference for mouse callback

    // Register components
    world.component<Position>();
    world.component<Local>();
    world.component<World>();
    world.component<Velocity>();
    world.component<Bounds>();
    world.component<RectRenderable>();
    world.component<TextRenderable>();
    world.component<ImageRenderable>();
    world.component<ZIndex>();
    world.component<CreateSprite>();
    world.component<DieRoll>();
    world.component<BoardPosition>();
    world.component<Player>();
    world.component<GameState>();
    world.component<Clickable>();
    world.component<ClickEvent>();
    world.component<DebugRender>();
    world.component<UIElementBounds>();
    world.component<RenderStatus>();
    world.component<Window>();
    world.component<Graphics>().add(flecs::Singleton);
    world.component<RenderQueue>();
    world.component<FloatingText>();
    world.component<MovementAnimation>();
    world.component<Employee>();
    world.component<MousePosition>().add(flecs::Singleton);

    // Create singleton entities for global resources
    auto windowEntity = world.entity("Window")
        .set<Window>({window, mode->width, mode->height});

    auto graphicsEntity = world.entity("Graphics")
        .set<Graphics>({vg});

    auto renderQueueEntity = world.entity("RenderQueue")
        .set<RenderQueue>({});

    // Create debug render entity
    g_debugRenderEntity = world.entity("DebugRender")
        .set<DebugRender>({});

    // Observer to automatically process CreateSprite components
    world.observer<CreateSprite>()
        .event(flecs::OnSet)
        .each([&](flecs::entity e, CreateSprite& createSprite) {
            // Load the image and get its dimensions
            int imageHandle = nvgCreateImage(vg, createSprite.imagePath.c_str(), 0);

            if (imageHandle != -1) {
                // Get image dimensions
                int imgWidth, imgHeight;
                nvgImageSize(vg, imageHandle, &imgWidth, &imgHeight);

                float finalWidth = (float)imgWidth;
                float finalHeight = (float)imgHeight;

                // If scaleToWindow is true, scale to fit the window size
                if (createSprite.scaleToWindow) {
                    finalWidth = (float)mode->width;
                    finalHeight = (float)mode->height;
                }

                // Apply additional horizontal and vertical scaling
                finalWidth *= createSprite.scaleX;
                finalHeight *= createSprite.scaleY;

                // Add ImageRenderable component with auto-detected or scaled size
                e.set<ImageRenderable>({imageHandle, finalWidth, finalHeight, createSprite.alpha});

                // Remove the temporary CreateSprite component
                e.remove<CreateSprite>();

                g_logger.log(LogCategory::SYSTEM, "Created sprite from " + createSprite.imagePath +
                         " (size: " + std::to_string(finalWidth) + "x" + std::to_string(finalHeight) + ")");
            } else {
                std::cerr << "Failed to load image: " << createSprite.imagePath << std::endl;
                // Remove the component even if loading failed to avoid infinite loops
                e.remove<CreateSprite>();
            }
        });

    // Hierarchical positioning system - computes world positions from local positions
    auto hierarchicalQuery = world.query_builder<const Position, const Position*, Position>()
        .term_at(0).second<Local>()      // Local position
        .term_at(1).second<World>()      // Parent world position
        .term_at(2).second<World>()      // This entity's world position
        .term_at(1).parent().cascade()   // Get parent position in breadth-first order
        .build();

    auto hierarchicalSystem = world.system()
        .kind(flecs::PreUpdate)  // Run first to compute world positions
        .each([&]() {
            // std::cout << "Update hierarchy" << std::endl;
            hierarchicalQuery.each([](const Position& local, const Position* parentWorld, Position& world) {
                world.x = local.x;
                world.y = local.y;
                if (parentWorld) {
                    world.x += parentWorld->x;
                    world.y += parentWorld->y;
                }
            });
        });

    // Movement system
    auto movementSystem = world.system<Position, Velocity, Bounds>()
        .each([](flecs::iter& it, size_t i, Position& pos, Velocity& vel, Bounds& bounds) {
            float deltaTime = it.delta_system_time();

            pos.x += vel.dx * deltaTime;
            pos.y += vel.dy * deltaTime;

            // Bounce off boundaries
            if (pos.x <= bounds.minX || pos.x >= bounds.maxX) {
                vel.dx *= -1.0f;
                pos.x = std::max(bounds.minX, std::min(bounds.maxX, pos.x));
            }
            if (pos.y <= bounds.minY || pos.y >= bounds.maxY) {
                vel.dy *= -1.0f;
                pos.y = std::max(bounds.minY, std::min(bounds.maxY, pos.y));
            }
        });

    // Movement animation system
    auto animationSystem = world.system<Position, MovementAnimation>()
        .term_at(0).second<Local>()
        .each([](flecs::iter& it, size_t i, Position& pos, MovementAnimation& anim) {
            if (!anim.active) return;

            float deltaTime = it.delta_system_time();
            anim.elapsed += deltaTime;

            // Calculate interpolation factor (0.0 to 1.0)
            float t = std::min(anim.elapsed / anim.duration, 1.0f);

            // Linear interpolation
            pos.x = anim.startPos.x + (anim.targetPos.x - anim.startPos.x) * t;
            pos.y = anim.startPos.y + (anim.targetPos.y - anim.startPos.y) * t;

            // Check if animation is complete
            if (t >= 1.0f) {
                anim.active = false;
                pos.x = anim.targetPos.x;
                pos.y = anim.targetPos.y;
            }
        });

    // Floating text animation system
    auto floatingTextSystem = world.system<Position, FloatingText, TextRenderable>()
        .term_at(0).second<Local>()
        .each([&](flecs::iter& it, size_t i, Position& pos, FloatingText& floating, TextRenderable& text) {
            if (!floating.active) return;

            float deltaTime = it.delta_system_time();
            floating.elapsed += deltaTime;

            // Rise upward
            pos.y -= floating.riseSpeed * deltaTime;

            // Calculate fade (alpha decreases over time)
            float alpha = 1.0f - (floating.elapsed / floating.duration);
            alpha = std::max(0.0f, std::min(1.0f, alpha));

            // Update text color with alpha
            uint32_t baseColor = text.color & 0xFFFFFF00; // Keep RGB, clear alpha
            uint8_t alphaValue = static_cast<uint8_t>(alpha * 255);
            text.color = baseColor | alphaValue;

            // Check if animation is complete
            if (floating.elapsed >= floating.duration) {
                floating.active = false;
                // Delete the entity after animation completes
                it.entity(i).destruct();
            }
        });

    // Virtual cursor tracking system - follows mouse but stays within radius
    auto cursorTrackingSystem = world.system<Position>()
        .term_at(0).second<Local>()
        .with(flecs::Name, "VirtualCursor")
        .kind(flecs::PreUpdate)
        .each([&](flecs::entity e, Position& pos) {
            const MousePosition* mousePos = world.try_get<MousePosition>();
            if (mousePos) {
                float follow_radius = 32.0f;
                float close_x, close_y;
                get_closest_radius_point(pos.x, pos.y, mousePos->x, mousePos->y, follow_radius, &close_x, &close_y);
                pos.x = close_x;
                pos.y = close_y;
            }
        });

    // Render queue collection system for rectangles
    auto rectQueueSystem = world.system<Position, RectRenderable, ZIndex>()
        .term_at(0).second<World>()
        .each([&](flecs::entity e, Position& worldPos, RectRenderable& renderable, ZIndex& zIndex) {
            RenderQueue& queue = world.ensure<RenderQueue>();
            queue.addRectCommand(worldPos, renderable, zIndex.layer);
        });

    // Render queue collection system for text
    auto textQueueSystem = world.system<Position, TextRenderable, ZIndex>()
        .term_at(0).second<World>()
        .each([&](flecs::entity e, Position& worldPos, TextRenderable& renderable, ZIndex& zIndex) {
            RenderQueue& queue = world.ensure<RenderQueue>();
            queue.addTextCommand(worldPos, renderable, zIndex.layer);
        });

    // Render queue collection system for images
    auto imageQueueSystem = world.system<Position, ImageRenderable, ZIndex>()
        .term_at(0).second<World>()
        .each([&](flecs::entity e, Position& worldPos, ImageRenderable& renderable, ZIndex& zIndex) {
            RenderQueue& queue = world.ensure<RenderQueue>();
            queue.addImageCommand(worldPos, renderable, zIndex.layer);
        });

    // Render execution system - sorts and renders all queued commands
    auto renderExecutionSystem = world.system<RenderQueue, Graphics>()
        .each([&](flecs::entity e, RenderQueue& queue, Graphics& graphics) {
            queue.sort();

            for (const auto& cmd : queue.commands) {
                switch (cmd.type) {
                    case RenderType::Rectangle: {
                        const auto& rect = std::get<RectRenderable>(cmd.renderData);
                        nvgBeginPath(graphics.vg);
                        nvgRect(graphics.vg, cmd.pos.x, cmd.pos.y, rect.width, rect.height);

                        uint8_t r = (rect.color >> 24) & 0xFF;
                        uint8_t g = (rect.color >> 16) & 0xFF;
                        uint8_t b = (rect.color >> 8) & 0xFF;

                        nvgFillColor(graphics.vg, nvgRGB(r, g, b));
                        nvgFill(graphics.vg);
                        break;
                    }
                    case RenderType::Text: {
                        const auto& text = std::get<TextRenderable>(cmd.renderData);
                        nvgFontSize(graphics.vg, text.fontSize);
                        nvgFontFace(graphics.vg, text.fontFace.c_str());
                        nvgTextAlign(graphics.vg, text.alignment);

                        uint8_t r = (text.color >> 24) & 0xFF;
                        uint8_t g = (text.color >> 16) & 0xFF;
                        uint8_t b = (text.color >> 8) & 0xFF;

                        nvgFillColor(graphics.vg, nvgRGB(r, g, b));
                        nvgText(graphics.vg, cmd.pos.x, cmd.pos.y, text.text.c_str(), nullptr);
                        break;
                    }
                    case RenderType::Image: {
                        const auto& image = std::get<ImageRenderable>(cmd.renderData);
                        if (image.imageHandle != -1) {
                            // Set global alpha for proper transparency blending
                            nvgGlobalAlpha(graphics.vg, image.alpha);

                            // Use a simple image render instead of pattern for better blending
                            nvgBeginPath(graphics.vg);
                            nvgRect(graphics.vg, cmd.pos.x, cmd.pos.y, image.width, image.height);

                            // Create image pattern with proper blending
                            NVGpaint imgPaint = nvgImagePattern(graphics.vg,
                                cmd.pos.x, cmd.pos.y,
                                image.width, image.height,
                                0.0f, image.imageHandle, 1.0f); // Use full alpha in pattern, control with globalAlpha

                            nvgFillPaint(graphics.vg, imgPaint);
                            nvgFill(graphics.vg);

                            // Reset global alpha after rendering
                            nvgGlobalAlpha(graphics.vg, 1.0f);
                        }
                        break;
                    }
                    case RenderType::Circle: {
                        const auto& circle = std::get<CircleRenderable>(cmd.renderData);
                        nvgBeginPath(graphics.vg);
                        nvgCircle(graphics.vg, cmd.pos.x, cmd.pos.y, circle.radius);

                        uint8_t fr = (circle.fillColor >> 24) & 0xFF;
                        uint8_t fg = (circle.fillColor >> 16) & 0xFF;
                        uint8_t fb = (circle.fillColor >> 8) & 0xFF;
                        uint8_t fa = circle.fillColor & 0xFF;

                        nvgFillColor(graphics.vg, nvgRGBA(fr, fg, fb, fa));
                        nvgFill(graphics.vg);

                        if (circle.strokeWidth > 0.0f) {
                            uint8_t sr = (circle.strokeColor >> 24) & 0xFF;
                            uint8_t sg = (circle.strokeColor >> 16) & 0xFF;
                            uint8_t sb = (circle.strokeColor >> 8) & 0xFF;
                            uint8_t sa = circle.strokeColor & 0xFF;

                            nvgStrokeColor(graphics.vg, nvgRGBA(sr, sg, sb, sa));
                            nvgStrokeWidth(graphics.vg, circle.strokeWidth);
                            nvgStroke(graphics.vg);
                        }
                        break;
                    }
                }
            }

            queue.clear();
        });

    // Debug rendering system - draws yellow boxes around all renderable objects
    auto debugRenderSystem = world.system<Position>()
        .term_at(0).second<World>()
        .kind(flecs::PostUpdate)  // Run after bounds calculation
        .each([&](flecs::entity e, Position& worldPos) {
            // Only render debug boxes if debug mode is enabled
            if (!g_debugRenderEntity.is_valid()) return;
            DebugRender debugRender = g_debugRenderEntity.get<DebugRender>();
            if (!debugRender.enabled) return;

            RenderQueue& queue = world.ensure<RenderQueue>();

            // Check if entity has any renderable component and draw appropriate debug box
            if (e.has<RectRenderable>()) {
                RectRenderable rect = e.get<RectRenderable>();
                // Draw yellow outline around rectangle
                queue.addRectCommand(
                    {worldPos.x - 2.0f, worldPos.y - 2.0f},
                    {rect.width + 4.0f, 2.0f, 0xFFFF00FF}, // Top border
                    1000 // High Z-index to render on top
                );
                queue.addRectCommand(
                    {worldPos.x - 2.0f, worldPos.y + rect.height},
                    {rect.width + 4.0f, 2.0f, 0xFFFF00FF}, // Bottom border
                    1000
                );
                queue.addRectCommand(
                    {worldPos.x - 2.0f, worldPos.y},
                    {2.0f, rect.height, 0xFFFF00FF}, // Left border
                    1000
                );
                queue.addRectCommand(
                    {worldPos.x + rect.width, worldPos.y},
                    {2.0f, rect.height, 0xFFFF00FF}, // Right border
                    1000
                );
            }

            if (e.has<ImageRenderable>()) {
                ImageRenderable img = e.get<ImageRenderable>();
                // Draw yellow outline around image (images are rendered with top-left corner at worldPos)
                queue.addRectCommand(
                    {worldPos.x - 2.0f, worldPos.y - 2.0f},
                    {img.width + 4.0f, 2.0f, 0xFFFF00FF}, // Top border
                    1000
                );
                queue.addRectCommand(
                    {worldPos.x - 2.0f, worldPos.y + img.height},
                    {img.width + 4.0f, 2.0f, 0xFFFF00FF}, // Bottom border
                    1000
                );
                queue.addRectCommand(
                    {worldPos.x - 2.0f, worldPos.y},
                    {2.0f, img.height, 0xFFFF00FF}, // Left border
                    1000
                );
                queue.addRectCommand(
                    {worldPos.x + img.width, worldPos.y},
                    {2.0f, img.height, 0xFFFF00FF}, // Right border
                    1000
                );
            }

            if (e.has<TextRenderable>()) {
                TextRenderable text = e.get<TextRenderable>();
                // For text, we'll approximate bounds (NanoVG would need actual text measurement)
                float approxWidth = text.text.length() * text.fontSize * 0.6f; // Rough estimate
                float approxHeight = text.fontSize;
                queue.addRectCommand(
                    {worldPos.x - 2.0f, worldPos.y - 2.0f},
                    {approxWidth + 4.0f, 2.0f, 0xFFFF00FF}, // Top border
                    1000
                );
                queue.addRectCommand(
                    {worldPos.x - 2.0f, worldPos.y + approxHeight},
                    {approxWidth + 4.0f, 2.0f, 0xFFFF00FF}, // Bottom border
                    1000
                );
                queue.addRectCommand(
                    {worldPos.x - 2.0f, worldPos.y},
                    {2.0f, approxHeight, 0xFFFF00FF}, // Left border
                    1000
                );
                queue.addRectCommand(
                    {worldPos.x + approxWidth, worldPos.y},
                    {2.0f, approxHeight, 0xFFFF00FF}, // Right border
                    1000
                );
            }
        });

    // Debug rendering system for UIElementBounds - draws cyan boxes around UI element bounds
    auto debugUIBoundsSystem = world.system<UIElementBounds>()
        .kind(flecs::PostUpdate)  // Run after bounds calculation
        .each([&](flecs::entity e, UIElementBounds& bounds) {
            // Only render debug boxes if debug mode is enabled
            if (!g_debugRenderEntity.is_valid()) return;
            DebugRender debugRender = g_debugRenderEntity.get<DebugRender>();
            if (!debugRender.enabled) return;

            RenderQueue& queue = world.ensure<RenderQueue>();

            // Skip invalid bounds (not yet calculated)
            if (bounds.xmin == FLT_MAX || bounds.ymin == FLT_MAX ||
                bounds.xmax == -FLT_MAX || bounds.ymax == -FLT_MAX) {
                return;
            }

            float width = bounds.xmax - bounds.xmin;
            float height = bounds.ymax - bounds.ymin;

            // Draw cyan outline around UIElementBounds (different color from renderable bounds)
            uint32_t boundColor = 0x00FFFFFF; // Cyan
            queue.addRectCommand(
                {bounds.xmin - 1.0f, bounds.ymin - 1.0f},
                {width + 2.0f, 1.0f, boundColor}, // Top border
                1001 // Slightly higher Z-index than renderable debug boxes
            );
            queue.addRectCommand(
                {bounds.xmin - 1.0f, bounds.ymax},
                {width + 2.0f, 1.0f, boundColor}, // Bottom border
                1001
            );
            queue.addRectCommand(
                {bounds.xmin - 1.0f, bounds.ymin},
                {1.0f, height, boundColor}, // Left border
                1001
            );
            queue.addRectCommand(
                {bounds.xmax, bounds.ymin},
                {1.0f, height, boundColor}, // Right border
                1001
            );

            // Add entity name label for debugging
            if (e.name()) {
                queue.addTextCommand(
                    {bounds.xmin, bounds.ymin - 15.0f},
                    {std::string(e.name()) + " bounds", "CharisSIL-Regular", 12.0f, boundColor, NVG_ALIGN_LEFT | NVG_ALIGN_TOP},
                    1002
                );
            }
        });

    // Bounds calculation system for renderable elements
    auto boundsCalculationSystem = world.system<Position, UIElementBounds>()
        .term_at(0).second<World>()
        .kind(flecs::OnUpdate)  // Run after hierarchical positioning
        .each([&](flecs::entity e, Position& worldPos, UIElementBounds& bounds) {
            // Reset bounds to invalid state at start of each frame
            bounds.xmin = FLT_MAX;
            bounds.ymin = FLT_MAX;
            bounds.xmax = -FLT_MAX;
            bounds.ymax = -FLT_MAX;

            // Calculate bounds based on renderable components
            if (e.has<RectRenderable>()) {
                auto rect = e.get<RectRenderable>();
                bounds.xmin = worldPos.x;
                bounds.ymin = worldPos.y;
                bounds.xmax = worldPos.x + rect.width;
                bounds.ymax = worldPos.y + rect.height;
            } else if (e.has<ImageRenderable>()) {
                auto img = e.get<ImageRenderable>();
                // Images are rendered with top-left corner at worldPos, not centered
                bounds.xmin = worldPos.x;
                bounds.ymin = worldPos.y;
                bounds.xmax = worldPos.x + img.width;
                bounds.ymax = worldPos.y + img.height;
            } else if (e.has<TextRenderable>()) {
                auto text = e.get<TextRenderable>();
                // Approximate text bounds
                float approxWidth = text.text.length() * text.fontSize * 0.6f;
                float approxHeight = text.fontSize;
                bounds.xmin = worldPos.x;
                bounds.ymin = worldPos.y;
                bounds.xmax = worldPos.x + approxWidth;
                bounds.ymax = worldPos.y + approxHeight;
            }
        });

    // BubbleUpBounds system - propagates bounds from children to parents
    auto bubbleUpBoundsQuery = world.query_builder<const UIElementBounds, UIElementBounds*, RenderStatus*>()
        .term_at(1).parent().cascade()  // Parent UIElementBounds
        .term_at(2).optional()          // Optional RenderStatus
        .build();

    auto bubbleUpBoundsSystem = world.system()
        .kind(flecs::OnValidate)  // Run after bounds calculation but before debug rendering
        .each([&]() {
            bubbleUpBoundsQuery.each([](flecs::entity e, const UIElementBounds& bounds, UIElementBounds* parent_bounds, RenderStatus* render) {
                if (parent_bounds && (!render || render->visible)) {
                    parent_bounds->xmin = std::min(parent_bounds->xmin, bounds.xmin);
                    parent_bounds->ymin = std::min(parent_bounds->ymin, bounds.ymin);
                    parent_bounds->xmax = std::max(parent_bounds->xmax, bounds.xmax);
                    parent_bounds->ymax = std::max(parent_bounds->ymax, bounds.ymax);
                }
            });
        });

    int fontHandle = nvgCreateFont(vg, "CharisSIL-Regular", "../assets/CharisSIL-Regular.ttf");

    // Create allocation background
    auto die_ux = world.entity("allocation_background")
        .add<Position, World>()
        .set<Position, Local>({500.0f, 900.0f})
        // .set<CreateSprite>({"../assets/allocator.png", 1.0f})
        .set<ZIndex>({5})
        .set<UIElementBounds>({});

    // Create first die (starts clickable)
    g_die1Entity = world.entity("die_1")
        .add<Position, World>()
        .set<Position, Local>({520.0f, 920.0f})
        .set<CreateSprite>({"../assets/die_allocation.png", 1.0f})
        .set<DieRoll>(DieRoll(0)) // 0 = unallocated
        .set<ZIndex>({10})
        .set<Clickable>({"die_click"})
        .set<UIElementBounds>({});

    // Create second die (starts inactive)
    g_die2Entity = world.entity("die_2")
        .add<Position, World>()
        .set<Position, Local>({620.0f, 920.0f})
        .set<CreateSprite>({"../assets/die_allocation.png", 0.5f}) // Dimmed when inactive
        .set<DieRoll>(DieRoll(0)) // 0 = unallocated
        .set<ZIndex>({10})
        .set<UIElementBounds>({});

    // Create employee pieces for all 4 players at the starting tile
    int startingTileIndex = 0;
    Position startPosition = g_boardGrid.getTileCenterPosition(startingTileIndex);
    // std::cout << startPosition.x << ", " << startPosition.y << std::endl;

    float piece_scale = 0.7f;
    float piece_offset = 0.0f; // Offset pieces so they don't overlap

    // Create multiple employees for each player in recruitment areas
    std::vector<std::string> playerTypes = {"triangles", "circles", "hexagons", "squares"};
    std::vector<std::vector<flecs::entity>> allEmployees(4);

    for (int playerIndex = 0; playerIndex < 4; ++playerIndex) {
        for (int employeeIndex = 0; employeeIndex < 3; ++employeeIndex) {
            flecs::entity employee = createEmployee(world, playerIndex, employeeIndex, playerTypes[playerIndex]);
            allEmployees[playerIndex].push_back(employee);
        }
    }
    
    // The tile grid starts at 771, 30 (the entire board size is 769, 1361)
    auto board = world.entity("board")
        .set<Position, World>({0.0f, 0.0f})
        .set<CreateSprite>({"../assets/board.png", 1.0f, true})
        .set<ZIndex>({0})
        .set<UIElementBounds>({});

    // Create 4 players with stats displayed on the right side
    float statsStartX = 1800; // Position stats 300px from right edge
    float statsStartY = 360.0f;
    float playerSpacing = 180.0f;

    auto triangles = world.entity("Triangles")
        .set<Player>({"triangles.com"})
        .add<Position, World>()
        .set<Position, Local>({statsStartX, statsStartY})
        .set<UIElementBounds>({});

    auto circles = world.entity("Circles")
        .set<Player>({"circles.com"})
        .add<Position, World>()
        .set<Position, Local>({statsStartX, statsStartY + playerSpacing})
        .set<UIElementBounds>({});

    auto hexagons = world.entity("Hexagons")
        .set<Player>({"hexagons.com"})
        .add<Position, World>()
        .set<Position, Local>({statsStartX, statsStartY + playerSpacing * 2})
        .set<UIElementBounds>({});

    auto squares = world.entity("Squares")
        .set<Player>({"squares.com"})
        .add<Position, World>()
        .set<Position, Local>({statsStartX, statsStartY + playerSpacing * 3})
        .set<UIElementBounds>({});

    // Create game state with player-employee mapping
    GameState gameState;
    gameState.playerEntities = {triangles, circles, hexagons, squares};
    gameState.allEmployeeEntities = allEmployees;

    g_gameStateEntity = world.entity("GameState")
        .set<GameState>(gameState);

    // No initial active piece - must recruit first
    g_gamePieceEntity = flecs::entity::null();

    // Create virtual cursor entity that follows mouse position
    auto cursorEntity = world.entity("VirtualCursor")
        .set<Position, Local>({0.0f, 0.0f})
        .set<Position, World>({0.0f, 0.0f})
        .set<ZIndex>({1000}); // High z-index to render on top

    // Initialize first player's turn
    g_logger.log(LogCategory::GAME_FLOW, "Game started! " + gameState.playerEntities[0].get<Player>().name + " goes first.");

    // Automatically start dice allocation for the first player
    startDiceAllocation();

    // Observer for click events
    world.observer<ClickEvent>()
        .event(flecs::OnSet)
        .each([&](flecs::entity e, ClickEvent& clickEvent) {
            g_logger.log(LogCategory::CLICK_DEBUG, "ClickEvent Observer triggered!");
            g_logger.log(LogCategory::CLICK_DEBUG, "  Entity: " + std::string(clickEvent.clickedEntity.name()));
            g_logger.log(LogCategory::CLICK_DEBUG, "  Event Type: " + clickEvent.eventType);
            g_logger.log(LogCategory::CLICK_DEBUG, "  Mouse Position: (" + std::to_string(clickEvent.mouseX) + ", " + std::to_string(clickEvent.mouseY) + ")");

            // Handle specific click events based on event type
            if (clickEvent.eventType == "die_click") {
                g_logger.log(LogCategory::CLICK_DEBUG, "  -> Die was clicked - starting allocation phase!");

                // Start dice allocation phase with 2 dice (values will be rolled after allocation)
                if (g_gameStateEntity.is_valid()) {
                    GameState gameState = g_gameStateEntity.get<GameState>();

                    if (!gameState.currentAllocation.allocationPhase) {
                        // Start allocation phase
                        gameState.currentAllocation.allocationPhase = true;
                        gameState.currentAllocation.availableDice = 2; // Each player gets 2 dice
                        gameState.currentAllocation.currentDieIndex = 0; // Start with first die
                        gameState.currentAllocation.dicePerEmployee.assign(gameState.allEmployeeEntities[gameState.currentPlayerIndex].size(), 0);
                        gameState.currentAllocation.diceValues.assign(2, 0); // Initialize 2 dice values to 0 (unrolled)
                        g_gameStateEntity.set<GameState>(gameState);
                        g_logger.log(LogCategory::DICE_ALLOCATION, "Click on employees to allocate dice (2 dice available, starting with die 1)");
                    }
                }
            } else if (clickEvent.eventType == "employee_click") {
                g_logger.log(LogCategory::CLICK_DEBUG, "  -> Employee was clicked!");
                // Find which employee was clicked and allocate dice
                if (g_gameStateEntity.is_valid()) {
                    GameState gameState = g_gameStateEntity.get<GameState>();
                    if (gameState.currentAllocation.allocationPhase) {
                        // The clicked entity is the piece child, get the parent employee
                        flecs::entity employeeEntity = clickEvent.clickedEntity.parent();
                        g_logger.log(LogCategory::CLICK_DEBUG, "  -> Clicked employee: " + std::string(employeeEntity.is_valid() ? employeeEntity.name() : "invalid"));

                        // Find the employee index for current player
                        bool found = false;
                        for (int i = 0; i < gameState.allEmployeeEntities[gameState.currentPlayerIndex].size(); ++i) {
                            if (gameState.allEmployeeEntities[gameState.currentPlayerIndex][i] == employeeEntity) {
                                g_logger.log(LogCategory::CLICK_DEBUG, "  -> Found employee at index " + std::to_string(i) + " for current player");
                                allocateDieToEmployee(i);
                                found = true;
                                break;
                            }
                        }

                        if (!found) {
                            g_logger.log(LogCategory::CLICK_DEBUG, "  -> Employee belongs to different player or not found");
                        }
                    } else {
                        g_logger.log(LogCategory::CLICK_DEBUG, "  -> Not in allocation phase. Click the die first!");
                    }
                }
            }

            // Remove the temporary click event entity
            e.destruct();
        });

    // System to automatically update clickable bounds from ImageRenderable components
    auto clickableBoundsSystem = world.system<Clickable, ImageRenderable>()
        .each([&](flecs::entity e, Clickable& clickable, ImageRenderable& image) {
            if (clickable.boundsNeedUpdate) {
                // Set bounds based on image dimensions (images are rendered with top-left corner at position)
                clickable.bounds.min.x = 0.0f;
                clickable.bounds.min.y = 0.0f;
                clickable.bounds.max.x = image.width;
                clickable.bounds.max.y = image.height;

                clickable.boundsNeedUpdate = false;

                g_logger.log(LogCategory::SYSTEM, "Updated clickable bounds for " + std::string(e.name()) +
                         " to " + std::to_string(image.width) + "x" + std::to_string(image.height) + " (top-left positioned)");
            }
        });

    // System to update Z-index based on Y position for proper orthogonal perspective
    auto depthSortingSystem = world.system<Position, ZIndex>()
        .term_at(0).second<World>()
        .each([&](flecs::entity e, Position& worldPos, ZIndex& zIndex) {
            // Only apply depth sorting to pieces, not shadows or UI elements
            std::string entityName = e.name().c_str();
            if (entityName.find("_piece") != std::string::npos && entityName.find("_shadow") == std::string::npos) {
                // Calculate Z-index based on Y position
                // Higher Y = lower on screen = higher Z-index (in front)
                // Base Z-index of 100, add Y position scaled down
                zIndex.layer = 100 + static_cast<int>(worldPos.y / 10.0f);
            }
        });

    // System to update piece opacity based on current player's turn
    auto pieceOpacitySystem = world.system<ImageRenderable>()
        .each([&](flecs::entity e, ImageRenderable& image) {
            // Only update pieces (not shadows or other images)
            std::string entityName = e.name().c_str();
            if (entityName.find("_piece_") != std::string::npos) {
                // Get the parent employee entity
                flecs::entity parent = e.parent();
                if (parent.is_valid() && parent.has<Employee>()) {
                    Employee emp = parent.get<Employee>();

                    // Check if this is the current player's piece
                    if (g_gameStateEntity.is_valid()) {
                        GameState gameState = g_gameStateEntity.get<GameState>();
                        bool isCurrentPlayer = (emp.playerIndex == gameState.currentPlayerIndex);

                        // Current player: 100% opacity, others: 70% opacity
                        image.alpha = isCurrentPlayer ? 1.0f : 0.7f;
                    }
                }
            }
        });

    // System to render dice allocation badges on employees
    auto employeeBadgeSystem = world.system<Position, Employee>()
        .term_at(0).second<World>()
        .each([&](flecs::entity e, Position& worldPos, Employee& employee) {
            if (employee.allocatedDice > 0) {
                RenderQueue& queue = world.ensure<RenderQueue>();

                // Create a circular badge above the employee's head
                float badgeX = worldPos.x; // Centered horizontally
                float badgeY = worldPos.y - 60.0f; // Further above the piece (head level)
                float badgeRadius = 18.0f;

                // Calculate Z-index based on employee position (same logic as depth sorting)
                // This ensures badge appears behind piece's head but in front of rows above
                int badgeZIndex = 100 + static_cast<int>(worldPos.y / 10.0f) + 1; // +1 to be in front of piece

                // Render circle badge with proper z-index
                queue.addCircleCommand(
                    {badgeX, badgeY},
                    {badgeRadius, 0xDC3232FF, 0xFFFFFF96, 2.0f}, // Red fill, white stroke (150 alpha)
                    badgeZIndex - 1 // Circle renders behind text but in front of pieces below
                );

                // Badge text (dice count) with proper z-index
                queue.addTextCommand(
                    {badgeX, badgeY + 5.0f}, // Slight offset for centering
                    {std::to_string(employee.allocatedDice), "CharisSIL-Regular", 20.0f, 0xFFFFFFFF, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE},
                    badgeZIndex // Text on top of circle
                );
            }
        });

    // System to render player stats as text
    auto playerStatsSystem = world.system<Position, Player>()
        .term_at(0).second<World>()
        .each([&](flecs::entity e, Position& worldPos, Player& player) {
            RenderQueue& queue = world.ensure<RenderQueue>();

            // Check if this is the current player's turn
            bool isCurrentPlayer = false;
            if (g_gameStateEntity.is_valid()) {
                GameState gameState = g_gameStateEntity.get<GameState>();
                if (gameState.currentPlayerIndex < gameState.playerEntities.size()) {
                    isCurrentPlayer = (e == gameState.playerEntities[gameState.currentPlayerIndex]);
                }
            }

            // Choose colors based on current turn
            uint32_t nameColor = isCurrentPlayer ? 0xFFFF00FF : 0xFFFFFFFF;  // Yellow if current player, white otherwise
            uint32_t labelColor = isCurrentPlayer ? 0xFFFF00FF : 0xFFFFFFFF;
            uint32_t valueColor = 0xFFFFFFFF;

            // Add background highlight for current player
            if (isCurrentPlayer) {
                queue.addRectCommand(
                    {worldPos.x - 10.0f, worldPos.y - 5.0f},
                    {200.0f, 120.0f, 0x333333AA},  // Semi-transparent dark background
                    14
                );
            }

            // Player name
            queue.addTextCommand(
                {worldPos.x, worldPos.y},
                {player.name + (isCurrentPlayer ? " (TURN)" : ""), "CharisSIL-Regular", 24.0f, nameColor, NVG_ALIGN_LEFT | NVG_ALIGN_TOP},
                15
            );

            // Money label and value
            queue.addTextCommand(
                {worldPos.x, worldPos.y + 30.0f},
                {"Cash:", "CharisSIL-Regular", 18.0f, labelColor, NVG_ALIGN_LEFT | NVG_ALIGN_TOP},
                15
            );
            queue.addTextCommand(
                {worldPos.x + 80.0f, worldPos.y + 30.0f},
                {std::to_string(player.money), "CharisSIL-Regular", 18.0f, valueColor, NVG_ALIGN_LEFT | NVG_ALIGN_TOP},
                15
            );

            // Tech label and value
            queue.addTextCommand(
                {worldPos.x, worldPos.y + 55.0f},
                {"Tech:", "CharisSIL-Regular", 18.0f, labelColor, NVG_ALIGN_LEFT | NVG_ALIGN_TOP},
                15
            );
            queue.addTextCommand(
                {worldPos.x + 80.0f, worldPos.y + 55.0f},
                {std::to_string(player.tech), "CharisSIL-Regular", 18.0f, valueColor, NVG_ALIGN_LEFT | NVG_ALIGN_TOP},
                15
            );

            queue.addTextCommand(
                {worldPos.x, worldPos.y + 80.0f},
                {"Recruit:", "CharisSIL-Regular", 18.0f, labelColor, NVG_ALIGN_LEFT | NVG_ALIGN_TOP},
                15
            );
            queue.addTextCommand(
                {worldPos.x + 80.0f, worldPos.y + 80.0f},
                {std::to_string(player.recruit), "CharisSIL-Regular", 18.0f, valueColor, NVG_ALIGN_LEFT | NVG_ALIGN_TOP},
                15
            );
        });

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        int winWidth, winHeight;
        glfwGetFramebufferSize(window, &winWidth, &winHeight);
        float pxRatio = (float)winWidth / (float)800;

        glViewport(0, 0, winWidth, winHeight);
        glClearColor(0.1f, 0.2f, 0.2f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Update window size in ECS
        windowEntity.set<Window>({window, winWidth, winHeight});

        nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

        // Run ECS systems (this will render all entities through the queue system)
        world.progress();

        nvgEndFrame(vg);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // nvgDeleteGL3(vg);
    glfwTerminate();
    return 0;
}
