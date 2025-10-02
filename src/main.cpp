#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <variant>
#include <string>
#include <fstream>
#include <sstream>
#include <random>
#include <iterator>
#include <cfloat>
#include <filesystem>
namespace fs = std::filesystem;

#include <fcntl.h>
#include <unistd.h> 
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <limits.h>

// https://www.glfw.org/docs/3.3/group__native.html
#define GLFW_EXPOSE_NATIVE_X11

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>

// Undef X11 macros that conflict with flecs (must be before flecs.h)
#ifdef Bool
#undef Bool
#endif

#include <flecs.h>

#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.h"

// Linux
#include "X11/X.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "libevdev/libevdev.h"

// LibVNC
#include <rfb/rfbclient.h>

// SDL (include after X11)
#include <SDL2/SDL.h>

// VNC Component to store client and framebuffer using SDL
struct VNCClient {
    rfbClient* client = nullptr;
    SDL_Window* sdlWindow = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
    SDL_Texture* sdlTexture = nullptr;
    SDL_Surface* surface = nullptr;
    int width = 0;
    int height = 0;
    bool needsUpdate = false;
};

// VNC resize callback - creates SDL surface for framebuffer (like SDL viewer)
static rfbBool vnc_resize_callback(rfbClient* client) {
    VNCClient* vncComp = (VNCClient*)rfbClientGetClientData(client, nullptr);

    int width = client->width;
    int height = client->height;
    int depth = client->format.bitsPerPixel;

    std::cout << "VNC resize callback: " << width << "x" << height << " depth: " << depth << std::endl;

    // Free old surface if it exists
    if (vncComp->surface) {
        SDL_FreeSurface(vncComp->surface);
    }

    // Create SDL surface for the framebuffer
    vncComp->surface = SDL_CreateRGBSurface(0, width, height, depth, 0, 0, 0, 0);
    if (!vncComp->surface) {
        std::cerr << "Failed to create SDL surface: " << SDL_GetError() << std::endl;
        return FALSE;
    }

    vncComp->width = width;
    vncComp->height = height;

    // Point VNC client's framebuffer to SDL surface pixels
    client->width = vncComp->surface->pitch / (depth / 8);
    client->frameBuffer = (uint8_t*)vncComp->surface->pixels;

    // Set pixel format based on SDL surface
    client->format.bitsPerPixel = depth;
    client->format.redShift = vncComp->surface->format->Rshift;
    client->format.greenShift = vncComp->surface->format->Gshift;
    client->format.blueShift = vncComp->surface->format->Bshift;
    client->format.redMax = vncComp->surface->format->Rmask >> client->format.redShift;
    client->format.greenMax = vncComp->surface->format->Gmask >> client->format.greenShift;
    client->format.blueMax = vncComp->surface->format->Bmask >> client->format.blueShift;

    std::cout << "SDL surface created: pitch=" << vncComp->surface->pitch << std::endl;
    std::cout << "  Format: bpp=" << (int)client->format.bitsPerPixel
              << " R=" << (int)client->format.redShift << "/" << (int)client->format.redMax
              << " G=" << (int)client->format.greenShift << "/" << (int)client->format.greenMax
              << " B=" << (int)client->format.blueShift << "/" << (int)client->format.blueMax << std::endl;
    std::cout << "  SDL Rmask=0x" << std::hex << vncComp->surface->format->Rmask
              << " Gmask=0x" << vncComp->surface->format->Gmask
              << " Bmask=0x" << vncComp->surface->format->Bmask
              << " Amask=0x" << vncComp->surface->format->Amask << std::dec << std::endl;

    SetFormatAndEncodings(client);

    // (Re)create SDL texture if we have a renderer
    if (vncComp->sdlRenderer) {
        if (vncComp->sdlTexture) {
            SDL_DestroyTexture(vncComp->sdlTexture);
        }
        vncComp->sdlTexture = SDL_CreateTexture(vncComp->sdlRenderer,
                                                SDL_PIXELFORMAT_ARGB8888,
                                                SDL_TEXTUREACCESS_STREAMING,
                                                width, height);
        if (!vncComp->sdlTexture) {
            std::cerr << "Failed to create SDL texture: " << SDL_GetError() << std::endl;
        } else {
            std::cout << "Created SDL texture for " << width << "x" << height << std::endl;
        }
    }

    return TRUE;
}

static void vnc_update_callback(rfbClient* client, int x, int y, int w, int h) {
    VNCClient* vncComp = (VNCClient*)rfbClientGetClientData(client, nullptr);
    vncComp->needsUpdate = true;

    static int updateCount = 0;
    if (updateCount++ < 5) {
        std::cout << "VNC update callback: region (" << x << "," << y << "," << w << "," << h << ")" << std::endl;
    }
}

rfbClient* connectToTurboVNC(const char* host, int port, VNCClient* vncComp) {
    rfbClient* client = rfbGetClient(8, 3, 4);

    client->appData.encodingsString = "tight copyrect";
    client->appData.compressLevel = 1;
    client->appData.qualityLevel = 8;
    client->appData.enableJPEG = TRUE;

    client->serverHost = strdup(host);
    client->serverPort = port;

    // Set callbacks
    client->MallocFrameBuffer = vnc_resize_callback;
    client->GotFrameBufferUpdate = vnc_update_callback;

    // Store VNCClient reference
    rfbClientSetClientData(client, nullptr, vncComp);

    if (!rfbInitClient(client, NULL, NULL)) {
        return NULL;
    }

    return client;
}

// ---- evdev
struct EvdevEventData
{
  unsigned int event_type;
  unsigned int event_code; 
};

struct DeviceEventListener
{
  const char* driver;
};

struct LibEvDevice
{
  libevdev *dev = NULL;
};


static void
print_abs_bits(struct libevdev *dev, int axis)
{
	const struct input_absinfo *abs;

	if (!libevdev_has_event_code(dev, EV_ABS, axis))
		return;

	abs = libevdev_get_abs_info(dev, axis);

	printf("	Value	%6d\n", abs->value);
	printf("	Min	%6d\n", abs->minimum);
	printf("	Max	%6d\n", abs->maximum);
	if (abs->fuzz)
		printf("	Fuzz	%6d\n", abs->fuzz);
	if (abs->flat)
		printf("	Flat	%6d\n", abs->flat);
	if (abs->resolution)
		printf("	Resolution	%6d\n", abs->resolution);
}

void run_python_script(const std::string& script_call) {
    system(script_call.c_str());
}

static void
print_code_bits(struct libevdev *dev, unsigned int type, unsigned int max)
{
	unsigned int i;
	for (i = 0; i <= max; i++) {
		if (!libevdev_has_event_code(dev, type, i))
			continue;

		printf("    Event code %i (%s)\n", i, libevdev_event_code_get_name(type, i));
		if (type == EV_ABS)
			print_abs_bits(dev, i);
	}
}

static void
print_bits(struct libevdev *dev)
{
	unsigned int i;
	printf("Supported events:\n");

	for (i = 0; i <= EV_MAX; i++) {
		if (libevdev_has_event_type(dev, i))
			printf("  Event type %d (%s)\n", i, libevdev_event_type_get_name(i));
		switch(i) {
			case EV_KEY:
				print_code_bits(dev, EV_KEY, KEY_MAX);
				break;
			case EV_REL:
				print_code_bits(dev, EV_REL, REL_MAX);
				break;
			case EV_ABS:
				print_code_bits(dev, EV_ABS, ABS_MAX);
				break;
			case EV_LED:
				print_code_bits(dev, EV_LED, LED_MAX);
				break;
		}
	}
}

static void
print_props(struct libevdev *dev)
{
	unsigned int i;
	printf("Properties:\n");

	for (i = 0; i <= INPUT_PROP_MAX; i++) {
		if (libevdev_has_property(dev, i))
			printf("  Property type %d (%s)\n", i,
					libevdev_property_get_name(i));
	}
}

static int
print_event(struct input_event *ev)
{
	if (ev->type == EV_SYN)
		printf("Event: time %ld.%06ld, ++++++++++++++++++++ %s +++++++++++++++\n",
				ev->input_event_sec,
				ev->input_event_usec,
				libevdev_event_type_get_name(ev->type));
	else
		printf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %d\n",
			ev->input_event_sec,
			ev->input_event_usec,
			ev->type,
			libevdev_event_type_get_name(ev->type),
			ev->code,
			libevdev_event_code_get_name(ev->type, ev->code),
			ev->value);
	return 0;
}

static int
print_sync_event(struct input_event *ev)
{
	printf("SYNC: ");
	// print_event(ev);
	return 0;
}

std::vector<std::string> find_devices(std::vector<EvdevEventData> capabilities)
{
  std::vector<std::string> devices;
  DIR *dir;
  struct dirent *ent;
  const char *path = "/dev/input";

  if ((dir = opendir(path)) != NULL) {
      while ((ent = readdir(dir)) != NULL) {
          if (strncmp(ent->d_name, "event", 5) == 0) {
              char full_path[PATH_MAX];
              snprintf(full_path, sizeof(full_path), "%s/%s", path, ent->d_name);

              int fd = open(full_path, O_RDONLY | O_NONBLOCK);
              if (fd < 0) {
                  std::cerr << "Cannot open " << full_path << std::endl;
                  continue;
              }

              struct libevdev *dev = NULL;
              int rc = libevdev_new_from_fd(fd, &dev);
              if (rc < 0) {
                  std::cerr << "Failed to init libevdev (" << full_path << "). Error: " << strerror(-rc) << std::endl;
                  close(fd);
                  continue;
              }

              bool has_capabilities = true;
              for (auto capability : capabilities) 
              {
                if (libevdev_has_event_type(dev, capability.event_type) && libevdev_has_event_code(dev, capability.event_type, capability.event_code)) {
                } else
                {
                  has_capabilities = false;
                }
              }
              if (has_capabilities)
              {
                devices.push_back(std::string(full_path));
              }

              // Free up the device and close file descriptor if it's not a keyboard
              libevdev_free(dev);
              close(fd);
          }
      }
      closedir(dir);
  } else {
      std::cerr << "Could not open directory: " << path << std::endl;
  }

  return devices;
}
// ---- evdev

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
    int spriteX = 0;      // Sprite sheet X offset (for animated sprites)
    int spriteY = 0;      // Sprite sheet Y offset (for animated sprites)
    int spriteW = 0;      // Total sprite sheet width (0 = not a sprite sheet)
    int spriteH = 0;      // Total sprite sheet height
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
    int rows = 1;  // Sprite sheet rows
    int cols = 1;  // Sprite sheet columns
};

struct Sprite {
    int img;        // NanoVG image handle
    int x, y;       // Current sprite sheet position
    int w, h;       // Total image width/height
    int c_w, c_h;   // Cell width/height (single frame size)
    int rows, cols; // Number of rows/columns in sprite sheet
};

struct SpriteAnimator {
    int frame = 0;
    float progress = 0.0f;
    float framerate = 0.15f;  // 150ms per frame
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

struct CursorState
{
  int x;
  int y;
};

struct KeyboardState
{
  std::unordered_map<unsigned short, int> key_states;
};

struct GameWindow {
    GLFWwindow* handle;
    int width, height;
};

struct Graphics {
    NVGcontext* vg;
};

enum class RenderType {
    Rectangle,
    Text,
    Image
};

struct RenderCommand {
    Position pos;
    std::variant<RectRenderable, TextRenderable, ImageRenderable> renderData;
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

    void sort() {
        std::sort(commands.begin(), commands.end());
    }
};

void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Global world reference for mouse callback
flecs::world* g_world = nullptr;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS && g_world) {
        // Get mouse position
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        c2v mousePoint = {(float)mouseX, (float)mouseY};
        bool clickHandled = false;

        std::cout << "\n=== MOUSE CLICK DEBUG ===" << std::endl;
        std::cout << "Mouse clicked at (" << mouseX << ", " << mouseY << ")" << std::endl;

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

            std::cout << "  Clickable entity #" << clickableCount << ": " << e.name()
                      << " world: (" << worldPos.x << ", " << worldPos.y << ")"
                      << " local: (" << localPos.x << ", " << localPos.y << ")"
                      << " hasParent: " << (hasParent ? "yes" : "no")
                      << " event: " << clickable.clickEventType << std::endl;
            std::cout << "    Bounds: (" << clickable.bounds.min.x << "," << clickable.bounds.min.y
                      << ") to (" << clickable.bounds.max.x << "," << clickable.bounds.max.y << ")" << std::endl;

            // Calculate actual clickable area
            c2AABB entityBounds;
            entityBounds.min.x = worldPos.x + clickable.bounds.min.x;
            entityBounds.min.y = worldPos.y + clickable.bounds.min.y;
            entityBounds.max.x = worldPos.x + clickable.bounds.max.x;
            entityBounds.max.y = worldPos.y + clickable.bounds.max.y;

            std::cout << "    World bounds: (" << entityBounds.min.x << "," << entityBounds.min.y
                      << ") to (" << entityBounds.max.x << "," << entityBounds.max.y << ")" << std::endl;
        });

        std::cout << "Total clickable entities: " << clickableCount << std::endl;

        std::cout << "\nChecking collisions..." << std::endl;

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
                    std::cout << "  Computed world pos for " << e.name()
                              << ": parent(" << parentPos.x << "," << parentPos.y
                              << ") + local(" << localPos.x << "," << localPos.y
                              << ") = (" << actualWorldPos.x << "," << actualWorldPos.y << ")" << std::endl;
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
            std::cout << "  Testing " << e.name() << " at (" << actualWorldPos.x << "," << actualWorldPos.y << "): " << (collision ? "HIT!" : "miss") << std::endl;

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

                std::cout << "*** CLICKED ON ENTITY: " << e.name() << " (event: " << clickable.clickEventType << ") ***" << std::endl;
                clickHandled = true;
            }
        });

        std::cout << "Click result: " << (clickHandled ? "HANDLED" : "NOT HANDLED") << std::endl;
        std::cout << "=== END CLICK DEBUG ===\n" << std::endl;

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
        std::cout << "Debug render mode: " << (debugRender.enabled ? "ON" : "OFF") << std::endl;
    }

    dotKeyPressed = dotKeyDown;
}

int main(int, char *[]) {

    // Initialize SDL (only need video subsystem for surface creation)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        SDL_Quit();
        return -1;
    }
    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    Display *display = XOpenDisplay(NULL);
    Window root = RootWindow(display, DefaultScreen(display));
    
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "GraphSail RPG", monitor, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    system("wmctrl -r \"GraphSail\" -b add,above");
    
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glfwSwapInterval(0);
    glViewport(0, 0, mode->width, mode->height);

    // Should add after gladLoadGL():

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // This doesn't get called if you need a transparent overlay
    // glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Hide the system cursor since we're drawing our own
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

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
    world.component<Clickable>();
    world.component<ClickEvent>();
    world.component<DebugRender>();
    world.component<UIElementBounds>();
    world.component<RenderStatus>();
    world.component<CursorState>();
    world.component<KeyboardState>();
    world.component<Sprite>();
    world.component<SpriteAnimator>();
    world.component<GameWindow>();
    world.component<Graphics>().add(flecs::Singleton);
    world.component<RenderQueue>();
    world.component<VNCClient>();

    // Create singleton entities for global resources
    auto windowEntity = world.entity("GameWindow")
        .set<GameWindow>({window, mode->width, mode->height});

    auto graphicsEntity = world.entity("Graphics")
        .set<Graphics>({vg});

    auto renderQueueEntity = world.entity("RenderQueue")
        .set<RenderQueue>({});

    // Create debug render entity
    g_debugRenderEntity = world.entity("DebugRender")
        .set<DebugRender>({});

    auto user_input = world.entity("user_input");
    user_input.add<KeyboardState>();
    user_input.add<CursorState>();

    world.observer<DeviceEventListener>("ListenToDevice").event(flecs::OnSet)
        .each([](flecs::entity e, DeviceEventListener& device)
        {
            libevdev *dev = NULL; // struct?
            const char *file;
            int fd;
            int rc = 1;

            // TODO: Find mouse/keyboard input event files/refactor...
            file = device.driver; // keyboard
            fd = open(file, O_RDONLY);
            if (fd < 0) {
            perror("Failed to open device");
            return;
            }

            rc = libevdev_new_from_fd(fd, &dev);
            if (rc < 0) {
            fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
            return;
            }

            // printf("Input device ID: bus %#x vendor %#x product %#x\n",
            //     libevdev_get_id_bustype(dev),
            //     libevdev_get_id_vendor(dev),
            //     libevdev_get_id_product(dev));
            // printf("Evdev version: %x\n", libevdev_get_driver_version(dev));
            // printf("Input device name: \"%s\"\n", libevdev_get_name(dev));
            // printf("Phys location: %s\n", libevdev_get_phys(dev));
            // printf("Uniq identifier: %s\n", libevdev_get_uniq(dev));
            // print_bits(dev);
            // print_props(dev);
            e.set<LibEvDevice>({dev});
        });

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

                // Check if this is a sprite sheet (rows/cols > 1)
                if (createSprite.rows > 1 || createSprite.cols > 1) {
                    // Calculate cell dimensions
                    int cellWidth = imgWidth / createSprite.cols;
                    int cellHeight = imgHeight / createSprite.rows;

                    // Create Sprite component for sprite sheet animation
                    e.set<Sprite>({
                        imageHandle,
                        0, 0,  // Initial sprite position (x, y)
                        imgWidth, imgHeight,  // Total image dimensions
                        cellWidth, cellHeight,  // Cell dimensions
                        createSprite.rows, createSprite.cols
                    });

                    // Add animator component to enable animation
                    e.set<SpriteAnimator>({});
                } else {
                    // Regular static image - use ImageRenderable
                    e.set<ImageRenderable>({imageHandle, finalWidth, finalHeight, createSprite.alpha});
                }

                // Remove the temporary CreateSprite component
                e.remove<CreateSprite>();

            } else {
                std::cerr << "Failed to load image: " << createSprite.imagePath << std::endl;
                // Remove the component even if loading failed to avoid infinite loops
                e.remove<CreateSprite>();
            }
        });
        

    // Create cursor entity with 8-frame animation
    auto cursorEntity = world.entity("Cursor")
        .set<Position, Local>({0.0f, 0.0f})
        .set<Position, World>({0.0f, 0.0f})
        .set<CreateSprite>({"../assets/arrow.png", 1.0f, 1.0f, 1.0f, false, 1, 8})  // 1 row, 8 columns
        .set<ZIndex>({1000}); // High z-index to render on top

    // Create SDL window/renderer from GLFW's X11 window
    Window x11Window = glfwGetX11Window(window);
    std::cout << "GLFW X11 window: " << x11Window << std::endl;

    // Create SDL window from existing X11 window
    SDL_Window* sdlWindow = SDL_CreateWindowFrom((void*)(uintptr_t)x11Window);
    if (!sdlWindow) {
        std::cerr << "Failed to create SDL window from X11: " << SDL_GetError() << std::endl;
    } else {
        std::cout << "Created SDL window from GLFW X11 window" << std::endl;
    }

    // Create SDL renderer
    SDL_Renderer* sdlRenderer = nullptr;
    if (sdlWindow) {
        sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!sdlRenderer) {
            std::cerr << "Failed to create SDL renderer: " << SDL_GetError() << std::endl;
        } else {
            std::cout << "Created SDL renderer" << std::endl;
            SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
        }
    }

    // Create VNC client entity
    auto vncEntity = world.entity("VNCClient");
    VNCClient vncComp;
    vncComp.sdlWindow = sdlWindow;
    vncComp.sdlRenderer = sdlRenderer;

    // Connect to x11vnc on localhost:5901
    vncComp.client = connectToTurboVNC("localhost", 5901, &vncComp);

    if (vncComp.client) {
        std::cout << "VNC connected successfully to localhost:5901" << std::endl;

        // Request initial framebuffer update
        SendFramebufferUpdateRequest(vncComp.client, 0, 0, vncComp.client->width, vncComp.client->height, FALSE);

        // Wait for and process the initial frame
        std::cout << "Waiting for initial VNC frame..." << std::endl;
        int timeout = 0;
        bool gotUpdate = false;

        // First wait creates the surface
        while (!vncComp.surface && timeout < 5000) {
            int result = WaitForMessage(vncComp.client, 100000); // 100ms timeout
            if (result > 0) {
                if (!HandleRFBServerMessage(vncComp.client)) {
                    std::cerr << "Failed to get initial frame" << std::endl;
                    break;
                }
            }
            timeout += 100;
        }

        std::cout << "Surface created, now waiting for actual frame data..." << std::endl;

        // Now wait for actual frame data to arrive
        timeout = 0;
        while (!gotUpdate && timeout < 5000) {
            int result = WaitForMessage(vncComp.client, 100000); // 100ms timeout
            if (result > 0) {
                if (!HandleRFBServerMessage(vncComp.client)) {
                    std::cerr << "Failed to get frame data" << std::endl;
                    break;
                }
                if (vncComp.needsUpdate) {
                    gotUpdate = true;
                    std::cout << "Got actual frame data!" << std::endl;
                }
            }
            timeout += 100;
        }

        if (!gotUpdate) {
            std::cout << "Warning: Never received frame data update" << std::endl;
        }

        if (vncComp.surface && vncComp.surface->pixels && gotUpdate) {
            std::cout << "Got VNC frame with data: " << vncComp.width << "x" << vncComp.height << std::endl;

            // Save SDL surface as BMP for debugging
            const char* bmpPath = "../assets/vnc_sdl_capture.bmp";
            if (SDL_SaveBMP(vncComp.surface, bmpPath) == 0) {
                std::cout << "Saved SDL surface to " << bmpPath << std::endl;
            }

            // Update SDL texture with surface data
            if (vncComp.sdlTexture) {
                SDL_Rect rect = {0, 0, vncComp.surface->w, vncComp.surface->h};
                if (SDL_UpdateTexture(vncComp.sdlTexture, &rect, vncComp.surface->pixels, vncComp.surface->pitch) < 0) {
                    std::cerr << "Failed to update SDL texture: " << SDL_GetError() << std::endl;
                } else {
                    std::cout << "Updated SDL texture with VNC frame data" << std::endl;
                }
            }
        } else {
            std::cerr << "Failed to get initial VNC frame with data" << std::endl;
        }

        // Disconnect - we don't need the connection anymore
        rfbClientCleanup(vncComp.client);
        vncComp.client = nullptr;
    } else {
        std::cerr << "Failed to connect to VNC server on localhost:5901" << std::endl;
    }

    vncEntity.set<VNCClient>(vncComp);

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

            static int debugCount = 0;
            if (debugCount++ % 60 == 0) {
                const char* name = e.name() ? e.name() : "unnamed";
                std::cout << "Queue image: " << name << " at (" << worldPos.x << "," << worldPos.y
                          << ") size: " << renderable.width << "x" << renderable.height
                          << " handle: " << renderable.imageHandle << " z: " << zIndex.layer << std::endl;
            }
        });

    // Sprite animation system
    auto spriteAnimationSystem = world.system<Sprite, SpriteAnimator>()
        .each([](flecs::iter& it, size_t i, Sprite& sprite, SpriteAnimator& animator) {
            animator.progress += it.delta_time();
            while (animator.progress > animator.framerate) {
                animator.progress -= animator.framerate;

                if (sprite.rows > 1) {
                    // Vertical animation (down the sprite sheet)
                    if (sprite.y + sprite.c_h >= sprite.h) {
                        sprite.y = 0;
                    } else {
                        sprite.y += sprite.c_h;
                    }
                } else if (sprite.cols > 1) {
                    // Horizontal animation (across the sprite sheet)
                    if (sprite.x + sprite.c_w >= sprite.w) {
                        sprite.x = 0;
                    } else {
                        sprite.x += sprite.c_w;
                    }
                }
            }
        });

    // Sprite rendering system
    auto spriteRenderSystem = world.system<Position, Sprite, ZIndex>()
        .term_at(0).second<World>()
        .each([&](flecs::entity e, Position& worldPos, Sprite& sprite, ZIndex& zIndex) {
            RenderQueue& queue = world.ensure<RenderQueue>();
            // Create an ImageRenderable for the current frame with sprite sheet info
            ImageRenderable frameRenderable = {
                sprite.img,
                (float)sprite.c_w,
                (float)sprite.c_h,
                1.0f,
                sprite.x,
                sprite.y,
                sprite.w,
                sprite.h
            };
            queue.addImageCommand(worldPos, frameRenderable, zIndex.layer);
        });

    // world.system<LibEvDevice, KeyboardState, CursorState>("PollEvents")
    // .term_at(1).src(user_input)
    // .term_at(2).src(user_input)
    // .kind(flecs::PreUpdate).write(flecs::Wildcard)
    // .each([display, root](flecs::entity e, LibEvDevice& levd, KeyboardState& keyboard, CursorState& cursor_state, Overlay& sail, DefaultSymbolInteraction& def)
    // {
    // });

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
                            static int imgDebugCount = 0;
                            if (imgDebugCount++ % 60 == 0) {
                                std::cout << "Actually rendering image at (" << cmd.pos.x << "," << cmd.pos.y
                                         << ") size: " << image.width << "x" << image.height
                                         << " handle: " << image.imageHandle << std::endl;
                            }

                            // Set global alpha for proper transparency blending
                            nvgGlobalAlpha(graphics.vg, image.alpha);

                            // Check if this is a sprite sheet (has sprite coordinates)
                            if (image.spriteW > 0 && image.spriteH > 0) {
                                // Render sprite sheet frame using scissor and offset pattern
                                nvgSave(graphics.vg);
                                nvgScissor(graphics.vg, cmd.pos.x, cmd.pos.y, image.width, image.height);

                                nvgBeginPath(graphics.vg);
                                nvgRect(graphics.vg, cmd.pos.x, cmd.pos.y, image.width, image.height);

                                // Offset the pattern to show only the current frame
                                float offsetX = cmd.pos.x - (float)image.spriteX;
                                float offsetY = cmd.pos.y - (float)image.spriteY;

                                NVGpaint imgPaint = nvgImagePattern(graphics.vg,
                                    offsetX, offsetY,
                                    (float)image.spriteW, (float)image.spriteH,
                                    0.0f, image.imageHandle, 1.0f);

                                nvgFillPaint(graphics.vg, imgPaint);
                                nvgFill(graphics.vg);
                                nvgRestore(graphics.vg);
                            } else {
                                // Regular image rendering
                                nvgBeginPath(graphics.vg);
                                nvgRect(graphics.vg, cmd.pos.x, cmd.pos.y, image.width, image.height);

                                NVGpaint imgPaint = nvgImagePattern(graphics.vg,
                                    cmd.pos.x, cmd.pos.y,
                                    image.width, image.height,
                                    0.0f, image.imageHandle, 1.0f);

                                nvgFillPaint(graphics.vg, imgPaint);
                                nvgFill(graphics.vg);
                            }

                            // Reset global alpha after rendering
                            nvgGlobalAlpha(graphics.vg, 1.0f);
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

                std::cout << "Updated clickable bounds for " << e.name()
                         << " to " << image.width << "x" << image.height << " (top-left positioned)" << std::endl;
            }
        });

    // Cursor tracking system - virtual cursor follows mouse but stays within radius
    auto cursorTrackingSystem = world.system<Position>()
        .term_at(0).second<Local>()
        .kind(flecs::PreUpdate)
        .each([&](flecs::entity e, Position& pos) {
            const CursorState* mousePos = world.try_get<CursorState>();
            if (mousePos) {
                float follow_radius = 32.0f;
                float close_x, close_y;
                get_closest_radius_point(pos.x, pos.y, mousePos->x, mousePos->y, follow_radius, &close_x, &close_y);
                pos.x = close_x;
                pos.y = close_y;

                // Debug output
                static int frame_count = 0;
                if (frame_count++ % 60 == 0) {
                    std::cout << "Cursor pos: (" << pos.x << ", " << pos.y << "), Mouse: ("
                              << mousePos->x << ", " << mousePos->y << ")" << std::endl;
                }
            } else {
                static bool warned = false;
                if (!warned) {
                    std::cout << "WARNING: CursorState not found!" << std::endl;
                    warned = true;
                }
            }
        });

    // VNC update system removed - we now capture one frame at startup

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
        windowEntity.set<GameWindow>({window, winWidth, winHeight});

        // Render SDL texture first (VNC desktop background)
        const VNCClient* vncData = vncEntity.try_get<VNCClient>();
        if (vncData && vncData->sdlRenderer && vncData->sdlTexture) {
            SDL_RenderClear(vncData->sdlRenderer);
            SDL_RenderCopy(vncData->sdlRenderer, vncData->sdlTexture, NULL, NULL);
            SDL_RenderPresent(vncData->sdlRenderer);
        }

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