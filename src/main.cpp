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

// Clean up X11 macro pollution before C++ libs that use `Bool`, `Status`, etc.
#ifdef Bool
#  undef Bool
#endif
#ifdef Status
#  undef Status
#endif
#ifdef None
#  undef None      // (Optional, X11 defines None; can bite with std::optional, enums, etc.)
#endif

#include <nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>

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

// SDL for texture creation from VNC framebuffer
#include <SDL2/SDL.h>

// VNC Client Components
#define NUM_VNC_CLIENTS 4

struct VNCClient {
    rfbClient* client = nullptr;
    SDL_Surface* surface = nullptr;
    bool connected = false;
    std::string host;
    int port;
    int width = 0;
    int height = 0;
    int quadrant = 0;  // Which quadrant this client belongs to (0-3)
};

struct VNCUpdateRect {
    int x, y, w, h;
};

struct VNCTexture {
    GLuint texture = 0;
    int nvgHandle = -1;
    int width = 0;
    int height = 0;
    bool needsUpdate = false;
    std::vector<VNCUpdateRect> dirtyRects;  // Track which regions need updating
};

// Global VNC view state
struct VNCViewState {
    bool quadrantView = true;  // true = quadrant view (no input), false = interactive mode
    int activeQuadrant = 0;    // 0=top-left, 1=top-right, 2=bottom-left, 3=bottom-right
};

void set_window_type_desktop(Display* dpy, Window win) {
    Atom wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    Atom wm_window_type_desktop = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_UTILITY", False);

    XChangeProperty(
        dpy,
        win,
        wm_window_type,
        XA_ATOM,
        32,
        PropModeReplace,
        (unsigned char*)&wm_window_type_desktop,
        1
    );
    XFlush(dpy);
}

void set_window_state_above(Display* dpy, Window win) {
    Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    Atom wm_above = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);

    XChangeProperty(
        dpy,
        win,
        wm_state,
        XA_ATOM,
        32,
        PropModeAppend,
        (unsigned char*)&wm_above,
        1
    );
    XFlush(dpy);
}

// Global reference to ECS world for VNC callbacks
static flecs::world* g_vnc_world = nullptr;

// Tag for VNC client data storage
static const char* VNC_SURFACE_TAG = "vnc_surface";

// VNC callback: Resize framebuffer
static rfbBool vnc_resize_callback(rfbClient* client) {
    std::cout << "[VNC] Resize callback - width: " << client->width
              << ", height: " << client->height
              << ", depth: " << client->format.bitsPerPixel << std::endl;

    int width = client->width;
    int height = client->height;
    int depth = client->format.bitsPerPixel;

    // Free old surface
    SDL_Surface* oldSurface = (SDL_Surface*)rfbClientGetClientData(client, (void*)VNC_SURFACE_TAG);
    if (oldSurface) {
        std::cout << "[VNC] Freeing old surface" << std::endl;
        SDL_FreeSurface(oldSurface);
    }

    // Create new surface for framebuffer
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, depth, 0, 0, 0, 0);
    if (!surface) {
        std::cerr << "[VNC ERROR] Failed to create surface: " << SDL_GetError() << std::endl;
        return FALSE;
    }

    std::cout << "[VNC] Created new surface: " << width << "x" << height
              << " @ " << depth << "bpp" << std::endl;

    // Store surface in client data
    rfbClientSetClientData(client, (void*)VNC_SURFACE_TAG, surface);

    // Configure framebuffer
    // Note: Don't modify client->width based on pitch - keep actual width
    // The pitch may be larger due to alignment, but width should be display width
    client->frameBuffer = (uint8_t*)surface->pixels;

    // Set pixel format
    client->format.bitsPerPixel = depth;
    client->format.redShift = surface->format->Rshift;
    client->format.greenShift = surface->format->Gshift;
    client->format.blueShift = surface->format->Bshift;
    client->format.redMax = surface->format->Rmask >> client->format.redShift;
    client->format.greenMax = surface->format->Gmask >> client->format.greenShift;
    client->format.blueMax = surface->format->Bmask >> client->format.blueShift;

    std::cout << "[VNC] Pixel format - R shift: " << client->format.redShift
              << ", G shift: " << client->format.greenShift
              << ", B shift: " << client->format.blueShift << std::endl;

    SetFormatAndEncodings(client);
    std::cout << "[VNC] Resize complete" << std::endl;

    return TRUE;
}

// Helper to get which quadrant a client belongs to
static int getClientQuadrant(rfbClient* client) {
    if (g_vnc_world) {
        auto query = g_vnc_world->query<VNCClient>();
        int foundQuadrant = 0;
        query.each([&](flecs::entity e, VNCClient& vnc) {
            if (vnc.client == client) {
                foundQuadrant = vnc.quadrant;
            }
        });
        return foundQuadrant;
    }
    return 0;
}

// VNC callback: Framebuffer update
static void vnc_update_callback(rfbClient* client, int x, int y, int w, int h) {
    int quadrant = getClientQuadrant(client);
    std::cout << "[VNC UPDATE] Quadrant " << quadrant << " framebuffer updated - rect: (" << x << "," << y
              << ") size: " << w << "x" << h << std::endl;

    // Mark texture region for update in ECS - find the texture for this specific client
    if (g_vnc_world) {
        auto query = g_vnc_world->query<VNCClient, VNCTexture>();
        int updateCount = 0;
        query.each([&](flecs::entity e, VNCClient& vnc, VNCTexture& tex) {
            if (vnc.client == client) {
                // Add the dirty rectangle to the update queue
                tex.dirtyRects.push_back({x, y, w, h});
                tex.needsUpdate = true;
                updateCount++;
                std::cout << "[VNC UPDATE] Added dirty rect to quadrant " << quadrant
                          << " - total rects: " << tex.dirtyRects.size() << std::endl;
            }
        });
        std::cout << "[VNC UPDATE] Marked " << updateCount << " textures for update" << std::endl;
    } else {
        std::cerr << "[VNC ERROR] g_vnc_world is null in update callback!" << std::endl;
    }
}

rfbClient* connectToTurboVNC(const char* host, int port) {
    std::cout << "[VNC] Connecting to " << host << ":" << port << std::endl;

    rfbClient* client = rfbGetClient(8, 3, 4);

    // Set callbacks
    client->MallocFrameBuffer = vnc_resize_callback;
    client->canHandleNewFBSize = TRUE;
    client->GotFrameBufferUpdate = vnc_update_callback;

    // Enable TurboVNC/TurboJPEG compression
    client->appData.encodingsString = "tight copyrect";
    client->appData.compressLevel = 1;
    client->appData.qualityLevel = 8;
    client->appData.enableJPEG = TRUE;

    client->serverHost = strdup(host);
    client->serverPort = port;

    std::cout << "[VNC] Initializing client..." << std::endl;
    if (!rfbInitClient(client, NULL, NULL)) {
        std::cerr << "[VNC ERROR] Failed to initialize client" << std::endl;
        return NULL;
    }

    std::cout << "[VNC] Connected successfully!" << std::endl;
    std::cout << "[VNC] Desktop: " << client->desktopName << std::endl;
    std::cout << "[VNC] Size: " << client->width << "x" << client->height << std::endl;

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

struct RenderTexture {
    GLuint fbo;
    GLuint texture;
    int width;
    int height;
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

// GLFW to X11 keysym mapping for VNC input
rfbKeySym glfw_key_to_rfb_keysym(int key) {
    switch (key) {
        case GLFW_KEY_BACKSPACE: return XK_BackSpace;
        case GLFW_KEY_TAB: return XK_Tab;
        case GLFW_KEY_ENTER: return XK_Return;
        case GLFW_KEY_PAUSE: return XK_Pause;
        case GLFW_KEY_ESCAPE: return XK_Escape;
        case GLFW_KEY_DELETE: return XK_Delete;
        case GLFW_KEY_KP_0: return XK_KP_0;
        case GLFW_KEY_KP_1: return XK_KP_1;
        case GLFW_KEY_KP_2: return XK_KP_2;
        case GLFW_KEY_KP_3: return XK_KP_3;
        case GLFW_KEY_KP_4: return XK_KP_4;
        case GLFW_KEY_KP_5: return XK_KP_5;
        case GLFW_KEY_KP_6: return XK_KP_6;
        case GLFW_KEY_KP_7: return XK_KP_7;
        case GLFW_KEY_KP_8: return XK_KP_8;
        case GLFW_KEY_KP_9: return XK_KP_9;
        case GLFW_KEY_KP_DECIMAL: return XK_KP_Decimal;
        case GLFW_KEY_KP_DIVIDE: return XK_KP_Divide;
        case GLFW_KEY_KP_MULTIPLY: return XK_KP_Multiply;
        case GLFW_KEY_KP_SUBTRACT: return XK_KP_Subtract;
        case GLFW_KEY_KP_ADD: return XK_KP_Add;
        case GLFW_KEY_KP_ENTER: return XK_KP_Enter;
        case GLFW_KEY_KP_EQUAL: return XK_KP_Equal;
        case GLFW_KEY_UP: return XK_Up;
        case GLFW_KEY_DOWN: return XK_Down;
        case GLFW_KEY_RIGHT: return XK_Right;
        case GLFW_KEY_LEFT: return XK_Left;
        case GLFW_KEY_INSERT: return XK_Insert;
        case GLFW_KEY_HOME: return XK_Home;
        case GLFW_KEY_END: return XK_End;
        case GLFW_KEY_PAGE_UP: return XK_Page_Up;
        case GLFW_KEY_PAGE_DOWN: return XK_Page_Down;
        case GLFW_KEY_F1: return XK_F1;
        case GLFW_KEY_F2: return XK_F2;
        case GLFW_KEY_F3: return XK_F3;
        case GLFW_KEY_F4: return XK_F4;
        case GLFW_KEY_F5: return XK_F5;
        case GLFW_KEY_F6: return XK_F6;
        case GLFW_KEY_F7: return XK_F7;
        case GLFW_KEY_F8: return XK_F8;
        case GLFW_KEY_F9: return XK_F9;
        case GLFW_KEY_F10: return XK_F10;
        case GLFW_KEY_F11: return XK_F11;
        case GLFW_KEY_F12: return XK_F12;
        case GLFW_KEY_F13: return XK_F13;
        case GLFW_KEY_F14: return XK_F14;
        case GLFW_KEY_F15: return XK_F15;
        case GLFW_KEY_NUM_LOCK: return XK_Num_Lock;
        case GLFW_KEY_CAPS_LOCK: return XK_Caps_Lock;
        case GLFW_KEY_SCROLL_LOCK: return XK_Scroll_Lock;
        case GLFW_KEY_RIGHT_SHIFT: return XK_Shift_R;
        case GLFW_KEY_LEFT_SHIFT: return XK_Shift_L;
        case GLFW_KEY_RIGHT_CONTROL: return XK_Control_R;
        case GLFW_KEY_LEFT_CONTROL: return XK_Control_L;
        case GLFW_KEY_RIGHT_ALT: return XK_Alt_R;
        case GLFW_KEY_LEFT_ALT: return XK_Alt_L;
        case GLFW_KEY_RIGHT_SUPER: return XK_Super_R;
        case GLFW_KEY_LEFT_SUPER: return XK_Super_L;
        case GLFW_KEY_MENU: return XK_Menu;
        case GLFW_KEY_PRINT_SCREEN: return XK_Print;
        default:
            // For regular character keys
            if (key >= GLFW_KEY_SPACE && key <= GLFW_KEY_GRAVE_ACCENT) {
                return key;
            }
            return 0;
    }
}

// Keyboard callback for VNC input passthrough
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (!g_world) return;

    // Check if we're in interactive mode
    const VNCViewState* viewState = g_world->try_get<VNCViewState>();
    if (!viewState || viewState->quadrantView) return;  // No input in quadrant view mode

    // Find the active VNC client
    auto query = g_world->query<VNCClient>();
    query.each([&](flecs::entity e, VNCClient& vnc) {
        if (vnc.quadrant == viewState->activeQuadrant && vnc.connected && vnc.client) {
            rfbKeySym keysym = glfw_key_to_rfb_keysym(key);
            if (keysym != 0 && key != GLFW_KEY_F1) {  // Don't send F1 to VNC (used for mode toggle)
                SendKeyEvent(vnc.client, keysym, action != GLFW_RELEASE);
            }
        }
    });
}

// Character callback for text input to VNC
void char_callback(GLFWwindow* window, unsigned int codepoint) {
    if (!g_world) return;

    // Check if we're in interactive mode
    const VNCViewState* viewState = g_world->try_get<VNCViewState>();
    if (!viewState || viewState->quadrantView) return;

    // Find the active VNC client
    auto query = g_world->query<VNCClient>();
    query.each([&](flecs::entity e, VNCClient& vnc) {
        if (vnc.quadrant == viewState->activeQuadrant && vnc.connected && vnc.client) {
            // Send as keysym (Unicode codepoint)
            SendKeyEvent(vnc.client, codepoint, TRUE);
            SendKeyEvent(vnc.client, codepoint, FALSE);
        }
    });
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS && g_world) {
        // Check if we're in interactive mode
        const VNCViewState* viewState = g_world->try_get<VNCViewState>();
        if (viewState && !viewState->quadrantView) {
            // Interactive mode - forward mouse input to active VNC client
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // Find the active VNC client
            auto query = g_world->query<VNCClient, Position, ImageRenderable>();
            query.each([&](flecs::entity e, VNCClient& vnc, Position& pos, ImageRenderable& img) {
                if (vnc.quadrant == viewState->activeQuadrant && vnc.connected && vnc.client) {
                    // Convert mouse coordinates from window space to VNC space
                    int win_w, win_h;
                    glfwGetWindowSize(window, &win_w, &win_h);

                    float scale_w = img.width / vnc.width;
                    float scale_h = img.height / vnc.height;

                    int offset_x = (int)pos.x;
                    int offset_y = (int)pos.y;

                    // Convert to VNC coordinates
                    int vnc_x = (int)((mouseX - offset_x) / scale_w);
                    int vnc_y = (int)((mouseY - offset_y) / scale_h);

                    // Clamp to VNC bounds
                    if (vnc_x < 0) vnc_x = 0;
                    if (vnc_y < 0) vnc_y = 0;
                    if (vnc_x >= vnc.width) vnc_x = vnc.width - 1;
                    if (vnc_y >= vnc.height) vnc_y = vnc.height - 1;

                    // Map GLFW button to RFB button mask
                    int buttonMask = 0;
                    if (button == GLFW_MOUSE_BUTTON_LEFT) buttonMask = rfbButton1Mask;
                    else if (button == GLFW_MOUSE_BUTTON_MIDDLE) buttonMask = rfbButton2Mask;
                    else if (button == GLFW_MOUSE_BUTTON_RIGHT) buttonMask = rfbButton3Mask;

                    SendPointerEvent(vnc.client, vnc_x, vnc_y, buttonMask);
                    std::cout << "[VNC INPUT] Mouse click at VNC coords (" << vnc_x << "," << vnc_y << ")" << std::endl;
                }
            });
            return;  // Don't process game clicks in interactive mode
        }

        // Quadrant view mode - process game clicks
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

void processInput(GLFWwindow *window, flecs::world& world) {
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

    // F1 key to toggle between quadrant and interactive mode
    static bool f1KeyPressed = false;
    bool f1KeyDown = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;

    if (f1KeyDown && !f1KeyPressed) {
        auto viewState = world.try_get<VNCViewState>();
        if (viewState) {
            VNCViewState newState = *viewState;

            if (newState.quadrantView) {
                // Switching from quadrant to interactive - determine which quadrant mouse is over
                double mouse_x, mouse_y;
                glfwGetCursorPos(window, &mouse_x, &mouse_y);
                int win_w, win_h;
                glfwGetWindowSize(window, &win_w, &win_h);

                int quad_x = (mouse_x < win_w / 2) ? 0 : 1;
                int quad_y = (mouse_y < win_h / 2) ? 0 : 1;
                newState.activeQuadrant = quad_y * 2 + quad_x;

                std::cout << "Entering interactive mode for quadrant " << newState.activeQuadrant << std::endl;
            } else {
                std::cout << "Returning to quadrant view" << std::endl;
            }

            newState.quadrantView = !newState.quadrantView;
            world.set<VNCViewState>(newState);
        }
    }

    f1KeyPressed = f1KeyDown;
}

int main(int, char *[]) {
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
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
    Window x11_window = glfwGetX11Window(window);
    // set_window_type_desktop(display, x11_window);
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
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);

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

    // Initialize SDL for VNC texture creation
    std::cout << "[SDL] Initializing SDL..." << std::endl;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "[SDL ERROR] Failed to initialize: " << SDL_GetError() << std::endl;
        glfwTerminate();
        return -1;
    }
    std::cout << "[SDL] SDL initialized successfully" << std::endl;

    // Create flecs world
    flecs::world world;
    g_world = &world; // Set global reference for mouse callback
    g_vnc_world = &world; // Set global reference for VNC callbacks

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
    world.component<RenderTexture>();
    world.component<VNCClient>();
    world.component<VNCTexture>();
    world.component<VNCViewState>();

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
        .set<Position, Local>({1000.0f, 1400.0f})
        .set<Position, World>({0.0f, 0.0f})
        .set<CreateSprite>({"../assets/arrow.png", 1.0f, 1.0f, 1.0f, false, 1, 8})  // 1 row, 8 columns
        .set<ZIndex>({1000}); // High z-index to render on top

    // Create VNC view state singleton
    world.set<VNCViewState>({true, 0});

    // Create multiple VNC client entities (4 quadrants)
    const char* hosts[NUM_VNC_CLIENTS] = {"localhost:5901", "localhost:5902", "localhost:5903", "localhost:5904"};
    flecs::entity vncEntities[NUM_VNC_CLIENTS];

    for (int i = 0; i < NUM_VNC_CLIENTS; i++) {
        std::cout << "[VNC] Creating VNC client entity for quadrant " << i << "..." << std::endl;
        std::string entityName = "VNCClient" + std::to_string(i);
        vncEntities[i] = world.entity(entityName.c_str());

        // Parse host:port
        std::string hostStr(hosts[i]);
        size_t colonPos = hostStr.find(':');
        std::string hostname = hostStr.substr(0, colonPos);
        int port = std::stoi(hostStr.substr(colonPos + 1));

        // Connect to VNC server
        rfbClient* vncClient = connectToTurboVNC(hostname.c_str(), port);
        if (vncClient) {
            std::cout << "[VNC] VNC client " << i << " connected successfully" << std::endl;
            SDL_Surface* surface = (SDL_Surface*)rfbClientGetClientData(vncClient, (void*)VNC_SURFACE_TAG);

            vncEntities[i].set<VNCClient>({
                vncClient,
                surface,
                true,
                hostname,
                port,
                vncClient->width,
                vncClient->height,
                i  // quadrant
            });

            // Create OpenGL texture for VNC framebuffer
            GLuint vncTexture;
            glGenTextures(1, &vncTexture);
            glBindTexture(GL_TEXTURE_2D, vncTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, vncClient->width, vncClient->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            std::cout << "[VNC] Created OpenGL texture: " << vncTexture << " for quadrant " << i << std::endl;

            // Create NanoVG image from OpenGL texture
            int nvgVNCHandle = nvglCreateImageFromHandleGL3(vg, vncTexture, vncClient->width, vncClient->height, 0);
            std::cout << "[VNC] Created NanoVG handle: " << nvgVNCHandle << " for quadrant " << i << std::endl;

            vncEntities[i].set<VNCTexture>({vncTexture, nvgVNCHandle, vncClient->width, vncClient->height, false});

            // Position based on quadrant (will be updated by render system)
            float posX = (i == 1 || i == 3) ? (float)(mode->width / 2) : 0.0f;
            float posY = (i == 2 || i == 3) ? (float)(mode->height / 2) : 0.0f;

            vncEntities[i].set<Position, Local>({posX, posY})
                         .set<Position, World>({posX, posY})
                         .set<ImageRenderable>({nvgVNCHandle, (float)vncClient->width, (float)vncClient->height, 1.0f})
                         .set<ZIndex>({10});

            std::cout << "[VNC] VNC display entity " << i << " created at (" << posX << ", " << posY << ")" << std::endl;
        } else {
            std::cerr << "[VNC ERROR] Failed to connect to VNC server " << hosts[i] << std::endl;
        }
    }

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

    // VNC view update system - updates VNC client positions and sizes based on view mode
    auto vncViewUpdateSystem = world.system<VNCClient, ImageRenderable>()
        .kind(flecs::PreUpdate)
        .each([&](flecs::entity e, VNCClient& vnc, ImageRenderable& img) {
            if (!vnc.connected || !vnc.client) return;

            const VNCViewState* viewState = world.try_get<VNCViewState>();
            if (!viewState) return;

            auto windowComp = world.try_get<GameWindow>();
            if (!windowComp) return;

            int win_w = windowComp->width;
            int win_h = windowComp->height;

            if (viewState->quadrantView) {
                // Quadrant view mode - show all 4 VNC streams in a 2x2 grid
                int quadrant_w = win_w / 2;
                int quadrant_h = win_h / 2;

                // Calculate scaled dimensions to fit within quadrant while maintaining aspect ratio
                float scale_w = (float)quadrant_w / vnc.width;
                float scale_h = (float)quadrant_h / vnc.height;
                float scale = (scale_w < scale_h) ? scale_w : scale_h;

                int scaled_w = (int)(vnc.width * scale);
                int scaled_h = (int)(vnc.height * scale);

                // Position based on quadrant
                int quad_offset_x = (vnc.quadrant == 1 || vnc.quadrant == 3) ? quadrant_w : 0;
                int quad_offset_y = (vnc.quadrant == 2 || vnc.quadrant == 3) ? quadrant_h : 0;

                // Center within quadrant
                float posX = quad_offset_x + (quadrant_w - scaled_w) / 2.0f;
                float posY = quad_offset_y + (quadrant_h - scaled_h) / 2.0f;

                e.set<Position, Local>({posX, posY});
                img.width = (float)scaled_w;
                img.height = (float)scaled_h;
                img.alpha = 1.0f;  // Fully visible
            } else {
                // Interactive mode - show only the active quadrant fullscreen
                if (vnc.quadrant == viewState->activeQuadrant) {
                    // Calculate scaled dimensions to fit window while maintaining aspect ratio
                    float scale_w = (float)win_w / vnc.width;
                    float scale_h = (float)win_h / vnc.height;
                    float scale = (scale_w < scale_h) ? scale_w : scale_h;

                    int scaled_w = (int)(vnc.width * scale);
                    int scaled_h = (int)(vnc.height * scale);

                    // Center in window
                    float posX = (win_w - scaled_w) / 2.0f;
                    float posY = (win_h - scaled_h) / 2.0f;

                    e.set<Position, Local>({posX, posY});
                    img.width = (float)scaled_w;
                    img.height = (float)scaled_h;
                    img.alpha = 1.0f;  // Fully visible
                } else {
                    // Hide other quadrants
                    img.alpha = 0.0f;
                }
            }
        });

    // VNC initialization system - requests initial framebuffer update
    auto vncInitSystem = world.system<VNCClient>()
        .kind(flecs::PreUpdate)
        .each([](flecs::iter& it, size_t i, VNCClient& vnc) {
            static bool initialized[NUM_VNC_CLIENTS] = {false};
            if (!initialized[vnc.quadrant] && vnc.connected && vnc.client) {
                std::cout << "[VNC INIT] Requesting full framebuffer update for quadrant " << vnc.quadrant << "..." << std::endl;
                SendFramebufferUpdateRequest(vnc.client, 0, 0, vnc.client->width, vnc.client->height, FALSE);
                initialized[vnc.quadrant] = true;
                std::cout << "[VNC INIT] Initial update request sent for quadrant " << vnc.quadrant << " " << vnc.client->width << "x" << vnc.client->height << std::endl;
            }
        });

    // VNC polling system - checks for VNC messages
    auto vncPollingSystem = world.system<VNCClient>()
        .kind(flecs::PreUpdate)
        .each([](flecs::entity e, VNCClient& vnc) {
            if (!vnc.connected || !vnc.client) {
                static bool warned = false;
                if (!warned) {
                    std::cout << "[VNC POLL] Client not connected" << std::endl;
                    warned = true;
                }
                return;
            }

            // Poll for VNC messages with 1ms timeout
            int result = WaitForMessage(vnc.client, 1);
            if (result < 0) {
                std::cerr << "[VNC ERROR] Connection lost (result=" << result << ")" << std::endl;
                vnc.connected = false;
            } else if (result > 0) {
                std::cout << "[VNC POLL] Message available, handling..." << std::endl;
                if (!HandleRFBServerMessage(vnc.client)) {
                    std::cerr << "[VNC ERROR] Failed to handle server message" << std::endl;
                    vnc.connected = false;
                } else {
                    std::cout << "[VNC POLL] Message handled successfully" << std::endl;
                }
            }
            // result == 0 means timeout (no message), which is normal
        });

    // VNC texture update system - updates OpenGL texture from SDL surface
    auto vncTextureUpdateSystem = world.system<VNCClient, VNCTexture>()
        .kind(flecs::OnUpdate)
        .each([](flecs::entity e, VNCClient& vnc, VNCTexture& tex) {
            if (!vnc.connected || !vnc.client) {
                static bool warned = false;
                if (!warned) {
                    std::cout << "[VNC TEX] Client not connected or null" << std::endl;
                    warned = true;
                }
                return;
            }

            if (!tex.needsUpdate) return;

            std::cout << "[VNC TEX] Updating OpenGL texture " << tex.texture
                      << " from framebuffer..." << std::endl;

            SDL_Surface* surface = (SDL_Surface*)rfbClientGetClientData(vnc.client, (void*)VNC_SURFACE_TAG);
            if (surface && surface->pixels) {
                std::cout << "[VNC TEX] Processing " << tex.dirtyRects.size() << " dirty rectangles" << std::endl;
                std::cout << "[VNC TEX] Surface info: " << surface->w << "x" << surface->h
                          << ", format: " << SDL_GetPixelFormatName(surface->format->format)
                          << ", pitch: " << surface->pitch
                          << ", bpp: " << (int)surface->format->BitsPerPixel << std::endl;

                // Update OpenGL texture from SDL surface pixels
                glBindTexture(GL_TEXTURE_2D, tex.texture);

                // Set pixel unpack alignment to handle pitch correctly
                int bytesPerPixel = surface->format->BytesPerPixel;
                int expectedPitch = surface->w * bytesPerPixel;

                // CRITICAL: When updating partial rects, GL_UNPACK_ROW_LENGTH tells OpenGL
                // how many pixels are in a row of the SOURCE data (the surface)
                // This must be set to surface->pitch / bytesPerPixel regardless of whether
                // we're updating the full surface or a partial rect
                int rowLengthPixels = surface->pitch / bytesPerPixel;

                std::cout << "[VNC TEX] Surface pitch: " << surface->pitch
                          << " bytes, row length: " << rowLengthPixels << " pixels"
                          << " (surface width: " << surface->w << " pixels)" << std::endl;

                // Always set row length for partial updates
                glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLengthPixels);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                // Determine the correct pixel format based on SDL surface format
                GLenum format = GL_BGRA;
                if (surface->format->Rmask == 0xFF) {
                    format = GL_RGBA;
                }

                std::cout << "[VNC TEX] Using OpenGL format: "
                          << (format == GL_BGRA ? "BGRA" : "RGBA") << std::endl;

                // Process each dirty rectangle
                for (const auto& rectIn : tex.dirtyRects) {
                    // Clamp rect to surface/texture bounds to be safe
                    int rx = std::max(0, rectIn.x);
                    int ry = std::max(0, rectIn.y);
                    int rw = std::min(rectIn.w, surface->w - rx);
                    int rh = std::min(rectIn.h, surface->h - ry);
                    if (rw <= 0 || rh <= 0) continue;

                    // Pointer to top-left of the dirty region
                    uint8_t* regionStart = // NOTE: Changed to non-const for modification
                        static_cast<uint8_t*>(surface->pixels) + ry * surface->pitch + rx * 4;

                    // --- CRITICAL ADDITION: Force full alpha for the dirty rectangle ---
                    // This assumes a 32-bit format (bpp=4) where the alpha channel is the
                    // last byte of the 4-byte pixel (e.g., RGBA or BGRA).
                    // If the format is different, the offset (bpp - 1) needs adjustment.
                    for (int y = 0; y < rh; ++y) {
                        uint8_t* rowStart = regionStart + y * surface->pitch;
                        for (int x = 0; x < rw; ++x) {
                            // Set the alpha component (last byte) to 0xFF (fully opaque)
                            rowStart[x * 4 + (4 - 1)] = 0xFF;
                        }
                    }
                    // ------------------------------------------------------------------

                    // Critical change: we keep GL_UNPACK_ROW_LENGTH set so GL steps by `pitch` each row.
                    glTexSubImage2D(GL_TEXTURE_2D, 0, rx, ry, rw, rh, format, GL_UNSIGNED_BYTE, regionStart);
                }

                // Reset to defaults
                glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

                std::cout << "[VNC TEX] All rects updated successfully" << std::endl;

                // Clear dirty rects and mark as updated
                tex.dirtyRects.clear();
                tex.needsUpdate = false;
            } else {
                std::cerr << "[VNC TEX ERROR] Surface or pixels is null" << std::endl;
            }
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

    // Create a render texture (framebuffer)
    GLuint fbo, renderTex;
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &renderTex);

    int texWidth = 512;
    int texHeight = 512;

    glBindTexture(GL_TEXTURE_2D, renderTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create NanoVG image from the texture
    int nvgTexHandle = nvglCreateImageFromHandleGL3(vg, renderTex, texWidth, texHeight, 0);

    auto renderTextureEntity = world.entity("RenderTexture")
        .set<RenderTexture>({fbo, renderTex, texWidth, texHeight})
        .set<Position, Local>({100.0f, 100.0f})
        .set<Position, World>({100.0f, 100.0f})
        .set<ImageRenderable>({nvgTexHandle, (float)texWidth, (float)texHeight, 1.0f})
        .set<ZIndex>({5});

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

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window, world);
        
        int winWidth, winHeight;
        glfwGetFramebufferSize(window, &winWidth, &winHeight);
        float pxRatio = (float)winWidth / (float)800;

        // Render to texture first
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, texWidth, texHeight);
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        nvgBeginFrame(vg, texWidth, texHeight, 1.0f);

        // Draw a rectangle on the texture
        nvgBeginPath(vg);
        nvgRect(vg, 50, 50, 200, 150);
        nvgFillColor(vg, nvgRGBA(255, 100, 50, 255));
        nvgFill(vg);

        nvgEndFrame(vg);

        // Unbind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render to main window
        glViewport(0, 0, winWidth, winHeight);
        // glClearColor(0.1f, 0.2f, 0.2f, 0.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Update window size in ECS
        windowEntity.set<GameWindow>({window, winWidth, winHeight});

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
