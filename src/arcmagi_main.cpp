// https://www.glfw.org/docs/3.3/group__native.html
#define GLFW_EXPOSE_NATIVE_X11

// apt install wmctrl

// TODO: Update flecs version to 4
// This will require updating query/system signatures to use each
// https://www.flecs.dev/flecs/md_docs_2MigrationGuide.html

// the 'mock' components should also be replaced with tags...

#define FLECS_CUSTOM_BUILD
#define FLECS_CPP
#define FLECS_SYSTEM
#define FLECS_TIMER
#define FLECS_SCRIPT
#include <flecs.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <regex>
#include <string.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <set>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/types.h>
#include <cstdlib>
#include <random>
#include <cctype>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include <nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>

// TODO: refactor to module, add X11 module
#include "X11/X.h"
#include <X11/Xatom.h>

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libevdev/libevdev.h"

#include <cglm/cglm.h>
// networking lib
#include <enet/enet.h>
// file watcher lib
#include "efsw/efsw.hpp"

#include <libavdevice/avdevice.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#undef Status
#include <SFML/Audio.hpp>

#include <unistd.h>
#include <libudev.h>
#include <linux/input.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// #include <xdo.h>

#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.h"

#include <tsl/htrie_map.h>
#include <tsl/htrie_set.h>

// Tracy ---
#include "public/TracyClient.cpp"
#include "public/tracy/Tracy.hpp"
//

struct ItemDescription
{
  std::string display_name;
  std::string description_para;
  std::vector<std::string> input_params;
  std::vector<std::string> output_params;
};

std::vector<std::string> items = {"intermediate_states", "varpad", "elementary_pairs", "mask", "outline", "vector_hammer", "polyomino_pickaxe", "query_pickaxe", "graphpad", "symmetry_shovel", "partition_axe", "measuring_rod", "slope_horn", "letters_book", "move_operator", "copy_operator", "flip_operator", "scale_operator", "rotate_operator", "query_select", "propagation", "color_bottle", "isomorphic_hash", "floodfill", "bkg", "visual_hierarchy", "negative_space", "noise", "list_to_list", "list_to_matrix", "merge", "sort_group", "boolean_branch", "world_progress", "guess", "shape_index", "match_color", "episodic_color_behavior", "static_memory_crystal", "occlusion_grain", "pathfinder", "subgraph_index", "sequence", "recursive_egg", "one_to_one", "one_to_many", "many_to_one"};

// Unconscious processors
std::unordered_map<std::string, ItemDescription> item_descriptions = {
  {"intermediate_states", {"Intermediate States", "Use sequential states to arrive at the solution.", {}, {}}},
  {"varpad", {"Variable Pad", "Find an equation to determine an unknown variable from a set of possible variables", {"variables: [[(name, int)]]", "target_values: [int]"}, {"equation: str"}}},
  {"mask", {"Mask", "From matrix layers: determine pixel transform rules", {"input_stack: [matrix]", "output: matrix"}, {"ruleset: [str]"}}},
  {"outline", {"Outline", "Function to get pixels around or within a selection", {"input_stack: [matrix]", "output: matrix"}, {"ruleset: [[pixel]->pixel]"}}},
  {"paint_operator", {"Paint Operator", "Change color of selected pixels", {"state: matrix", "coords: [vec2]", "color: Color"}, {"state: matrix"}}}, 
  {"move_operator", {"Move Operator", "Move selected pixels, replacing with background", {"state: matrix", "coords: [vec2]", "x: int", "y: int", "bkg: Color"}, {"state: matrix"}}},
  {"copy_operator", {"Copy Operator", "Copy selected pixels", {"state: matrix", "coords: [vec2]", "x: int", "y: int"}, {"state: matrix"}}},
  // TODO: These operators can apply to state or independently, and the distinction between processes must be known
  {"flip_operator", {"Flip Operator", "", {"state: matrix", "coords: [vec2]", "axis: Axis", "anchor: int"}, {"state: matrix"}}},
  {"rotate_operator", {"Rotate Operator", "", {"state: matrix", "coords: [vec2]", "z_group: int", "anchor: vec2"}, {"state: matrix"}}},
  {"scale_operator", {"Scale Operator", "", {"state: matrix", "coords: [vec2]", "scale_x: int", "scale_y: int"}, {"substate: matrix"}}},

  {"polyomino_pickaxe", {"Polyomino Pickaxe", "Mines same-value-adjacent polyominoes from matrix (or monochrome query mask)", {"state: matrix", "diagonal: bool"}, {"polyominoes: [(sparse_matrix, x, y)]"}}},
  {"query_pickaxe", {"Query Pickaxe", "Mines adjacent polys from query sparse matrix mask.", {"mask: matrix", "diagonal: bool"}, {"polys: [(sparse_matrix, x, y)]"}}},
  {"symmetry_shovel", {"Symmetry Shovel", "Identifies regions and axes of symmetry", {"state: matrix"}, {"polyominoes: [[vec2]]"}}},
  {"vector_hammer", {"Vector Hammer", "Iterates through rows and columns as entities", {"state: matrix"}, {"[lines]"}}},
  {"partition_axe", {"Partition Axe", "Separates a matrix into discrete cell regions", {"state: matrix"}, {"cells: matrix"}}},
  {"letters_book", {"Book of Letters", "Strides a matrix with a fixed window shape, identifying unique submatrices and io patterns", {"state: matrix", "width: int", "height: int"}, {"letters: [matrix]"}}},
  
  {"query_select", {"Submatrix Crop", "Creates a new state from the bounds of a selection", {"state: matrix", "coords: [vec2]"}, {"state: matrix"}}},
  {"create_state", {"Create State", "Creates an empty state of a particular size", {"width: int, height: int"}, {"state: matrix"}}},
  {"expand", {"Fold State", "Expands or reduces an existing state in a particular direction", {"state: matrix", "direction: Direction", "distance: int"}, {"state: matrix"}}},

  {"measuring_rod", {"Measuring Rod", "Measure prepositions between polyominoes", {"[polyomino]"}, {"state: matrix"}}},
  {"visual_hierarchy", {"Visual Hierarchy", "Create a graph of visual hierarchy relationships", {"polyominoes [poly]"}, {"state: matrix"}}},
  {"occlusion_grain", {"Occlusion Layers", "There is a z-index where some polyominoes might occlude others.", {"polyominoes: [poly]"}, {}}},
  {"slope_horn", {"Edge Slope Detector", "Detects edges and their rise/run slopes in lines and rectilinear lattice polygons. Sparse matrix should be a single shape.", {"state: sparse_matrix", "coords: [vec2]"}, {"edges: [slope]"}}},

  {"sequence", {"Sequence Detector", "Identifies a repeating sequence.", {"state: sparse_matrix", "coords: [vec2]"}, {"edges: [slope]"}}},
  {"subgraph_index", {"Submatrix Detector", "Finds a matrix as a probabalistic or repeated submatrix of another matrix.", {"source: matrix", "target: matrix"}, {"submatrix: [vec2]"}}},

  {"bkg", {"Background", "This task has Background and Foreground layers", {}, {}}},
  {"noise", {"Noise", "This task has noise obscuring a pattern or object", {}, {}}},
  {"pathfinder", {"Pathfinding", "This task has Obstacles and a Pathfinder agent", {}, {}}},

  {"color_bottle", {"Wildcard Color", "Select a set of colors using EcsWildcard with exclusion", {"exclude_colors: [Color]"}, {"terms: [str]"}}},
  {"isomorphic_hash", {"Behavior Set", "There are multiple unknown colors with distinct behaviors.", {""}, {"terms: [str]"}}},
  {"congruency", {"Congruency", "Above move down is to left-of move right, etc", {}, {}}},

  {"shape_index", {"Shape Index", "Use shape index in query", {}, {}}},
  {"negative_space", {"Negative Space", "Use Hole or Bound in query", {}, {}}},

  {"propagation", {"Propagation", "A letter should iteratively spread", {}, {}}},
  {"boolean_branch", {"Boolean Branch", "Branch hypothesis on if/else", {}, {}}},
  {"world_progress", {"World Progress", "There are multiple iterative programs that should progress in a loop together", {}, {}}},

  {"merge", {"Merge", "Treat an entire query as a single sparse matrix entity", {"query_output: [pixel]"}, {"merged: sparse_matrix"}}},
  {"graphpad", {"Graphpad", "Compose nearest neighbor polyominoes together", {"polyominoes: [polyomino]"}, {"composites: [sparse_matrix]"}}},


};

struct ItemCategory
{
  std::string name;
  std::vector<std::string> items;
};

std::vector<ItemCategory> item_categories =
{
  {"SYMBOLIC REGRESSION", {"varpad"}},
  {"ABSTRACTION TOOLS", {"polyomino_pickaxe", "symmetry_shovel", "vector_hammer", "partition_axe", "letters_book", "query_pickaxe"}},

  {"STANDARD OPERATORS", {"paint_operator", "move_operator", "copy_operator", "flip_operator", "rotate_operator", "scale_operator"}},
  {"MATRIX OPERATORS", {"query_select", "create_state", "expand"}},

  {"SPATIAL GRAPHS", {"measuring_rod", "visual_hierarchy", "occlusion_grain", "slope_horn"}},
  {"STATE SPACE SEARCH", {"intermediate_states"}},
  {"PBE TRAINING", {"elementary_pairs", "match_color", "episodic_color_behavior", "static_memory_crystal"}},
  {"TASK TAGS", {"bkg", "noise", "pathfinder"}},
  {"CONTROL FLOW", {"propagation", "boolean_branch", "world_progress"}},

  {"DECOMPOSITION", {"sequence", "subgraph_index", "mask", "range"}},
  {"GENERALIZATION", {"color_bottle", "isomorphic_hash", "congruency"}},
  {"FUNCTIONAL GROUPS", {"merge", "sort_group", "graphpad", "outline"}},
  {"MAPPING", {"list_to_list", "list_to_matrix", "diagonal_proj"}},
  {"SHAPE PROPERTIES", {"shape_index", "negative_space"}},

  {"CARDINALITY MODEL", {"one_to_one", "one_to_many", "many_to_one", "multiple_entity"}},

  // {"EMBEDDINGS", {"dino"}},

};

struct WordPos
{
  size_t seek;
  size_t index;
};

struct WordData
{
  std::string word;
  bool space_postfix;
  size_t sentence_index;
};

struct SentenceData
{
  size_t section;
  size_t paragraph;
};

struct BookGraph {};

struct BookData
{
  std::vector<std::vector<WordData>> pages;
  std::vector<SentenceData> sentences;
};

// ---- Scene graph

struct SG
{
  const char* name;
};

struct SceneGraph
{
    ecs_entity_t symbol;
    bool user_mark_expanded = true;
    bool is_expanded = true;
    bool is_visible = true;
    int32_t children_count;
    int32_t index;
    int32_t child_index;
    int32_t depth;
};

struct SceneGraphInteractionNode 
{
  flecs::entity node;
};

enum ScopeIndicator {
    EXPANDED,
    REDUCED
};

struct SceneGraphSelection 
{
  bool active;
  flecs::entity default_select_node;
  flecs::entity selected_node;
  flecs::entity scene_graph_root;
  
  flecs::entity bode_telos_source;
  flecs::entity interaction_graph_selector;
  // contextual intent expression
  // TODO: Multiple intent expressions?
  flecs::entity context_graph;
};

struct SceneGraphSelectionChanged 
{
  int mock = 0;
};

struct SceneGraphRoot 
{
  flecs::entity selector;
};

// ----

bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// ---- evdev
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
// ---- evdev

struct UIDebugRender
{
  bool active = false;
};

struct NanoVG
{
  NVGcontext* ctx = nullptr;
};

struct MNISTDigit
{
  int index;
  int char_set_index;
};

struct SpriteCreator
{
  fs::path path;
  int r = 1;
  int c = 1;
  float desired_height = 0;
};

struct SpriteAlpha
{
  float alpha;
};

struct SpriteTint
{
  NVGcolor tint = nvgRGBA(255, 255, 255, 255);
};

struct Sprite
{
  int img; // nanovg img index
  int x;
  int y;
  int w;
  int h;
  int c_w;
  int c_h;
  int rows;
  int cols;
};

enum RenderMode
{
  FILL,
  STROKE
};

struct LoadSpeechGraphEvent
{
  std::string filepath;
};

struct UpdateGraphReaderPageEvent
{
  size_t page;  
};

struct RenderStatus
{
  bool visible = true;
  int mode = RenderMode::FILL;
  NVGcolor color = nvgRGBA(255, 255, 255, 255);
};

struct RenderOrder
{
  int z_index = 0;
};

struct SymbolBoundAnnotation {};

struct EdgeRenderStatus
{
  float stroke_width = 2.0f;
  NVGcolor color = nvgRGBA(255, 255, 255, 255);
  bool show_arrow = true;
};

struct FontStatus
{
  float font_size = 16.0f;
  float font_blur = 0.0f;
};

struct SpriteAnimator
{
  int frame = 0;
  float progress = 0;
  float framerate;
};

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

struct TrackCursor 
{
  flecs::entity from;
};

struct CursorState
{
  int x;
  int y;
  // Putting diffs on state is a bad software architecture
  int x_diff;
  int y_diff;
  int middle;
};

struct MouseMotionEvent
{
  int x_diff;
  int y_diff;
};

struct CreateSFX
{
  std::string filepath;
};

struct SFX
{
  sf::SoundBuffer buffer;
  sf::Sound sound;
};

struct KeyboardState
{
  std::unordered_map<unsigned short, int> key_states;
};

struct LibEvInput
{
  struct input_event ev;
};

struct ConsumeEvent{};

struct Position
{
    float x = 0.0f;
    float y = 0.0f;
};
struct Source {};
struct Local {};
struct World {};
struct Relative {};

struct ZIndex 
{
  int z = 0;
};
int compare_zindex(
    flecs::entity_t e1,
    const ZIndex *index1,
    flecs::entity_t e2,
    const ZIndex *index2)
{
    (void)e1;
    (void)e2;
    return (index1->z > index2->z) - (index1->z < index2->z);
}

struct OnClickEvent {};
struct Clicked 
{
  float x;
  float y;
}; // TODO: Get symbol at this location?
struct CallMoveToScript {};
struct SetCartridge {};

struct HoverEvent
{
  bool hovered = false;
};

struct UIElementBounds
{
  float xmin = 10000.0f;
  float ymin = 10000.0f;
  float xmax = 0.0f;
  float ymax = 0.0f;
};

struct HorizontalLayoutBox
{
  float x_progress;
  float padding = 0.0f;
};

struct VerticalLayoutBox 
{
  float y_progress;
  float padding;
};

struct TableLayoutBox 
{
  float x_progress;
  float y_progress;
  float x_padding;
  float y_padding;
  int max_cols;
};
// TODO: Sort...

struct Align {
  float parent_horizontal_percent;
  float parent_vertical_percent;
  float self_horizontal_percent;
  float self_vertical_percent;
};

// TODO: Symbolic interaction should be controlled via entities
// in an open-ended way...
enum SymbolInteraction // temporary test patchwork
{
  NONE,
  LEFT_CLICK,
  LEFT_MOUSE_DOWN,
  LEFT_MOUSE_UP
};

// TODO: How to consider negative space?
// TODO: We should consider prefab frame generalization and playful random variance decisions (eg don't always click on the exact same pixel) 
struct TargetSymbolLocation
{
  int x;
  int y;
};

struct MoveTarget { };
struct ReachedTargetEvent { };
struct ClickOnTarget 
{
  // Left mouse is 1, middle is 2, right is 3, wheel up is 4, wheel down is 5.
  int button;
};
struct MouseDownOnTarget 
{
  int button;
};
struct MouseUpOnTarget 
{
  int button;
};

struct DefaultSymbolInteraction
{
  // flecs::pair<ReachedTargetEvent, flecs::Wildcard> interaction;
  SymbolInteraction interaction;
};

struct SymbolNavigator
{
  TargetSymbolLocation location;
  SymbolInteraction interaction;
};

struct SequenceStepCompleted 
{
  int mock;
};

struct SymbolicTargetSeq
{
  std::vector<SymbolNavigator> navigations; // TODO: Make this symbolic (rather than coordinate based!)
  int step = 0;
};

struct LinearMovement
{
  float speed;
};

struct CubicMovement {
    float speed;
    float progress;
};

struct HorizontalList
{
  float x = 0.0f;
};

struct PathMovement
{

};

struct TakeScreenshot {};
struct TakeVirtScreenshot {};

struct LoadBookEvent 
{
  std::string name;
};

struct Thread
{
  pthread_t pthread;
  int iret;
};

struct TrackpadStatus
{
  bool tx;
  bool ty;
  Position start;
  Position last;
};

struct GraphSimClient
{
    const char* ip;
    ENetHost* client = nullptr;
    ENetPeer* peer = nullptr;
};

struct SendSimPacket
{
    std::string data;
};

// TODO: Subgraph anchored convolutional neural network bound

// TODO: Memorized symbol...

// TODO: OCR Textbox offscreen pathfinding?

struct OCRTextbox
{
  std::string text;
};

struct BoxShape {
    float width, height;
    float stroke_width = 1.0f;
};

struct BackgroundBoxShape {
    float width, height;
    float stroke_width;
};

struct LineWrapBox
{
  float width, height;
  float length;
  float quantity;
  float time_to_rotate;
  std::vector<NVGcolor> colors;
  float progress;
};

struct LineShape {
    float offset_x, offset_y;
    int stroke_width = 1;
};

struct SelectionSpanIndicator {};
struct SelectionStartIndicator {};
struct SelectionEndIndicator {};

struct ModCircle
{
  std::vector<NVGcolor> colors;
};

struct CircleShape
{
    float radius;
};

struct RoundTriShape
{
    float corner_radius;
    float triangle_radius;
};

struct DiamondShape
{
    float radius;
};

struct CapsuleShape
{
  float radius;
  float width;
};

// Box node
struct Bode
{
  float width;
  float height;
  float curve_radius;
  float node_slot_radius;
  float node_edge_slider = 0.5f;
};

// Gridbox
struct Grox
{
  float width;
  float height;
  float curve_radius;
  int tile_d;
  NVGcolor secondary;
};

struct BookmarkResponder {};

struct RainbowPlane
{
  float acceleration_rate;
  float velocity;
  float height;
};

struct TrackEdge 
{
  flecs::entity source;
  flecs::entity target;
};
struct Edge
{
    float head_x;
    float head_y;
    float tail_x;
    float tail_y;
    float Magnitude() const
    {
        return sqrt(pow(tail_x - head_x, 2) + pow(tail_y - head_y, 2));
    }
};

enum ECS_NodeType
{
  STANDARD,
  // GNN
  DATASET,
  MODEL,
  CLUSTER,
  PROJECTION,
  LIFT,
  // ECS
  PREFAB,
  SLOT,
  ENTITY,
  INT,
  FLOAT,
  STRING,
  TENSOR,
  COMPONENT,
  PLECS,
  SYSTEM,
  MODULE,
  // OS
  COMPANION,
  PYTHON_SCRIPT, // TODO HEONAE: GraphSim
  LINUX,
  SYMBOL_ANCHOR
};

enum ECS_NodeMetaType
{
  META,
  GRAPH,
  ECS,
  OS
};

struct NodeType
{
  ECS_NodeType ecs;
};

struct SceneGraphAmbLoader 
{
  std::unordered_map<ECS_NodeType, flecs::entity> graphs;
};

struct SetupNodeMenuSceneGraph 
{
  flecs::entity telos;
  ECS_NodeType node_type;
};

struct LoadGraphData {
    int node_type;
    int x;
    int y;
    std::string graph_str;
};

// Function to process the scroll and summon the graph
std::vector<LoadGraphData> process_scroll(const std::string& scrollPath) {
    std::vector<LoadGraphData> graph;
    std::ifstream cartridge(scrollPath);
    std::string line;

    if (cartridge.is_open()) {
        while (std::getline(cartridge, line)) {
            std::istringstream iss(line);
            LoadGraphData node;
            if (iss >> node.node_type >> node.x >> node.y) {
                std::getline(iss, node.graph_str); // Capturing the essence
                if (!node.graph_str.empty() && node.graph_str[0] == ' ') { // Trim leading space
                    node.graph_str.erase(0, 1);
                }
                std::cout << "Graph node is " << node.graph_str << std::endl;
                graph.push_back(node); // Summoning the node to the graph
            }
        }
    }
    return graph; // The graph, a tapestry of nodes, is now alive
}

struct Next {};
struct Prev {};
struct Head {};

struct EventOnGrab {};
struct MoveGraph {};
struct GraphIsGrabbed {};
struct SlideGraphRail {};
struct SelectGraphNode {};

struct DynamicConnector {};
struct ConnectGraph {};
struct EventOnConnect {};
struct EventOnSelect {};

struct CursorDistanceVisible 
{
  float max;
};

struct ReadGraph 
{
  flecs::entity edge_source;
};

struct AttemptLoadBook {};

struct OpenECS_SceneGraph {};

// TODO: I'm not sure how to refactor this since it needs to be loaded from plecs
// I'm not sure if it can be an enum? Or whether string var is preferred?
// This is messy...

struct CreateDatasetFromGraph {};
struct CreateModelFromDatasetGraph {};
// TODO: Add Transformer
// TODO: Add Subgraph Miner
// TODO: Add Runtime Optimizer
// TODO: Add Compile Time Optimizer

struct CreatePrefabFromGraph {};
struct CreateSlotFromGraph {};
struct CreateEntityFromGraph {};
struct InstantiateComponent {};
struct CreateComponentFromGraph {};

// Basically just saying the scene graph item wants a temporary contextual graph for naming the variable :)
struct IntentExpression {};

struct AddIntFromGraph {};
struct AddFloatFromGraph {};
struct AddStringFromGraph {};
// TODO: Add Vector (scene graph children -> 2d, 3d)
struct LoadTensorFromFile {};

struct DeployCompanionFromGraph {};
struct InterfacePythonServerFunction {};
struct RunAutonomousLinuxDesktop {};
struct AnchorSymbol {};

// TODO: load dynamic scene graph HUDs during engine loop...
struct SceneGraphLoader {};

struct LoadSceneGraph 
{
  flecs::entity graph;
  flecs::entity telos_source;
  flecs::entity telos_target;
};

struct SceneGraphGraphHUD
{
  flecs::entity active;
  flecs::entity to_select;
};

// TODO: Refactor!
struct OpenSceneGraph 
{
  flecs::entity graph;
};

struct GenPrefabEntity {}; // TODO: EventOnConnect which displays temporary UI graph, uses a GNN to generate sequences of user action

struct Button 
{
  int mock;  
};
struct EventSelectBtn 
{
  int mock;
};

struct EventHoverBtn
{
  int mock;
};

struct Toggle
{
  flecs::entity menu;
};

struct ToggleMenu
{
  flecs::entity selected;
  std::vector<flecs::entity> buttons;
};

enum struct GraphEditMode
{
  READ,
  CREATE_GRAPH,
  CRAFT_TELOS,
  DEPLOY_OS,
};

struct GraphEditor
{
  // TODO: Jef Raskin: The Humane Interface, remove modes...
  GraphEditMode edit_mode = GraphEditMode::CREATE_GRAPH;
};

struct HorizontalBoundaryChisel
{
  bool snap = true;
  bool lock_anchor = false;
  bool lock_selection = false;
  float anchor_x;
  float selection_x;
};
struct FollowChiselAnchor
{
  flecs::entity chisel;
};
struct FollowChiselSelection 
{
  flecs::entity chisel;
};
struct SpanChiselBounds
{
  flecs::entity anchor;
  flecs::entity selection;
};

struct NodeIsSelected {};
struct ContiguousSelection {};

struct ExportGraphSelection 
{
  int mock;
};

struct GrabbedEntity {};

struct FontCreator
{
  const char* name;
  const char* path;
};

struct NanoFont
{
  int handle;
};

struct TextInput {};

struct TextCreator
{
    std::string word;
    std::string font_name = "CharisSIL";
};

struct NodeTypeLabel {};

struct Overlay
{
  GLsizei width;
  GLsizei height;
  GLFWwindow* window;
  Window x11_window;
  Display* display;
  Window x11_root;
  bool is_framing_virt_desk;
};

struct VirtRigControls
{
  bool recording = false;
};

struct SengraNode {};

// I know this is really really bad but I cannot figure out how to iterate through all wildcard relationships
struct SengraNodes
{
  std::vector<flecs::entity> subgraph;
};

struct HighlightSentence {};

// TODO: Selected subgraph/para...
struct SelectedSentence {};

struct SentenceSubgraph
{
  size_t index;
};

struct BookGraphSelection {};

struct GraphData 
{
  std::string filepath;
  int start;
  int len;
  int max;
  // TODO: Head/Tail... etc
  // Evaluate beyond sengras
};

struct CartridgeBlock
{
  size_t value;
  bool selected;
  bool loaded;
  bool load_on_startup;
};

struct SaveGraphLayer 
{
  size_t cartridge;
};

struct SaveGraph
{
  size_t cartridge; // TODO: Figure out how to save/load modules vs cartridge layers...
};

// Using temporarily to indicate a single graph target for the chisel...
struct ActiveGraph {};

struct GraphRail 
{
  float x_per_second;
};

// TODO: Graph streams may require variable speeds and branching navigation
// eg: Read book tabs to gain instrumental capability as needed...
struct GraphStream
{
  float nodes_per_second;
  float progress;
};

struct GraphReader
{
  BookData book_data;
  size_t active_page; // TODO JANE: Multiple active pages
  flecs::entity page_graph;
};

struct GraphSpeak
{
  float progress;
};

struct GraphAudio
{
  std::string filepath;
  sf::Music* music;
};

struct GraphNodeData
{
  size_t seq;
  // tiktoken
};

struct SengraSelection
{
  flecs::entity node_start;
  flecs::entity node_end;
};

struct DragNodeSelector {};

enum DragBoundMode
{
  SELECT,
  ZONE
};

struct DragBoundGraphSelector 
{
  bool active;
  float anchor_x, anchor_y;
  DragBoundMode mode = DragBoundMode::SELECT;
};

// TODO: Also need to add loaders to drag in offscreen nodes from memory as req
struct GraphQueryGraphSelector {};
struct GraphQueryStreamGraphSelector {};

// struct GraphSelector
// {
//   std::vector<flecs::entity> selected_nodes;
// };

struct Background
{
  NVGcolor color;
};

struct Interpolator
{
  float time;
  float progress;
  float percent;
  float delay = 0.0f;
};

struct ColorAnimation
{
  NVGcolor start;
  NVGcolor end;
  NVGcolor current;
};

struct X11KeyPress
{
  XKeyPressedEvent event;
};

// TODO Observer init diff!
struct X11CursorMovement
{
  float x;
  float y;
  float x_diff;
  float y_diff;
};

struct X11FlecsScrollEvent 
{
  int dir;
};

struct X11HorizontalScrollEvent 
{
  int dir;
};

struct StreamCreator
{
  flecs::entity graph;
};

struct BookPlayerEventPlay {};
struct RegenSpeechEvent {};

struct SpokenWord
{
  std::string word;
  float start;
  float end;
};

// Manages loading/unloading graph nodes for a streaming speech graph
struct SpeechStream
{
  float start_time;
  float duration;
  size_t start_index;
  size_t end_index;
  std::vector<SpokenWord> words;
};

struct EmojiSlideWord
{
  // flecs::ref<SpokenWord>
  flecs::entity graph;
  flecs::entity rail;
};

struct SelectionStatic {};

struct LevelHUD 
{
  int mock;
};

struct AnnotatorHUD 
{
  int mock;
};

struct RectilinearGrid
{
  std::vector<int> col_sizes;
  std::vector<int> row_sizes;

  std::vector<std::vector<flecs::entity>> elements;
};

struct RectilinearGridSelector
{
  int row = 0;
  int col = 0;
  flecs::entity highlight;
};

struct GraphicsProgramHypothesis
{
  int index;
  int priority;
  flecs::entity viz;
};

struct SortCol
{
  int sort_index = 0;
  std::unordered_map<int, std::vector<GraphicsProgramHypothesis>> sorts;
};

struct ReloadCol 
{
  size_t col;
  size_t sort_index;
};

struct SaveProgramSort 
{
  size_t sort_index = 0;
  bool next_stage = false;
};

struct UnixSocket {
    int fd;
    std::string path = "/tmp/my_socket";
};

struct SocketDataReceived
{
  char buffer[256];
  int connfd;
};

struct SocketConnection {
    int connfd;
};

UnixSocket create_socket(int fd, const std::string& path) {
    UnixSocket s;
    s.fd = fd;
    s.path = path;
    return s;
}

enum PbeType
{
  TRAIN,
  TEST,
};

enum IOType
{
  INPUT,
  INTERMEDIATE,
  OUTPUT
};

// ARC AGI Module
// TODO: Update to ATen LibTorch
struct ArcData
{
  std::vector<std::vector<unsigned char>> matrix;
  std::vector<std::vector<flecs::entity>> pixel_entities_matrix;
  int task_num;
  IOType io = IOType::INPUT;
  PbeType pbe_type = PbeType::TRAIN;
};

struct ArcPattern
{
  int id;
  std::vector<std::vector<unsigned char>> bitmask;
  std::vector<std::vector<unsigned char>> monochrome;
  int x, y;
  int color;
  int pixel_count = 0;
};

struct ArcAbstraction
{
  IOType matrix_type;
  std::string stats_bar;
};

struct ArcColorPair {
  int src_color;
  int target_color;

  bool operator==(const ArcColorPair& other) const {
    return src_color == other.src_color && target_color == other.target_color;
  }
};

struct ArcColorPairHash {
  std::size_t operator()(const ArcColorPair& pair) const {
    auto h1 = std::hash<int>{}(pair.src_color);
    auto h2 = std::hash<int>{}(pair.target_color);
    return h1 ^ h2;
  }
};


struct ArcColorTransfer
{
  int src_grid_color;
  int target_grid_color;
  int count;
};

struct ArcMeasurement
{
  ArcPattern a;
  ArcPattern b;
  std::vector<unsigned char> x_measure;
  std::vector<unsigned char> y_measure; // TODO: More nuanced measurements than bounds (ie 'interlocked')
};

struct ArcDataLoader
{
  std::string dir;
  std::vector<std::string> data;
  std::vector<std::string> task_ids;
  int active_index = 0;
  int active_task = 0;
  flecs::entity viz;

  std::vector<flecs::entity> tasks;

  // The loader is used to traverse program visualization for tasks
  // This could be refactored to another component
  // Especially if multiple programs can be visible at once
  // This is'working' code and needs to be refactored
  std::vector<ArcData> task_data;
  std::vector<std::unordered_map<int, std::vector<ArcPattern>>> input_grid_patterns;
  std::vector<std::unordered_map<int, std::vector<ArcPattern>>> output_grid_patterns;

  // train/test
  std::unordered_map<std::string, int> track_counts;
  
  std::string get_active_problem()
  {
    std::filesystem::path arc_problem_path(data[active_index]);
    return arc_problem_path.stem().string();
  }
  int get_index(std::string spell_id)
  {
    std::cout << "Get Index" << std::endl;
    int i = 0;
    for (const auto & entry : fs::directory_iterator(dir))
    {
      std::cout << "Get Index from ID" << std::endl;
        std::cout << spell_id << std::endl;
        std::cout << entry.path().stem().string() << std::endl;
      if (spell_id == entry.path().stem().string())
      {
        std::cout << i << std::endl;
        return i;
      }
      i++;
    }
    return 0;
  }
};


struct ArcColorBitmaskMatrix {};

// Just a single color
struct ArcColorMask 
{
  char value; // -1 is null matrix
  bool enabled;
  flecs::entity color_mask;
};
struct ArcColorSetMask
{
  std::vector<bool> colors;
  bool wildcard = false;
  bool void_value = false;
};

struct ArcViz {};

struct ArcOutput {};

pthread_mutex_t mutex =     PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition =  PTHREAD_COND_INITIALIZER;

flecs::entity get_telos(flecs::entity graph)
{
  flecs::entity telos;
  graph.children([&](flecs::entity child) 
  {
    if (child.has<EventOnGrab>(flecs::Wildcard))
    {
      child.children([&](flecs::entity t) {
          telos = t;
      });
    }
  });
  return telos;
}

int getRandomInt(int min, int max) {
    // rand() returns a pseudo-random integer between 0 and RAND_MAX
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

std::string getRandomFileFromDir(const std::string& path) {
    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path().string());
        }
    }
    if (files.empty()) {
        return "";
    }
    std::srand(std::time(nullptr));
    int index = std::rand() % files.size();
    return files[index];
}

std::vector<NVGcolor> get_palette(std::string filepath, float alpha = 255)
{
  std::ifstream file(filepath);
  std::string line;
  std::vector<NVGcolor> colors;
  while (std::getline(file, line)) {
      if(line.length() >= 6) { // Ensure the line has at least 6 characters for RGB
          unsigned int r, g, b;
          std::stringstream ss;
          ss << std::hex << line.substr(0, 2);
          ss >> r;
          ss.clear();
          ss << std::hex << line.substr(2, 2);
          ss >> g;
          ss.clear();
          ss << std::hex << line.substr(4, 2);
          ss >> b;
          colors.push_back(nvgRGBA(r, g, b, alpha));
      }
  }
  return colors;
}

std::string getClipboardString(Display* display, Window window) {
    std::string clipboardString;

    Atom clipboard = XInternAtom(display, "CLIPBOARD", False);
    Atom utf8String = XInternAtom(display, "UTF8_STRING", True);
    if (clipboard != None && utf8String != None) {
        XConvertSelection(display, clipboard, utf8String, XA_PRIMARY, window, CurrentTime);
        XFlush(display);

        Atom actual_type;
        int actual_format;
        unsigned long nitems, remaining;
        unsigned char* data = nullptr;
        int status = XGetWindowProperty(display, window, XA_PRIMARY, 0, (~0L), False, AnyPropertyType,
                                        &actual_type, &actual_format, &nitems, &remaining, &data);
        if (status == Success && actual_type == utf8String && actual_format == 8) {
            clipboardString = reinterpret_cast<char*>(data);
            XFree(data);
        }
    }

    return clipboardString;
}

/* new window size */
void reshape( GLFWwindow* window, int width, int height )
{
  GLfloat h = (GLfloat) height / (GLfloat) width;
  GLfloat xmax, znear, zfar;

  znear = 5.0f;
  zfar  = 30.0f;
  xmax  = znear * 0.5f;

  glViewport( 0, 0, (GLint) width, (GLint) height );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glFrustum( -xmax, xmax, -xmax*h, xmax*h, znear, zfar );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glTranslatef( 0.0, 0.0, -20.0 );
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static std::mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
// TODO: Get range max from dir...
static std::uniform_int_distribution<int> set_distribution(0, 499);

int random_mnist_char_set() {
    int random_set_index = set_distribution(rng);
    return random_set_index;
}

std::string random_mnist(int digit, int set_index) {

    std::string path = "../assets/mnist/unique/";
    path += "set_";
    path += std::to_string(set_index);
    path += "/";
    path += std::to_string(digit);
    path += ".png";

    return path;
}

std::string random_neighbor_mnist(int digit, int set_index) {
    std::string base_path = "../assets/mnist/unique/"; // Path from your snippet
    std::string nn_file_path = base_path + "set_" + std::to_string(set_index) +
                               "/" + std::to_string(digit) + "_nn.txt";

    std::vector<int> neighbor_set_indices;
    std::ifstream nn_file(nn_file_path);
    
    if (nn_file.is_open()) {
        int neighbor_idx;
        while (nn_file >> neighbor_idx) {
            neighbor_set_indices.push_back(neighbor_idx);
        }
    }

    if (!neighbor_set_indices.empty()) {
        // Select one neighbor set index randomly from the populated list
        std::uniform_int_distribution<size_t> neighbor_choice_dist(0, neighbor_set_indices.size() - 1);
        int selected_neighbor_set_index = neighbor_set_indices[neighbor_choice_dist(rng)];

        // Construct the path to the selected neighbor image
        return base_path + "set_" + std::to_string(selected_neighbor_set_index) +
               "/" + std::to_string(digit) + ".png";
    } else {
        // Fallback: _nn.txt file wasn't opened, was empty, 
        // or contained no valid integers at the start.
        int set_index = random_mnist_char_set();
        return random_mnist(digit, set_index); // Assumes random_mnist function is available
    }
}


// Get closest point that is radius away from src to dest...
void get_closest_radius_point(vec2 src, vec2 dest, float radius, vec2* output)
{
  vec2 diff;
  glm_vec2_sub(dest, src, diff);
  glm_vec2_normalize(diff);
  vec2 closest;
  glm_vec2_copy(dest, closest);
  glm_vec2_scale(diff, radius, diff);
  glm_vec2_sub(closest, diff, closest);
  glm_vec2_copy(closest, *output);
}

flecs::world ecs;
efsw::FileWatcher* fileWatcher;
efsw::WatchID watchID;
std::vector<LoadGraphData> graphs_to_load;

std::vector<NVGcolor> arc_element_colors = {
  nvgRGBA(055, 0x55, 0x55, 0xFF),
  nvgRGBA(0x00, 0x74, 0xD9, 0xFF),
  nvgRGBA(0xFF, 0x41, 0x36, 0xFF),
  nvgRGBA(0x2E, 0xCC, 0x40, 0xFF),
  nvgRGBA(0xFF, 0xDC, 0x00, 0xFF),
  nvgRGBA(0xAA, 0xAA, 0xAA, 0xFF),
  nvgRGBA(0xF0, 0x12, 0xBE, 0xFF),
  nvgRGBA(0xFF, 0x85, 0x1B, 0xFF),
  nvgRGBA(0x7F, 0xDB, 0xFF, 0xFF),
  nvgRGBA(0x98, 0x58, 0x98, 0xFF)
};
std::vector<std::string> arc_element_names = 
{
  "slate", "zen", "rose", "grass", "sun", "grey", "purse", "tang", "sky", "space" 
}; 

std::unordered_map<std::string, NVGcolor> arc_element_colors_map = {
    {"slate", nvgRGBA(0x55, 0x55, 0x55, 0xFF)},
    {"zen", nvgRGBA(0x00, 0x74, 0xD9, 0xFF)},
    {"rose", nvgRGBA(0xFF, 0x41, 0x36, 0xFF)},
    {"grass", nvgRGBA(0x2E, 0xCC, 0x40, 0xFF)},
    {"sun", nvgRGBA(0xFF, 0xDC, 0x00, 0xFF)},
    {"grey", nvgRGBA(0xAA, 0xAA, 0xAA, 0xFF)},
    {"purse", nvgRGBA(0xF0, 0x12, 0xBE, 0xFF)},
    {"tang", nvgRGBA(0xFF, 0x85, 0x1B, 0xFF)},
    {"sky", nvgRGBA(0x7F, 0xDB, 0xFF, 0xFF)},
    {"space", nvgRGBA(0x98, 0x58, 0x98, 0xFF)}
};

std::unordered_map<std::string, std::string> graph_emoji_keywords = {
    {"paint", "../assets/paint_operator.png"},
    {"move", "../assets/move_operator.png"},
    {"copy", "../assets/copy_operator.png"},
    {"flip", "../assets/flip_operator.png"},
    {"rotate", "../assets/rotate_operator.png"},
    {"scale", "../assets/scale_operator.png"},

    {"polyomino", "../assets/polyomino_pickaxe.png"},
    {"polyominoe", "../assets/polyomino_pickaxe.png"},
    {"polyominoes", "../assets/polyomino_pickaxe.png"},

    {"partition", "../assets/partition_axe.png"},

    {"cluster", "../assets/isomorphic_hash.png"},
    {"cluster_", "../assets/isomorphic_hash.png"},
    {"cluster_0", "../assets/isomorphic_hash.png"},
    {"cluster_1", "../assets/isomorphic_hash.png"},
    {"cluster_2", "../assets/isomorphic_hash.png"},
    {"cluster_3", "../assets/isomorphic_hash.png"},
    {"cluster_4", "../assets/isomorphic_hash.png"},
    {"cluster_5", "../assets/isomorphic_hash.png"},

    {"create", "../assets/create_state.png"},
    {"submatrix", "../assets/query_select.png"},
    {"reduce", "../assets/expand.png"},
    {"expand", "../assets/expand.png"},

    {"row", "../assets/vector_hammer.png"},
    {"rows", "../assets/vector_hammer.png"},
    {"column", "../assets/vector_hammer.png"},
    {"columns", "../assets/vector_hammer.png"},
    {"line", "../assets/vector_hammer.png"},
    {"lines", "../assets/vector_hammer.png"},

    {"sort", "../assets/sort_group.png"},
    {"compose", "../assets/graphpad.png"},
    {"background", "../assets/bkg.png"},
    {"foreground", "../assets/bkg.png"},
    
    {"input", "../assets/stellar/mag_0.PNG"},
    {"output", "../assets/stellar/mag_2.PNG"},
    {"train", "../assets/stellar/mag_1.PNG"},
    {"test", "../assets/stellar/mag_3.PNG"},

    {"outline", "../assets/outline.png"},
    {"interior", "../assets/outline.png"},
    {"inside", "../assets/outline.png"},

    {"shape", "../assets/shape_index.png"},
    
    {"branch", "../assets/boolean_branch.png"},
    {"propagate", "../assets/propagation.png"},
    {"progress", "../assets/world_progress.png"},

    {"region", "../assets/deductive_reasoning.png"},

};

// Create component above
std::set<std::string> component_symbol = {
  "slate", "zen", "rose", "grass", "sun", "grey", "purse", "tang", "sky", "space",
  "polyomino", "polyominoe", "polyominoes", "task", "background", "foreground",
  "pixel", "pixels", "wildcard", "test", "train", "input", "output", "io"
};

std::set<std::string> relationships = {
  "within", "outside",
  "above", "below", "leftof", "rightof", 
  "equalx", "separatex", "withinx", "surroundingx", "overlapx", "overhangx", "alignedx",
  "equaly", "separatey", "withiny", "surroundingy", "overlapy", "overhangy", "alignedy",
  "hole", "bound", "boundingbox",
};

// Indicate 'any color except'
std::string color_set = "wildcard";

// Create component with number above
std::set<std::string> numeric_component_symbol = {
  "x", "y", "height", "width", "pixel_count", "shape", "rotation", "area"
  "up", "down", "left", "right",
};


std::vector<flecs::entity> arc_color_entities;

struct ArcValue {};
struct ArcColorIndex { int value; };

void add_arc_value_component(flecs::entity entity, int value)
{
  entity.add<ArcValue>(arc_color_entities[value]);
  entity.set<ArcColorIndex>({value});
}

void visualize_coord_delta(flecs::entity parent, std::vector<std::vector<unsigned char>> delta_matrix)
{
  int height = delta_matrix.size();
  int width = delta_matrix[0].size();
  float base_size = 8;
  auto ui_element = ecs.lookup("ui_element");
  float y = 0;
  for (auto& row : delta_matrix)
  {
    float x = 0;
    for (auto& v : row)
    {
      if (v >= 0)
      {
        if (v == 0)
        {
          flecs::entity arc_element = ecs.entity()
            .is_a(ui_element)
            .add<LevelHUD>()
            .set<Position, Local>({x * (base_size+2) + 1, y * (base_size+2) + 1})
            .set<BoxShape>({base_size-2, base_size-2, 1.0f})
            .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(0x22, 0x22, 0x22, 0xFF)})
            .child_of(parent);
        } else if (v == 1)
        {
          flecs::entity arc_element = ecs.entity()
            .is_a(ui_element)
            .add<LevelHUD>()
            .set<Position, Local>({x * (base_size+2), y * (base_size+2)})
            .set<BoxShape>({base_size, base_size, 0.0f})
            .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0xFF, 0xFF, 0xFF, 0xFF)})
            .child_of(parent);
        }
        x++;
      }
    }
    y++;
  }
}

void visualize_bin(flecs::entity parent, std::vector<std::vector<unsigned char>> matrix)
{
  int height = matrix.size();
  int width = matrix[0].size();
  float base_size = 16;
  if (height >= 16)
  {
    base_size = 10;
  }
  auto ui_element = ecs.lookup("ui_element");
  float y = 0;
  for (auto& row : matrix)
  {
    float x = 0;
    for (auto& v : row)
    {
      if (v >= 0)
      {
          if (v == 0)
          {
            flecs::entity arc_element = ecs.entity()
              .is_a(ui_element)
              .add<LevelHUD>()
              .set<Position, Local>({x * (base_size+2) + 1, y * (base_size+2) + 1})
              .set<BoxShape>({base_size-2, base_size-2, 1.0f})
              .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(0xFF, 0xFF, 0xFF, 0xFF)})
              .child_of(parent);
          } else if (v == 1)
          {
            flecs::entity arc_element = ecs.entity()
              .is_a(ui_element)
              .add<LevelHUD>()
              .set<Position, Local>({x * (base_size+2), y * (base_size+2)})
              .set<BoxShape>({base_size, base_size, 0.0f})
              .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0xFF, 0xFF, 0xFF, 0xFF)})
              .child_of(parent);
          }
        x++;
      }
    }
    y++;
  }
}

void visualize_matrix(flecs::entity parent, ArcData& task_data, bool has_arc_values=true)
{
  int height = task_data.matrix.size();
  int width = task_data.matrix[0].size();
  float base_size = 16;
  if (height >= 16)
  {
    base_size = 10;
  }
  auto ui_element = ecs.lookup("ui_element");
  float y = 0;
  for (auto& row : task_data.matrix)
  {
    float x = 0;
    std::vector<flecs::entity> entity_row;
    for (auto& v : row)
    {
      if (v >= 0)
      {
        std::cout << "Adding " << v << "to box" << std::endl;
        auto color = arc_element_colors[v];
        std::cout << "Color for v=" << v << ": " << color.r << "," << color.g << "," << color.b << "," << color.a << std::endl;
          flecs::entity pixel_box = ecs.entity()
            .is_a(ui_element)
            .add<LevelHUD>()
            .set<Position, Local>({x * (base_size+2), y * (base_size+2)})
            .set<BoxShape>({base_size, base_size, 0.0f})
            .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(arc_element_colors[v].r*255, arc_element_colors[v].g*255, arc_element_colors[v].b*255, 16)})
            // .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(arc_element_colors[v].r*255, arc_element_colors[v].g*255, arc_element_colors[v].b*255, 255)})
            // .set<RenderStatus>({true, RenderMode::FILL, arc_element_colors[v]})
            // .set<RenderStatus>({true, RenderMode::FILL, color})
            // .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(255, 255, 255, 255)});
            
            .child_of(parent);
          if (has_arc_values)
          {
            add_arc_value_component(pixel_box, v);
            entity_row.push_back(pixel_box);
          }
        }
        x++;
      }
      if (has_arc_values)
      {
        task_data.pixel_entities_matrix.push_back(entity_row);
      }
      y++;
    }
}

// Used to extract global measurement variables
void visualize_null_matrix(flecs::entity parent, std::vector<std::vector<unsigned char>> matrix)
{
  auto ui_element = ecs.lookup("ui_element");
  float base_size = 16.0f;
  float padding = 2.0f;
  for (size_t y = 0; y < matrix.size() + 2; y++)
  {
    auto row = matrix[0];
    for (size_t x = 0; x < row.size() + 2; x++)
    {
      if (y == 0 || y == matrix.size() + 1 || x == 0 || x == row.size() + 1)
      {
        flecs::entity arc_element = ecs.entity()
          .is_a(ui_element)
          .add<LevelHUD>()
          .set<Position, Local>({x * (base_size+padding) + 2, y * (base_size+padding) + 2})
          .set<BoxShape>({12.0f, 12.0f, 3.0f})
          .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(0x99, 0x00, 0x00, 0xFF)})
          .child_of(parent);
      }
    }
  }
}

void visualize_object_spatial_measurement(flecs::entity parent, std::vector<std::vector<unsigned char>> a, std::vector<std::vector<unsigned char>> b, std::vector<std::vector<unsigned char>> measurement)
{

}

void highlight_query_pixels()
{
  // std::string ops_dir = "../../synthesis/build/ops/";
  std::string ops_dir = "../../jane_eyre/ops/";
  // TODO: First, when a matrix is loaded, we should store those entities in an array such that they may be referenced by index
  // Matrix of 'pixel box ui element' entities is stored in ArcData... 
  auto q = ecs.query_builder<ArcData>()
              .build();
  q.each([&](flecs::entity e, ArcData& arc_data) 
  {
    // Based on the ARC Data, load the corresponding op_src file...
    std::string op_src = ops_dir + std::string(arc_data.pbe_type == PbeType::TRAIN ? "train" : "test") + "_" + std::string(arc_data.io == IOType::INPUT ? "input" : "output") + "_" + std::to_string(arc_data.task_num) + ".txt";
    // TODO: Store ops by task id and query for later retrieval
    if (std::filesystem::exists(op_src)) {
      std::ifstream file(op_src);
      std::set<std::pair<int, int>> coords;
      std::string line;
      int num1, num2;
      while (std::getline(file, line)) {
        std::stringstream ss(line);
        // Extract the two integers from the line.
        if (ss >> num1 >> num2) {
          // Insert the pair of numbers as a vector into the set.
          coords.insert({num1, num2});
          std::cout << "Added coord " << num1 << " " << num2 << std::endl;
        }
      }
      
      
      for (size_t row = 0; row < arc_data.pixel_entities_matrix.size(); row++)
      {
        std::cout << "Iterate pixel_entities_matrix" << std::endl;
        for (size_t col = 0; col < arc_data.pixel_entities_matrix[row].size(); col++)
        {
          flecs::entity pixel_box = arc_data.pixel_entities_matrix[row][col];
          const ArcColorIndex* aci = pixel_box.try_get<ArcColorIndex>();
          NVGcolor color = arc_element_colors[aci->value];
          if (coords.count({col, row}))
          {
            std::cout << "Set pixel render status" << std::endl;
            pixel_box.set<RenderStatus>({true, RenderMode::FILL, color});
          }
          // nvgRGBA(color.r*255, color.g*255, color.b*255, 64)
        }
      } 
    }
  });

  // We will need to refactor visualize_matrix
  // Then, we will iterate through the op_src files, parsing the pixel coordinates into a vector
  // Finally, we will  iterate through all the tasks, ensuring the visual opacity of query indicated pixels is 100% while others are dimmed
}

// newly created
void add_intermediate_state(ArcDataLoader* loader)
{
  flecs::entity ui_element = ecs.lookup("ui_element");
  for (auto& task_state : loader->tasks)
  {
      flecs::entity pattern_coordinate = ecs.entity()
        .is_a(ui_element)
        .set<TextCreator>({"Intermediate State", "ATARISTOCRAT"})
        .add<RenderStatus>()
        .add<LevelHUD>() 
        .child_of(task_state);
  }
  std::string new_states_path = "../../synthesis/build/task_set_states";
}

// Update color mask matrix
void visualize_color_matrix(flecs::entity parent)
{
  auto ui_element = ecs.lookup("ui_element");
  
  flecs::entity color_mask = ecs.entity()
    .set<ArcColorSetMask>({{false, false, false, false, false, false, false, false, false ,false}, false, false})
    .child_of(parent);

  float base_size = 16.0f;
  float padding = 2.0f;
  for (size_t y = 0; y < 4; y++)
  {
    for (size_t x = 0; x < 3; x++)
    {
      char color_index = y*3 + x;
      if (color_index < arc_element_colors.size())
      {
        flecs::entity arc_element = ecs.entity()
          .is_a(ui_element)
          .add<LevelHUD>()
          .set<Position, Local>({x * (base_size+padding) + 1, y * (base_size+padding) + 1})
          .set<BoxShape>({14.0f, 14.0f, 1.0f})
          .set<RenderStatus>({true, RenderMode::STROKE, arc_element_colors[color_index]})
          .set<ArcColorMask>({color_index, false, color_mask})
          .add<Button>()
          .child_of(parent);
        add_arc_value_component(arc_element, color_index);
      } else if (color_index == arc_element_colors.size() + 1)
      {
        flecs::entity arc_void = ecs.entity() // Null matrix
          .is_a(ui_element)
          .add<LevelHUD>()
          .set<Position, Local>({x * (base_size+padding) + 2, y * (base_size+padding) + 2})
          .set<BoxShape>({13.0f, 13.0f, 3.0f})
          .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(0x99, 0x00, 0x00, 0xFF)})
          .set<ArcColorMask>({-1, false, color_mask})
          .add<Button>()
          .child_of(parent);
      } else if (color_index = arc_element_colors.size())
      {
        flecs::entity arc_element_wildcard = ecs.entity() // Wildcard matrix
          .is_a(ui_element)
          .add<LevelHUD>()
          .set<Position, Local>({x * (base_size+padding) + 1, y * (base_size+padding) + 1})
          .set<SpriteCreator>({"../assets/wildcard_unselected.png"})
          .add<Button>()
          .set<ArcColorMask>({-2, false, color_mask})
          .child_of(parent);
      }
    }
  }
}

ArcPattern load_arc_pattern(const std::string& path) {
  std::ifstream f(path);
  json data = json::parse(f);

  ArcPattern pattern;
  pattern.id = data["pattern"]["id"];
  // TODO: Unify row/col, actually x is processed first which is cols...
  for (auto& row : data["pattern"]["bitmask"])
  {
    std::vector<unsigned char> row_values;
    for (auto& v : row)
    {
      row_values.push_back(static_cast<unsigned char>(v));
      if (static_cast<unsigned char>(v))
      {
        pattern.pixel_count += 1;
      }
    }
    pattern.bitmask.push_back(row_values);
  }

  for (auto& row : data["pattern"]["monochrome"])
  {
    std::vector<unsigned char> row_values;
    for (auto& v : row)
    {
      row_values.push_back(static_cast<unsigned char>(v));
    }
    pattern.monochrome.push_back(row_values);
  }

  pattern.x = data["pattern"]["coordinates"][0];
  pattern.y = data["pattern"]["coordinates"][1];

  pattern.color = data["pattern"]["color"];

  return pattern;
}

unsigned char extract_number(std::istream& is) {
    int number;
    is >> number;
    return static_cast<unsigned char>(number);
}

// The base case for the recursion: parses a single dimension of unsigned chars.
void parse_tensor(std::istream& is, std::vector<unsigned char>& current_dimension) {
    char ch;
    is >> ch; // Consume the opening '['

    while (is.peek() != ']') { // Check if the next character is not the closing ']'
        current_dimension.push_back(extract_number(is));
        
        is >> ch; // Consume the separator, either ',' or ']'
        if (ch == ']') {
            is.unget(); // Push the ']' back if it was read prematurely
        }
    }
    is >> ch; // Consume the closing ']'
}

// Generic recursive function for all other dimensions (templates).
template<typename T>
void parse_tensor(std::istream& is, T& current_dimension) {
    char ch;
    is >> ch; // Consume the opening '['

    while (is.peek() != ']') {
        typename T::value_type next_dimension;
        parse_tensor(is, next_dimension);
        current_dimension.push_back(next_dimension);
        
        is >> ch; // Consume the separator, either ',' or ']'
        if (ch == ']') {
            is.unget();
        }
    }
    is >> ch; // Consume the closing ']'
}

void print_3d_tensor(const std::vector<std::vector<std::vector<unsigned char>>>& tensor) {
    std::cout << "[\n";
    for (const auto& matrix : tensor) {
        std::cout << "  [\n";
        for (const auto& row : matrix) {
            std::cout << "    [";
            for (size_t i = 0; i < row.size(); ++i) {
                std::cout << row[i] << (i == row.size() - 1 ? "" : ", ");
            }
            std::cout << "]" << (matrix.back() == row ? "" : ",\n");
        }
        std::cout << "\n  ]" << (tensor.back() == matrix ? "" : ",\n");
    }
    std::cout << "\n]\n";
}


void render_letters(ArcDataLoader* loader)
{
  std::string filename = "../save/arc/" + loader->task_ids[loader->active_index] + "/letters.json";
  std::cout << "Render Letters " << filename << std::endl;

  flecs::entity arc_task_set_letters = ecs.lookup("arc_task_letters");
  arc_task_set_letters.children([&](flecs::entity child)
  {
    child.destruct();
  });
  // Read the tensor data from the file
  std::ifstream file(filename);
  if (!file.is_open()) {
      std::cerr << "Error: Unable to open file." << std::endl;
      return;
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string tensor_data = buffer.str();
  file.close();

  // Use an istringstream to parse the string data
  std::istringstream ss(tensor_data);

  // Define the 4D vector to hold the tensor data
  std::vector<std::vector<std::vector<std::vector<unsigned char>>>> tensor_4d;

  // Start the parsing process
  parse_tensor(ss, tensor_4d);

  // Print the first 3D tensor
  if (!tensor_4d.empty()) {
      std::cout << "First 3D Tensor:\n";
      print_3d_tensor(tensor_4d[0]);
  } else {
      std::cout << "Tensor is empty." << std::endl;
  }

  auto ui_element = ecs.lookup("ui_element");

  int letter_index = 0;
  for (auto letter : tensor_4d)
  {
    flecs::entity letter_pair = ecs.entity()
    .is_a(ui_element)
    // .set<HorizontalLayoutBox>({0.0f, 2.0f})
    .child_of(arc_task_set_letters);
    
    int letter_spacing = 18*3+4;
    
    flecs::entity letter_input_ent = ecs.entity()
    .is_a(ui_element)
    .set<Position, Local>({0, letter_index*letter_spacing})
    .child_of(letter_pair);

    flecs::entity letter_output_ent = ecs.entity()
    .is_a(ui_element)
    .set<Position, Local>({letter_spacing, letter_index*letter_spacing})
    .child_of(letter_pair);
  
    // We only need to populate 'matrix' data for these...
    ArcData letter_input;
    ArcData letter_output;
    letter_input.matrix = letter[0];
    letter_output.matrix = letter[1];
  
    visualize_matrix(letter_input_ent, letter_input);
    visualize_matrix(letter_output_ent, letter_output);
    letter_index++;
  }

  // Call a Python script for Jane Eyre to process letters from a task
  // to 
  // Load the letters inputs and outputs from a file



  // for (letter in letters)
  // {

  // }

}

void populate_spellcast_bar(ArcDataLoader* loader)
{
  std::cout << "Populate spellcast bar" << std::endl;
  ArcData* task_data = &loader->task_data[loader->active_task*2];
  std::unordered_map<int, std::vector<ArcPattern>>& input_grid_patterns = loader->input_grid_patterns[loader->active_index];
  std::unordered_map<int, std::vector<ArcPattern>>& output_grid_patterns = loader->output_grid_patterns[loader->active_index];
  std::unordered_map<std::string, int>& track_counts = loader->track_counts;

  loader->input_grid_patterns.clear();
  loader->output_grid_patterns.clear();

    // -- same as load_arc_task
    auto ui_element = ecs.lookup("ui_element");
    std::vector<std::string> tracks = {"train", "test"}; ;
    std::vector<ArcAbstraction> abstractions = {{IOType::INPUT, "spellbar"}, {IOType::OUTPUT, "cast_status"}};

    int task_count = 0;
    for (std::string track : tracks) // train/test
    {
    for (int task_num = 0; task_num < track_counts[track]; task_num++)
    {
    for (auto abstraction : abstractions)
    {
      std::unordered_map<int, std::vector<ArcPattern>> grid_patterns;

      if (abstraction.matrix_type == IOType::INPUT)
      {
        loader->input_grid_patterns.push_back(grid_patterns);
      } else if (abstraction.matrix_type == IOType::OUTPUT)
      {
        loader->output_grid_patterns.push_back(grid_patterns);
      }
      std::string patterns_dir = "../save/arc/" + loader->task_ids[loader->active_index] + "/" + track + "/task_" + std::to_string(task_num) + "/" + (abstraction.matrix_type == IOType::INPUT ? "input" : "output") + "/patterns";
      for (const auto& entry : std::filesystem::directory_iterator(patterns_dir)) {
        if (entry.is_regular_file()) {
          std::cout << entry.path().string() << std::endl;
          ArcPattern pattern = load_arc_pattern(entry.path());

          if (abstraction.matrix_type == IOType::INPUT)
          {
            loader->input_grid_patterns[task_count][pattern.color].push_back(pattern);
          } else if (abstraction.matrix_type == IOType::OUTPUT)
          {
            loader->output_grid_patterns[task_count][pattern.color].push_back(pattern);
          }

          // TODO: Load the pattern visualizations interactively (ie on hover)
          // flecs::entity arc_pattern = ecs.entity()
          // flecs::entity arc_pattern_box = ecs.entity()
          // .is_a(ui_element)
          // .add<VerticalLayoutBox>()
          //   .is_a(ui_element)
          //   .child_of(loader->viz);
          //   .child_of(tasks[0]);
          // visualize_matrix(arc_pattern, pattern.monochrome);

          // flecs::entity pattern_coordinate = ecs.entity()
          //   .is_a(ui_element)
          //   .set<TextCreator>({("(" + std::to_string(pattern.x) + ", " + std::to_string(pattern.y) + ")").c_str(), "ATARISTOCRAT"})
          //   .add<RenderStatus>()
          //   .add<LevelHUD>() 
          //   .child_of(tasks[0]);

          // Shape viz
          // TODO: Visualize shapes as compositions of letters and finite cyclic group indexing
          // flecs::entity arc_bitmask = ecs.entity()
          //   .is_a(ui_element)
          //   .child_of(loader->tasks[0]);
          // visualize_bin(arc_bitmask, pattern.monochrome);

        }
      }
    }
    task_count++;
    }
    }
    
    int frame = 0; 
    for (auto abstraction : abstractions)
    {
      std::unordered_map<int, std::vector<ArcPattern>>* grid_patterns;
      if (abstraction.matrix_type == IOType::INPUT)
      {
        grid_patterns = &loader->input_grid_patterns[loader->active_task];
      } else if (abstraction.matrix_type == IOType::OUTPUT)
      {
        grid_patterns = &loader->output_grid_patterns[loader->active_task];
      }

    // -- same as load_arc_task

    auto bar = ecs.lookup(abstraction.stats_bar.c_str());
    if (bar)
    {
      bar.children([&](flecs::entity child)
      {
        child.destruct();
      });
    }

    flecs::entity output_grid_size = ecs.entity()
      .is_a(ui_element)
      .set<TextCreator>({std::string(std::to_string(task_data[frame].matrix[0].size()) + "x" + std::to_string(task_data[frame].matrix.size()) + " ").c_str(), "ATARISTOCRAT"})
      .add<RenderStatus>()
      .add<LevelHUD>()
      .set<FontStatus>({24.0f, 0.0f})
      .child_of(bar);

    auto sorted_by_pixel_count = std::vector<std::pair<int, std::vector<ArcPattern>>>(grid_patterns->begin(), grid_patterns->end());
    std::sort(sorted_by_pixel_count.begin(), sorted_by_pixel_count.end(),
              [](const auto& a, const auto& b) {
                int total_a = std::accumulate(a.second.begin(), a.second.end(), 0,
                                            [](int sum, const ArcPattern& pattern) { return sum + pattern.pixel_count; });
                int total_b = std::accumulate(b.second.begin(), b.second.end(), 0,
                                            [](int sum, const ArcPattern& pattern) { return sum + pattern.pixel_count; });
                return total_a < total_b;
              });

    // Sort the grid patterns by number of patterns
    auto sorted_by_pattern_count = std::vector<std::pair<int, std::vector<ArcPattern>>>(grid_patterns->begin(), grid_patterns->end());
    std::sort(sorted_by_pattern_count.begin(), sorted_by_pattern_count.end(),
              [](const auto& a, const auto& b) {
                return a.second.size() < b.second.size();
              });

    for (const auto& pair : sorted_by_pixel_count) {
      int mnist_index = pair.first;
      if (mnist_index >= 0 && mnist_index < 10) {
        int total_pixel_count = std::accumulate(pair.second.begin(), pair.second.end(), 0,
                                                [](int sum, const ArcPattern& pattern) { return sum + pattern.pixel_count; });

        int mnist_char_set_index = random_mnist_char_set();
        flecs::entity heuristic_sort_index = ecs.entity()
          .is_a(ui_element)
          .set<SpriteCreator>({random_mnist(mnist_index, mnist_char_set_index)})
          .set<MNISTDigit>({mnist_index, mnist_char_set_index})
          .set<SpriteTint>({arc_element_colors[mnist_index]})
          .add<LevelHUD>()
          .child_of(bar);

        flecs::entity sort_index_label = ecs.entity()
          .is_a(ui_element)
          .set<TextCreator>({std::to_string(total_pixel_count), "ATARISTOCRAT"})
          .set<RenderStatus>({true, RenderMode::FILL, arc_element_colors[mnist_index]})
          .add<LevelHUD>()
          .set<Position, Local>({4.0f, 38.0f})
          .child_of(heuristic_sort_index);

        // Pattern count
        flecs::entity sort_index_label_2 = ecs.entity()
          .is_a(ui_element)
          .set<TextCreator>({std::to_string(pair.second.size()), "ATARISTOCRAT"})
          .set<RenderStatus>({true, RenderMode::FILL, arc_element_colors[mnist_index]})
          .add<LevelHUD>()
          .set<Position, Local>({4.0f, 38.0f + 14.0f})
          .child_of(heuristic_sort_index);
      }
    }
    frame++; // spellbar -> cast_status is the cannonical two frame spell
  }

  flecs::entity elementary_color_pairs = ecs.lookup("elementary_color_pairs");
  if (elementary_color_pairs)
  {
    elementary_color_pairs.children([&](flecs::entity child)
    {
      child.destruct();
    });
  }

  // TODO: Render Alhabima alphabet
  // TODO: Render quantized and fincyc functional compression of alphabet

  // TODO: Render alphabet abstraction tree
  // TODO: Render isolated alphabet search

  // TODO: Color transfers per task_num
  // ie down arrow to next task
  ArcData input_data = task_data[0];
  ArcData output_data = task_data[1];

  // std::vector<std::vector<unsigned char>> delta_matrix(output_data.matrix.size(), std::vector<unsigned char>(output_data.matrix[0].size()));
  std::vector<std::vector<unsigned char>> delta_matrix = output_data.matrix;

  int output_color_delta_counts[10];
  for (size_t c_i = 0; c_i < 10; c_i++)
  {
    output_color_delta_counts[c_i] = 0;
  }

  // The number of pixels to solve for a task, this is the sum of delta pixels on matrices of the same size, or 
  int pixel_solve_diff = 0;
  bool io_size_equivalent = input_data.matrix.size() == output_data.matrix.size() && input_data.matrix[0].size() == output_data.matrix[0].size();

  if (io_size_equivalent)
  {
    for (int y = 0; y < input_data.matrix.size(); y++) {
      for (int x = 0; x < input_data.matrix[y].size(); x++) {
        int src_color = input_data.matrix[y][x];
        int target_color = output_data.matrix[y][x];
        if (src_color != target_color)
        {
          pixel_solve_diff++;
          output_color_delta_counts[target_color]++;
          delta_matrix[y][x] = 1;
        } else
        {
          delta_matrix[y][x] = 0;
        }
      }
    }
  } 
  else 
  {
    pixel_solve_diff = output_data.matrix.size() * output_data.matrix[0].size();
    // TODO: If there is a io submatrix mapping, then we should consider elementary pairs on that...
    for (int y = 0; y < output_data.matrix.size(); y++) {
      for (int x = 0; x < output_data.matrix[y].size(); x++) {
        int target_color = output_data.matrix[y][x];
        output_color_delta_counts[target_color]++;
      }
    }
  }

  if (io_size_equivalent)
  {
    // Matrices match, create color transfer pairs 
    std::unordered_map<ArcColorPair, ArcColorTransfer, ArcColorPairHash> color_transfer_pairs;
    // for (int y = 0; y < input_data.matrix.size(); y++)
    // {
    //   for (int x = 0; x < input_data.matrix[y].size(); x++)
    //   {
    //     int src_color = input_data.matrix[y][x];
    //     int target_color = output_data.matrix[y][x];
        
    //   }
    // }

    for (int y = 0; y < input_data.matrix.size(); y++) {
      for (int x = 0; x < input_data.matrix[y].size(); x++) {
        int src_color = input_data.matrix[y][x];
        int target_color = output_data.matrix[y][x];
        if (src_color != target_color)
        {
          ArcColorPair color_pair = {src_color, target_color};

          if (color_transfer_pairs.find(color_pair) != color_transfer_pairs.end()) {
            color_transfer_pairs[color_pair].count++;
          } else {
            ArcColorTransfer color_transfer;
            color_transfer.src_grid_color = src_color;
            color_transfer.target_grid_color = target_color;
            color_transfer.count = 1;

            color_transfer_pairs[color_pair] = color_transfer;
          }
        }

      }
    }

    std::vector<ArcColorTransfer> color_transfers;
    for (const auto& pair : color_transfer_pairs) {
      color_transfers.push_back(pair.second);
    }

    std::sort(color_transfers.begin(), color_transfers.end(), 
              [](const ArcColorTransfer& a, const ArcColorTransfer& b) {
                return a.count > b.count;
              });

    for (auto color_transfer : color_transfers)
    {
      flecs::entity color_pair = ecs.entity()
        .is_a(ui_element)
        .add<HorizontalLayoutBox>()
        .child_of(elementary_color_pairs);

      int src_mnist_char_set_index = random_mnist_char_set();
      flecs::entity color_pair_src = ecs.entity()
        .is_a(ui_element)
        .set<SpriteCreator>({random_mnist(color_transfer.src_grid_color, src_mnist_char_set_index)})
        .set<MNISTDigit>({color_transfer.src_grid_color, src_mnist_char_set_index})
        .set<SpriteTint>({arc_element_colors[color_transfer.src_grid_color]})
        .add<LevelHUD>()
        .child_of(color_pair);

      flecs::entity color_transfer_count = ecs.entity()
        .is_a(ui_element)
        .set<TextCreator>({std::to_string(color_transfer.count), "ATARISTOCRAT"})
        .add<RenderStatus>()
        .add<LevelHUD>()
        .set<Position, Local>({4.0f, 16.0f})
        .child_of(color_pair);

      int target_mnist_char_set_index = random_mnist_char_set();
      flecs::entity color_pair_target = ecs.entity()
        .is_a(ui_element)
        .set<SpriteCreator>({random_mnist(color_transfer.target_grid_color, target_mnist_char_set_index)})
        .set<MNISTDigit>({color_transfer.target_grid_color, target_mnist_char_set_index})
        .set<SpriteTint>({arc_element_colors[color_transfer.target_grid_color]})
        .add<LevelHUD>()
        .child_of(color_pair);

    }

    for (const auto& color_transfer : color_transfers) {
      std::cout << "Source color: " << color_transfer.src_grid_color << ", Target color: " << color_transfer.target_grid_color << ", Count: " << color_transfer.count << std::endl;
    }

    }
    

  
  // Programs with Common Sense
  // 4. The machine must have or evolve concepts of partial success because on difficult problems decisive sucesses or failures come too infrequently.
  // Visualization of progress towards solving an ARC program, these metrics can act as a fitness score for genetic algorithms evolving heuristic ARC solvers.
  flecs::entity partial_success = ecs.entity();

  // 
  flecs::entity grid_io_program = ecs.lookup("grid_io_program");
  grid_io_program.children([&](flecs::entity child)
  {
    child.destruct();
  });
  int grid_io_size = (output_data.matrix.size() + output_data.matrix[0].size());

  // TODO: Get grid IO prediction
  bool correct_width_prediction = input_data.matrix[0].size() == output_data.matrix[0].size();
  bool correct_height_prediction = input_data.matrix.size() == output_data.matrix.size();

  grid_io_program.set<BoxShape>({grid_io_size+6, 14.0f});
  if (correct_width_prediction && correct_height_prediction)
  {
    grid_io_program.set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(0, 255, 0, 255)});
  } else
  {
    grid_io_program.set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 0, 0, 255)});
  }

  grid_io_program.set<Position, Local>({2560.0f/2 - (grid_io_size+6)/2, 1440.0f-52.0f});

  flecs::entity width_prediction_status = ecs.entity()
    .is_a(ui_element)
    .set<BoxShape>({output_data.matrix[0].size(), 10.0, 1.0f})
    .set<Position, Relative>({2.0f, 2.0f})
    .set<RenderStatus>({true, RenderMode::FILL, correct_width_prediction ? nvgRGBA(0, 255, 0, 255) : nvgRGBA(255, 0, 0, 255)})
    .add<LevelHUD>()
    .child_of(grid_io_program);

  flecs::entity height_prediction_status = ecs.entity()
    .is_a(ui_element)
    .set<BoxShape>({output_data.matrix.size(), 10.0, 1.0f})
    .set<Position, Relative>({2.0f, 2.0f})
    .set<RenderStatus>({true, RenderMode::FILL, correct_height_prediction ? nvgRGBA(0, 255, 0, 255) : nvgRGBA(255, 0, 0, 255)})
    .add<LevelHUD>()
    .child_of(grid_io_program);

  flecs::entity pixel_perfect = ecs.lookup("pixel_perfect");
  pixel_perfect.children([&](flecs::entity child)
  {
    child.destruct();
  });

  // TODO: Implement dynamic resizing Horizontal/Vertical Box components
  float progress_bar_width = (pixel_solve_diff * 4);
  pixel_perfect.set<Position, Local>({2560.0f/2 - (4 + progress_bar_width)/2, 1440.0f-34.0f});
  pixel_perfect.set<BoxShape>({progress_bar_width+2, 14.0f});


  flecs::entity coord_delta = ecs.lookup("coord_delta");
  coord_delta.children([&](flecs::entity child)
  {
    child.destruct();
  });

  coord_delta.set<Position, Local>({2560-delta_matrix[0].size()*10.0f - 4.0f, 1440-delta_matrix.size()*10.0f - 4.0f});
  if (io_size_equivalent)
  {
    visualize_coord_delta(coord_delta, delta_matrix);
  }

  flecs::entity colors_bar = ecs.lookup("colors_bar");

  colors_bar.set<Position, Local>({2560.0f/2 - (4 + progress_bar_width)/2, 1440.0f-16.0f});

  // TODO: Inject program synthesis results here...
  // int pixels_solved = pixel_solve_diff/2;
  int pixels_solved = 0;
  int color_pixels_solved[10];
  for (size_t c_i = 0; c_i < 10; c_i++)
  {
    // color_pixels_solved[c_i] = output_color_delta_counts[c_i]/2;
    color_pixels_solved[c_i] = 0;
  }

  for (size_t i = 0; i < pixel_solve_diff; i++)
  {
    flecs::entity pixel_solved = ecs.entity()
      .is_a(ui_element)
      .set<BoxShape>({2.0, 10.0, 1.0f})
      .set<Position, Relative>({2.0f, 2.0f})
      .set<RenderStatus>({true, RenderMode::FILL, i >= pixels_solved ? nvgRGBA(128, 128, 128, 255) : nvgRGBA(255, 255, 255, 255)})
      .add<LevelHUD>()
      .child_of(pixel_perfect);
  }

  for (size_t c_i = 0; c_i < 10; c_i++)
  {
    flecs::entity pixel_perfect_color = ecs.lookup(("colors_bar::pixel_perfect_" + std::to_string(c_i)).c_str());
    if (pixel_perfect_color.is_valid() && pixel_perfect_color.is_alive())
    {
      pixel_perfect_color.children([&](flecs::entity child)
      {
        child.destruct();
      });
      pixel_perfect_color.destruct();
    }
  }

  for (size_t c_i = 0; c_i < 10; c_i++)
  {
    if (output_color_delta_counts[c_i] > 0)
    {
      flecs::entity pixel_perfect_color = ecs.entity(("pixel_perfect_" + std::to_string(c_i)).c_str())
        .is_a(ui_element)
        .set<HorizontalLayoutBox>({0.0f, 2.0f})
        .add<LevelHUD>()
        .set<BoxShape>({3, 14})
        .set<RenderStatus>({true, RenderMode::STROKE, arc_element_colors[c_i]})
        .child_of(colors_bar);

      pixel_perfect_color.set<BoxShape>({output_color_delta_counts[c_i]*4+2, 14.0f});
      pixel_perfect_color.set<RenderStatus>({true, RenderMode::STROKE, arc_element_colors[c_i]});

      for (size_t i = 0; i < output_color_delta_counts[c_i]; i++)
      {
        flecs::entity colored_pixel_solved = ecs.entity()
          .is_a(ui_element)
          .set<BoxShape>({2.0, 10.0, 1.0f})
          .set<Position, Relative>({2.0f, 2.0f})
          .set<RenderStatus>({true, RenderMode::FILL, i >= color_pixels_solved[c_i] ? nvgTransRGBA(arc_element_colors[c_i], 64) : arc_element_colors[c_i]})
          .add<LevelHUD>()
          .child_of(pixel_perfect_color);
      }
    }
  }
}

flecs::entity load_arc_task(ArcDataLoader* loader, const std::string& path)
{

  // First, we need to destruct the existing ArcData items...
  auto q = ecs.query_builder<ArcData>()
              .build();
  q.each([&](flecs::entity e, ArcData& arc_data) 
  {
    e.destruct();
  });
  loader->tasks.clear();
  loader->active_task = 0;
  std::cout << "Load ARC task " << path << std::endl;
  auto ui_element = ecs.lookup("ui_element");
  std::ifstream f(path);
  json data = json::parse(f);

  // TODO: UI config singleton...
  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  GLsizei width = mode->width;
  GLsizei height = mode->height;
  float bwidth = 80;
  float bheight = 96;
  float bpad = 8;


  flecs::entity arc_viz = ecs.entity()
  .is_a(ui_element)
  .add<ArcViz>()
  .set<Position, Local>({width/2-bwidth*2 - bwidth*2 - 16.0f, 128.0f});

  auto arc_task_debug = ecs.lookup("arc_task_debug");
  if (arc_task_debug)
  {
    arc_task_debug.children([&](flecs::entity child)
    {
      child.destruct();
    });
  }

  int c_y = 0;
  int y_steps = 0;
  std::vector<std::string> tracks = {"train", "test"}; 
  // TODO: For test track, center demos vertically and horizontally

  std::vector<ArcData> task_data;

  float y_arc_pos = 0.0f;
  for (std::string track : tracks)
  {
    int task_n = 0;
    // TODO: Add horizontal line
    if (track == "test")
    {
      flecs::entity test_train_divider = ecs.entity()
      .is_a(ui_element)
      .add<LevelHUD>()
      .set<Position, Local>({0.0f, y_arc_pos - 12.0f})
      .set<LineShape>({512, 2, 1})
      .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(255, 255, 255, 255)})
      .child_of(arc_viz);
    }
    loader->track_counts[track] = data[track].size();
    for (auto& demo : data[track])
    {
      auto e_in = ecs.entity();
      ArcData ad;
      ad.task_num = task_n;
      ad.io = IOType::INPUT;

      // ArcData ad_state;
      // ad.task_num = task_n;
      // ad.io = IOType::INTERMEDIATE;

      if (track == "test")
      {
        ad.pbe_type = PbeType::TEST;
        // ad_state.pbe_type = PbeType::TEST;
      }

      flecs::entity arc_task = ecs.entity((track + "_task_" + std::to_string(task_n)).c_str())
        .is_a(ui_element)
        .set<Position, Local>({0.0f, y_arc_pos}) // c_y * (16+2) + y_steps * 4
        .set<HorizontalLayoutBox>({0.0f, 16.0f})
        .child_of(arc_viz);
      loader->tasks.push_back(arc_task);

      flecs::entity arc_input = ecs.entity()
        .is_a(ui_element)
        .child_of(arc_task);

      int max_y = 0;
      int y = 0;
      for (auto& row : demo["input"])
      {
        std::vector<unsigned char> row_values;
        for (auto& v : row)
        {
          row_values.push_back(static_cast<unsigned char>(v));
        }
        ad.matrix.push_back(row_values);
        // ad_state.matrix.push_back(row_values);
        y++;
      }
      visualize_matrix(arc_input, ad);

      // TODO: 
      // visualize_matrix(arc_intermediate, ad_state);

      std::cout << "Create arc element with " << y << " rows and " << ad.matrix[0].size() << " cols" << std::endl;
      y_steps += 1;
      e_in.set<ArcData>(ad);

      flecs::entity arc_output = ecs.entity()
        .is_a(ui_element)
        .child_of(arc_task);

      auto e_out = ecs.entity();
      ArcData ad_out;
      ad_out.task_num = task_n;
      ad_out.io = IOType::OUTPUT;
      if (track == "test")
      {
        ad_out.pbe_type = PbeType::TEST;
      }
      
      max_y = y;

      y = 0;
      for (auto& row : demo["output"])
      {
        std::vector<unsigned char> row_values;
        for (auto& v : row)
        {
          row_values.push_back(static_cast<unsigned char>(v));
        }
        ad_out.matrix.push_back(row_values);
        y++;
      }
      visualize_matrix(arc_output, ad_out);

      e_out.set<ArcData>(ad_out);
      e_in.add<ArcOutput>(e_out);
      max_y = std::max(y, max_y);
      c_y += max_y;

      float base_size = 16.0f;
      if (ad.matrix.size() >= 16 || ad_out.matrix.size() >= 16)
      {
        base_size = 10.0f;
      }
      y_arc_pos = y_arc_pos + (float)((max_y+2) * (base_size+2));
      // task_data.push_back(e_in);
      task_data.push_back(ad);
      task_data.push_back(ad_out);
      
      task_n++;
    }
  }

  std::filesystem::path file_path(path);
  std::string arc_problem = file_path.stem().string();

  std::vector<ArcAbstraction> abstractions = {{IOType::INPUT, "spellbar"}, {IOType::OUTPUT, "cast_status"}};

  std::unordered_map<int, std::vector<ArcPattern>> input_grid_patterns;
  std::unordered_map<int, std::vector<ArcPattern>> output_grid_patterns;

  flecs::entity dataset_status = ecs.entity()
    .is_a(ui_element)
    .set<TextCreator>({"ARC-AGI Training", "ATARISTOCRAT"})
    .add<RenderStatus>()
    .add<LevelHUD>() 
    .child_of(arc_task_debug);

  flecs::entity task_num = ecs.entity()
    .is_a(ui_element)
    .set<TextCreator>({std::to_string(loader->active_index) + "/" + std::to_string(loader->task_ids.size()-1), "ATARISTOCRAT"})
    .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(255, 255, 0, 255)})
    .add<LevelHUD>() 
    .child_of(arc_task_debug);

  flecs::entity task_name = ecs.entity()
    .is_a(ui_element)
    .set<TextCreator>({arc_problem.c_str(), "ATARISTOCRAT"})
    .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(255, 0, 255, 255)})
    .add<LevelHUD>() 
    .child_of(arc_task_debug);

  // TODO: Visualize elementary pair causes
  // Show grid size
  // If the task input/output grid sizes are distinct, then show cause (ie multiple, subregion, fixed size, reduced/expanded, etc)

  loader->task_data = task_data;
  populate_spellcast_bar(loader);
  render_letters(loader);

  return arc_viz;
}

int get_active_cartridge()
{
  int selected = 0;
  auto q = ecs.query_builder<CartridgeBlock>()
              .build();
  q.run([&](flecs::iter& it) 
  {
    while (it.next())
    {
        for (auto i : it) {
        auto block = it.field<CartridgeBlock>(0);
        if (block[i].selected)
        {
          selected = i;
        }
      }
    }
  });
  return selected;
}

bool matchSortStageDir(const std::string& dir) {  
  std::regex pattern("sort_stage/\\d+/$");
    return std::regex_search(dir, pattern);
}

class UpdateListener : public efsw::FileWatchListener {
  public:
    void handleFileAction( efsw::WatchID watchid, const std::string& dir,
                           const std::string& filename, efsw::Action action,
                           std::string oldFilename ) override {
        switch ( action ) {
            case efsw::Actions::Add:
                std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Added"
                          << std::endl;
                break;
            case efsw::Actions::Delete:
                std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Delete"
                          << std::endl;
                break;
            case efsw::Actions::Modified:
                std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Modified"
                          << std::endl;
                if (dir == "../data/network/")
                {
                    fileWatcher->removeWatch( watchID );
                    std::fstream file("../data/network/create.txt");
                    if (file.is_open()) {
                        std::string line;
                        while (std::getline(file, line)) {
                            auto packet = ecs.entity();
                            packet.set<SendSimPacket>({line});
                        }
                        file.close();
                    }
                    watchID = fileWatcher->addWatch( "../data/network", this, true );
                }
                else if (dir == "../save/audio/")
                {
                  if (endsWith(filename, "json"))
                  {
                    fileWatcher->removeWatch( watchID );
                    std::cout << "Stream speech graph from audio file!" << std::endl;
                    flecs::entity speech_stream_loader = ecs.entity()
                      .set<LoadSpeechGraphEvent>({dir + filename});
                    watchID = fileWatcher->addWatch( "../save/audio/", this, true );
                  }
                }
                // Use AF_UNIX socket instead of this....
                // else if (matchSortStageDir(dir))
                // {
                //   fileWatcher->removeWatch( watchID );
                //   std::cout << "Sort iteration representation updated" << std::endl;
                //   auto slhf_interface = ecs.lookup("sort_learning_from_human_feedback");
                //   slhf_interface.set<ReloadCol>({4}); // This needs to be done from outer loop query or it will crash GraphSail :)
                //   neural_sort_learning_watcher->addWatch( "/home/heonae/arc/ec_pathfinder/neuralsort/pytorch/sort_stage", neural_sort_listener, true );
                // }
                break;
            case efsw::Actions::Moved:
                std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Moved from ("
                          << oldFilename << ")" << std::endl;
                break;
            default:
                std::cout << "Should never happen!" << std::endl;
        }
    }
};

enum BoxEdge
{
  TOP,
  RIGHT,
  BOTTOM,
  LEFT
};

flecs::entity wrap_line_around_box(size_t j, flecs::entity& parent, LineWrapBox& lwb, float start_pos, float& next_length, bool should_wrap, const flecs::entity& ui_element)
{
  NVGcolor color = lwb.colors[j%lwb.colors.size()];
  BoxEdge edge = TOP;
  float start_r = start_pos;
  // std::cout << "Line wrap has start pos " << start_r << std::endl;
  if (start_r < lwb.width)
  {
    edge = TOP;
  } else
  {
    start_r -= lwb.width;
    if (start_r < lwb.height)
    {
      edge = RIGHT;
    } else
    {
      start_r -= lwb.height;
      if (start_r < lwb.width)
      {
        edge = BOTTOM;
      } else
      {
        start_r -= lwb.width;
        edge = LEFT;
      }
    }
  }
  float length = lwb.length;
  if (should_wrap)
  {
    length = next_length;
  }
  if (edge == TOP)
  {
    float to_end = lwb.width-start_r;
    if (to_end < length)
    {
      next_length = length - to_end;
      length = to_end;
    }
    flecs::entity line = ecs.entity()
      .is_a(ui_element)
      .set<Position, Local>({start_r, 0.0f})
      .set<LineShape>({length, 0, 1})
      .set<RenderStatus>({true, RenderMode::FILL, color})
      .add<LevelHUD>()
      .child_of(parent);
    return line;
  }
  if (edge == RIGHT)
  {
    float to_end = lwb.height-start_r;
    if (to_end < length)
    {
      next_length = length - to_end;
      length = to_end;
    }
    flecs::entity line = ecs.entity()
      .is_a(ui_element)
      .set<Position, Local>({lwb.width, start_r})
      .set<LineShape>({0, length, 1})
      .set<RenderStatus>({true, RenderMode::FILL, color})
      .add<LevelHUD>()
      .child_of(parent);
    return line;
  }
  if (edge == BOTTOM)
  {
    float to_end = lwb.width-start_r;
    if (to_end < length)
    {
      next_length = length - to_end;
      length = to_end;
    }
    flecs::entity line = ecs.entity()
      .is_a(ui_element)
      .set<Position, Local>({lwb.width-start_r, lwb.height})
      .set<LineShape>({-length, 0, 1})
      .set<RenderStatus>({true, RenderMode::FILL, color})
      .add<LevelHUD>()
      .child_of(parent);
    return line;
  }
  float to_end = lwb.height-start_r;
  if (to_end < length)
  {
    next_length = length - to_end;
    length = to_end;
  }
  flecs::entity line = ecs.entity()
    .is_a(ui_element)
    .set<Position, Local>({0.0f, lwb.height-start_r})
    .set<LineShape>({0, -length, 1})
    .set<RenderStatus>({true, RenderMode::FILL, color})
    .add<LevelHUD>()
    .child_of(parent);
  return line;
}

void update_wrap_lines(Position* line_pos, LineShape* line_shape, LineWrapBox& lwb, float start_pos, float& next_length, bool should_wrap)
{
  BoxEdge edge = TOP;
  float start_r = start_pos;
  // std::cout << "Line wrap has start pos " << start_r << std::endl;
  if (start_r < lwb.width)
  {
    edge = TOP;
  } else
  {
    start_r -= lwb.width;
    edge = RIGHT;
    if (start_r < lwb.height)
    {

    } else
    {
      start_r -= lwb.height;
      edge = BOTTOM;
      if (start_r < lwb.width)
      {
      } else
      {
        start_r -= lwb.width;
        edge = LEFT;
      }
    }
  }
  float length = lwb.length;
  if (should_wrap)
  {
    length = next_length;
  }
  if (edge == TOP)
  {
    float to_end = lwb.width-start_r;
    if (to_end < length)
    {
      next_length = length - to_end;
      length = to_end;
    }
    line_pos->x = start_r;
    line_pos->y = 0.0f;
    line_shape->offset_x = length;
    line_shape->offset_y = 0.0f;
  }
  if (edge == RIGHT)
  {
    float to_end = lwb.height-start_r;
    if (to_end < length)
    {
      next_length = length - to_end;
      length = to_end;
    }
    line_pos->x = lwb.width;
    line_pos->y = start_r;
    line_shape->offset_x = 0.0f;
    line_shape->offset_y = length;
  }
  if (edge == BOTTOM)
  {
    float to_end = lwb.width-start_r;
    if (to_end < length)
    {
      next_length = length - to_end;
      length = to_end;
    }
    line_pos->x = lwb.width-start_r;
    line_pos->y = lwb.height;
    line_shape->offset_x = -length;
    line_shape->offset_y = 0.0f;
  }
  if (edge == LEFT)
  {
    float to_end = lwb.height-start_r;
    if (to_end < length)
    {
      next_length = length - to_end;
      length = to_end;
    }
    line_pos->x = 0.0f;
    line_pos->y = lwb.height-start_r;
    line_shape->offset_x = 0.0f;
    line_shape->offset_y = -length;
  }
}

// TODO: Integrate entity color persistence with book graph
int entities_created = 0;

void set_node_type(flecs::entity graph_root, std::string label_text, ECS_NodeType node_type, NVGcolor color, RenderMode emoji_mode = RenderMode::FILL)
{

  if (node_type == ECS_NodeType::ENTITY)
  {
    std::vector<NVGcolor> colors = get_palette("../assets/palettes/entity_mod.hex");
    color = colors[entities_created];
    entities_created++;
  }

  graph_root.set<NodeType>({node_type});
  auto ui_element = ecs.lookup("ui_element");
  std::cout << "Interaction confirmed! Create " << label_text << "!" << std::endl;
  auto first_node = graph_root.target<Next>();
  auto node_type_label = ecs.entity()
    .is_a(ui_element)
    .set<Position, Local>({4.0f, -13.0f})
    .set<TextCreator>({label_text, "ATARISTOCRAT"})
    .add<NodeTypeLabel>()
    .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0x6b, 0x7c, 0x99, 0xFF)})
    .child_of(first_node);
  auto node = first_node;
  node.lookup("emoji").set<RenderStatus>({true, emoji_mode, color});
  node.lookup("emoji").set<EdgeRenderStatus>({2.0f, color});
  node.lookup("label").set<RenderStatus>({true, RenderMode::FILL, color});
  while (node.has<Next>(flecs::Wildcard))
  {
    node.lookup("emoji").set<EdgeRenderStatus>({2.0f, color});
    node = node.target<Next>();
    node.lookup("emoji").set<RenderStatus>({true, emoji_mode, color});
    node.lookup("label").set<RenderStatus>({true, RenderMode::FILL, color});
  }

  if (node_type == ECS_NodeType::PYTHON_SCRIPT)
  {
    auto emoji = first_node.lookup("emoji");
    emoji.set<RenderStatus>({false, emoji_mode, color});
    auto python_emoji = ecs.entity("python_emoji")
      .is_a(ui_element)
      .set<SpriteCreator>({"../assets/python_emoji.png"})
      .set<Align>({0.0f, 0.0f, 0.5f, 0.5f})
      .child_of(emoji);
  }

  if (node_type == ECS_NodeType::SYMBOL_ANCHOR)
  {
    auto emoji = first_node.lookup("emoji");
    emoji.set<RenderStatus>({false, emoji_mode, nvgRGBA(0, 0, 0, 0)});
    auto anchor_emoji = ecs.entity("anchor_emoji")
      .is_a(ui_element)
      .set<RoundTriShape>({3.0f, 11.0f})
      .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 0, 0, 255)})
      .child_of(emoji);
  }

  if (node_type == ECS_NodeType::PREFAB || node_type == ECS_NodeType::COMPONENT)
  {
    std::cout << "Setup node graph menu" << std::endl;
    SetupNodeMenuSceneGraph menu;
    menu.telos = get_telos(graph_root);
    menu.node_type = node_type;
    auto setup_prefab = ecs.entity()
      .set<SetupNodeMenuSceneGraph>(menu);
  }
}

void *segment_function(void *ptr);

// https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    // TODO: Support punctuation delimiters
    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

void create_graph_node(float x, float y, flecs::entity* output, float emoji_size = 7.0f, NVGcolor emoji_color = nvgRGBA(255, 255, 255, 255), NVGcolor label_color = nvgRGBA(255, 255, 255, 255), bool add_emoji_circle = true)
{
  auto ui_element = ecs.lookup("ui_element");
  auto hud = ecs.lookup("hud");
  // TODO: Use enum pair
  bool level_hud_active = hud.has<LevelHUD>();
  bool annotator_hud_active = hud.has<AnnotatorHUD>();
  // auto GraphNode = ecs.lookup("GraphNodePrefab");
  // auto NodeEmoji = ecs.lookup("NodeEmojiPrefab");
  // auto NodeLabel = ecs.lookup("NodeLabelPrefab");
  // TODO: Use prefab child matching...
  auto node = ecs.entity()
    .is_a(ui_element)
    .set<Position, Local>({x, y})
    .set<Position, World>({x, y});
  output[0] = node;

  auto emoji = ecs.entity("emoji")
    .is_a(ui_element)
    .set<RenderStatus>({true, RenderMode::FILL, emoji_color})
    .set<Align>({0.5f, 0.0f, 0.0f, 0.0f})
    .child_of(node);
  output[1] = emoji;

  auto nodeli = ecs.entity("label")
    .is_a(ui_element)
    .set<Position, Local>({0.0f, -24.0f})
    .add<TextCreator>()
    .add<TextInput>()
    .set<RenderStatus>({true, RenderMode::FILL, label_color})
    .child_of(node);
  output[2] = nodeli;

  // TODO JANE: Node format templates
  if (add_emoji_circle)
  {
    emoji.set<CircleShape>({emoji_size}); // TODO JANE: More advanced emoji rendering technology (like an instanced mesh shader)
  } else
  {
    emoji.set<Align>({0.0f, 0.0f, 0.0f, 0.0f});
    nodeli.set<Position, Local>({0.0f, -4.0f});
  }

  for (size_t i = 0; i < 3; i++)
  {
    if (level_hud_active)
    {
      output[i].add<LevelHUD>();
    } 
    else if (annotator_hud_active)
    {
      output[i].add<AnnotatorHUD>();
    }
  }
  // node.target(NodeLabel) = nodeli;
}

int compare_layer(
    flecs::entity_t e1,
    const RenderOrder* ui1,
    flecs::entity_t e2,
    const RenderOrder* ui2)
{
    return ui1->z_index - ui2->z_index;
}

void spawn_graph(flecs::entity hud, flecs::entity ui_element, float x, float y, flecs::entity* graph_output, bool should_save = false, int save=0)
{
  flecs::entity output[3];
  create_graph_node(13.0f, 0.0f, output);
  output[0].set<GraphNodeData>({0});
  NVGcolor selector_color = nvgRGBA(0x6b, 0x7c, 0x99, 0xFF);
  NVGcolor telos_color = nvgRGBA(200, 55, 0, 128);

  // TODO: Use enum pair
  bool level_hud_active = hud.has<LevelHUD>();
  bool annotator_hud_active = hud.has<AnnotatorHUD>();

  printf("Create new sengra from click\n");
  flecs::entity graph_root;
  if (should_save)
  {
    graph_root = ecs.entity()
      .is_a(ui_element)
      .add<GraphData>()
      .set<SaveGraph>({(size_t)save})
      .set<Position, Local>({x, y});    
  } else
  {
    graph_root = ecs.entity()
      .is_a(ui_element)
      .add<GraphData>()
      .set<Position, Local>({x, y});
  }
  graph_output[0] = graph_root;
  printf("Load sengra\n");
  auto graph_selector = ecs.entity()
    .is_a(ui_element)
    .set<CursorDistanceVisible>({256.0f})
    .set<CircleShape>({11.0f})
    .set<RenderStatus>({true, RenderMode::STROKE, selector_color})
    .add<EventOnGrab, MoveGraph>()
    .child_of(graph_root);
  graph_output[1] = graph_selector;
    
  auto graph_telos_indicator = ecs.entity()
    .is_a(ui_element)
    .set<DiamondShape>({8.0f})
    .add<DynamicConnector>()
    .set<RenderStatus>({true, RenderMode::STROKE, telos_color})
    .add<LevelHUD>()
    .add<EventOnGrab, ConnectGraph>()
    .child_of(graph_selector);
    output[0].add<Head>(graph_root);
    graph_root.add<Next>(output[0]);
    output[0].child_of(graph_root); // TODO: Alt parent?
  graph_output[2] = graph_telos_indicator;

  if (level_hud_active)
  {
    graph_root.add<LevelHUD>();
    graph_selector.add<LevelHUD>();
  } 
  else if (annotator_hud_active)
  {
    graph_root.add<AnnotatorHUD>();
    graph_selector.add<AnnotatorHUD>();
  }

  for (size_t z = 0; z < 3; ++z)
  {
    graph_output[z+3] = output[z];
  }

  // TODO: Add graph to a graph_rail if it is hovered?
  auto q_rail = ecs.query_builder<GraphRail, const Position>()
      .term_at(0).second<World>()
      .build();
  q_rail.each([&graph_root, x, y](flecs::entity e, GraphRail& rail, const Position& pos)
  {
    // Determine which rail to add it too? If parent has ActiveGraph?
    // TODO: We may want the graph to be placed on graph_root so that the Y value is followed but not rail X...
    if (abs(pos.y - y) < 200.0f) // This is a hack that needs to be refactored and replaced when a better UI event propagation system is created
    {
      graph_root.child_of(e);
      // const Position* rail_selector_pos = graph_rail.parent().try_get<Position, World>();
      graph_root.set<Position, Local>({x - pos.x, y - pos.y});
    }
  });
}

// TODO: Load txt file
// TODO: Design audio graph analysis tooling (diatarization etc)

std::vector<SpokenWord> parse_speech_json(const std::string& filepath) {
    std::vector<SpokenWord> spokenWords;

    // Read the JSON file
    std::ifstream file(filepath);
    nlohmann::json j;
    file >> j;

    // Parsing logic
    for (const auto& item : j["results"]["channels"][0]["alternatives"][0]["words"]) {
        SpokenWord word;
        word.word = item["word"];
        word.start = item["start"];
        word.end = item["end"];
        spokenWords.push_back(word);
    }
    return spokenWords;
}

float interp(float a, float b, float p)
{
  return a * (1.0 - p) + b * p;
}

// Given screen coordinate, find nearest node on graph
flecs::entity find_closest_node(float x, flecs::entity graph_root)
{
  if (!graph_root.has<Next>(flecs::Wildcard))
  {
    return graph_root; 
  }
  flecs::entity closest = graph_root.target<Next>();
  flecs::entity last = closest;
  float min_distance = abs(closest.try_get<Position, World>()->x - x);
  float distance = min_distance;
  do
  {
    min_distance = distance;
    closest = last; 
    if (closest.has<Next>(flecs::Wildcard))
    {
      last = closest.target<Next>();
      distance = abs(x - last.try_get<Position, World>()->x);
    }
  } while (distance < min_distance);
  return closest;
}

std::string get_graph_selection_string(flecs::entity graph)
{
  std::string str;
  size_t min = std::numeric_limits<size_t>::max();
  size_t max = 0;
  flecs::entity min_node;

  graph.children([&](flecs::entity child)
  {
    if (child.has<NodeIsSelected>())
    {
      const GraphNodeData* node_data = child.try_get<GraphNodeData>();
      if (node_data->seq < min)
      {
        min_node = child;
        min = node_data->seq;
      }
      max = std::max(max, node_data->seq);
    }
  });

  if (min_node.is_valid())
  {
    flecs::entity node = min_node;
    std::cout << "Min " << min << " Max " << max << str << std::endl;
    for (size_t x = min; x <= max; x++)
    {
      node.children([&](flecs::entity child)
      {
        if (child.has<TextCreator>() && !child.has<NodeTypeLabel>())
        {
          str += child.try_get<TextCreator>()->word;
        }
      });
      if (node.has<Next>(flecs::Wildcard)) // should always be true unless tail...
      {
        flecs::entity next_node = node.target<Next>();
        node = next_node;
      }
    }
  }
  std::cout << "String to read is " << str << std::endl;
  return str;
}

  // auto graph_rail = ecs.entity()
  //   .is_a(ui_element_prefab)
  //   .add<LevelHUD>()
  //   .set<GraphRail>({200.0f})
  //   .set<SpeechStream>({0.0f, 0.0f, 0, 0, parse_speech_json(json)})
  //   .child_of(graph_root);

  // Position head_pos = {0.0f, 0.0f};
  // flecs::entity last_node;
  // flecs::entity last_text;
  // flecs::entity last_emoji;

  // SpeechStream* ss = graph_rail.get_mut<SpeechStream>();
  // ss->end_index = i;
  // if (speech[i].end > ss->start_time+ss->duration) break;

flecs::entity stream_speech_graph(float x, float y, std::vector<SpokenWord> speech, flecs::entity ui_element_prefab, NanoVG* vg, int start_index = 0)
{
  flecs::entity output[3];
  NVGcolor selector_color = nvgRGBA(255, 85, 0, 255);
  NVGcolor ss_color = nvgRGBA(255, 85, 0, 255);
  NVGcolor telos_color = nvgRGBA(255, 42, 0, 255);
  // TODO: The graph selector should definitely not be the root, because it needs to remain stationary during graph streams!
  auto graph_root = ecs.entity()
    .is_a(ui_element_prefab)
    .add<GraphData>()
    .set<Position, Local>({x, y})
    .add<LevelHUD>();
  auto graph_selector = ecs.entity()
    .is_a(ui_element_prefab)
    .set<CircleShape>({11.0f})
    .set<RenderStatus>({true, RenderMode::STROKE, selector_color})
    .add<LevelHUD>()
    .add<EventOnGrab, MoveGraph>()
    .child_of(graph_root);
  auto graph_telos_indicator = ecs.entity()
    .is_a(ui_element_prefab)
    .set<DiamondShape>({8.0f})
    .add<DynamicConnector>()
    .set<RenderStatus>({true, RenderMode::STROKE, telos_color})
    .add<LevelHUD>()
    .add<EventOnGrab, ConnectGraph>()
    .child_of(graph_selector);
  auto graph_rail = ecs.entity()
    .is_a(ui_element_prefab)
    .add<LevelHUD>()
    .set<GraphRail>({200.0f})
    .set<SpeechStream>({0.0f, 0.0f, 0, 0, speech})
    .child_of(graph_root);

  SpeechStream& ss = graph_rail.ensure<SpeechStream>();

  Position head_pos = {0.0f, 0.0f}; //graph_selector.try_get<Position, World>();
  flecs::entity last_node;
  flecs::entity last_text;
  flecs::entity last_emoji;
  for (size_t i = 0; i < speech.size(); i++)
  {
    // TODO: Detect text bounds overlaps and offset y
    // Time to x pos...
    float cx = (speech[i].start * 200.0f);
    Position pos = {cx, 0};
    create_graph_node(pos.x, pos.y, output);
    // output[0].set_mut<RenderStatus>{}
    if (last_node.is_valid())
    {
      output[0].add<Prev>(last_node);
      last_node.add<Next>(output[0]);
    }
    output[0].set<GraphNodeData>({start_index + i});
    output[0].set<SpokenWord>(speech[i]);
    
    flecs::entity mel_spectrogram = ecs.entity()
      .is_a(ui_element_prefab)
      .set<SpriteCreator>({"./../library/memory/speech/mel/" + std::to_string(i) + "_" + speech[i].word + ".png"})
      .set<Position, Local>({0.0f, 24.0f})
      .child_of(output[0]);

    output[2].set<TextCreator>({speech[i].word, "CharisSIL"});
    // output[2].set<Position, Local>({((speech[i].end - speech[i].start)/2.0f) * 200.0f, -24.0f}); // offset text
    output[2].set<Position, Local>({0.0f, -24.0f}); // offset text

    //output[0].add<EventOnGrab, SelectGraphNode>(); 
    output[1].add<EventOnGrab, SelectGraphNode>();
    output[1].set<EmojiSlideWord>({graph_root, graph_rail});
    output[1].remove<Align>();
    output[1].set<RenderStatus>({true, RenderMode::FILL, ss_color});
    // SPEECH

    auto speech_slider_background = ecs.entity()
      .is_a(ui_element_prefab)
      .set<CapsuleShape>({7.0f, 14.0f + (speech[i].end - speech[i].start) * 200.0f})
      .add<LevelHUD>()
      .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0, 0, 0, 255)})
      .add<SelectionStatic>()
      .child_of(output[0]);

    auto speech_slider = ecs.entity()
      .is_a(ui_element_prefab)
      .set<CapsuleShape>({7.0f, 14.0f + (speech[i].end - speech[i].start) * 200.0f})
      .add<LevelHUD>()
      .set<RenderStatus>({true, RenderMode::STROKE, ss_color})
      .child_of(output[0]);
    
    output[0].child_of(graph_rail);
    output[2].remove<TextInput>();
    if (i > 0)
    {
      last_emoji.add<Edge>(output[1]);
      last_emoji.set<EdgeRenderStatus>({2.0f, ss_color});
    }
    last_emoji = output[1];
    if (i == 0)
    {
      graph_root.add<Next>(output[0]);
      output[0].add<Head>(graph_root);
    }
    last_node = output[0];
    last_text = output[2];
    ss.end_index = i;
    if (speech[i].end > ss.start_time+ss.duration) break;
  }

  graph_root.add<GraphSpeak>();
  // TODO: Load from specific .json audio file...
  graph_root.set<GraphAudio>({"/home/wesxdz/paphos/maids/heonae/speech/pers.wav", nullptr});

  return graph_root;
}

flecs::entity load_speech_graph(float x, float y, std::vector<SpokenWord> speech, flecs::entity ui_element_prefab, NanoVG* vg, int start_index = 0)
{
  flecs::entity output[3];
  NVGcolor selector_color = nvgRGBA(255, 180, 0, 255);
  NVGcolor telos_color = nvgRGBA(255, 0, 0, 255);
  // TODO: The graph selector should definitely not be the root, because it needs to remain stationary during graph streams!
  auto graph_root = ecs.entity()
    .is_a(ui_element_prefab)
    .add<GraphData>()
    .set<Position, Local>({x, y})
    .add<LevelHUD>();
  auto graph_selector = ecs.entity()
    .is_a(ui_element_prefab)
    .set<CircleShape>({11.0f})
    .set<RenderStatus>({true, RenderMode::STROKE, selector_color})
    .add<LevelHUD>()
    .add<EventOnGrab, MoveGraph>()
    .child_of(graph_root);
  auto graph_telos_indicator = ecs.entity()
    .is_a(ui_element_prefab)
    .set<DiamondShape>({8.0f})
    .add<DynamicConnector>()
    .set<RenderStatus>({true, RenderMode::STROKE, telos_color})
    .add<LevelHUD>()
    .add<EventOnGrab, ConnectGraph>()
    .child_of(graph_selector);
  auto graph_rail = ecs.entity()
    .is_a(ui_element_prefab)
    .add<LevelHUD>()
    .set<GraphRail>({200.0f})
    .child_of(graph_root);

  Position head_pos = {0.0f, 0.0f}; //graph_selector.try_get<Position, World>();
  flecs::entity last_node;
  flecs::entity last_text;
  flecs::entity last_emoji;
  for (size_t i = 0; i < speech.size(); i++)
  {
    // TODO: Detect text bounds overlaps and offset y
    // Time to x pos...
    float cx = (speech[i].start * 200.0f);
    Position pos = {cx, 0};
    create_graph_node(pos.x, pos.y, output);
    output[1].set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(255, 0, 0, 255)});
    // output[0].set_mut<RenderStatus>{}
    if (last_node.is_valid())
    {
      output[0].add<Prev>(last_node);
      last_node.add<Next>(output[0]);
    }
    output[0].set<GraphNodeData>({start_index + i});
    output[0].set<SpokenWord>(speech[i]);
    
    flecs::entity mel_spectrogram = ecs.entity()
      .is_a(ui_element_prefab)
      .set<SpriteCreator>({"/home/heonae/graphlife/library/memory/speech/mel/" + std::to_string(i) + "_" + speech[i].word + ".png"})
      .set<Position, Local>({0.0f, 24.0f})
      .child_of(output[0]);

    output[2].set<TextCreator>({speech[i].word, "CharisSIL"});
    // output[2].set<Position, Local>({((speech[i].end - speech[i].start)/2.0f) * 200.0f, -24.0f}); // offset text
    output[2].set<Position, Local>({0.0f, -24.0f}); // offset text

    //output[0].add<EventOnGrab, SelectGraphNode>(); 
    output[1].add<EventOnGrab, SelectGraphNode>();
    output[1].set<EmojiSlideWord>({graph_root, graph_rail});
    output[1].remove<Align>();
    // SPEECH

    auto speech_slider_background = ecs.entity()
      .is_a(ui_element_prefab)
      .set<CapsuleShape>({7.0f, 14.0f + (speech[i].end - speech[i].start) * 200.0f})
      .add<LevelHUD>()
      .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0, 0, 0, 255)})
      .add<SelectionStatic>()
      .child_of(output[0]);

    auto speech_slider = ecs.entity()
      .is_a(ui_element_prefab)
      .set<CapsuleShape>({7.0f, 14.0f + (speech[i].end - speech[i].start) * 200.0f})
      .add<LevelHUD>()
      .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 0, 0, 255)})
      .child_of(output[0]);
    
    output[0].child_of(graph_rail);
    output[2].remove<TextInput>();
    if (i > 0)
    {
      last_emoji.add<Edge>(output[1]);
      last_emoji.add<EdgeRenderStatus>();
    }
    last_emoji = output[1];
    if (i == 0)
    {
      graph_root.add<Next>(output[0]);
      output[0].add<Head>(graph_root);
    }
    last_node = output[0];
    last_text = output[2];
  }

  graph_root.add<GraphSpeak>();
  graph_root.set<GraphAudio>({"/home/heonae/graphlife/library/memory/speech/dg_finis_merged_0.wav", nullptr});

  return graph_root;
}

flecs::entity load_graph(float x, float y, std::string string, flecs::entity ui_element_prefab, NanoVG* vg, std::string delimiter = " ", int start_index = 0)
{
  std::cout << "Load graph with string " << string << std::endl;
  std::vector<std::string> sengra_tokens = split(string, delimiter);
  // Tiktoken
  // MiniLM-L6-v2
  float cx = 13.0f;
  flecs::entity output[3];
  NVGcolor selector_color = nvgRGBA(0x6b, 0x7c, 0x99, 0xFF);
  NVGcolor telos_color = nvgRGBA(255, 0, 0, 255);
  auto graph_root = ecs.entity()
    .is_a(ui_element_prefab)
    .add<GraphData>()
    .set<SaveGraph>({get_active_cartridge()})
    .set<Position, Local>({x, y})
    .add<LevelHUD>();
  auto graph_selector = ecs.entity()
    .is_a(ui_element_prefab)
    .set<CircleShape>({11.0f})
    .set<CursorDistanceVisible>({256.0f})
    .set<RenderStatus>({true, RenderMode::STROKE, selector_color})
    .add<LevelHUD>()
    .add<EventOnGrab, MoveGraph>()
    .child_of(graph_root);
  auto graph_telos_indicator = ecs.entity()
    .is_a(ui_element_prefab)
    .set<DiamondShape>({8.0f})
    .add<DynamicConnector>()
    .set<RenderStatus>({true, RenderMode::STROKE, telos_color})
    .add<LevelHUD>()
    .add<EventOnGrab, ConnectGraph>()
    .child_of(graph_selector);

  Position head_pos = {0.0f, 0.0f};
  flecs::entity last_node;
  flecs::entity last_text;
  flecs::entity last_emoji;
  for (size_t i = 0; i < sengra_tokens.size(); i++)
  {
    Position pos = {cx, 0};
    create_graph_node(pos.x, pos.y, output);
    if (last_node.is_valid())
    {
      output[0].add<Prev>(last_node);
      last_node.add<Next>(output[0]);
    }
    output[0].set<GraphNodeData>({start_index + i});
    output[2].set<TextCreator>({sengra_tokens[i], "CharisSIL"});

    output[1].add<EventOnGrab, SelectGraphNode>();
    UIElementBounds& bounds = output[2].ensure<UIElementBounds>();
    
    nvgFontFace(vg->ctx, "CharisSIL");
    nvgTextBounds(vg->ctx, pos.x, pos.y, sengra_tokens[i].c_str(), NULL, &bounds.xmin);

    cx = bounds.xmax + 6 - head_pos.x;
    output[0].child_of(graph_root);
    output[2].remove<TextInput>();
    if (i > 0)
    {
      last_emoji.add<Edge>(output[1]);
      last_emoji.add<EdgeRenderStatus>();
    }
    last_emoji = output[1];
    if (i == 0)
    {
      graph_root.add<Next>(output[0]);
      output[0].add<Head>(graph_root);
    }
    last_node = output[0];
    last_text = output[2];
  }
  return graph_root;
}

// sengra :)
flecs::entity load_sengra(float x, float y, std::vector<std::vector<WordData>> sengra_tokens, flecs::entity ui_element_prefab, NanoVG* vg, int start_index = 0)
{
  // Tiktoken
  // MiniLM-L6-v2
  float cx = 13.0f;
  flecs::entity output[3];
  NVGcolor selector_color = nvgRGBA(255, 85, 0, 255);
  NVGcolor telos_color = nvgRGBA(255, 42, 0, 255);
  // TODO: The graph selector should definitely not be the root, because it needs to remain stationary during graph streams!
  auto graph_root = ecs.entity()
    .is_a(ui_element_prefab)
    .add<GraphData>()
    .set<Position, Local>({x, y})
    .add<LevelHUD>();
  printf("Load sengra\n");
  auto graph_selector = ecs.entity()
    .is_a(ui_element_prefab)
    .set<CircleShape>({11.0f})
    .set<RenderStatus>({true, RenderMode::STROKE, selector_color})
    .add<LevelHUD>()
    .add<EventOnGrab, MoveGraph>()
    .child_of(graph_root);
  auto graph_telos_indicator = ecs.entity()
    .is_a(ui_element_prefab)
    .set<DiamondShape>({8.0f})
    .add<DynamicConnector>()
    .set<RenderStatus>({true, RenderMode::STROKE, telos_color})
    .add<LevelHUD>()
    .add<EventOnGrab, ConnectGraph>()
    .child_of(graph_selector);
  // auto graph_rail = ecs.entity()
  //   .is_a(ui_element_prefab)
  //   .add<LevelHUD>()
  //   .set<GraphRail>({200.0f})
  //   .child_of(graph_root);

  Position head_pos = {0.0f, 0.0f}; //graph_selector.try_get<Position, World>();
  flecs::entity last_node;
  flecs::entity last_text;
  flecs::entity last_emoji;

  std::vector<NVGcolor> colors = get_palette("../assets/palettes/checker.hex", 64);

  size_t w_i = 0;
  int last_sentence = -1;
  flecs::entity sentence_entity;
  for (size_t line_num = 0; line_num < sengra_tokens.size(); ++line_num)
  {
    cx = 0;
    // if (indents.size() > 0)
    // {
    //   if (indents[line_num]) cx = 6*4;
    // }
    for (size_t i = 0; i < sengra_tokens[line_num].size(); i++)
    {
      WordData wd = sengra_tokens[line_num][i];
      int sentence = wd.sentence_index;
      if (sentence != last_sentence)
      {
        sentence_entity = ecs.entity()
          .set<SentenceSubgraph>({sentence})
          .add<SengraNodes>()
          .child_of(graph_root);
      }
      Position pos = {cx, line_num * 18}; 
      // create_graph_node(pos.x, pos.y, output, 0.0f, colors[i%colors.size()], nvgRGBA(0xc7, 0xdc, 0xd0, 255), false);
      create_graph_node(pos.x, pos.y, output, 0.0f, colors[i%colors.size()], nvgRGBA(0x5a, 0x69, 0x88, 255), false);
      output[2].add<SengraNode>(sentence_entity); // For now just add this directly to text...
      sentence_entity.ensure<SengraNodes>().subgraph.push_back(output[2]);
      // TOODO: should Next/Prev be on node root or text?
      // IT SHOULD DEFINITELY BE ON NODE ROOT
      // if (last_text.is_valid())
      // {
      //   output[2].add<Prev>(last_text);
      //   last_text.add<Next>(output[2]);
      // }
      if (last_node.is_valid())
      {
        output[0].add<Prev>(last_node);
        last_node.add<Next>(output[0]);
      }
      output[0].set<GraphNodeData>({start_index + w_i});
      output[2].set<TextCreator>({sengra_tokens[line_num][i].word, "CharisSIL"});

      output[0].add<EventOnGrab, SelectGraphNode>(); 
      // output[1].add<EventOnGrab, SelectGraphNode>();
      //output[2].add<EventOnGrab, SelectGraphNode>(); 

      // const Position* pos = output[0].try_get<Position, World>();
      UIElementBounds& bounds = output[2].ensure<UIElementBounds>();
      
      // TODO: Support sengra fonts
      nvgFontFace(vg->ctx, "CharisSIL");
      nvgTextBounds(vg->ctx, pos.x, pos.y, sengra_tokens[line_num][i].word.c_str(), NULL, &bounds.xmin);
      // The bounds does not have the appropriate size yet :(
        // TODO: Consider dynamic node spacer
      cx = bounds.xmax - head_pos.x;
      if (sengra_tokens[line_num][i].space_postfix)
      {
        cx += 6;
        // std::cout << sengra_tokens[line_num][i].c_str() << " has no space postfix" << std::endl;
      }
      output[0].child_of(graph_root);
      output[2].remove<TextInput>();
      if (i > 0)
      {
        last_emoji.add<Edge>(output[1]);
        auto edge_color = colors[i%colors.size()];
        edge_color.a = 0.0f;
        last_emoji.set<EdgeRenderStatus>({2.0f, edge_color, false});
      }
      last_emoji = output[1];
      if (i == 0 && line_num == 0)
      {
        graph_root.add<Next>(output[0]);
        output[0].add<Head>(graph_root);
      }
      // if (line_num > 0 && i == 0)
      // {

      // }
      last_node = output[0];
      last_text = output[2];
      last_sentence = sentence;
      w_i++;
    }
  }
  return graph_root;
}

void create_label(flecs::entity& parent, std::string text, flecs::entity& ui_element)
{
  flecs::entity e = ecs.entity()
    .is_a(ui_element)
    .set<TextCreator>({text, "ATARISTOCRAT"})
    .add<LevelHUD>()
    .child_of(parent);
}

std::string graph_to_string(flecs::entity root)
{
  // TODO: WIldcard case for 
  flecs::entity base = root;
  std::string str_graph;
  while (base.has<Next>(flecs::Wildcard))
  {
    base = base.target<Next>();
    // TextCreator
    base.children([&](flecs::entity child)
    {
      if (child.has<TextCreator>() && !child.has<NodeTypeLabel>())
      {
        std::string node_string = child.try_get<TextCreator>()->word;
        str_graph += node_string;
        if (node_string == "wildcard" || node_string == "Wildcard")
        {
          base.children([&](flecs::entity child_2)
          {
            if (child_2.has<CircleShape>())
            {
              child_2.children([&](flecs::entity emoji_child)
              {
                if (emoji_child.has<ArcColorSetMask>())
                {
                  const ArcColorSetMask* set_mask = emoji_child.try_get<ArcColorSetMask>();
                  for (size_t v = 0; v < 10; v++)
                  {
                    str_graph += set_mask->colors[v] ? "1" : "0";
                  }
                }
              });
            }
          });
        } else
        {
          std::cout << "found graph text string " << node_string << std::endl;

        }
        if (base.has<Next>(flecs::Wildcard))
        {
          str_graph += " ";
        }
      }
    });
  }
  return str_graph;
}


// TODO: This is pretty much irrelevant now given the graphreader format for sengra layout...
// flecs::entity load_sengra_from_file(float x, float y, std::string filepath, flecs::entity ui_element_prefab, NanoVG* vg)
// {
//   std::string text;
//   std::string line;
//   std::ifstream file(filepath);
//   if (file.is_open())
//   {
//     // TODO: Is getline slow, should ifstream pos move with seekg?
//     while (std::getline(file, line)) // std::count(text.begin(), text.end(), '\n') < 100
//     {
//       text += line + "\n"; // This line is probably the slow part...
//     }
//     file.close();
//   }
//   std::vector<std::string> lines_text;
//   lines_text.push_back(text);
//   flecs::entity graph = load_sengra(x, y, lines_text, ui_element_prefab, vg, "\n"); // TODO: Not sure what delimiter to use for tiktoken
//   graph.set<GraphData>({filepath});
//   return graph;
// }

// // TODO: Calculate len from text layout size...
// // TODO: Pass in std::ifstream file instead of filepath?
// flecs::entity stream_sengra_from_file(float x, float y, std::string filepath, int start, int len, flecs::entity ui_element_prefab, NanoVG* vg)
// {
//   std::string text;
//   std::string line;
//   std::ifstream file(filepath);
//   flecs::entity graph;
//   if (file.is_open())
//   {
//     int l = 0;
//     while (std::getline(file, line)) // l < start + len // std::count(text.begin(), text.end(), '\n') < 100
//     {
//       if (l >= start && l < start + len)
//       {
//         text += line + "\n"; // TODO: String builder...
//       }
//       l++;
//     }
//     // TODO: Load raw token bytes and convert to string in C++ for BPE tokenization
//     std::vector<std::string> lines_text;
//     lines_text.push_back(text);
//     graph = load_sengra(x, y, lines_text, ui_element_prefab, vg, "\n", start);
//     graph.set<GraphData>({filepath, start, len, l});
//     graph.set<GraphStream>({0.0f});
//     file.close();
//   }
//   return graph;
// }

void load_book_pages(BookData& book_data, std::string filepath, flecs::entity ui_element_prefab, NanoVG* vg)
{
  std::vector<WordData> page;
  std::string text;
  std::string word;

  std::ifstream file(filepath);
  flecs::entity graph;

  std::string si_str;
  std::ifstream token_sentence_indices("../library/memory/encode/jane_eyre.t2s");

  std::string sector_str;
  std::ifstream sector("../library/memory/encode/jane_eyre.sector");

  std::string sentence_str;
  std::string remaining_sentence_str;
  std::ifstream sentences_file("../library/memory/encode/jane_eyre.sentences");

  int line_height = 18;
  int max_lines = 860/line_height;
  int max_width = 550; // TODO: Grox inner padding layout 
  // TODO: Paragraph/headers layout
  // TODO: Hover sentence highlight (map token to sentences)
  
  if (file.is_open() && token_sentence_indices.is_open())
  {
    int lines = 0;
    int w_i;
    int seek_pos = 0;
    float current_line_width = 0;
    nvgFontFace(vg->ctx, "CharisSIL");
    nvgFontSize(vg->ctx, 16.0f);
    UIElementBounds bounds;
    std::vector<std::string> line_text;
    std::vector<bool> line_indent;
    int prev_section = 0;
    int prev_para = 0;
    int prev_sentence_index = -1;
    while (std::getline(file, word) && std::getline(token_sentence_indices, si_str))
    {
      size_t sentence_index = std::stoi(si_str);
      size_t section;
      size_t para;
      if (prev_sentence_index != sentence_index)
      {
        std::getline(sector, sector_str);
        std::getline(sentences_file, sentence_str);
        // remaining_sentence_str = sentence_str;
        std::istringstream iss(sector_str);
        iss >> section >> para;
        book_data.sentences.push_back({section, para});
      }
      // remaining_sentence_str = remaining_sentence_str.substr(word.size(), remaining_sentence_str.size());
      // std::cout << remaining_sentence_str << " sentence remaining " << std::endl;
      nvgTextBounds(vg->ctx, 0, 0, word.c_str(), NULL, &bounds.xmin);
      bool space_postfix = true;
      // if (!remaining_sentence_str.empty())
      // {
      //   space_postfix = remaining_sentence_str[0] == ' ';
      //   // if (book_data.pages.size() == 0 && !space_postfix)
      //   // {
      //   //   std::cout << word << " has no space postfix" << std::endl;
      //   // }
      // } else
      // {
      //   space_postfix = false;
      // }
      if (lines >= max_lines || section != prev_section)
      {
        book_data.pages.push_back(page);
        page.clear();
        lines = 0;
      }
      float word_width = bounds.xmax - bounds.xmin;
      current_line_width += word_width;
      if ((space_postfix && current_line_width > max_width) || ((w_i > 0) && para != prev_para))
      {
        line_text.push_back(text);
        text = word;
        current_line_width = word_width;
        lines++;
      } else
      {
        if (text.size() > 0)
        {
          text += " ";
        }
        text += word;
      }
      if (space_postfix)
      {
        // remaining_sentence_str = remaining_sentence_str.substr(1, remaining_sentence_str.size());
        // std::cout << remaining_sentence_str << " removed postfix space" << std::endl;
      }
      page.push_back({word, false, sentence_index});
      line_indent.push_back((w_i == 0) || para != prev_para || section != prev_section); // OR /n
      prev_section = section;
      prev_para = para;
      w_i++;
      seek_pos = file.tellg();
      prev_sentence_index = sentence_index;
    }
    file.close();
    token_sentence_indices.close();
    sector.close();
    sentences_file.close();
  }
  std:: cout << "Jane Eyre has " << book_data.pages.size() << " pages" << std::endl;
}

flecs::entity load_page(BookData& book_data, int page, flecs::entity ui_element_prefab, NanoVG* vg)
{
  std::string text;
  std::string word;
  flecs::entity graph;

  int line_height = 18;
  int max_lines = 860/line_height;
  int max_width = 550; // TODO: Grox inner padding layout 
  // TODO: Paragraph/headers layout
  // TODO: Hover sentence highlight (map token to sentences)

  // TODO: These values should be preparsed over the book... including with dialogue/user-created tokenization schemas
  
  int lines = 0;
  float current_line_width = 0;
  nvgFontFace(vg->ctx, "CharisSIL");
  nvgFontSize(vg->ctx, 16.0f);
  UIElementBounds bounds;
  std::vector<WordData> line_tokens;
  std::vector<std::vector<WordData>> sengra_tokens;
  std::vector<bool> line_indent;
  std::vector<bool> space_postfix;
  
  for (size_t w_i = 0; w_i < book_data.pages[page].size(); w_i++)
  {
    std::string word = book_data.pages[page][w_i].word;
    space_postfix.push_back(book_data.pages[page][w_i].space_postfix);
    nvgTextBounds(vg->ctx, 0, 0, (word + " ").c_str(), NULL, &bounds.xmin);
    float word_width = bounds.xmax - bounds.xmin;
    current_line_width += word_width;
    size_t paragraph = book_data.sentences[book_data.pages[page][w_i].sentence_index].paragraph;
    size_t last_paragraph = paragraph;
    if (w_i > 0)
    {
      last_paragraph = book_data.sentences[book_data.pages[page][w_i-1].sentence_index].paragraph;
    } 
    if ((current_line_width > max_width) || paragraph != last_paragraph)
    {
      sengra_tokens.push_back(line_tokens);
      line_tokens.clear();
      text = word;
      current_line_width = word_width;
      lines++;
    } else
    {
      // if (text.size() > 0)
      // {
      //   text += " ";
      // }
      text += word;
    }
    line_tokens.push_back(book_data.pages[page][w_i]);
  }
  graph = load_sengra(0, lines*line_height, sengra_tokens, ui_element_prefab, vg, 0);
  graph.add<BookGraph>();
  graph.children([&](flecs::entity child) 
  {
    if (child.has<flecs::pair<EventOnGrab, MoveGraph>>())
    {
      child.set<Position, Local>({-64.0f, -64.0f});
    }
  });
  return graph;
}

// TODO: Use mmap
std::vector<std::string> get_lines_at(std::string filepath, int n, int len)
{
  std::vector<std::string> lines;
  std::string line;
  std::ifstream file(filepath);
  flecs::entity graph;
  if (file.is_open())
  {
    int l = 0;
    while (std::getline(file, line)) // l < start + len // std::count(text.begin(), text.end(), '\n') < 100
    {
      if (l >= n + len)
      {
        return lines;
      }
      if (l >= n)
      {
        lines.push_back(line);
      }
      l++;
    }
  }
  return lines;
}

// Streams a sengra loaded from a file in a positive or negative direction
void stream_sengra_flow(flecs::entity graph, int token_count, flecs::entity ui_element_prefab, NanoVG* vg)
{
  // TODO: Determine which entities within the existing sengra to keep
  // Prepend or append new entities
  
  // For now, we're just going to reload everything...zzz
  GraphData& data = graph.ensure<GraphData>();
  int start = std::clamp(data.start + token_count, 0, data.max);
  int len = data.len;
  printf("Stream sengra START %i, LEN %i, FILE %s\n", start, len, data.filepath.c_str());
  // Position& pos = graph.ensure<Position, Local>();
  // TODO
  // Iterate children of graph root
  // Based on the number of tokens, find the text width
  // Shift the X of subsequent or prior nodes
  size_t i = 0;
  float x = 0;

  ecs.defer_begin();
  flecs::entity node_it = graph.target<Next>();
  flecs::entity updated_head;
  flecs::entity tail;
  bool has_next = true;
  while (node_it.is_valid() && has_next)
  {
    if (i < token_count)
    {
      node_it.destruct();
    } else
    {
      if (i == token_count)
      {
        graph.remove<Next>(flecs::Wildcard);
        updated_head = node_it;
        x = node_it.try_get<Position, Local>()->x - 13; // TODO: Const for root to head offset...
      }
      Position& node_pos = node_it.ensure<Position, Local>();
      node_it.set<Position, Local>({node_pos.x - x, node_pos.y});
    }
    if (node_it.has<Next>(flecs::Wildcard))
    {
      node_it = node_it.target<Next>();
    } else
    {
      tail = node_it;
      has_next = false;
    }
    i++;
  }
  ecs.defer_end();
  if (updated_head.is_valid())
  {
    graph.add<Next>(updated_head);
    updated_head.add<Head>(graph);
  }
  if (tail.is_valid())
  {
    flecs::entity output[3];
    auto lines = get_lines_at(data.filepath, data.start+1, token_count);
    const Position* tail_pos = tail.try_get<Position, Local>();
    const UIElementBounds* tail_bounds = tail.try_get<UIElementBounds>();
    float sengra_tail_x = tail_pos->x + (tail_bounds->xmax - tail_bounds->xmin); //- updated_head.pox.x
    flecs::entity last_node = tail;
    const GraphNodeData* tail_data = tail.try_get<GraphNodeData>();
    flecs::entity last_emoji;
    tail.children([&](flecs::entity child)
    {
      if (child.has<CircleShape>())
      {
        last_emoji = child;
      }
    });
    flecs::entity last_text;
    printf("Stream node count %i\n", lines.size());
    for (size_t l = 0; l < lines.size(); l++)
    {
      create_graph_node(sengra_tail_x, 0, output);
      if (last_node.is_valid())
      {
        output[0].add<Prev>(last_node);
        last_node.add<Next>(output[0]);
      }

      output[0].set<GraphNodeData>({tail_data->seq});
      output[2].set<TextCreator>({lines[l], "CharisSIL"});
      output[1].add<EventOnGrab, SelectGraphNode>();
      UIElementBounds& bounds = output[2].ensure<UIElementBounds>();
      printf("Line is %s\n", lines[l].c_str());
      nvgTextBounds(vg->ctx, sengra_tail_x, 0, lines[l].c_str(), NULL, &bounds.xmin);
      printf("Bounds width is %f\n", bounds.xmax - bounds.xmin);

      sengra_tail_x += bounds.xmax - bounds.xmin + 6;
      output[0].child_of(graph);
      output[2].remove<TextInput>();
      if (token_count > 0)
      {
        last_emoji.add<Edge>(output[1]);
        last_emoji.add<EdgeRenderStatus>();
      }
      last_emoji = output[1];
      last_node = output[0];
      last_text = output[2];
    }
    data.start += token_count;
  }
  // TODO: now... we need to append the new token nodes onto the last node

  // graph.children([&](flecs::entity node) 
  // {
  //   if (i < token_count) // TODO: Implement backward flow
  //   {
  //     // TODO: Refactor to use prefab slots
  //     node.children([&](flecs::entity child) 
  //     {
  //       if (child.has<TextCreator>())
  //       {
  //         const UIElementBounds* bounds = child.try_get<UIElementBounds>();   
  //         float width = bounds->xmax - bounds->xmin;
  //         x += width;
  //       }
  //     });
  //     node.destruct();
  //   }
  //   i++;
  // });
  // ecs.defer_end();
  // flecs::entity created_graph = stream_sengra_from_file(pos->x, pos->y, data->filepath, start, len, ui_element_prefab, vg);
  // created_graph.child_of(graph.parent());
  // graph.destruct();
}

// using Query_SelectedNodes = flecs::query<GraphNodeData, NodeIsSelected, GraphData>;
using UI_Query = flecs::query<UIElementBounds>;
using SceneGraph_Query = flecs::query<SceneGraphRoot, Position>;

struct DualQuery
{
  UI_Query ui;
  flecs::query<GraphIsGrabbed> move;
};

flecs::entity create_bode(std::string name, std::string text, NVGcolor color, flecs::entity ui_element, flecs::entity* created, float acceleration, float delay, int layer = 1000)
{
  flecs::entity create_menu = ecs.entity(name.c_str())
  .set<Position, Local>({0, 0.0f})
  .add<BookmarkResponder>()
  .add<LevelHUD>()
  .is_a(ui_element);

  flecs::entity bkg = ecs.entity("bkg") // "bkg"
    .is_a(ui_element)
    .add<LevelHUD>()
    .child_of(create_menu);
  created[0] = bkg;

  // flecs::entity ux_bode_bkg = ecs.entity("ux_bode_bkg")
  //   .is_a(ui_element)
  //   .set<Bode>({128.0f, 40.0f, 17.0f, 12.0f})
  //   .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(25, 60, 62, 255)})
  //   .child_of(create_menu);

  flecs::entity ux_bode_outline = ecs.entity("ux_bode_outline") // "ux_bode_outline"
    .is_a(ui_element)
    .add<LevelHUD>()
    .set<RenderOrder>({layer})
    .set<Bode>({128.0f, 40.0f, 4.0f, 12.0f, 0.5f})
    // .set<RainbowPlane>({acceleration, delay, 40.0f})
    .set<RenderStatus>({true, RenderMode::STROKE, color})
    .child_of(create_menu);
  created[1] = ux_bode_outline;

  flecs::entity ux_text_shadow = ecs.entity("ux_text_shadow") // "ux_text_shadow"
    .is_a(ui_element)
    .add<LevelHUD>()
    .set<Position, Local>({32.0f/2.0f + 6.0f, 26.0f})
    .set<TextCreator>({text.c_str()})
    .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0, 0, 0, 255)})
    .set<FontStatus>({3.0f, 16.0f})
    .child_of(create_menu);
  created[2] = ux_text_shadow;

  flecs::entity ux_text = ecs.entity("ux_text") // "ux_text"
    .is_a(ui_element)
    .add<LevelHUD>()
    .set<Position, Local>({32.0f/2.0f + 6.0f, 26.0f})
    .set<TextCreator>({text.c_str()})
    .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(255, 255, 255, 255)})
    .child_of(create_menu);
  created[3] = ux_text;

  // flecs::entity ux_node = ecs.entity() // "ux_node"
  //   .is_a(ui_element)
  //   .add<LevelHUD>()
  //   .set<Position, Local>({128.0f/2.0f, 40.0f})
  //   .set<CircleShape>({7.0f})
  //   .set<RenderStatus>({true, RenderMode::FILL, color})
  //   .child_of(create_menu);
  //   created[4] = ux_node;

  flecs::entity ux_node = ecs.entity("ux_node") // "ux_node"
    .is_a(ui_element)
    .add<LevelHUD>()
    .set<Position, Local>({128.0f/2.0f, 40.0f})
    .set<DiamondShape>({8.0f})
    .set<RenderStatus>({true, RenderMode::STROKE, color})
    .add<EventOnGrab, ConnectGraph>()
    .child_of(create_menu);
    created[4] = ux_node;

  // flecs::entity rainbow = ecs.entity()
  //   .is_a(ui_element)
  //   .set<Position, Local>({1.0f, 0.0f})
  //   .set<RainbowPlane>({acceleration, delay, 0.0f})
  //   .set<BoxShape>({126.0f, 0.0f})
  //   .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(color.rgba[0]*255, color.rgba[1]*255, color.rgba[2]*255, 255)}) // nvgRGBA(color.r, color.g, color.b, 128)
  //   .child_of(create_menu);

  return create_menu;
}

flecs::entity gen_forebode_graph(float x, float y, flecs::entity ui_element)
{
  auto forebode = ecs.entity("forebode")
    .is_a(ui_element)
    .set<Position, Local>({x, y});
  // TODO: Dynamic actions
  flecs::entity bode[5];
  auto book_bode = create_bode("load_book", "Book", nvgRGBA(255, 85, 0, 255), ui_element, &bode[0], 1000.0f, 0.0f);
  book_bode.set<Position, Local>({-128, -96-48});
  book_bode.child_of(forebode);

  flecs::entity output[3];
  create_graph_node(13.0f, 0.0f, output);

  output[2].remove<TextInput>();
  output[2].set<TextCreator>({"load"});
  output[1].set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(255, 85, 0, 255)});
  output[1].add<Edge>(bode[4]); // ux_node slot
  output[1].set<EdgeRenderStatus>({2.0f, nvgRGBA(255, 85, 0, 255)});
  output[0].child_of(bode[4]);
  output[0].set<Position, Local>({32.0f, 48.0f});

  // create_graph_node(13.0f, 0.0f, output);
  // output[2].remove<TextInput>();
  // output[2].set<TextCreator>({"search"});
  // output[1].set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(255, 85, 0, 255)});
  // output[1].add<Edge>(bode[4]); // ux_node slot
  // output[1].set<EdgeRenderStatus>({2.0f, nvgRGBA(255, 85, 0, 255)});
  // output[0].child_of(bode[4]);
  // output[0].set<Position, Local>({32.0f-112.0f, 48.0f});

  auto note_bode = create_bode("idea", "Idea", nvgRGBA(0, 72, 255, 255), ui_element, &bode[0], 1000.0f, 0.0f);
  note_bode.set<Position, Local>({-128, -32});
  note_bode.child_of(forebode);

  create_graph_node(13.0f, 0.0f, output);
  output[2].remove<TextInput>();
  output[2].set<TextCreator>({"think"});
  output[1].set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0, 72, 255, 255)});
  output[1].add<Edge>(bode[4]); // ux_node slot
  output[1].set<EdgeRenderStatus>({2.0f, nvgRGBA(0, 72, 255, 255)});
  output[0].child_of(bode[4]);
  output[0].set<Position, Local>({32.0f, 48.0f});

  auto last_node = output[1];

  create_graph_node(13.0f, 0.0f, output);
  output[2].remove<TextInput>();
  output[2].set<TextCreator>({""});
  output[1].set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(255, 0, 0, 255)});
  output[1].add<Edge>(last_node); // ux_node slot
  output[1].set<EdgeRenderStatus>({2.0f, nvgRGBA(255, 0, 0, 128)});
  output[0].child_of(bode[4]);
  // TODO: The forebode graph should be offset based on the text width of the selected action 
  output[0].set<Position, Local>({32.0f+48.0f, 48.0f});

  // TODO: Modify the graph selector to be the color of the selection action

  return forebode;
}

void remove_forebode_graph()
{
  auto forebode = ecs.lookup("forebode");
  if (forebode.is_valid() && forebode.is_alive())
  {
    forebode.destruct();
  }
}

void setAlwaysOnTop(Display* display, Window window) {
    Atom atom = XInternAtom(display, "_NET_WM_STATE", False);
    Atom value = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);

    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.window = window;
    e.xclient.message_type = atom;
    e.xclient.format = 32;
    e.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
    e.xclient.data.l[1] = value;
    e.xclient.data.l[2] = 0; // no second property to change
    e.xclient.data.l[3] = 0;
    e.xclient.data.l[4] = 0;

    XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &e);
}

void set_hud_scene(flecs::entity& entity)
{
  auto hud = ecs.lookup("hud");
  // TODO: Use enum pair
  bool level_hud_active = hud.has<LevelHUD>();
  bool annotator_hud_active = hud.has<AnnotatorHUD>();
  if (level_hud_active)
  {
    entity.add<LevelHUD>();
  } else if (annotator_hud_active)
  {
    entity.add<AnnotatorHUD>();
  }
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

flecs::entity create_scene_graph(std::string name, flecs::entity scene_graph_node_prefab, flecs::entity ui_element_prefab, bool show_scope_arrow = false)
{
  // TODO: Generate scene graphs from flecs entity rather than manually named...
  // std::cout << "Create sg with name " << (name + std::string("_scene_graph")).c_str() << std::endl;
  flecs::entity sgn = ecs.entity() // (name + std::string("_scene_graph")).c_str()
    .is_a(scene_graph_node_prefab);
  auto sgt = sgn.lookup("sg_viz::sg_text");
  sgt.set<TextCreator>({name, "ATARISTOCRAT"});

  if (show_scope_arrow)
  {
    flecs::entity scope_arrow = ecs.entity("scope_arrow")
      .is_a(ui_element_prefab)
      .set<SpriteCreator>({"../assets/arrow_down.png"})
      .child_of(sgn.lookup("sg_viz"));
  }

  return sgn;
}

flecs::entity parse_scene_graph(flecs::entity root, flecs::entity scene_graph_selection_indicator_prefab, flecs::entity scene_graph_node_prefab, flecs::entity ui_element_prefab, std::string default_selection)
{
  // flecs::entity scene_graph_root = ecs.entity()
  //   .is_a(ui_element_prefab)
  //   .add<SceneGraphRoot>();
  std::vector<flecs::entity> scene_graph_nodes;
  // std::unordered_map<flecs::entity, size_t> node_depths;
  std::unordered_map<size_t, size_t> node_to_sg_node;
  std::unordered_map<size_t, size_t> node_to_child_index;
  std::unordered_map<size_t, size_t> node_depth;
  node_to_child_index[root] = 0;
  node_depth[root] = 0;
  // node_depths[root] = 0;
  std::vector<flecs::entity> stack{root}; // root nodes to parse

  printf("Start stacking\n");
  size_t sg_index;
  while (!stack.empty())
  {
    size_t child_count = 0;
    flecs::entity node = stack.back();
    stack.pop_back();
    std::vector<flecs::entity> children;
    node.children([&](flecs::entity child)
    {
      children.push_back(child);
      node_to_child_index[child] = child_count;
      child_count++;
      node_depth[child] = node_depth[node]+1;
    });
    std::reverse(children.begin(),children.end());
    // for (size_t i = children.size()-1; i >=0; --i)
    // {
    //   stack.insert(stack.begin(), children[i], 0);
    // }
    // for (size_t z = children.size()-1; z >=0; --z)
    // {
    //   stack.insert(stack.begin(), children[z]);
    // }
    stack.insert(stack.begin(), children.begin(), children.end());

    std::string name = node.try_get<SG>()->name;
    flecs::entity sg;
    sg = create_scene_graph(name, scene_graph_node_prefab, ui_element_prefab, child_count > 0);
    // TODO: Figure out a better way to transfer plecs configs
    // ex prob just add SceneGraph to existing nodes...
    if (node.has<CreatePrefabFromGraph>())
    {
      sg.add<EventOnSelect, CreatePrefabFromGraph>();
    } else if (node.has<CreateSlotFromGraph>())
    {
      sg.add<EventOnSelect, CreateSlotFromGraph>();
    } else if (node.has<CreateEntityFromGraph>())
    {
      sg.add<EventOnSelect, CreateEntityFromGraph>();
    } else if (node.has<InstantiateComponent>())
    {
      sg.add<EventOnSelect, InstantiateComponent>();
    }
    else if (node.has<CreateComponentFromGraph>())
    {
      sg.add<EventOnSelect, CreateComponentFromGraph>();
    }
    else if (node.has<DeployCompanionFromGraph>())
    {
      sg.add<EventOnSelect, DeployCompanionFromGraph>();
    } else if (node.has<RunAutonomousLinuxDesktop>())
    {
      sg.add<EventOnSelect, RunAutonomousLinuxDesktop>();
    }
    else if (node.has<AddIntFromGraph>())
    {
      sg.add<EventOnSelect, AddIntFromGraph>();
    }
    else if (node.has<AddFloatFromGraph>())
    {
      sg.add<EventOnSelect, AddFloatFromGraph>();
    }
    else if (node.has<AddStringFromGraph>())
    {
      sg.add<EventOnSelect, AddStringFromGraph>();
    }
    else if (node.has<LoadTensorFromFile>())
    {
      sg.add<EventOnSelect, LoadTensorFromFile>();
    }
    else if (node.has<CreateModelFromDatasetGraph>())
    {
      sg.add<EventOnSelect, CreateModelFromDatasetGraph>();
    }
    else if (node.has<CreateDatasetFromGraph>())
    {
      sg.add<EventOnSelect, CreateDatasetFromGraph>();
    } else if (node.has<InterfacePythonServerFunction>())
    {
      sg.add<EventOnSelect, InterfacePythonServerFunction>();
    } else if (node.has<AnchorSymbol>())
    {
      sg.add<EventOnSelect, AnchorSymbol>();
    }
    if (node.has<IntentExpression>())
    {
      std::cout << "Add intent expression!" << std::endl;
      sg.add<IntentExpression>();
    }
    SceneGraph& sg_data = sg.ensure<SceneGraph>();
    sg_data.child_index = node_to_child_index[node];
    sg_data.children_count = child_count;
    sg_data.depth = node_depth[node];
    sg_data.index = sg_index;

    printf("Set scene graph data\n");

    // TODO: Should the scene graph UI itself have a hierarchy? (NO)
    node_to_sg_node[node] = sg;
    flecs::entity node_parent = node.parent();
    if (node_parent.is_valid())
    {
      sg.child_of(node_to_sg_node[node_parent]);
    }
    // sg.child_of(scene_graph_root);

    if (name == default_selection)
    {
      flecs::entity selector = ecs.entity()
        .is_a(scene_graph_selection_indicator_prefab)
        .set<SceneGraphSelection>({false, sg, sg, scene_graph_nodes[0]});
      scene_graph_nodes[0].set<SceneGraphRoot>({selector});
    }
    scene_graph_nodes.push_back(sg);
    sg_index++;
  }
  printf("Create SG nodes\n");

  for (size_t i = 0; i < scene_graph_nodes.size() - 1; ++i)
  {
    scene_graph_nodes[i].add<Next>(scene_graph_nodes[i+1]);
  }
  for (size_t i = 1; i < scene_graph_nodes.size(); ++i)
  {
    scene_graph_nodes[i].add<Prev>(scene_graph_nodes[i-1]);
  }

  printf("Added SG relationships\n");

  int i = 0;
  for (auto sg : scene_graph_nodes)
  {
    sg.set<Position, Local>({0, (sg.try_get<SceneGraph>()->child_index+1) * 14});
    i++;
  }
  
  printf("Add location positions\n");

  // scene_graph_root.set<Position, Local>({256, 256});
  scene_graph_nodes[0].set<Position, Local>({-1000.0f, -1000.0f});
  printf("Add local and SGR\n");
  return scene_graph_nodes[0];
}

// flecs::entity load_book(const std::string& book_name, flecs::entity ui_element, flecs::entity renderer) {
//     // Create a book entity as a UI element with a LevelHUD component
//     // ecs.defer_begin();
//     flecs::entity book = ecs.entity()
//         .is_a(ui_element)
//         .set<Position, Local>({0.0f, 256.0f})
//         .add<LevelHUD>();

//     // Set the sprite for the book cover
//     std::string cover_path = "../library/memory/cover/" + book_name + "_cover.png";
//     book.set<SpriteCreator>({cover_path, 1, 1, 512});

//     // Get the cover width from the Sprite component
//     // int cover_width = book.try_get<Sprite>()->w;

//     // Stream the book content and attach it as a child to the book entity
//     std::string encode_path = "../library/memory/encode/" + book_name + ".token";
//     // auto sengra = load_sengra(512 + 32, 264, "Fablehaven!", ui_element, renderer.get_mut<NanoVG>());
//     // sengra.child_of(book);
//     // ecs.defer_end();
//     stream_sengra_from_file(512, 256, encode_path, 0, 100, ui_element, renderer.get_mut<NanoVG>())
//     .child_of(book);
//     // .set<Position, Relative>({100.0f, 300.0f});

//     return book;
// }


/* program entry */
int main(int argc, char *argv[])
{

    srand(time(NULL));
    std::unordered_map<ECS_NodeMetaType, NVGcolor> meta_syntax_theme;

    meta_syntax_theme[META] = nvgRGBA(255, 255, 255, 255);
    meta_syntax_theme[GRAPH] = nvgRGBA(32, 210, 19, 255);
    meta_syntax_theme[ECS] = nvgRGBA(0, 255, 242, 255);
    meta_syntax_theme[OS] = nvgRGBA(255, 0, 255, 255);

    std::unordered_map<ECS_NodeType, std::string> node_type_labels;
    node_type_labels[ECS_NodeType::PREFAB] = "prefab";
    node_type_labels[ECS_NodeType::SLOT] = "slot";
    node_type_labels[ECS_NodeType::ENTITY] = "entity";
    node_type_labels[ECS_NodeType::COMPANION] = "companion";
    node_type_labels[ECS_NodeType::LINUX] = "linux";
    node_type_labels[ECS_NodeType::DATASET] = "dataset";
    node_type_labels[ECS_NodeType::MODEL] = "model";
    node_type_labels[ECS_NodeType::COMPONENT] = "component";
    node_type_labels[ECS_NodeType::PYTHON_SCRIPT] = "python";
    node_type_labels[ECS_NodeType::SYMBOL_ANCHOR] = "symbol";

    for (std::string preposition : relationships)
    {
      graph_emoji_keywords[preposition] = "../assets/measuring_rod.png";
    }
    graph_emoji_keywords["within"] = "../assets/visual_hierarchy.png";
    graph_emoji_keywords["outside"] = "../assets/visual_hierarchy.png";
    graph_emoji_keywords["hole"] = "../assets/negative_space.png";
    graph_emoji_keywords["bound"] = "../assets/negative_space.png";
    graph_emoji_keywords["boundingbox"] = "../assets/negative_space.png";

    // tsl::htrie_map<char, int> sengra_trie_search = {{"cropped data", 0}, {"crows", 1}, {"pattern", 2}};
    // auto prefix_range = sengra_trie_search.equal_prefix_range("cr");
    // for(auto it = prefix_range.first; it != prefix_range.second; ++it) {
    //     std::cout << "{" << it.key() << ", " << *it << "}" << std::endl;
    // }

    std::unordered_map<ECS_NodeType, NVGcolor> syntax_theme;
    // Starter dark literacy color theme
    syntax_theme[STANDARD] = meta_syntax_theme[META];
    for (size_t i = DATASET; i < PROJECTION; i++)
    {
      syntax_theme[(ECS_NodeType)i] = meta_syntax_theme[GRAPH];
    }
    for (size_t i = PREFAB; i < MODULE; i++)
    {
      syntax_theme[(ECS_NodeType)i] = meta_syntax_theme[ECS];
    }
    for (size_t i = COMPANION; i < SYMBOL_ANCHOR; i++)
    {
      syntax_theme[(ECS_NodeType)i] = meta_syntax_theme[OS];
    }

    syntax_theme[ECS_NodeType::PYTHON_SCRIPT] = nvgRGBA(0, 140, 255, 255);
    syntax_theme[ECS_NodeType::SYMBOL_ANCHOR] = nvgRGBA(0, 140, 255, 255);
  
    fileWatcher = new efsw::FileWatcher();
    UpdateListener* listener = new UpdateListener();
    watchID = fileWatcher->addWatch( "../data/network", listener, true );
    fileWatcher->watch();

    efsw::FileWatcher* vision_screenshot_watcher = new efsw::FileWatcher();
    UpdateListener* vision_listener = new UpdateListener();
    watchID = vision_screenshot_watcher->addWatch( ".", vision_listener, true );
    vision_screenshot_watcher->watch();

    efsw::FileWatcher* audio_read_watcher = new efsw::FileWatcher();
    UpdateListener* audio_listener = new UpdateListener();
    audio_read_watcher->addWatch( "../save/audio", audio_listener, true );
    audio_read_watcher->watch();

    efsw::FileWatcher* neural_sort_learning_watcher = new efsw::FileWatcher();
    UpdateListener* neural_sort_listener = new UpdateListener();
    neural_sort_learning_watcher->addWatch( "/home/heonae/arc/ec_pathfinder/neuralsort/pytorch/sort_stage", neural_sort_listener, true );
    neural_sort_learning_watcher->watch();

    Display *display = XOpenDisplay(NULL);
    Window root = RootWindow(display, DefaultScreen(display));
    // setAlwaysOnTop(display, root);
    // xdo_t* xdo = xdo_new(NULL);
    // XSelectInput(display, root, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);
    // XCloseDisplay(display);
    // Get X11 root
    // GLFWwindow* share = nullptr;
    // GLFWwindow* glfw_root = glfwAttachWindow(root, share);
    GLFWwindow* window;
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }
    glfwSetErrorCallback(error_callback);

    // glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // Enables transparency for the framebuffer
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE); // Makes the window always-on-top
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Optional: Removes window decorations (title bar, borders)
    glfwWindowHint(GLFW_ALPHA_BITS, 8); // Ensure an alpha channel is created for transparency

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    // Consider a direct X11 call instead for fullscreen...
    // XRaiseWindow(display, w) 
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
    // glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    GLsizei width = mode->width;
    GLsizei height = mode->height;
    // window = glfwCreateWindow( width, height, "GraphSail", NULL, NULL ); //glfwGetPrimaryMonitor()
    window = glfwCreateWindow( width, height, "GraphSail", glfwGetPrimaryMonitor(), NULL );
    if (!window)
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }
    system("wmctrl -r \"GraphSail\" -b add,above");

    auto hud = ecs.entity("hud");
    Overlay sail = {width, height, window, glfwGetX11Window(window), display, root, false};
    hud.set<Overlay>(sail);
    hud.set<UIElementBounds>({0, 0, width, height});
    hud.add<SceneGraphGraphHUD>();
    hud.add<VirtRigControls>();

    //flecs::entity speech_stream_loader = ecs.entity()
                      // .set<LoadSpeechGraphEvent>({"../save/audio/pers.json"});

    // TODO: When a node is selected, we need to determine if a contiguous sengra to an already selected node should be selected
    // TODO: Evaluate tradeoffs between entity tagging vs centralized approach
    // TODO: Consider prefab swapping (esp child framing defaults)

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(0);

    auto vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    flecs::entity transformable = ecs.prefab("transformable")
      .add<Position, World>()
      .add<Position, Local>();

    flecs::entity ui_element = ecs.prefab("ui_element")
      .is_a(transformable)
      .add<Position, Relative>()
      .add<UIElementBounds>()
      .add<RenderStatus>()
      .add<RenderOrder>();

    // TODO: Support multiple
    flecs::entity scene_graph_selection_indicator = ecs.prefab("scene_graph_selection_indicator")
      .is_a(ui_element)
      .set<BoxShape>({64, 12, 1})
      .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(0, 255, 0, 255)});
      
    flecs::entity scene_graph_node = ecs.prefab("sg_node")
      .is_a(ui_element)
      .add<SceneGraph>();

    flecs::entity scene_graph_viz = ecs.prefab("sg_viz")
      .is_a(ui_element)
      .add<HorizontalLayoutBox>()
      .child_of(scene_graph_node);

    flecs::entity scene_graph_text = ecs.prefab("sg_text")
      .is_a(ui_element)
      .set<TextCreator>({"default_node", "ATARISTOCRAT"})
      .set<Position, Local>({0.0f, 6.0f})
      .child_of(scene_graph_viz);

    flecs::entity component_layout = ecs.prefab("component_layout")
      .is_a(ui_element)
      .add<VerticalLayoutBox>();

    flecs::entity component_field = ecs.prefab("component_field")
      .is_a(ui_element)
      .add<HorizontalLayoutBox>();

    // flecs::entity data_primitive_symbol = ecs.prefab("symbol")
    //   .is_a(ui_element)
    //   .add<SpriteCreator>()
    //   .override<SpriteCreator>()
    //   .child_of(component_field);

    // flecs::entity variable_name = ecs.prefab("variable")
    //   .is_a(ui_element)
    //   .add<TextCreator>()
    //   .override<TextCreator>()
    //   .child_of(component_field);

    // flecs::entity GraphNodePrefab = ecs.prefab("GraphNodePrefab")
    //   .is_a(ui_element);
    
    // flecs::entity NodeEmoji = ecs.prefab("NodeEmojiPrefab")
    //   .is_a(ui_element)
    //   .child_of(GraphNodePrefab)
    //   .slot_of(GraphNodePrefab);

    // flecs::entity NodeLabelPrefab = ecs.prefab("NodeLabelPrefab")
    //   .child_of(GraphNodePrefab)
    //   .slot_of(GraphNodePrefab);

    auto renderer = ecs.entity("renderer");
    renderer.set<NanoVG>({vg});
    renderer.add<UIDebugRender>();

    auto user_input = ecs.entity("user_input");
    user_input.add<KeyboardState>();
    user_input.add<CursorState>();
    user_input.add<GraphEditor>();

    auto graph_RTS = ecs.entity("graph_RTS")
      .is_a(ui_element)
      .set<BoxShape>({0, 0, 1.0f})
      .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(32, 210, 19, 255)})
      .set<DragBoundGraphSelector>({false});

    auto chisel = ecs.entity("chisel")
      .is_a(ui_element)
      .add<HorizontalBoundaryChisel>();
    // Snap to graph
    auto anchor_bound = ecs.entity()
      .is_a(ui_element)
      .set<FollowChiselAnchor>({chisel})
      .set<LineShape>({0, 64})
      .set<RenderStatus>({false, RenderMode::FILL, nvgRGBA(128, 128, 128, 255)})
      .child_of(chisel);

    auto selection_bound = ecs.entity()
      .is_a(ui_element)
      .set<FollowChiselSelection>({chisel})
      .set<LineShape>({0, 64})
      .set<RenderStatus>({false, RenderMode::FILL, nvgRGBA(128, 128, 128, 255)})
      .child_of(chisel);

    auto chisel_enter_highlight = ecs.entity("chisel_center_highlight")
      .is_a(ui_element)
      .set<SpanChiselBounds>({anchor_bound, selection_bound})
      .set<BoxShape>({0.0, 64.0f})
      .set<RenderStatus>({false, RenderMode::FILL, nvgRGBA(128, 128, 128, 64)})
      .child_of(chisel);

		ecs.observer<UnixSocket>()
            .event(flecs::OnSet)
            .each([](UnixSocket& s) {
                // Create a socket file
                struct sockaddr_un addr;
                addr.sun_family = AF_UNIX;

                // Use strncpy to prevent buffer overflow
                strncpy(addr.sun_path, s.path.c_str(), sizeof(addr.sun_path) - 1);
                addr.sun_path[sizeof(addr.sun_path) - 1] = '\0'; // Ensure null-termination

                unlink(addr.sun_path);

                if (bind(s.fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                    perror("bind");
                    return;
                }

                // Listen for incoming connections
                if (listen(s.fd, 1) == -1) {
                    perror("listen");
                    return;
                }

                // Set the socket to non-blocking mode
                int flags = fcntl(s.fd, F_GETFL);
                if (flags == -1) {
                    perror("fcntl");
                    return;
                }

                // Set the socket to non-blocking mode
                if (fcntl(s.fd, F_SETFL, flags | O_NONBLOCK) == -1) {
                    perror("fcntl");
                    return;
                }
            });

    flecs::entity socket_entity = ecs.entity("socket_slhf")
        .set<UnixSocket>(create_socket(socket(AF_UNIX, SOCK_STREAM, 0), "/tmp/my_socket"));

    // auto diurnal_cycle = ecs.entity("diurnal")
    //   .is_a(ui_element)
    //   .set<CircleShape>({96.0f})
    //   .set<Position, Local>({128.5f, 128.5f})
    //   .add<ModCircle>();

    // auto sunlight = diurnal_cycle.get_mut<ModCircle>();
    // std::vector<NVGcolor> di_colors = get_palette("../assets/palettes/diurnal.hex");
    // // TODO: Use JPL's Ephemeris System to get daylight at coordinates
    // for (size_t n = 0; n < 10; n++)
    // {
    //   sunlight->colors.push_back(n % 2 == 0 ? di_colors[0] : di_colors[1]);
    // }
    // for (size_t d = 0; d < 14; d++)
    // {
    //   sunlight->colors.push_back(d % 2 == 0 ? di_colors[2] : di_colors[3]);
    // }

		ecs.system<FollowChiselAnchor, Position>("FollowChiselAnchor").term_at(1).second<Local>()
    .each([](FollowChiselAnchor& follow, Position& pos) {
      auto chisel = follow.chisel.try_get<HorizontalBoundaryChisel>();
      pos.x = chisel->anchor_x;
    });

		ecs.system<FollowChiselSelection, Position>("FollowChiselSelection").term_at(1).second<Local>()
		.each([](FollowChiselSelection& follow, Position& pos) {
		  auto chisel = follow.chisel.try_get<HorizontalBoundaryChisel>();
		  pos.x = chisel->selection_x;
		});

		ecs.system<SpanChiselBounds, Position, BoxShape>("SpanChiselBounds").term_at(1).second<Local>()
		    .each([](SpanChiselBounds& span, Position& pos, BoxShape& box) {
		      auto anchor_pos = span.anchor.try_get<Position, Local>();
		      auto selection_pos = span.selection.try_get<Position, Local>();
		      box.width = selection_pos->x - anchor_pos->x;
		      pos.x = anchor_pos->x;
		    });
    // ecs.observer
    // TODO: Merge into enum pair scene management...
		ecs.system<RenderStatus, CursorDistanceVisible, Position, X11CursorMovement>("HideFarGraphSelectors").term_at(2).second<World>().term_at(3).src(user_input)
		.each([](RenderStatus& rstat, CursorDistanceVisible& dist, Position& pos, X11CursorMovement& cursor)
		{
		  vec2 cpos = {cursor.x, cursor.y};
		  vec2 gpos = {pos.x, pos.y};
		  float d = glm_vec2_distance(cpos, gpos);
		  bool show = d < dist.max;
		  if (show)
		  {
		    rstat.color.a = 1.0f - (d/dist.max);
		  } else
		  {
		    rstat.color.a = 0.0f;
		  }
		});

    // TODO: Refactor for composable tag visibility
		ecs.system<RenderStatus, LevelHUD, LevelHUD*>("UpdateLevelHUDVisibility").term_at(2).src(hud).optional().kind(flecs::PreUpdate)
		.each([](RenderStatus& rstat, LevelHUD& lvl, LevelHUD* show_level_hud)
		{
		  rstat.visible = (bool)show_level_hud;
		});
		ecs.system<RenderStatus, AnnotatorHUD, AnnotatorHUD*>("UpdateAnnotatorHUDVisibility").term_at(2).src(hud).optional().kind(flecs::PreUpdate)
		.each([](RenderStatus& rstat, AnnotatorHUD& annotation, AnnotatorHUD* show_annotator_hud)
		{
		  rstat.visible = (bool)show_annotator_hud;
		});

		ecs.observer<SortCol, SaveProgramSort>()
		    .write(flecs::Wildcard).event(flecs::OnSet)
		    .each([](flecs::entity e, SortCol& sort_col, SaveProgramSort& save)
		    {
		      std::string base = "/home/heonae/arc/ec_pathfinder/neuralsort/pytorch/hf/" + std::to_string(save.sort_index) + "/";
		      system(("mkdir -p " + base).c_str());
		
		      std::ofstream program_state_hypotheses(base + "program_state.txt", std::ios::trunc);
		      if (program_state_hypotheses.is_open()) {
		        for (const auto& hypothesis : sort_col.sorts[1])
		        {
		          program_state_hypotheses << hypothesis.index << " " << hypothesis.priority << std::endl;
		        }
		        program_state_hypotheses.close();
		      }
		
		      std::ofstream var_expand_hypotheses(base + "var_expand.txt", std::ios::trunc);
		      if (var_expand_hypotheses.is_open()) {
		        for (const auto& hypothesis : sort_col.sorts[3])
		        {
		          var_expand_hypotheses << hypothesis.index << " " << hypothesis.priority << std::endl;
		        }
		        var_expand_hypotheses.close();
		      }
		
		      if (save.next_stage)
		      {
		        e.world().query<SocketConnection>()
		          .each([](SocketConnection& conn) {
		            char* response = "next_stage";
		            send(conn.connfd, response, strlen(response), 0);
		          });
		      } else
		      {
		        e.world().query<SocketConnection>()
		          .each([](SocketConnection& conn) {
		            char* response = "feedback_update";
		            send(conn.connfd, response, strlen(response), 0);
		          });
		      }
		
		    });
    
    flecs::query<RectilinearGrid, RectilinearGridSelector, SortCol> q_slhf = ecs.query_builder<RectilinearGrid, RectilinearGridSelector, SortCol>()
        .write(flecs::Wildcard)
        .build();

		ecs.observer<X11KeyPress>("KeyNavSortLearningHumanFeedback").write(flecs::Wildcard).event(flecs::OnSet)
		.each([display, q_slhf](flecs::entity e, X11KeyPress& key) {
		  KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
		  auto user_input = ecs.lookup("user_input");
		  KeyboardState& keyboard = user_input.ensure<KeyboardState>();
		  q_slhf.each([&](flecs::entity ent, RectilinearGrid& grid, RectilinearGridSelector& selector, SortCol& sort_col) {
		    if (symbol == XK_Return)
		    {
		      if (keyboard.key_states[KEY_LEFTALT] || keyboard.key_states[KEY_RIGHTALT])
		      {
		        e.world().query<SocketConnection>()
		        .each([](flecs::entity conn_e, SocketConnection& conn) {
		          const char* response = "next_graphics_program";
		          send(conn.connfd, response, strlen(response), 0);
		        });
		        sort_col.sort_index = 0;
		      } else
		      {
		        ent.set<SaveProgramSort>({sort_col.sort_index, true});
		      }
		    }
		    if (symbol == XK_a)
		    {
		      selector.col = std::max(selector.col - 1, 0);
		    }
		    else if (symbol == XK_d)
		    {
		      selector.col = std::min(selector.col + 1, (int)grid.col_sizes.size()-1);
		    }
		    else if (symbol == XK_w)
		    {
		      if ((keyboard.key_states[KEY_LEFTSHIFT] || keyboard.key_states[KEY_RIGHTSHIFT]))
		      {
		        if (sort_col.sorts.count(selector.col))
		        {
		          if (selector.row - 1 >= 0)
		          {
		            GraphicsProgramHypothesis hypothesis = sort_col.sorts[selector.col][selector.row];
		            GraphicsProgramHypothesis upper_hypothesis = sort_col.sorts[selector.col][selector.row-1];
		            Position current_pos = hypothesis.viz.ensure<Position, Local>();
		            Position upper_pos = upper_hypothesis.viz.ensure<Position, Local>();
		            upper_hypothesis.viz.set<Position, Local>(current_pos);
		            hypothesis.viz.set<Position, Local>(upper_pos);
		            sort_col.sorts[selector.col][selector.row-1] = hypothesis;
		            sort_col.sorts[selector.col][selector.row] = upper_hypothesis;
		            if (selector.row != 5) // TODO: Dynamic sorted cols (that become invalidated or hidden)
		            {
		              ent.set<SaveProgramSort>({sort_col.sort_index});
		            }
		          }
		        }
		      }
		      selector.row = std::max(selector.row - 1, 0);
		    }
		    else if (symbol == XK_s)
		    {
		      if ((keyboard.key_states[KEY_LEFTSHIFT] || keyboard.key_states[KEY_RIGHTSHIFT]))
		      {
		        if (sort_col.sorts.count(selector.col))
		        {
		          if (selector.row + 1 < (int)grid.row_sizes.size())
		          {
		            GraphicsProgramHypothesis hypothesis = sort_col.sorts[selector.col][selector.row];
		            GraphicsProgramHypothesis lower_hypothesis = sort_col.sorts[selector.col][selector.row+1];
		            Position current_pos = hypothesis.viz.ensure<Position, Local>();
		            Position lower_pos = lower_hypothesis.viz.ensure<Position, Local>();
		            lower_hypothesis.viz.set<Position, Local>(current_pos);
		            hypothesis.viz.set<Position, Local>(lower_pos);
		            sort_col.sorts[selector.col][selector.row] = lower_hypothesis;
		            sort_col.sorts[selector.col][selector.row+1] = hypothesis;
		            if (selector.row != 5)
		            {
		              ent.set<SaveProgramSort>({sort_col.sort_index});
		            }
		          }
		        }
		      }
		      selector.row = std::min(selector.row + 1, (int)grid.row_sizes.size()-1);
		    }
		    int h_x = 0;
		    for (int x = 0; x < selector.col; x++)
		    {
		      h_x += grid.col_sizes[x] + 2;
		    }
		    int h_y = 0;
		    for (int y = 0; y < selector.row; y++)
		    {
		      h_y += grid.row_sizes[y] + 2;
		    }
		    selector.highlight.set<Position, Local>({h_x-1, h_y-1});
		    selector.highlight.set<BoxShape>({grid.col_sizes[selector.col]+2, grid.row_sizes[selector.row]+2, 1.0f});
		    if (selector.row == 0)
		    {
		      selector.highlight.set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(0x00, 0xFF, 0x00, 0xFF)});
		    } else if (sort_col.sorts.count(selector.col))
		    {
		      selector.highlight.set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(0xFF, 0x00, 0x00, 0xFF)});
		    } else
		    {
		      selector.highlight.set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(0xFF, 0xFF, 0xFF, 0xFF)});
		    }
		  });
		});

		ecs.observer<RectilinearGrid, RectilinearGridSelector, SortCol, ReloadCol>().event(flecs::OnSet)
		.each([](flecs::entity e, RectilinearGrid& grid, RectilinearGridSelector& selector, SortCol& sort_col, ReloadCol& reload)
		{
		  for (auto& element : grid.elements[reload.col])
		  {
			element.destruct();
		  }
		  sort_col.sorts[reload.col].clear();
		  auto ui_element = ecs.lookup("ui_element");
		  grid.col_sizes = {28, 28*2, 28, 28, 28*2};
		  // TODO: Support jagged row counts for cols, larger than screen vertical list...
		  grid.row_sizes = {28, 28, 28, 28};
		  int x_pos = 0;
		  for (int x = 0; x < grid.col_sizes.size(); x++)
		  {
			std::vector<flecs::entity> col_elements;
			int y_pos = 0;
			for (int y = 0; y < grid.row_sizes.size(); y++)
			{
			  if (x == reload.col)
			  {
				flecs::entity grid_element = ecs.entity()
				  .is_a(ui_element)
				  .add<LevelHUD>()
				  .set<Position, Local>({x_pos, y_pos})
				  .set<BoxShape>({grid.col_sizes[x], grid.row_sizes[y], 1.0f})
				  .set<RenderStatus>({false, RenderMode::STROKE, nvgRGBA(0x00, 0x00, 0x00, 0xFF)})
				  .child_of(e);
				col_elements.push_back(grid_element);

				std::string path = "/home/heonae/arc/ec_pathfinder/neuralsort/pytorch/sort_stage/" + std::to_string(reload.sort_index) + "/program_" + std::to_string(y) + ".png";
				if (std::filesystem::exists(path))
				{
				  flecs::entity program_state_index = ecs.entity()
					.is_a(ui_element)
					.set<SpriteCreator>({path})
					.add<LevelHUD>()
					.child_of(grid_element);
				}
				if (reload.col == 1)
				{
				  sort_col.sorts[reload.col].push_back({y, 0, grid_element});
				}

				grid.elements[reload.col] = col_elements;
			  }
			  y_pos += grid.row_sizes[y] + 2;
			}
			x_pos += grid.col_sizes[x] + 2;
		  }
		  e.remove<ReloadCol>();
		});

	ecs.observer<RectilinearGrid, RectilinearGridSelector, SortCol>().event(flecs::OnAdd)
    .each([](flecs::entity e, RectilinearGrid& grid, RectilinearGridSelector& selector, SortCol& sort_col)
    {
      auto ui_element = ecs.lookup("ui_element");
      grid.col_sizes = {28, 28*2, 28, 28, 28*2};
      // TODO: Support jagged row counts for cols, larger than screen vertical list...
      grid.row_sizes = {28, 28, 28, 28};
      int x_pos = 0;
      for (int x = 0; x < grid.col_sizes.size(); x++)
      {
        std::vector<flecs::entity> col_elements;
        int y_pos = 0;
        for (int y = 0; y < grid.row_sizes.size(); y++)
        {
          if (y == 0 && x == 0)
          {
          flecs::entity grid_selector = ecs.entity()
            .is_a(ui_element)
            .add<LevelHUD>()
            .set<Position, Local>({x_pos-1, y_pos-1})
            .set<BoxShape>({grid.col_sizes[x]+2, grid.row_sizes[y]+2, 1.0f})
            .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(0xFF, 0xFF, 0xFF, 0xFF)})
            .child_of(e);
          selector.highlight = grid_selector;
          }
          // if (y == 0 && x == 4)
          // {
          // flecs::entity sort_expand_indicator = ecs.entity()
          //   .is_a(ui_element)
          //   .add<LevelHUD>()
          //   .set<Position, Local>({x_pos-1, y_pos-1})
          //   .set<BoxShape>({grid.col_sizes[x]+2, 28*4+2, 1.0f})
          //   .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(0x00, 0x00, 0xFF, 0xFF)})
          //   .child_of(e);
          // }
          // TODO: How to infill these with MNIST/program/variable
          flecs::entity grid_element = ecs.entity()
            .is_a(ui_element)
            .add<LevelHUD>()
            .set<Position, Local>({x_pos, y_pos})
            .set<BoxShape>({grid.col_sizes[x], grid.row_sizes[y], 1.0f})
            .set<RenderStatus>({false, RenderMode::STROKE, nvgRGBA(0x00, 0x00, 0x00, 0xFF)})
            .child_of(e);
          col_elements.push_back(grid_element);
          if (x == 0 || x == 2)
          {
            // MNIST ROW
            if (y == 0)
            {
              // Place MNIST 0
              flecs::entity sort_index = ecs.entity()
                .is_a(ui_element)
                .set<SpriteCreator>({random_mnist(0, 0)})
                .set<MNISTDigit>({0, 0})
                .add<LevelHUD>()
                .child_of(grid_element);
            }
          }
          if (x == 1)
          {
            std::string path = "/home/heonae/arc/ec_pathfinder/neuralsort/pytorch/sort_stage/" + std::to_string(sort_col.sort_index) + "/program_" + std::to_string(y) + ".png";
            if (std::filesystem::exists(path))
            {
              // TODO: Index these for swap and P_true export
              flecs::entity program_state_index = ecs.entity()
                .is_a(ui_element)
                .set<SpriteCreator>({path})
                .add<LevelHUD>()
                .child_of(grid_element);
              sort_col.sorts[1].push_back({y, 0, grid_element});
            }
          }
          if (x == 3)
          {
            flecs::entity variable_expansion_index = ecs.entity()
              .set<TextCreator>({{'a' + ((char)y)}, "ATARISTOCRAT"})
              .add<Position, World>()
              .set<Position, Local>({10.0f, 16.0f})
              .add<RenderStatus>()
              .add<LevelHUD>()
              .child_of(grid_element);
            sort_col.sorts[3].push_back({y, 0, grid_element});
          }
          if (x == 4)
          {
            std::string path = "/home/heonae/arc/ec_pathfinder/neuralsort/pytorch/sort_stage/" + std::to_string(sort_col.sort_index+1) + "/program_" + std::to_string(y) + ".png";
            if (std::filesystem::exists(path))
            {
              flecs::entity program_state_index = ecs.entity()
                .is_a(ui_element)
                .set<SpriteCreator>({path})
                .add<LevelHUD>()
                .child_of(grid_element);
              // Need to track elements here for reload...
            }
          }
          y_pos += grid.row_sizes[y] + 2;
        }
        grid.elements.push_back(col_elements);
        x_pos += grid.col_sizes[x] + 2;
      }
    });

    for (std::string arc_element_name : arc_element_names)
    {
      flecs::entity arc_color_entity = ecs.entity(arc_element_name.c_str());
      arc_color_entities.push_back(arc_color_entity);
    }

		ecs.observer<ArcDataLoader>("LoadArcDataset").event(flecs::OnSet)
		.each([](ArcDataLoader& loader) {
		  std::cout << "Load arc dataset!" << std::endl;
		  size_t i = 0;
		  for (const auto & entry : fs::directory_iterator(loader.dir))
		  {
		    loader.data.push_back(entry.path());
		    std::string task_id = entry.path().stem().string();
		    loader.task_ids.push_back(task_id);
		    std::ifstream file("start_arc_task.txt");
		    std::string start_task;
		    std::getline(file, start_task);
		    if (task_id == start_task)
		    {
		      loader.active_index = i;
		    }
		    i++;
		  }
		  // loader.active_index = loader.get_index("673ef223");
		  std::cout << loader.data.size() << "arc data" << std::endl;
		  loader.viz = load_arc_task(&loader, loader.data[loader.active_index]);
		});

		ecs.observer<SequenceStepCompleted, SymbolicTargetSeq>("ProgressActionSequence").write(flecs::Wildcard).event(flecs::OnSet)
		.each([](flecs::entity e, SequenceStepCompleted& ssc, SymbolicTargetSeq& target_seq)
		{
		  printf("Seq step completed!\n");
		  target_seq.step++;
		  if (target_seq.step >= target_seq.navigations.size())
		  {
		    printf("Seq completed %i\n", target_seq.step);
		    e.remove<SequenceStepCompleted>();
		    target_seq.step = 0;
		    // Sequence completed event!
		  } else
		  {
		    printf("Seq step next %i\n", target_seq.step);
		    SymbolNavigator sn = target_seq.navigations[target_seq.step];
		    SymbolInteraction si = sn.interaction;
		    if (si == LEFT_CLICK)
		    {
		      e.set<ClickOnTarget>({1});
		    } else if (si == LEFT_MOUSE_DOWN)
		    {
		      e.set<MouseDownOnTarget>({1});
		    } else if (si == LEFT_MOUSE_UP)
		    {
		      e.set<MouseUpOnTarget>({1});
		    }
		    auto tsl = sn.location;
		    e.set<Position, MoveTarget>({(float)tsl.x, (float)tsl.y});
		    e.set<SpriteCreator>({"../assets/move.png", 8, 1});
		  }
		});

    // ecs.observer<LoadSpeechGraphEvent, NanoVG>("LoadSpeechStream").term_at(1).src(renderer).event(flecs::OnSet).iter([ui_element](flecs::iter& it, LoadSpeechGraphEvent* load, NanoVG* vg)
    // {
      
    // });

		ecs.observer<SpeechStream, GraphRail, Overlay>("SetStreamDurationFromScreenWidth").event(flecs::OnSet).term_at(2).src(hud)
		.each([](SpeechStream& ss, GraphRail& rail, Overlay& overlay)
		{
		  ss.duration = overlay.width / rail.x_per_second;
		});

		ecs.observer<ReachedTargetEvent, ClickOnTarget>("ClickOnceTargetReached")
		    .event(flecs::OnAdd)
		    .each([](flecs::entity e, ReachedTargetEvent* rmt, ClickOnTarget* click)
		    {
		      char xdo_cmd[32];
		      snprintf(xdo_cmd, sizeof(xdo_cmd), "xdotool click %i", click->button);
		      system(xdo_cmd);
		      // TODO: Evaluate entity organization for event pathing
		      // TODO: Remove ClickOnTarget component...
		      e.remove<ReachedTargetEvent>();
		      e.remove<ClickOnTarget>();
		      e.remove<Position, MoveTarget>();
		      e.set<SpriteCreator>({"../assets/arrow.png", 8, 1});
		      e.set<SequenceStepCompleted>({0});

		      // auto& sprite = e.ensure<Sprite>();
		    });

		ecs.observer<ReachedTargetEvent, MouseDownOnTarget>("MouseDownOnceTargetReached").event(flecs::OnAdd)
		.each([](flecs::entity e, ReachedTargetEvent& rmt, MouseDownOnTarget& click) {
		  printf("Mouse down mode\n");
		  char xdo_cmd[32];
		  snprintf(xdo_cmd, sizeof(xdo_cmd), "xdotool mousedown %i", click.button);
		  system(xdo_cmd);
		  // TODO: Evaluate entity organization for event pathing
		  // TODO: Remove ClickOnTarget component...
		  e.remove<ReachedTargetEvent>();
		  e.remove<MouseDownOnTarget>();
		  e.remove<Position, MoveTarget>();
		  e.set<SequenceStepCompleted>({0});
		});

		ecs.observer<ReachedTargetEvent, MouseUpOnTarget>("MouseUpOnceTargetReached").event(flecs::OnAdd)
		.each([](flecs::entity e, ReachedTargetEvent& rmt, MouseUpOnTarget& click)
		{
		  char xdo_cmd[32];
		  snprintf(xdo_cmd, sizeof(xdo_cmd), "xdotool mouseup %i", click.button);
		  system(xdo_cmd);
		  // TODO: Evaluate entity organization for event pathing
		  // TODO: Remove ClickOnTarget component...
		  e.remove<ReachedTargetEvent>();
		  e.remove<MouseUpOnTarget>();
		  e.remove<Position, MoveTarget>();
		  e.set<SpriteCreator>({"../assets/arrow.png", 8, 1});
		  e.set<SequenceStepCompleted>({0});
		});

		ecs.observer<DeviceEventListener>("ListenToDevice").event(flecs::OnSet)
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

		ecs.observer<FontCreator, NanoVG>("LoadFont").term_at(1).src(renderer).event(flecs::OnSet)
		.each([](flecs::entity e, FontCreator& creator, NanoVG& vg) {
		    int handle = nvgCreateFont(vg.ctx, creator.name, creator.path);
		    e.set<NanoFont>({handle});
		});

		ecs.observer<NanoVG, SpriteCreator>("CreateSprite")
		  .term_at(0).src(renderer)
		  .event(flecs::OnSet)
		  .each([](flecs::entity e, NanoVG& vg, SpriteCreator& sc) {
		    auto img = nvgCreateImage(vg.ctx, sc.path.string().c_str(), 0);
		    int imgw, imgh;
		    nvgImageSize(vg.ctx, img, &imgw, &imgh);

		    float scale = 1.0f;
		    if (sc.desired_height > 0) {
		      scale = sc.desired_height / imgh;
		      imgw = static_cast<int>(imgw * scale);
		      imgh = static_cast<int>(sc.desired_height);
		    }

		    float spriteWidth = imgw / sc.c;
		    float spriteHeight = imgh / sc.r;

		    e.set<Sprite>({img, 0, 0, imgw, imgh, spriteWidth, spriteHeight, sc.r, sc.c});

		    if (sc.r > 1 || sc.c > 1) {
		      e.set<SpriteAnimator>({0, 0, 150.0f/1000.0f});
		    }
		  });


		ecs.system<NanoVG, Background, ColorAnimation, Overlay>("RenderBackground").term_at(0).src(renderer).kind(flecs::OnUpdate)
    .each([](NanoVG& vg, Background& bkg, ColorAnimation& color, Overlay& sail) {
      if (!sail.is_framing_virt_desk)
      {
        nvgBeginPath(vg.ctx);
        nvgRect(vg.ctx, 0, 0, sail.width, sail.height);
        nvgFillColor(vg.ctx, color.current);
        nvgFill(vg.ctx);
      }
    });

		ecs.system<MNISTDigit, Sprite>("BabaIsYouAnim").interval(0.33)
			.each([](flecs::entity e, MNISTDigit& digit, Sprite& sprite) {
				e.set<SpriteCreator>({random_neighbor_mnist(digit.index, digit.char_set_index)});
			});

    // JANE: I am helpless
    // ecs.system<NanoVG, BackgroundBoxShape, Position, RenderStatus>("RenderBackgroundBoxes")
    // .term_at(0).src(renderer).term_at(2).second<World>().kind(flecs::OnUpdate)
    // .iter([](flecs::iter& it, NanoVG* vg, BackgroundBoxShape* bs, Position* pos, RenderStatus* rstat) {
    //     for (size_t i : it) {
    //         if (rstat[i].visible) {
    //             nvgBeginPath(vg->ctx);
    //             nvgRect(vg->ctx, pos[i].x + 0.5f, pos[i].y +0.5f, bs[i].width, bs[i].height);
    //             if (rstat[i].mode == RenderMode::FILL) {
    //                 nvgFillColor(vg->ctx, rstat[i].color);
    //                 nvgFill(vg->ctx);
    //             } else {
    //                 nvgLineCap(vg->ctx, NVGlineCap::NVG_SQUARE);
    //                 nvgStrokeWidth(vg->ctx, bs[i].stroke_width);
    //                 nvgStrokeColor(vg->ctx, rstat[i].color);
    //                 nvgStroke(vg->ctx);
    //             }
    //             nvgClosePath(vg->ctx);
    //         }
    //     }
    // });

	// ecs.system<GraphAudio, GraphSpeak, BookPlayerEventPlay>("ToggleGraphSpeak").term_at(2).src(user_input).kind(flecs::PreUpdate)
	//     .each([](GraphAudio& audio, GraphSpeak& speak, BookPlayerEventPlay& play)
	//     {
	//       std::cout << "ToggleGraphSpeak" << std::endl;
	//       if (audio.music->getStatus() == sf::SoundSource::Status::Paused)
	//       {
	//         audio.music->play();
	//       } else
	//       {
	//         audio.music->pause();
	//       }
	//       auto user_input = ecs.lookup("user_input");
	//       user_input.remove<BookPlayerEventPlay>();
	//     });

		// ecs.system<GraphRail, GraphAudio, RegenSpeechEvent, HorizontalBoundaryChisel>("RegenSpeechEvent").term_at(1).parent().term_at(2).src(user_input).term_at(3).src(chisel).kind(flecs::PreUpdate)
		// .each([](GraphRail& graph_rail, GraphAudio& graph_audio, RegenSpeechEvent& regen, HorizontalBoundaryChisel& chisel)
		// {
		// 	printf("Regenerate Speech\n");
		// 	system((std::string("python3 regen_speech.py ") + std::to_string(chisel.anchor_x/graph_rail.x_per_second) + " " + std::to_string(chisel.selection_x/graph_rail.x_per_second) + std::string(" /home/heonae/graphlife/library/memory/speech/fablehaven_part_0.json")).c_str());
		// 	// PYTHON
		// 	// Identify the word sequence to be regenerated x
		// 	// XTTS over it x
		// 	// Deepgram over outputs
		// 	// Offset Deepgram JSON values based on new sequence insertion position
		// 	// Modify .wav to cut the removed duration to an archive storage (for undo ops)
		// 	// Insert the new audio in place x

		// 	// C++
		// 	// Reload GraphAudio

		// 	// Update SpokenWord positions... Just replace them all since there there may be a JSON mismatch with the existing nodes (esp quant diff)
		// 	sf::Time playing_offset = graph_audio.music->getPlayingOffset();
		// 	sf::SoundSource::Status status = graph_audio.music->getStatus();
		// 	graph_audio.music->openFromFile(graph_audio.filepath.c_str());
		// 	graph_audio.music->setPlayingOffset(playing_offset);
		// 	graph_audio.music->play();
		// 	graph_audio.music->pause();
		// 	ecs.lookup("user_input").remove<RegenSpeechEvent>();
		// });
    

		ecs.observer<X11KeyPress, UIDebugRender>().term_at(1).src(renderer).event(flecs::OnSet)
		.each([display](X11KeyPress& key, UIDebugRender& ui_debug_render)
		{
		  KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
		  if (symbol == XK_grave)
		  {
		    ui_debug_render.active = !ui_debug_render.active;
		  }
		});

		ecs.observer<X11KeyPress>("ClusterGraph")
			.each([display](flecs::entity e, X11KeyPress& key) {
				KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
				if (symbol == XK_c)
				{
					// Cluster selected graphs
				}
			});

		ecs.system<X11CursorMovement, DragBoundGraphSelector, BoxShape, Position>("UpdateDragBound").term_at(0).src(user_input).term_at(3).second<Local>()
		.each([display](X11CursorMovement& cursor, DragBoundGraphSelector& selector, BoxShape& box, Position& pos) {
			if (selector.active) {
				// std::cout << "Update drag bound!" << std::endl;
				pos.x = std::min(cursor.x, selector.anchor_x);
				pos.y = std::min(cursor.y, selector.anchor_y);
				box.width = std::abs(cursor.x - selector.anchor_x);
				box.height = std::abs(cursor.y - selector.anchor_y);
			} else {
				pos.x = -10;
				box.width = 0;
				box.height = 0;
			}
		});

		ecs.observer<X11KeyPress, GraphEditor>().write(flecs::Wildcard).term_at(1).src(user_input).event(flecs::OnSet)
		.each([display](X11KeyPress* key, GraphEditor* graph_editor)
		{
		  KeySym symbol = XKeycodeToKeysym(display, key->event.keycode, 0);
		  auto user_input = ecs.lookup("user_input");
		  KeyboardState& keyboard = user_input.ensure<KeyboardState>();
		  if (symbol == XK_space && (keyboard.key_states[KEY_LEFTCTRL] || keyboard.key_states[KEY_RIGHTCTRL]))
		  {
		    std::cout << "BookPlayerEventPlay" << std::endl;
		    user_input.add<BookPlayerEventPlay>(); // TODO: Add to selected book
		    // if (graph_editor->edit_mode == GraphEditMode::READ)
		    // {
		    // }
		  }
		  else if (symbol == XK_r)
		  {
		    // TODO: Fix this... make it require ctrl...
		    // user_input.add<RegenSpeechEvent>();
		  }
		});

		ecs.observer<X11KeyPress>("LoadCartridge").write(flecs::Wildcard).event(flecs::OnSet)
		    .each([display, ui_element](X11KeyPress& key)
		    {
		      KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
		      auto user_input = ecs.lookup("user_input");
		      KeyboardState& keyboard = user_input.ensure<KeyboardState>();
		      int num_keys[] = {XK_KP_Insert, XK_KP_End, XK_KP_Down, XK_KP_Page_Down, XK_KP_Left, XK_KP_Begin, XK_KP_Right, XK_KP_Home, XK_KP_Up, XK_KP_Page_Up};
		      std::cout << "XK " << symbol << std::endl;
		      for (int n = 0; n < 10; n++)
		      {
		        if (symbol == num_keys[n])
		        {
		          std::cout << "Load cartridge " << n << std::endl;
		          auto q = ecs.query_builder<CartridgeBlock, SpriteCreator>()
		            .build();
		          q.each([&](flecs::entity e2, CartridgeBlock& block, SpriteCreator& sprite) {
		            if (block.value != n)
		            {
		              block.selected = false;
		              if (block.loaded == true)
		              {
		                e2.set<SpriteCreator>({"../assets/loaded_layer.png"});
		              } else
		              {
		
		              }
		            }
		          });
		          q.each([&](flecs::entity e2, CartridgeBlock& block, SpriteCreator& sprite) {
		            if (block.value == n)
		            {
		              if (block.loaded)
		              {
		                if (block.selected)
		                {
		                  auto q_g = ecs.query_builder<SaveGraph>()
		                    .build();
		                  q_g.each([&](flecs::entity e3, SaveGraph& save)
		                  {
		                    if (save.cartridge == n)
		                    {
		                      e3.destruct();
		                    }
		                  });
		                  block.loaded = false;
		                  block.selected = false; // TODO: Update selected layer for new graphs to first loaded or zero?
		                  e2.set<SpriteCreator>({"../assets/save_layer_" + std::to_string(block.value%2) + ".png"});
		                }
		                else
		                {
		                  block.selected = true;
		                  e2.set<SpriteCreator>({"../assets/selected_layer.png"});
		                }
		              }
		              else
		              {
		                block.loaded = true;
		                block.selected = true;
		                graphs_to_load = process_scroll("../save/cartridge/save_" + std::to_string(n) + ".txt");
		                e2.set<SpriteCreator>({"../assets/selected_layer.png"});
		              }
		
		            }
		          });
		        }
		      }
		      // if ((keyboard->key_states[KEY_LEFTCTRL] || keyboard->key_states[KEY_RIGHTCTRL]))
		      // {
		      // }
		    });

	ecs.observer<X11KeyPress>("SaveCartridge").write(flecs::Wildcard).event(flecs::OnSet)
	.each([display](flecs::entity /*e*/, X11KeyPress& key)
	{
	  KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
	  auto user_input = ecs.lookup("user_input");
	  KeyboardState& keyboard = user_input.ensure<KeyboardState>();
	  ArcDataLoader& arc_loader = ecs.lookup("arc").ensure<ArcDataLoader>();
	  if (symbol == XK_s && (keyboard.key_states[KEY_LEFTCTRL] || keyboard.key_states[KEY_RIGHTCTRL]))
	  {
		auto q = ecs.query_builder<SaveGraph, Position>()
		  .term_at(1).second<Local>()
		  .build();
		for (size_t c_i = 0; c_i < 10; c_i++)
		{
		  std::string save_path = "../save/cartridge/save_" + std::to_string(c_i) + ".txt";
		  if (c_i == 0)
		  {
			std::string dir = "../save/arc_annotation/" + arc_loader.get_active_problem();
			std::filesystem::path path(dir);
			save_path = dir + "/save_" + std::to_string(c_i) + ".txt";
		  }
		  std::ofstream save_cartridge(save_path.c_str());
		  q.each([&](flecs::entity e, SaveGraph& save_graph, Position& pos)
		  {
			  if (save_graph.cartridge == c_i)
			  {
				std::string graph_str = graph_to_string(e);
				if (save_cartridge.is_open())
				{
				  if (e.has<NodeType>())
				  {
					save_cartridge << e.try_get<NodeType>()->ecs;
				  } else
				  {
					save_cartridge << 0;
				  }
				  save_cartridge << " " << pos.x << " " << pos.y  << " " << graph_str << std::endl;
				}
			  }
		  });
		  save_cartridge.close();
		}
	  }
	});

    // ecs.observer<SaveGraphLayer>("SaveCartridge").write(flecs::Wildcard).event(flecs::OnSet).iter([](flecs::iter& it, SaveGraphLayer* layer)
    // {
    //   std::cout << "Save graph layer!" << std::endl;
    //   auto q = ecs.query_builder<SaveGraph, Position>()
    //     .term_at(1).second<Local>()
    //     .build();
    //   std::ofstream save_cartridge("../save/cartridge/save_" + std::to_string(layer->cartridge) + ".txt");
    //   q.iter([&](flecs::iter& sit, SaveGraph* save_graph, Position* pos) 
    //   {
    //     if (save_graph->cartridge == layer->cartridge)
    //     {
    //       for (size_t j : sit)
    //       {
    //         std::string graph_str = graph_to_string(sit.entity(j));
    //         if (save_cartridge.is_open())
    //         {
    //           if (sit.entity(j).has<NodeType>())
    //           {
    //             save_cartridge << sit.entity(j).try_get<NodeType>()->ecs;
    //           } else
    //           {
    //             save_cartridge << 0;
    //           }
    //           save_cartridge << " " << pos[j].x << " " << pos[j].y  << " " << graph_str << std::endl;
    //         }
    //         // Format to save to? plecs vs custom txt??
    //       }
    //     }
    //   });
    //   save_cartridge.close();
    // });

    // ecs.system<CreateSFX>().write(flecs::Wildcard).iter([](flecs::iter& it, CreateSFX* create)
    // {

    // });

		ecs.observer<X11KeyPress>().write(flecs::Wildcard).event(flecs::OnSet)
		    .each([display](X11KeyPress& key)
    {
      auto user_input = ecs.lookup("user_input");
      KeyboardState& keyboard = user_input.ensure<KeyboardState>();
      KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
      if (symbol == XK_e && (keyboard.key_states[KEY_LEFTCTRL] || keyboard.key_states[KEY_RIGHTCTRL]))
      {
        printf("Export graph selection\n");
        user_input.add<ExportGraphSelection>();
      }
    });

		ecs.observer<X11KeyPress>()
		    .write(flecs::Wildcard)
		    .event(flecs::OnSet)
		    .each([&](flecs::entity e, X11KeyPress& key) {
		      KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
		      // TODO: Need to learn how to read from GNN on sengra stream, otherwise words will be missed, misplaced, misemphasiszed
		      // TODO: Need index markers on chapters/pages
		      if (symbol == XK_Page_Up)
		      {

		      }
		      else if (symbol == XK_Page_Down)
		      {

		      }
		    });

		ecs.observer<X11KeyPress>("SelectSceneGraphNode").write(flecs::Wildcard).event(flecs::OnSet)
		  .each([&](flecs::entity e, X11KeyPress& key)
		  {
		    KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
		    if (symbol == XK_Return || symbol == XK_Escape)
		    {
		      auto q = ecs.query_builder<SceneGraphSelection>()
		        .write(flecs::Wildcard)
		        .build();
		      q.each([&](flecs::entity sel_e, SceneGraphSelection& sg_selection)
		      {
		        if (sg_selection.active)
		        {
		          auto ecs_scene_graph = sg_selection.scene_graph_root;
		          // printf("Hit enter\n");
		          // if (ecs_scene_graph.is_valid())
		          // {
		          //   printf("Scene graph engaged, captain!\n");
		          // }
		          if (ecs_scene_graph.has<SceneGraphInteractionNode>())
		          {
		            if (symbol == XK_Return)
		            {
		              if (sg_selection.selected_node.has<flecs::pair<EventOnSelect, RunAutonomousLinuxDesktop>>())
		              {
		                auto sound_effect = ecs.entity()
		                  .set<CreateSFX>({getRandomFileFromDir("../assets/autonomy")});
		              } else
		              {
		                auto sound_effect = ecs.entity()
		                  .set<CreateSFX>({getRandomFileFromDir("../assets/create")});
		              }

		              auto& scene_gin = ecs_scene_graph.ensure<SceneGraphInteractionNode>();
		              printf("Scene graph engaged, captain!\n");
		              if (sg_selection.selected_node.has<flecs::pair<EventOnSelect, CreatePrefabFromGraph>>())
		              {
		                printf("Interaction confirmed! Create prefab!\n");
		                std::string ux_query = std::string("python3 gen_prefab.py \"") + graph_to_string(scene_gin.node) + std::string("\"");
		                system(ux_query.c_str());
		                set_node_type(scene_gin.node, "prefab", ECS_NodeType::PREFAB, syntax_theme[ECS_NodeType::PREFAB], RenderMode::STROKE);

		                // TODO: Update color of graph to cyan...
		                // TODO: Update graph to single node in PascalCase?
		              }

		              if (sg_selection.selected_node.has<flecs::pair<EventOnSelect, CreateSlotFromGraph>>())
		              {
		                printf("Interaction confirmed! Create slot!\n");
		                std::string ux_query = std::string("python3 gen_slot.py \"") + graph_to_string(scene_gin.node) + std::string("\"");
		                system(ux_query.c_str());

		                std::string slot_name = "slot";
		                flecs::entity telos_graph_root = sg_selection.bode_telos_source.parent().parent();
		                // TODO Determine if source is Prefab
		                if (telos_graph_root.has<NodeType>() && telos_graph_root.try_get<NodeType>()->ecs == ECS_NodeType::PREFAB)
		                {
		                  std::cout << "Create slot from PREFAB" << std::endl;
		                  telos_graph_root.target<Next>().children([&](flecs::entity child)
		                  {
		                    if (child.has<TextCreator>() && !child.has<NodeTypeLabel>())
		                    {
		                      slot_name = child.try_get<TextCreator>()->word + " slot";
		                    }
		                  });
		                }
		                set_node_type(scene_gin.node, slot_name, ECS_NodeType::SLOT, syntax_theme[ECS_NodeType::SLOT]);
		              }

		              if (sg_selection.selected_node.has<flecs::pair<EventOnSelect, CreateEntityFromGraph>>())
		              {
		                // TODO: Get entity name for text creator if created from a prefab...
		                scene_gin.node.set<NodeType>({ECS_NodeType::ENTITY});
		                printf("Interaction confirmed! Create entity!\n");
		                std::string ux_query = std::string("python3 gen_entity.py \"") + graph_to_string(scene_gin.node) + std::string("\"");
		                system(ux_query.c_str());

		                std::string entity_name = "entity";
		                flecs::entity telos_graph_root = sg_selection.bode_telos_source.parent().parent();
		                // TODO Determine if source is Prefab
		                if (telos_graph_root.has<NodeType>() && telos_graph_root.try_get<NodeType>()->ecs == ECS_NodeType::PREFAB)
		                {
		                  std::cout << "Create entity from PREFAB" << std::endl;
		                  telos_graph_root.target<Next>().children([&](flecs::entity child)
		                  {
		                    if (child.has<TextCreator>() && !child.has<NodeTypeLabel>())
		                    {
		                      entity_name = child.try_get<TextCreator>()->word;
		                    }
		                  });
		                }
		                set_node_type(scene_gin.node, entity_name, ECS_NodeType::ENTITY, syntax_theme[ECS_NodeType::ENTITY]);
		              }

		              if (sg_selection.selected_node.has<flecs::pair<EventOnSelect, DeployCompanionFromGraph>>())
		              {
		                std::string ux_query = std::string("python3 deploy_companion.py \"") + graph_to_string(scene_gin.node) + std::string("\"");
		                system(ux_query.c_str());

		                set_node_type(scene_gin.node, "companion", ECS_NodeType::COMPANION, syntax_theme[ECS_NodeType::COMPANION]);
		              }

		              if (sg_selection.selected_node.has<flecs::pair<EventOnSelect, InterfacePythonServerFunction>>())
		              {
		                std::string ux_query = std::string("python3 deploy_companion.py \"") + graph_to_string(scene_gin.node) + std::string("\"");
		                system(ux_query.c_str());

		                set_node_type(scene_gin.node, "python", ECS_NodeType::PYTHON_SCRIPT, syntax_theme[ECS_NodeType::PYTHON_SCRIPT]);
		              }

		              if (sg_selection.selected_node.has<flecs::pair<EventOnSelect, AnchorSymbol>>())
		              {
		                // std::string ux_query = std::string("python3 deploy_companion.py \"") + graph_to_string(scene_gin->node) + std::string("\"");
		                // system(ux_query.c_str());

		                set_node_type(scene_gin.node, "symbol", ECS_NodeType::SYMBOL_ANCHOR, syntax_theme[ECS_NodeType::SYMBOL_ANCHOR]);
		              }

		              if (sg_selection.selected_node.has<flecs::pair<EventOnSelect, RunAutonomousLinuxDesktop>>())
		              {
		                set_node_type(scene_gin.node, "linux", ECS_NodeType::LINUX, syntax_theme[ECS_NodeType::LINUX]);
		                std::string ux_query = std::string("python3 run_linux_desktop.py \"") + graph_to_string(scene_gin.node) + std::string("\"");
		                system(ux_query.c_str());

		              }

		              if (sg_selection.selected_node.has<flecs::pair<EventOnSelect, CreateDatasetFromGraph>>())
		              {
		                std::string ux_query = std::string("python3 gen_dataset.py \"") + graph_to_string(scene_gin.node) + std::string("\"");
		                system(ux_query.c_str());

		                set_node_type(scene_gin.node, "dataset", ECS_NodeType::DATASET, syntax_theme[ECS_NodeType::DATASET]);
		              }

		              if (sg_selection.selected_node.has<flecs::pair<EventOnSelect, CreateModelFromDatasetGraph>>())
		              {
		                std::string ux_query = std::string("python3 gen_model.py \"") + graph_to_string(scene_gin.node) + std::string("\"");
		                system(ux_query.c_str());

		                set_node_type(scene_gin.node, "model", ECS_NodeType::MODEL, syntax_theme[ECS_NodeType::MODEL]);
		              }

		              if (sg_selection.selected_node.has<flecs::pair<EventOnSelect, CreateComponentFromGraph>>())
		              {
		                std::string ux_query = std::string("python3 gen_component.py \"") + graph_to_string(scene_gin.node) + std::string("\"");
		                system(ux_query.c_str());

		                set_node_type(scene_gin.node, "component", ECS_NodeType::COMPONENT, syntax_theme[ECS_NodeType::COMPONENT]);

		                auto first_node = scene_gin.node.target<Next>();
		                flecs::entity component_layout = ecs.entity("layout")
		                  .is_a(ui_element)
		                  .set<Position, Local>({16.0f, 24.0f})
		                  .add<VerticalLayoutBox>()
		                  .child_of(first_node);


		                // TODO: Update color of graph to cyan...
		                // TODO: Update graph to single node in PascalCase?
		              }

		              if (sg_selection.selected_node.has<flecs::pair<EventOnSelect, InstantiateComponent>>())
		              {
		                flecs::entity telos_graph_root = sg_selection.bode_telos_source.parent().parent();
		                if (telos_graph_root.has<NodeType>() && telos_graph_root.try_get<NodeType>()->ecs == ECS_NodeType::COMPONENT)
		                {
		                  std::string component_name = graph_to_string(telos_graph_root);
		                  std::string ux_query = std::string("python3 gen_plecs.py \"") + component_name + std::string("\"");
		                  system(ux_query.c_str());

		                  set_node_type(scene_gin.node, component_name + " plecs", ECS_NodeType::PLECS, syntax_theme[ECS_NodeType::PLECS]);
		                }
		              }

		              bool add_int = sg_selection.selected_node.has<flecs::pair<EventOnSelect, AddIntFromGraph>>();
		              bool add_float = sg_selection.selected_node.has<flecs::pair<EventOnSelect, AddFloatFromGraph>>();
		              bool add_string = sg_selection.selected_node.has<flecs::pair<EventOnSelect, AddStringFromGraph>>();
		              bool load_tensor = sg_selection.selected_node.has<flecs::pair<EventOnSelect, LoadTensorFromFile>>();

		              if (add_int || add_float || add_string)
		              {
		                std::cout << "Add field!" << std::endl;
		                if (scene_gin.node.try_get<NodeType>()->ecs == ECS_NodeType::COMPONENT)
		                {
		                  std::cout << "Create data field on component node!" << std::endl;
		                  auto node = scene_gin.node.target<Next>();
		                  auto layout = node.lookup("layout");

		                  if (layout.is_valid())
		                  {
		                    std::cout << "Component node has layout" << std::endl;
		                  }

		                  flecs::entity field_inst = ecs.entity()
		                    .is_a(component_field)
		                    .child_of(layout);

		                  // flecs::entity symbol = field_inst.target(data_primitive_symbol);

		                  flecs::entity symbol = ecs.entity()
		                    .is_a(ui_element)
		                    .child_of(field_inst);
		                  // TODO: Figure out how to make prefab slots work within observer/system iterators...
		                  //  = field_inst.lookup("symbol");
		                  std::string var_type = "int";
		                  if (add_int)
		                  {
		                    symbol.set<SpriteCreator>({"../assets/int.png"});
		                  } else if (add_float)
		                  {
		                    var_type = "float";
		                    symbol.set<SpriteCreator>({"../assets/float.png"});
		                  } else if (add_string)
		                  {
		                    var_type = "std::string";
		                    symbol.set<SpriteCreator>({"../assets/string.png"});
		                  }

		                  std::string var_name = graph_to_string(sg_selection.context_graph);
		                  std::string ux_query = std::string("python3 add_component_field.py ") + graph_to_string(scene_gin.node) + " " + var_name + " " + var_type;
		                  system(ux_query.c_str());

		                  flecs::entity var = ecs.entity()
		                    .is_a(ui_element)
		                    .set<TextCreator>({var_name, "ATARISTOCRAT"})
		                    .set<Position, Local>({0.0f, 12.0f})
		                    .child_of(field_inst);

		                  flecs::entity default_value = ecs.entity()
		                    .is_a(ui_element)
		                    .set<Position, Local>({0.0f, 12.0f})
		                    .child_of(field_inst);

		                  // TODO: Create a graph node which tracks y to the right of the scene graph, in which the user can input or edit the default value
		                  // if (add_int)
		                  // {
		                  //   symbol.set<TextCreator>({"test", "ATARISTOCRAT"})
		                  // } else if (add_float)
		                  // {
		                  //   symbol.set<SpriteCreator>({"../assets/float.png"});
		                  // } else if (add_string)
		                  // {
		                  //   symbol.set<SpriteCreator>({"../assets/string.png"});
		                  // }

		                }
		              }



		            }
		          }
		          // TODO: Iterate through scene graph, expanding node user marks?
		          sg_selection.active = false;
		          if (sg_selection.context_graph.is_valid())
		          {
		            sg_selection.context_graph.destruct();
		          }
		          sg_selection.bode_telos_source.remove<Edge>(sg_selection.interaction_graph_selector);
		          // TODO: Remove the edge from the bode to the selector...
		          sg_selection.selected_node = sg_selection.default_select_node;
		          ecs_scene_graph.set<Position, Local>({-1000.0f, -1000.0f});

		        }
		      });
		    }
		  });

    ecs.observer<X11KeyPress>().write(flecs::Wildcard).event(flecs::OnSet)
    .each([&](X11KeyPress& key)
    {
      printf("KEY EVENT\n");
      auto q = ecs.query<const flecs::pair<Position, World>, const UIElementBounds, TextCreator, TextInput>();
      flecs::entity created_node;
      flecs::entity prev_node;
      flecs::entity prev_node_emoji;
      flecs::entity prev_text;
      flecs::entity created_emoji;
      flecs::entity created_text;
      
      KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
      KeyboardState& keyboard = ecs.lookup("user_input").ensure<KeyboardState>();
      if (symbol == XK_v && (keyboard.key_states[KEY_LEFTCTRL] || keyboard.key_states[KEY_RIGHTCTRL]))
      {
        auto cursor = ecs.lookup("user_input").try_get<X11CursorMovement>();
        std::cout << "Paste clipboard" << std::endl;
        std::string clipboard = getClipboardString(display, root);
        flecs::entity output[6];
        spawn_graph(hud, ui_element, cursor->x, cursor->y, &output[0]);
        output[5].remove<TextInput>();
        output[5].set<TextCreator>({clipboard});
      }

      q.each([&](flecs::entity text_entry, const Position& pos, const UIElementBounds& bounds, TextCreator& text, TextInput& input) {
          char unicode_input[255];
          auto edit_node = text_entry.parent();
          
          if (symbol == XK_BackSpace) {
            if (text.word.empty()) {
              printf("Word is empty\n");
              if (edit_node.has<Prev>(flecs::Wildcard)) // EDIT NODE DOES NOT HAVE PREV... WHY?
              {
                printf("Jump to prev\n");
                flecs::entity prev_node = edit_node.target<Prev>();
                flecs::entity prev_node_text;
                prev_node.children([&](flecs::entity child) { // Really need slots lmao
                  if (child.has<TextCreator>())
                  {
                    prev_node_text = child;
                  }
                });
                prev_node_text.add<TextInput>();
              }
              if (edit_node.has<Head>(flecs::Wildcard))
              {
                remove_forebode_graph();
                edit_node.target<Head>().destruct();
              }
              edit_node.destruct();
            } else
            {
              text.word = text.word.substr(0, text.word.size() - 1);
            }
          } else if (symbol == XK_Return)
          {

            text_entry.remove<TextInput>();

            // TODO: Get the head
            flecs::entity head = edit_node.parent();
            std::string graph_str = graph_to_string(head);
            std::cout << graph_str << std::endl;
            ArcDataLoader& loader = ecs.lookup("arc").ensure<ArcDataLoader>(); 
            std::string task_id = loader.get_active_problem();
            auto prep_sound_effect = ecs.entity()
              .set<CreateSFX>({"../assets/think/hmm.wav"});
            //system(("cd ../../synthesis/jinja && python3 gen_nl_query.py --task-id " + task_id + " --query-prompt " + "\"" + graph_str + "\"").c_str());
            
            system(("cd ../../jane_eyre && python3 nlp_query.py --task-id " + task_id + " --query-prompt " + "\"" + graph_str + "\"").c_str());
            highlight_query_pixels();
            // add_intermediate_state(ecs.lookup("arc").get_mut<ArcDataLoader>());
            auto sound_effect = ecs.entity()
              .set<CreateSFX>({"../assets/think/graph_it.wav"});
            // This is the language query we'll pass to the synthesis engine...

            // TODO: If book title is in graph
            flecs::entity library = ecs.lookup("library");

            // TODO: Program synthesis
            // Save the entire sengra to a file
            // Figure out how to easily sync Python/C++ data for natural language bindings
            // Parse the text in Python, generate a flecs query for the currently loaded task
            // Refactor the synthesis engine to support single query execution (perhaps even as a server?)
            // Compile and execute the query
            // Get the 'task table output' (counts on task io) and outputs
            // Render the query's selected pixels
            // TODO: generate graph neural network UX
            // most simple action: determine if user wants to load a book!

            // TODO: Only run a query if indicated from a bode after graph is created...
            // std::string ux_query = std::string("python3 ux_query.py \"") + graph_to_string(edit_node.parent()) + std::string("\"");
            // system(ux_query.c_str());
            
            // std::ifstream file("book.txt");
            // std::string bookTitle;
            // std::getline(file, bookTitle);
            // file.close();

            // std::cout << "Read:" << bookTitle << std::endl;
            // flecs::entity load_book = ecs.entity()
            //   .set<LoadBookEvent>({bookTitle});
            // remove_forebode_graph();
          } else if (symbol == XK_V && (keyboard.key_states[KEY_LEFTCTRL] || keyboard.key_states[KEY_RIGHTCTRL]))
          {
          }
          else if (symbol == XK_space)
          {
            prev_text = text_entry;
            prev_node = edit_node;
            // TODO: We should set the edit_node color

            flecs::entity output[3];
            const Position* p_pos = text_entry.parent().try_get<flecs::pair<Position, World>>();
            flecs::entity head = edit_node.parent();
            const Position* head_pos = head.try_get<Position, World>();
            create_graph_node(bounds.xmax + 6 - head_pos->x, 0.0f, output);
            created_node = output[0];
            created_emoji = output[1];
            created_text = output[2];
            created_emoji.add<EventOnGrab, SelectGraphNode>();
            created_node.child_of(head); // set the sequential line graph nodes parented to selector/Head
            created_node.set<GraphNodeData>({edit_node.try_get<GraphNodeData>()->seq+1});
            prev_node.add<Next>(created_node);
            created_node.add<Prev>(prev_node);
            // TODO: EDGE RENDER
            text_entry.remove<TextInput>();


            // TODO: Insert new node sequentially with edge
          }
          else if (XLookupString(&key.event,unicode_input,255,&symbol,0)==1)
          {
            text.word += unicode_input[0];
            const fs::path dir_path{"../library/symbol_repo"};  // Replace with your directory path
            for (const auto& entry : fs::directory_iterator(dir_path)) {
                if (!fs::is_directory(entry))
                {
                  if (text.word == entry.path().filename())
                  {
                    auto symbol_icon = ecs.entity()
                      .is_a(ui_element)
                      .set<Position, Local>({0.0f, -64.0f})
                      .set<Align>({0.5f, 0.5f, 0.5f, 0.5f})
                      .set<SpriteCreator>({entry.path()})
                      .child_of(edit_node);

                    set_hud_scene(symbol_icon);

                    printf("Search for symbol\n");
                    system("python3 features.py");
                    printf("Begin search\n");
                    system((std::string("python3 search.py ../library/symbol_repo/") + std::string(entry.path().stem().string())).c_str());
                    if (symbol_icon.has<AnnotatorHUD>())
                    {
                      printf("Symbol icon has annotator\n");
                      // TODO: Do we see this symbol (or its prefab graph) anywhere on the screen?
                      // If so, render an edge to it...
                    }
                  }

                }
            }
          }
          // 
          // // TODO: Text input...
          // char* keystring = XKeysymToString(symbol);

                    // ------
          edit_node.children([&](flecs::entity child) {
            if (child.has<CircleShape>())
            {
              prev_node_emoji = child;
            }
          });

          prev_node_emoji.children([&](flecs::entity child) {
            child.destruct();
          });

          std::string word = text_entry.try_get<TextCreator>()->word;
          std::transform(word.begin(), word.end(), word.begin(), 
                        [](unsigned char c){ return std::tolower(c); });

          std::cout << word << " word!" << std::endl;
          
          NVGcolor tint_color = nvgRGBA(0xFF, 0xFF, 0xFF, 0xFF);
          if (arc_element_colors_map.count(word))
          {
              tint_color = arc_element_colors_map.at(word);
              text_entry.set<RenderStatus>({true, RenderMode::FILL, tint_color});
              prev_node_emoji.set<RenderStatus>({true, RenderMode::FILL, tint_color});
          } else
          {
              NVGcolor color = nvgRGBA(0xFF, 0xFF, 0xFF, 0xFF);
              text_entry.set<RenderStatus>({true, RenderMode::FILL, tint_color});
              prev_node_emoji.set<RenderStatus>({true, RenderMode::FILL, tint_color});
          }

          if (graph_emoji_keywords.count(word))
          {
            prev_node_emoji.set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(0x22, 0x22, 0x22, 0xFF)});
            prev_node_emoji.set<CircleShape>({16.0f});
            auto sprite_emoji = ecs.entity()
              .is_a(ui_element)
              // .set<Align>({0.5f, 0.5f, 0.5f, 0.5f})
              .set<SpriteCreator>({graph_emoji_keywords[word]})
              .add<LevelHUD>()
              .set<Position, Local>({-12, -12})
              .child_of(prev_node_emoji);
          } else
          {
            prev_node_emoji.set<CircleShape>({7.0f});
          }

          if (component_symbol.count(word))
          {
            auto component_emoji = ecs.entity()
              .is_a(ui_element)
              .set<SpriteCreator>({"../assets/component_hud.png"})
              .add<LevelHUD>()
              .set<Position, Local>({-12, -58})
              .set<SpriteTint>({tint_color})
              .child_of(prev_node_emoji);
          }

          if (relationships.count(word))
          {
            auto component_emoji = ecs.entity()
              .is_a(ui_element)
              .set<SpriteCreator>({"../assets/relationship.png"})
              .add<LevelHUD>()
              .set<Position, Local>({-12, -60})
              .set<SpriteTint>({tint_color})
              .child_of(prev_node_emoji);
          }

          if (numeric_component_symbol.count(word))
          {
            auto component_var = ecs.entity()
              .is_a(ui_element)
              // TODO: Get variable
              .set<TextCreator>({"A", "ATARISTOCRAT"})
              .set<FontStatus>({24.0f, 0.0f})
              .add<LevelHUD>()
              .set<Position, Local>({-4, -46})
              .child_of(prev_node_emoji);
          }

          if (!word.empty() && std::isdigit(static_cast<unsigned char>(word[0])))
          {
            // C (Constant)
            auto component_const = ecs.entity()
              .is_a(ui_element)
              // This is a constant numeric value
              .set<TextCreator>({word, "ATARISTOCRAT"}) // Use the digit as text
              .set<FontStatus>({24.0f, 0.0f})
              .add<LevelHUD>()
              .set<Position, Local>({-8, -46})
              .child_of(prev_node_emoji);
          } else
          {
            // Try to detect vec2 (x, y) format
            std::regex vec2_pattern("\\((-?\\d+(\\.\\d+)?),(-?\\d+(\\.\\d+)?)\\)");
            std::smatch matches;

            if (std::regex_match(word, matches, vec2_pattern))
            {
              // Vec2 detected
              // matches[0] is the whole match, matches[1] is x, matches[3] is y (if you needed them as separate numbers)
              auto component_vec2 = ecs.entity()
                .is_a(ui_element)
                // This is a vec2 value
                .set<TextCreator>({word, "ATARISTOCRAT"}) // Use the entire (x, y) string as text
                .set<FontStatus>({24.0f, 0.0f}) // Adjust font size as needed
                .add<LevelHUD>() // Or a different tag if it's a different type of HUD element
                .set<Position, Local>({-16, -46}) // Adjust position as needed
                .child_of(prev_node_emoji);
            }
          }

          
          // TODO: Detect digits: these are constants
          // TODO: On wildcard: spawn color selector above...
          if (word == "wildcard")
          {
            // int emoji_child_count = 0;
            // prev_node_emoji.children([&](flecs::entity emoji_child)
            // {
            //   emoji_child_count++;
            // });
            // // We don't want to recreate a bunch of these...
            // if (emoji_child_count == 0)
            // {
            // }
            visualize_color_matrix(prev_node_emoji);
          }
          // ------
        // printf("Subquery on key press!\n");
      });

      if (ecs.is_valid(prev_node_emoji) && ecs.is_valid(created_emoji))
      {
        printf("Valid created\n");
        prev_node_emoji.add<Edge>(created_emoji);
        prev_node_emoji.add<EdgeRenderStatus>();
      }
    });

    // ecs.observer<MoveGraph, X11CursorMovement>("CursorMoveSelectedGraph").term_at(1).src(user_input).event(flecs::OnSet).iter([](flecs::iter& it, MoveGraph* graph, X11CursorMovement* cursor) 
    // {
    //   printf("Observe cursor movement of graph\n");
    // });

    // ---- Scene graph implementation

    // void SceneGraphSettingsCascadeHierarchy(ecs_iter_t *it) {
    //     // printf("SceneGraphSettingsCascadeHierarchy\n");
    //     SceneGraph* sc_parent = ecs_field(it, SceneGraph, 1);
    //     SceneGraph *sc = ecs_field(it, SceneGraph, 2);
    //     // Renderable* settings = ecs_field(it, Renderable, 3);
    //     for (int i = 0; i < it->count; i++) 
    //     {
    //         if (sc_parent)
    //         {
    //             log_trace("%s's parent is %d expanded\n", ecs_get_name(it->world, it->entities[i]), sc_parent->is_expanded);
    //             if (!sc[i].user_mark_expanded)
    //             {
    //                 if (sc_parent->user_mark_expanded == false)
    //                 {
    //                     sc[i].is_expanded = false;
    //                 } else
    //                 {
    //                     sc[i].is_expanded = sc_parent->is_expanded;
    //                 }
    //             } else
    //             {
    //                 if (sc_parent->user_mark_expanded == false)
    //                 {
    //                     sc[i].is_expanded = false;
    //                 } else
    //                 {
    //                     sc[i].is_expanded = sc_parent->is_expanded;
    //                 }
    //             }
    //         } else 
    //         { 
    //             sc[i].is_expanded = true;
    //         }
    //         // printf("%s is %d expanded\n", ecs_get_name(it->world, it->entities[i]), sc[i].is_expanded);
    //         // settings[i].visible = sc[i].is_expanded;
    //     }
    // }

		ecs.system<SceneGraph, SceneGraph*>("ExpandCascadeHierarchy").term_at(1).parent().cascade().optional()
		    .each([](SceneGraph& sg, SceneGraph* sg_parent)
		    {
		        if (!sg_parent)
		        {
		            sg.is_expanded = sg.user_mark_expanded;
		        }
		        else if (!sg.user_mark_expanded)
		        {
		            sg.is_expanded = false;
		        } else
		        {
		            sg.is_expanded = sg_parent->is_expanded;
		        }
		        sg.is_visible = !sg_parent || sg_parent->is_expanded;
		    });

		ecs.system<SceneGraph>("UpdateSceneGraphVisibility")
		    .each([](flecs::entity e, SceneGraph& scene_graph)
		    {
		      auto sg_text = e.lookup("sg_viz::sg_text");
		      sg_text.ensure<RenderStatus>().visible = scene_graph.is_visible;
		      auto arrow = e.lookup("sg_viz::scope_arrow");
		      if (arrow.is_valid())
		      {
		        arrow.ensure<RenderStatus>().visible = scene_graph.is_visible;
		      }
		    });

    // ecs.system<SceneGraphRoot>("LayoutSceneGraph").write(flecs::Wildcard).iter([](flecs::iter& it, SceneGraphRoot* root) 
    // {
    //   for (size_t i : it)
    //   {
    //     flecs::entity sg_root = it.entity(i);
    //     int visible_index = 0;
    //     sg_root.children([&](flecs::entity child)
    //     {
    //       const SceneGraph* scene_graph = child.try_get<SceneGraph>();
    //       if (scene_graph->is_visible)
    //       {
    //         visible_index++;
    //       }
    //       Position* pos = child.get_mut<Position, Local>();
    //       pos->y = (float)(visible_index*14.0f);
    //     });
    //   }
    // });

    // ecs.system<SceneGraphRoot>("LayoutSceneGraph").write(flecs::Wildcard).iter([](flecs::iter& it, SceneGraphRoot* root) 
    // {
    //   for (size_t i : it)
    //   {
    //     flecs::entity sg_root = it.entity(i);
    //     int visible_index = 0;
    //     sg_root.children([&](flecs::entity child)
    //     {
    //       const SceneGraph* scene_graph = child.try_get<SceneGraph>();
    //       if (scene_graph->is_visible)
    //       {
    //         visible_index++;
    //       }
    //       Position* pos = child.get_mut<Position, Local>();
    //       pos->y = (float)(visible_index*14.0f);
    //     });
    //   }
    // });

    // void UpdateArrowDirection(ecs_iter_t* it)
    // {
    //     SceneGraph* sc = ecs_field(it, SceneGraph, 1);
    //     Position* pos = ecs_field(it, Position, 2);
    //     Sprite* sprite = ecs_field(it, Sprite, 3);
    //     ArrowStatus* status = ecs_field(it, ArrowStatus, 4);
    //     SDL_Interface* sdl = ecs_field(it, SDL_Interface, 5);

    //     for (int i = 0; i < it->count; i++)
    //     {
    //         if (sc->children_count)
    //         {
    //             if (sc->user_mark_expanded)
    //             {
    //                 if (status[i].scope == REDUCED)
    //                 {
    //                     status[i].scope = EXPANDED;
    //                     // Switch pos/sprite to down arrow
    //                     Sprite update = loadSprite(sdl->renderer, "../res/arrow_down.png");
    //                     ecs_set(it->world, it->entities[i], Sprite, {update.texture, update.width, update.height});
    //                     ecs_set_pair(it->world, it->entities[i], Position, Local, {-11 , 6});
    //                 }
    //             } else
    //             {
    //                 if (status[i].scope == EXPANDED)
    //                 {
    //                     status[i].scope = REDUCED;
    //                     // Switch pos/sprite to right arrow
    //                     Sprite update = loadSprite(sdl->renderer, "../res/arrow_right.png");
    //                     ecs_set(it->world, it->entities[i], Sprite, {update.texture, update.width, update.height});
    //                     ecs_set_pair(it->world, it->entities[i], Position, Local, {-9 , 4});

    //                 }
    //             }
    //         }
    //     }
    // }

    // void UpdateSceneGraphLines(ecs_iter_t* it)
    // {
    //     SceneGraph* sc = ecs_field(it, SceneGraph, 1);
    //     // Line* l = ecs_field(it, Line, 2);

    //     for (int i = 0; i < it->count; i++)
    //     {
    //         if (sc[i].children_count)
    //         {
    //             ecs_entity_t next = sc[i].next;
    //             int expanded_children_count = 0;
    //             int x = 0;
    //             while (ecs_is_valid(it->world, next) && ecs_is_alive(it->world, next) && x < sc[i].children_count)
    //             {
    //                 SceneGraph* scNext = ecs_get_mut(it->world, next, SceneGraph);
    //                 next = scNext->next;
    //                 if (scNext->is_expanded)
    //                 {
    //                     expanded_children_count++;
    //                 }
    //                 x++;
    //             }
    //             if (ecs_is_valid(it->world, it->entities[i]))
    //             {
    //                 if (expanded_children_count == 0)
    //                 {
    //                     ecs_remove(it->world, it->entities[i], Line);
    //                 } else
    //                 {
    //                     ecs_set(it->world, it->entities[i], Line, {-11, 16, -11, 16 + expanded_children_count*12-8});
    //                 }
    //             }
    //         }
    //     }
    // }

    // void SetupSelectedNodeIndicator(ecs_iter_t* it)
    // {
    //     Box* b = ecs_field(it, Box, 2);
    //     SceneGraph* sc = ecs_field(it, SceneGraph, 3);
    //     Text* text = ecs_field(it, Text, 4);

    //     for (int i = 0; i < it->count; i++)
    //     {
    //         int txtWidth = 0;
    //         int txtHeight = 0;
    //         SDL_QueryTexture(text->texture, NULL, NULL, &txtWidth, &txtHeight);
    //         b[i].w = txtWidth+4;
    //         b[i].h = txtHeight-1;
    //     }
    // }


    // ecs.observer<X11KeyPress, SceneGraphSelection>("KeyNavSceneGraph").term_at(0).src(user_input).term_at(1).second(flecs::Wildcard).event(flecs::OnSet).iter([display](flecs::iter& it, X11KeyPress* key, SceneGraphSelection* selection) 
    // {
    //   printf("Key pressed SCENE GRAPH NAV!\n");
    //   KeySym symbol = XKeycodeToKeysym(display, key->event.keycode, 0);
    //   if (symbol == XK_Up || symbol == XK_w)
    //   {
    //     printf("Key press up!\n");
    //     auto selected_sg_node = it.entity(0).target<SceneGraphSelection>();
    //     if (selected_sg_node.has<Prev>(flecs::Wildcard))
    //     {
    //       printf("Selection prev!\n");
    //       flecs::entity prev_sg_node = selected_sg_node.target<Prev>();
    //       it.entity(0).remove<SceneGraphSelection>();
    //       it.entity(0).add<SceneGraphSelection>(prev_sg_node);
    //     }
    //   }
    // });

    flecs::query<SceneGraphSelection> q_sgs = ecs.query_builder<SceneGraphSelection>()
        .write(flecs::Wildcard)
        .build();

		ecs.observer<SceneGraphSelection, SceneGraphSelectionChanged>("GenerateSceneGraphIntention")
		    .write(flecs::Wildcard)
		    .event(flecs::OnSet)
		    .each([hud, ui_element](SceneGraphSelection& sgs, SceneGraphSelectionChanged& update)
		    {
		      if (sgs.active && sgs.selected_node.has<IntentExpression>())
		      {
		        flecs::entity output[6];
		        if (sgs.context_graph.is_valid())
		        {
		          sgs.context_graph.child_of(sgs.selected_node);
		        }
		        else
		        {
		          spawn_graph(hud, ui_element, 100.0f, 0.0f, &output[0], false);
		          output[0].child_of(sgs.selected_node);
		          output[1].set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 255, 0, 255)});
		          output[4].set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(255, 255, 0, 255)});
		          output[5].set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(255, 255, 0, 255)});
		          sgs.context_graph = output[0];
		        }
		        // output[0].set<Position, Local>({100.0f, 0.0f}); // TODO: Get bounds
		      }
		    });

    flecs::query<GraphReader> q_graphreader = ecs.query_builder<GraphReader>()
        .write(flecs::Wildcard)
        .build();

		ecs.observer<X11KeyPress>("KeyNavGraphReader").write(flecs::Wildcard).event(flecs::OnSet)
    .each([display, q_graphreader](X11KeyPress& key)
    {
      KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
      q_graphreader.each([&](flecs::entity e, GraphReader& stream_graph)
      {
        if (symbol == XK_Right && stream_graph.active_page < stream_graph.book_data.pages.size()-1)
        {
          std::cout << "Turn page right" << std::endl;
          e.set<UpdateGraphReaderPageEvent>({stream_graph.active_page + 1});
          auto sound_effect = ecs.entity()
            .set<CreateSFX>({"../assets/page_turn_sfx.wav"});
        }
        else if (symbol == XK_Left && stream_graph.active_page > 0)
        {
          std::cout << "Turn page left" << std::endl;
          e.set<UpdateGraphReaderPageEvent>({stream_graph.active_page - 1});
          auto sound_effect = ecs.entity()
            .set<CreateSFX>({"../assets/page_turn_sfx.wav"});
        }
      });
    });

	ecs.observer<X11KeyPress>("KeyNavSceneGraph").write(flecs::Wildcard).event(flecs::OnSet)
    .each([display, q_sgs](flecs::entity e, X11KeyPress& key)
    {
      KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
      q_sgs.each([&](flecs::entity sg_ent, SceneGraphSelection& sgs) {
        if (sgs.active)
        {
          flecs::entity selected_sg_node = sgs.selected_node;
          if (symbol == XK_Up) //  || symbol == XK_w
          {
            if (selected_sg_node.has<Prev>(flecs::Wildcard)) // TODO: Navigation through reduced
            {
              flecs::entity prev_sg_node = selected_sg_node.target<Prev>();
              sgs.selected_node = prev_sg_node;
              sg_ent.set<SceneGraphSelectionChanged>({0});
            }
          }
          else if (symbol == XK_Down) // || symbol == XK_s
          {
            if (selected_sg_node.has<Next>(flecs::Wildcard))
            {
              flecs::entity next_sg_node = selected_sg_node.target<Next>();
              sgs.selected_node = next_sg_node;
              sg_ent.set<SceneGraphSelectionChanged>({0});
            }
          }
          else if (symbol == XK_Right || symbol == XK_d)
          {
            sgs.selected_node.ensure<SceneGraph>().user_mark_expanded = true;
            // Update arrow here...
            flecs::entity arrow = sgs.selected_node.lookup("sg_viz::scope_arrow");
            if (arrow.is_valid())
            {
              arrow.set<SpriteCreator>({"../assets/arrow_down.png"});
            }
          }
          else if (symbol == XK_Left || symbol == XK_a)
          {
            sgs.selected_node.ensure<SceneGraph>().user_mark_expanded = false;
            flecs::entity arrow = sgs.selected_node.lookup("sg_viz::scope_arrow");
            if (arrow.is_valid())
            {
              arrow.set<SpriteCreator>({"../assets/arrow_right.png"});
            }
          }
        }
      });
    });

    // void MouseWheelNavSceneGraph(ecs_iter_t* it)
    // {
    //     SceneGraph* sc = ecs_field(it, SceneGraph, 1);
    //     EventMouseWheel* event = ecs_field(it, EventMouseClick, 3);

    //     if (event->y < 0)
    //     {
    //         for (int i = 0; i < it->count; i++)
    //         {
    //             if (ecs_is_valid(it->world, sc[i].next))
    //             {
    //                 ecs_add_pair(it->world, it->entities[i], EcsChildOf, sc[i].next);
    //             } else
    //             {
    //                 printf("%s next is not valid\n", ecs_get_name(it->world, it->entities[i]));
    //             }
    //         }
    //     }
    //     else if (event->y > 0) 
    //     {
    //         for (int i = 0; i < it->count; i++)
    //         {
    //             if (ecs_is_valid(it->world, sc[i].prev))
    //             {
    //                 ecs_add_pair(it->world, it->entities[i], EcsChildOf, sc[i].prev);
    //             } else
    //             {
    //                 printf("%s prev is not valid\n", ecs_get_name(it->world, it->entities[i]));
    //             }
    //         }
    //     }
    // }
    // ----

    ecs.system<Position, TrackCursor, X11CursorMovement>("TrackCursor").term_at(0).second<Local>().term_at(2).src(user_input).kind(flecs::OnUpdate)
    .each([](Position& pos, TrackCursor& track, X11CursorMovement& cursor)
    {
      pos.x = cursor.x;
      pos.y = cursor.y;
    });

		ecs.system<Position, Position*, GraphIsGrabbed, X11CursorMovement>("CursorMoveSelectedGraph").term_at(0).second<Local>().term_at(1).up().optional().second<World>().term_at(3).src(user_input).kind(flecs::OnUpdate)
    .each([](Position& pos, Position* parent_world_pos, GraphIsGrabbed& graph, X11CursorMovement& cursor)
    {
      if (parent_world_pos)
      {
        pos.x = cursor.x - parent_world_pos->x;
        pos.y = cursor.y - parent_world_pos->y;
      }
      else
      {
        pos.x = cursor.x;
        pos.y = cursor.y;
      }
    });

		ecs.system<UIElementBounds, RenderStatus, SymbolBoundAnnotation, X11CursorMovement>("HoverAnnotation").term_at(3).src(user_input).kind(flecs::OnUpdate)
		.each([](flecs::entity e, UIElementBounds& bounds, RenderStatus& render, SymbolBoundAnnotation& sboa, X11CursorMovement& cursor) {
			c2v cursor_pos {cursor.x, cursor.y};
			c2AABB box = {{bounds.xmin, bounds.ymin}, {bounds.xmax, bounds.ymax}};
			if (c2AABBtoPoint(box, cursor_pos)) {
				auto c = render.color;
				render.color = nvgRGBA(255, 0, 0, 255);
			}
		});

    // ecs.system<RainbowPlane, BoxShape>("RainbowAirstrike").iter([](flecs::iter& it, RainbowPlane* rainbow, BoxShape* box) 
    // {
    //   for (size_t i : it)
    //   {
    //     rainbow[i].velocity+= it.delta_time() * rainbow[i].acceleration_rate;
    //     rainbow[i].height += it.delta_time() * rainbow[i].velocity;
    //     box[i].height = rainbow[i].height;
    //   }
    // });

		ecs.system<Bode, BookmarkResponder, Position, Position, CursorState>("DescentAndromeda").term_at(1).parent().term_at(2).second<World>().parent().term_at(3).parent().second<Local>().term_at(4).src(user_input)
			.each([](flecs::entity e, Bode& bode, BookmarkResponder& bookmark, Position& world_pos, Position& pos, CursorState& cursor)
			{
				if (cursor.x > world_pos.x && cursor.x < world_pos.x + bode.width)
				{
					if (cursor.y < 96)
					{
						bode.node_edge_slider = (cursor.x - world_pos.x) / bode.width;
						float edge_margin = (bode.curve_radius + bode.node_slot_radius) / bode.width;
						if (bode.node_edge_slider < edge_margin) bode.node_edge_slider = edge_margin;
						if (bode.node_edge_slider > 1.0f - edge_margin) bode.node_edge_slider = 1.0f - edge_margin;
						bode.height = cursor.y;
						e.parent().lookup("ux_node").ensure<Position, Local>().y = cursor.y;
					}
				} else
				{
					e.parent().lookup("ux_node").ensure<Position, Local>().y = 40.0f;
					bode.height = 40.0f;
					bode.node_edge_slider = 0.5f;
				}
				e.parent().lookup("ux_node").ensure<Position, Local>().x = bode.node_edge_slider * bode.width;
				if (cursor.y < 96) // TODO: Add mouse cursor is on an upward velocity towards bodes
				{
					pos.y = 0.0f;
				} else
				{
					bode.node_edge_slider = 0.5f;
					pos.y = -28.0f;
				}
			});

		ecs.system<RainbowPlane, Bode>("OpticsAirstrike")
			.each([](flecs::entity e, RainbowPlane& rainbow, Bode& bode) {
				float dt = e.world().delta_time();
				rainbow.velocity += dt * rainbow.acceleration_rate;
				rainbow.height += dt * rainbow.velocity;
				if (rainbow.height > 1440) {
					rainbow.height = 1440;
				}
				bode.height = rainbow.height;
				float max_rainbow_curve = (bode.width - bode.curve_radius/2)/2.0f;
				if (bode.node_slot_radius > max_rainbow_curve) {
					bode.node_slot_radius = max_rainbow_curve;
				}
			});

		ecs.observer<SocketDataReceived>().write(flecs::Wildcard).event(flecs::OnSet)
		.each([](const SocketDataReceived& data)
		{
		  std::string message = std::string(data.buffer);
		  std::cout << "Received: " << message << std::endl;
		  auto slhf_interface = ecs.lookup("sort_learning_from_human_feedback");
		  SortCol& sort_col = slhf_interface.ensure<SortCol>();
		  if (message == "next_stage")
		  {
			sort_col.sort_index += 1;
			slhf_interface.set<SaveProgramSort>({sort_col.sort_index, false});
			slhf_interface.set<ReloadCol>({1, sort_col.sort_index});
		  }
		  if (message == "init_sort")
		  {
			slhf_interface.set<SaveProgramSort>({sort_col.sort_index, false});
			slhf_interface.set<ReloadCol>({1, sort_col.sort_index});
		  }
		  slhf_interface.set<ReloadCol>({4, sort_col.sort_index+1});
		});

	ecs.system<UnixSocket>("ReadSockets")
	.each([](flecs::entity e, UnixSocket& s) {
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(s.fd, &read_fds);

		// Wait for incoming connections using select with a timeout of 0
		struct timeval timeout = {0, 0};
		int ret = select(s.fd + 1, &read_fds, NULL, NULL, &timeout);
		if (ret == -1) {
			perror("select");
			return;
		}

		if (FD_ISSET(s.fd, &read_fds)) {
			// Accept the incoming connection
			int connfd = accept(s.fd, NULL, NULL);
			if (connfd == -1) {
				// perror("accept");
				return;
			}

			// Handle the connection...
			SocketDataReceived data;
			int bytes_read = read(connfd, data.buffer, 256);
			if (bytes_read == -1) {
				perror("read");
				return;
			}

			// Null-terminate the buffer
			data.buffer[bytes_read] = '\0';
			data.connfd = connfd;
			e.set<SocketDataReceived>(data);

			SocketConnection conn;
			conn.connfd = connfd;
			e.set<SocketConnection>(conn);
		}
	});

		ecs.system<Position, GraphRail, SlideGraphRail, GraphAudio, GraphSpeak, X11CursorMovement>("CursorSlideGraphRail").term_at(0).second<Local>().term_at(3).parent().term_at(4).parent().term_at(5).src(user_input).kind(flecs::OnUpdate)
		.each([](Position& pos, GraphRail& rail, SlideGraphRail& graph, GraphAudio& graph_audio, GraphSpeak& graph_speak, X11CursorMovement* cursor)
		{
			graph_audio.music->pause();
			printf("Cursor move slide graph rail!\n");
			pos.x = std::min(0.0f, pos.x - cursor->x_diff); // TODO: Max or init new speech_graph
			graph_speak.progress = (-pos.x/rail.x_per_second);
		});

		ecs.system<EventOnGrab, RenderStatus, DynamicConnector, GraphEditor>("HighlightTelos").term_at(0).second<ConnectGraph>().term_at(3).src(user_input)
			.each([](EventOnGrab& on_grab, RenderStatus& render, DynamicConnector& dynamic_connector, GraphEditor& editor) {
				bool visible = editor.edit_mode == GraphEditMode::CRAFT_TELOS;
				render.visible = visible;
			});

		ecs.system<ReadGraph>("ReadGraph").write(flecs::Wildcard)
		.each([&](flecs::entity root, ReadGraph& read_graph) {
			std::string read;
			if (root.has<BookGraphSelection>()) {
				auto q = ecs.query<GraphData, BookGraph>();
				q.each([&](flecs::entity e, GraphData& graph_data, BookGraph& book) {
					read = get_graph_selection_string(e);
				});
			} else {
				read = graph_to_string(root);
			}
			std::string ux_query = std::string("python3 tts_style.py \"") + read + std::string("\"");
			system(ux_query.c_str());
			// ecs.lookup("read_bode").remove<Edge>(flecs::Wildcard);
			// read_graph[i].edge_source.remove<Edge>(root);
			read_graph.edge_source.remove<Edge>(flecs::Wildcard);
			// root.target<Edge>().add<ReadGraph>(read_graph[i]);
			root.remove<ReadGraph>();
			// root.destruct();
		});

    // ecs.system<AttemptLoadBook>("AttemptLoadBook").write(flecs::Wildcard).iter([](flecs::iter& it, AttemptLoadBook* attempt_load)
    // {
    //   for (auto i : it)
    //   {
    //     // AttemptLoadBook should be added to the sengra indicating the book name
    //     flecs::entity root = it.entity(i);
    //     std::cout << "Attempt to load book: " << graph_to_string(root) << std::endl;
    //     std::string ux_query = std::string("python3 ux_query.py \"") + graph_to_string(root) + std::string("\"");
    //     system(ux_query.c_str());
        
    //     std::ifstream file("book.txt");
    //     std::string bookTitle;
    //     std::getline(file, bookTitle);
    //     file.close();

    //     std::cout << "Read:" << bookTitle << std::endl;
    //     flecs::entity load_book = ecs.entity()
    //       .set<LoadBookEvent>({bookTitle});
    //     root.remove<AttemptLoadBook>();
    //     root.destruct();
    //   }
    // });

		ecs.system<GenPrefabEntity>("GenPrefabEntity").write(flecs::Wildcard)
		.each([](flecs::entity root) {
		  std::cout << graph_to_string(root) << std::endl;
		  std::string ux_query = std::string("python3 gen_prefab.py \"") + graph_to_string(root) + std::string("\"");
		  printf("%s\n", ux_query.c_str());
		});

    // ecs.observer<X11KeyPress>().term_at(2).src(user_input).event(flecs::OnSet).iter([](flecs::iter& it, TextCreator* text, TextInput* input, X11KeyPress* key)
    // {
    //   printf("Enter key into text input\n");
    //   text->word += key->event.keycode;
    // });    

    DualQuery q_dual = {ecs.query<UIElementBounds>(), ecs.query<GraphIsGrabbed>()};
    //.write<RenderStatus>().write<MoveGraph>().
    // TODO: Eventually refactor into multiple systems...
    ecs.system<Overlay, KeyboardState, HorizontalBoundaryChisel, SpanChiselBounds>("CreateGraphNode").term_at(1).src(user_input).term_at(2).src(chisel).term_at(3).src(chisel_enter_highlight).write(flecs::Wildcard).ctx(&q_dual).kind(flecs::PreUpdate)
    .run([ui_element, user_input](flecs::iter& it, Overlay* sail, KeyboardState* keyboard, HorizontalBoundaryChisel* chisel, SpanChiselBounds* chisel_bounds) 
    {
      // printf("Create graph node\n");
      UI_Query q = it.ctx<DualQuery>()->ui; 
      XEvent event;
      while(XEventsQueued(sail->display, QueuedAlready))
      {
        // printf("X has pending events\n");
        XNextEvent(sail->display, &event);
        // https://tronche.com/gui/x/xlib/events/types.html
        // if (event.type == MotionNotify)
        if (event.type == ButtonRelease)
        {
          flecs::query<GraphIsGrabbed> mq = it.ctx<DualQuery>()->move;
          mq.run([&](flecs::iter& sit, GraphIsGrabbed* graph) 
          {
            for (size_t x : sit)
            {
              sit.entity(x).remove<GraphIsGrabbed>();
            }
          });
          auto q_rail = ecs.query<GraphRail, flecs::pair<Position, Local>>();
          q_rail.each([](flecs::entity r_entity, GraphRail& rail, Position& pos)
          {

            float progress = -pos.x/rail.x_per_second;
            GraphAudio& ga = r_entity.parent().ensure<GraphAudio>();
            ga.music->setPlayingOffset(sf::seconds(progress));
            float actual = ga.music->getPlayingOffset().asSeconds();
            r_entity.parent().ensure<GraphSpeak>().progress = actual;
            r_entity.remove<SlideGraphRail>();
          });
          if (event.xbutton.button == 1)
          {
            auto ui = it.world().lookup("user_input");
            if (ui.try_get<GraphEditor>()->edit_mode == GraphEditMode::CRAFT_TELOS) // TODO: Should graph selection be in graph sculpt or craft telos? Bounding box over graphs for snapping...
            {
              // TODO: 'Lock' anchor_bound/selection_bound or reset chisel
              // auto chisel = ecs.lookup("chisel").get_mut<HorizontalBoundaryChisel>();
              // TODO: If reclick on anchor node snap, unlock anchor...
              if (chisel->lock_anchor)
              {
                chisel->lock_selection = true;
                float start_x = std::min(chisel_bounds->anchor.try_get<Position, Local>()->x, chisel_bounds->selection.try_get<Position, Local>()->x);
                float sel_width = abs(chisel_bounds->anchor.try_get<Position, Local>()->x - chisel_bounds->selection.try_get<Position, Local>()->x);
                float end_x = start_x + sel_width;
                auto selected_graph_scope = ecs.entity()
                  .is_a(ui_element)
                  .set<LineShape>({sel_width, 0.0f, 1})
                  .set<Position, Local>({start_x, -64.0f})
                  .add<SelectionSpanIndicator>();
                auto selected_graph_scope_start = ecs.entity()
                  .is_a(ui_element)
                  .set<LineShape>({0, 64.0f, 1})
                  .set<Position, Local>({start_x, -64.0f})
                  .add<SelectionStartIndicator>();
                auto selected_graph_scope_end = ecs.entity()
                  .is_a(ui_element)
                  .set<LineShape>({0, 64.0f, 1})
                  .set<Position, Local>({end_x, -64.0f})
                  .add<SelectionEndIndicator>();
                auto selected_scope_hub = ecs.entity()
                  .is_a(ui_element)
                  .set<Position, Local>({start_x + (sel_width/2.0f), -64.0f})
                  .set<CircleShape>({11.0f})
                  // .add<EventOnGrab, ConnectGraph>()
                  .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255,0,0,255)});
                auto graph_telos_indicator = ecs.entity()
                  .is_a(ui_element)
                  .set<DiamondShape>({8.0f})
                  .add<DynamicConnector>()
                  .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255,0,0,255)})
                  .add<LevelHUD>()
                  .add<EventOnGrab, ConnectGraph>()
                  .child_of(selected_scope_hub);
                auto q_rail = ecs.query<GraphRail>();
                q_rail.each([&](flecs::entity e, GraphRail& rail)
                {
                  // TODO: Prefab hierarchy?
                  selected_graph_scope.child_of(e);
                  selected_graph_scope_start.child_of(e);
                  selected_graph_scope_end.child_of(e);
                  selected_scope_hub.child_of(e);
                });
                // TODO: Generate selection node
              }
            }
          }
        }
        printf("X11 event, type %i, pointer %i, %i state %i, keycode %i\n", event.type, event.xbutton.x, event.xbutton.y, event.xbutton.state, event.xbutton.button);
        if (event.xbutton.button == 4) // scroll up
        {
          auto ui = it.world().lookup("user_input");
          ui.set<X11FlecsScrollEvent>({-1});
        }
        if (event.xbutton.button == 5) // scroll down
        {
          auto ui = it.world().lookup("user_input");
          ui.set<X11FlecsScrollEvent>({1});
        }
        if (event.xbutton.button == 6) // scroll right
        {
          auto ui = it.world().lookup("user_input");
          ui.set<X11HorizontalScrollEvent>({-1});
        }
        if (event.xbutton.button == 7) // scroll left
        {
          auto ui = it.world().lookup("user_input");
          ui.set<X11HorizontalScrollEvent>({1});
        }
        if (event.type == ButtonPress && event.xbutton.button == 2)
        {
          auto ui = it.world().lookup("user_input");
          auto q_rail = ecs.query<GraphRail>();
          q_rail.each([](flecs::entity e, GraphRail& rail)
          {
            printf("Added slide graph rail!\n");
            e.add<SlideGraphRail>();
          });
        }

        if (event.type == ButtonRelease && event.xbutton.button == 1)
        {
          auto ui = it.world().lookup("user_input");
          ui.remove<DragNodeSelector>();
          std::cout << "Release event!" << std::endl;
          auto graph_rts = ecs.lookup("graph_RTS");
          const DragBoundGraphSelector* dbgs = graph_rts.try_get<DragBoundGraphSelector>();
          const Position* rts_pos = graph_rts.try_get<Position, Local>();
          const BoxShape* rts_box = graph_rts.try_get<BoxShape>();
          bool select = dbgs->active;
          if (dbgs->mode == DragBoundMode::ZONE)
          {
            // TODO: Interface for mapping bounds to graphs...
            auto software_perception_stream = ecs.lookup("software_perception_stream");
            auto software_pos = software_perception_stream.get_mut<Position, Local>();

            flecs::entity virt_desktop_bound = ecs.entity()
              .is_a(ui_element)
              .set<Position, Local>(*rts_pos) // {rts_pos->x-software_pos->x, rts_pos->y-software_pos->y} 
              .set<BoxShape>(*rts_box)
              .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 255, 255, 128)});
              // .child_of(software_perception_stream);
          }
          c2AABB selection_box = {{rts_pos->x, rts_pos->y}, {rts_pos->x + rts_box->width, rts_pos->y + rts_box->height}};
          graph_rts.set<DragBoundGraphSelector>({false, -1.0f, 0.0f});

          auto hud = ecs.lookup("hud");
          if (hud.has<Background>())
          {
            // remove_forebode_graph();
            std::vector<flecs::entity> overlap_elements;

            if (select)
            {
              std::cout << "Select is true!" << std::endl;
              auto q_bounds = ecs.query_builder<UIElementBounds, GraphData>()
                .build();
                
              q_bounds.each([&](flecs::entity e, UIElementBounds& bounds, GraphData& graph) 
              {
                c2AABB box = {{bounds.xmin, bounds.ymin}, {bounds.xmax, bounds.ymax}};
                if (c2AABBtoAABB(box, selection_box))
                {
                  std::cout << "Overlap drag entity" << std::endl;
                  // hit.get_mut<RenderStatus>()->color = nvgRGBA(255, 0, 0, 255);
                  overlap_elements.push_back(e);
                }
              });
              for (flecs::entity e : overlap_elements)
              {
                e.destruct();
              }
              overlap_elements.clear();

            } else
            {
              c2v click {(float)event.xbutton.x, (float)event.xbutton.y};
              // For each Bounds which should detect click...
              // TODO: Make this more efficient
              q.each([&](flecs::entity e, UIElementBounds& bounds)
              {
                  c2AABB box = {{bounds.xmin, bounds.ymin}, {bounds.xmax, bounds.ymax}};
                  if (c2AABBtoPoint(box, click))
                  {
                    // hit.get_mut<RenderStatus>()->color = nvgRGBA(255, 0, 0, 255);
                    overlap_elements.push_back(e);
                  }
              });
            }

            auto ui = it.world().lookup("user_input");
            // Trigger corresponding observer event, such as a selection of a graph
            int overlap_event_count = 0;
            if (overlap_elements.size() > 0)
            {
              for (flecs::entity overlap_ui_element : overlap_elements)
              {
                // Everything wrapped prior is needs to be refactored into utility function
                if (overlap_ui_element.has<flecs::pair<EventOnGrab, ConnectGraph>>())
                {
                  printf("Released telos source onto connection!\n");
                  auto telos_source = it.world().lookup("telos_source");
                  if (telos_source.is_valid() && telos_source.is_alive())
                  {
                    std::cout << "TELOS SOURCE IS VALID" << std::endl;
                    flecs::entity from = telos_source.ensure<TrackCursor>().from;
                    if (from.is_valid() && from != overlap_ui_element)
                    {
                      std::cout << "TELOS SOURCE TRACK CURSOR FROM IS VALID" << std::endl;
                      flecs::entity interaction_graph_root = overlap_ui_element.parent().parent();

                      if (interaction_graph_root.has<NodeType>() && (interaction_graph_root.try_get<NodeType>()->ecs == ECS_NodeType::LINUX || interaction_graph_root.try_get<NodeType>()->ecs == ECS_NodeType::LINUX))
                      {
                        std::cout << "Interaction with Linux virt desk" << std::endl;
                        auto from_graph_root = from.parent().parent();
                        if (from_graph_root.has<NodeType>() && from_graph_root.try_get<NodeType>()->ecs == ECS_NodeType::PYTHON_SCRIPT)
                        {
                          std::cout << "Call forth the Droid Knight!" << std::endl;
                          // TODO: Support drag and drop over virt desktop rig?
                          // std::string ux_query = std::string("python3 droid_knight.py \"") + graph_to_string(from_graph_root) + std::string("\" &");
                          std::string ux_query = std::string("python3 droid_knight.py create_cube");
                          system(ux_query.c_str());
                        }
                      }

                      if (from.has<EventOnConnect>(flecs::Wildcard))
                      {
                        std::cout << "HAS EVENT ON CONNECT" << std::endl;
                        // TODO: Use slots boy

                        if (from.has<EventOnConnect, ReadGraph>())
                        {
                          interaction_graph_root.set<ReadGraph>({from});
                        }

                        if (from.has<EventOnConnect, AttemptLoadBook>())
                        {
                          interaction_graph_root.add<AttemptLoadBook>();
                        }

                        if (from.has<EventOnConnect, OpenECS_SceneGraph>())
                        {
                          std::cout << "EVENT ON CONNECT" << std::endl;
                          // hud.get_mut<SceneGraphGraphHUD>()->to_select = from.try_get<SceneGraphLoader>()->graph_to_load;
                          // overlap_ui_element.parent().parent().add<OpenECS_SceneGraph>();
                          // auto graph_to_load = from.target<SceneGraphLoader>();
                          auto sound_effect = ecs.entity()
                            .set<CreateSFX>({getRandomFileFromDir("../assets/think")});
                          ECS_NodeType node_type_int = ECS_NodeType::STANDARD;
                          if (interaction_graph_root.has<NodeType>())
                          {
                            NodeType& node_type = interaction_graph_root.ensure<NodeType>();
                            node_type_int = node_type.ecs;
                          }
                          auto graph_to_load = from.ensure<SceneGraphAmbLoader>().graphs[node_type_int];
                          std::cout << "Update graph to load: " << graph_to_load << std::endl;
                          interaction_graph_root.set<LoadSceneGraph>({graph_to_load, from, overlap_ui_element});
                        }

                      } else
                      {
                        NVGcolor telos_color = nvgRGBA(255, 0, 128, 255);
                        auto edge_telos_indicator = ecs.entity()
                          .is_a(ui_element)
                          .set<DiamondShape>({6.0f})
                          .add<DynamicConnector>()
                          .set<RenderStatus>({true, RenderMode::STROKE, telos_color})
                          .add<LevelHUD>()
                          .set<TrackEdge>({from, overlap_ui_element})
                          .add<EventOnGrab, ConnectGraph>();
                      }
                      from.remove<Edge>(telos_source);
                      from.add<Edge>(overlap_ui_element);
                      telos_source.destruct();
                    }
                  }
                }
              }
            }
          }
          auto telos_source = it.world().lookup("telos_source");
          if (telos_source.is_valid())
          {
            telos_source.ensure<TrackCursor>().from.remove<Edge>(telos_source);
            telos_source.destruct();
          }
        }

        if (event.type == ButtonPress && event.xbutton.button == 1)
        {
          auto hud = ecs.lookup("hud");
          if (hud.has<Background>())
          {
            std::vector<flecs::entity> overlap_elements;
            c2v click {(float)event.xbutton.x, (float)event.xbutton.y};
            // For each Bounds which should detect click...
            // TODO: Make this more efficient
            q.each([&](flecs::entity e, UIElementBounds& bounds) 
            {
              c2AABB box = {{bounds.xmin, bounds.ymin}, {bounds.xmax, bounds.ymax}};
              if (c2AABBtoPoint(box, click))
              {
                // hit.get_mut<RenderStatus>()->color = nvgRGBA(255, 0, 0, 255);
                printf("Overlap hit!\n");
                overlap_elements.push_back(e);
              }
            });

            auto ui = it.world().lookup("user_input");
            // Trigger corresponding observer event, such as a selection of a graph
            int overlap_event_count = 0;
            bool deploy_virt_bound = false;
            if (overlap_elements.size() > 0)
            {
              for (flecs::entity overlap_ui_element : overlap_elements)
              {
                if (overlap_ui_element.has<flecs::pair<EventOnGrab, SelectGraphNode>>())
                {
                  // Deselect all nodes
                  auto q_selected = ecs.query<NodeIsSelected>();
                  q_selected.each([](flecs::entity e, NodeIsSelected& node)
                  {
                    e.remove<NodeIsSelected>();
                  });
                  // Start drag
                  overlap_event_count++;
                  printf("SelectGraphNode!\n");
                  overlap_ui_element.add<NodeIsSelected>(); // STACK TODO 0: Add to base node
                  ui.add<DragNodeSelector>();
                  
                  // if (keyboard->key_states[KEY_LEFTCTRL] || keyboard->key_states[KEY_RIGHTCTRL])
                  // {
                  //   auto contig = ecs.entity()
                  //     .add<ContiguousSelection>()
                  //     .add<ConsumeEvent>();
                  // }
                }
                if (overlap_ui_element.has<EventOnGrab>(flecs::Wildcard))
                {
                  overlap_event_count++;
                  printf("EventOnGrab\n");
                  if (overlap_ui_element.has<flecs::pair<EventOnGrab, MoveGraph>>())
                  {
                    printf("MoveGraph\n");
                    // ui.add<GrabbedEntity>(overlap_ui_element.parent());
                    if (ui.try_get<GraphEditor>()->edit_mode == GraphEditMode::CRAFT_TELOS)
                    {
                      // TODO: Drag yellow node from graph selector tracking mouse
                      // If dropped into another graph selector, that's how we craft telos!
                    } else
                    {
                      overlap_ui_element.parent().add<GraphIsGrabbed>();
                    }
                  } 
                  else if (overlap_ui_element.has<flecs::pair<EventOnGrab, ConnectGraph>>())
                  {
                    // TODO: All graph selectors should have EventOnGrab ConnectGraph when in CRAFT_TELOS edit mode
                    std::cout << "CREATE TELOS SOURCE" << std::endl;
                    flecs::entity cursor_tracker = ecs.entity("telos_source")
                      .add<Position, World>()
                      .add<Position, Local>()
                      .set<CircleShape>({0.0f})
                      .add<RenderStatus>()
                      .set<TrackCursor>({overlap_ui_element});
                    overlap_ui_element.add<Edge>(cursor_tracker);
                    
                    overlap_ui_element.set<EdgeRenderStatus>({2.0f, overlap_ui_element.try_get<RenderStatus>()->color});
                  }
                  // TODO: Other event type tags... :)
                }
                else if (overlap_ui_element.has<Button>())
                {
                  overlap_ui_element.set<EventSelectBtn>({1});
                  overlap_event_count++;
                } else if (overlap_ui_element.has<OnClickEvent>(flecs::Wildcard))
                {
                  if ((keyboard->key_states[KEY_LEFTSHIFT] || keyboard->key_states[KEY_RIGHTSHIFT]))
                  {
                    deploy_virt_bound = true;
                  } else
                  {
                    overlap_event_count++;
                    overlap_ui_element.set<Clicked>({(float)event.xbutton.x, (float)event.xbutton.y});
                    if (overlap_ui_element.has<flecs::pair<OnClickEvent, SetCartridge>>())
                    {
                      flecs::entity e = ecs.entity("save_cartridge_command")
                        .set<SaveGraphLayer>({overlap_ui_element.try_get<CartridgeBlock>()->value});
                    }
                  }
                }
              }
            }
            if (overlap_event_count == 0 && ui.try_get<GraphEditor>()->edit_mode == GraphEditMode::CREATE_GRAPH)
            {
              remove_forebode_graph();
              // gen_forebode_graph((float)event.xbutton.x-36.0f+12.0f, (float)event.xbutton.y - 64+8, ui_element);
              // TODO: integrate w load_sengra
              auto hud = ecs.lookup("hud");
              // TODO: Set pos..?
              if ((keyboard->key_states[KEY_LEFTSHIFT] || keyboard->key_states[KEY_RIGHTSHIFT]))
              {
                auto graph_rts = ecs.lookup("graph_RTS"); // TODO: Release select graphs
                graph_rts.set<DragBoundGraphSelector>({true, (float)event.xbutton.x, (float)event.xbutton.y, deploy_virt_bound ? DragBoundMode::ZONE : DragBoundMode::SELECT});
                graph_rts.set<Position, Local>({(float)event.xbutton.x, (float)event.xbutton.y});
              } 
              else
              {
                flecs::entity output[6];
                spawn_graph(hud, ui_element, (float)event.xbutton.x, (float)event.xbutton.y, &output[0], true, get_active_cartridge());
              }
            }
            else if (overlap_event_count == 0 && ui.try_get<GraphEditor>()->edit_mode == GraphEditMode::READ)
            {
              auto q_rail = ecs.query<GraphRail>();
              q_rail.each([](flecs::entity e, GraphRail& rail)
              {
                printf("Added slide graph rail!\n");
                e.add<SlideGraphRail>();
              });
            } 
            else if (overlap_event_count == 0 && ui.try_get<GraphEditor>()->edit_mode == GraphEditMode::CRAFT_TELOS) // TODO: Should graph selection be in graph sculpt or craft telos? Bounding box over graphs for snapping...
            {
              // TODO: 'Lock' anchor_bound/selection_bound or reset chisel
              // auto chisel = ecs.lookup("chisel").get_mut<HorizontalBoundaryChisel>();
              // TODO: If reclick on anchor node snap, unlock anchor...
              printf("Chisel lock anchor!\n");
              chisel->lock_anchor = true;
            }
          }
        }
        if (event.type == KeyPress)
        {
          // printf("X key press event\n");
          auto input = ecs.lookup("user_input");
          input.set<X11KeyPress>({event.xkey});
        }
      }
    });

    // ContiguousSelection
    // auto q_select_nodes = ecs.query<GraphNodeData, NodeIsSelected, GraphData>;
    // .ctx(&q_select_nodes)
    // .term_at(2).parent().
		ecs.system<GraphData, ExportGraphSelection*>("ExportSelectedNodes")
		    .term_at(1).src(user_input).optional() // Also updates color!
		    .write(flecs::Wildcard)
		    .kind(flecs::OnUpdate)
		    .each([user_input](flecs::entity graph, GraphData& graph_data, ExportGraphSelection* export_graph)
		    {
		      size_t min = std::numeric_limits<size_t>::max();
		      size_t max = 0;
		      flecs::entity min_node;
		      graph.children([&](flecs::entity child)
		      {
		        if (child.has<NodeIsSelected>())
		        {
		          const GraphNodeData* node_data = child.try_get<GraphNodeData>();
		          if (node_data)
		          {
		            if (node_data->seq < min)
		            {
		              min_node = child;
		              min = node_data->seq;
		            }
		            max = std::max(max, node_data->seq);
		          }
		        }
		      });
		      if (min_node.is_valid())
		      {
		        flecs::entity node = min_node;
		        int avg_y = 0;
		        for (size_t x = min; x < max; x++)
		        {
		          if (auto pos = node.try_get<Position, Local>())
		          {
		            avg_y += pos->y;
		          }
		          node.children([&](flecs::entity child) {
		            if (child.has<RenderStatus>() && !child.has<SelectionStatic>())
		            {
		              child.ensure<RenderStatus>().color = nvgRGBA(0xff, 0xa2, 0x14, 255);
		            }
		            if (child.has<EdgeRenderStatus>())
		            {
		              child.ensure<EdgeRenderStatus>().color = nvgRGBA(0xff, 0xa2, 0x14, 255);
		            }
		          });
		          if (node.has<Next>(flecs::Wildcard))
		          {
		            flecs::entity next_node = node.target<Next>();
		            next_node.add<NodeIsSelected>();
		            node = next_node;
		          }
		          node.children([&](flecs::entity child) {
		            if (child.has<RenderStatus>() && !child.has<SelectionStatic>())
		            {
		              child.ensure<RenderStatus>().color = nvgRGBA(0xff, 0xa2, 0x14, 255);
		            }
		          });
		        }
		        if (max > min) avg_y /= (int)(max - min);
		        auto book_selection = user_input.target<BookGraphSelection>();
		        book_selection.set<Position, Local>({0.0f, (float)avg_y + 32.0f + 22.0f});
		
		        if (export_graph)
		        {
		          auto graph_reader = ecs.lookup("book_reader").try_get<GraphReader>();
		          auto book_selection2 = user_input.target<BookGraphSelection>();
		          book_selection2.set<Position, Local>({0.0f, 0.0f});
		          std::ofstream outputFile("output.txt");
		          if (outputFile.is_open()) {
		              if (graph_reader) {
		                for (size_t x = min; x <= max; ++x)
		                {
		                  outputFile << graph_reader->book_data.pages[graph_reader->active_page][x].word;
		                }
		              }
		              outputFile.close();
		          } else {
		              std::cerr << "Unable to export data" << std::endl;
		          }
		          while (node.has<NodeIsSelected>() && node.has<Prev>(flecs::Wildcard))
		          {
		            printf("Remove node selection!\n");
		            node.remove<NodeIsSelected>();
		            node.children([&](flecs::entity child) {
		              if (child.has<RenderStatus>())
		              {
		                child.ensure<RenderStatus>().color = nvgRGBA(255, 255, 255, 255);
		              }
		            });
		            node = node.target<Prev>();
		            node.children([&](flecs::entity child) {
		              if (child.has<EdgeRenderStatus>())
		              {
		                child.ensure<EdgeRenderStatus>().color = nvgRGBA(255, 255, 255, 255);
		              }
		            });
		          }
		        }
		      }
		      ecs.lookup("user_input").remove<ExportGraphSelection>();
		    });

		ecs.system<Position, const Align, const UIElementBounds, const UIElementBounds>("UpdateAlignment").term_at(0).second<Local>().term_at(2).term_at(3).parent().kind(flecs::OnUpdate)
    .each([](Position& pos, const Align& align, const UIElementBounds& bounds, const UIElementBounds& p_bounds)
    {
      float self_width = bounds.xmax - bounds.xmin;
      float p_container_width = p_bounds.xmax - p_bounds.xmin;
      pos.x = (p_container_width * align.parent_horizontal_percent) - self_width * align.self_horizontal_percent;
      float self_height = bounds.ymax - bounds.ymin;
      float p_container_height = p_bounds.ymax - p_bounds.ymin;
      pos.y = (p_container_height * align.parent_vertical_percent) - self_height * align.self_vertical_percent;

      // printf("Bounds are %f, %f, %f, %f\n", bounds.xmin, bounds.ymin, bounds.xmax, bounds.ymax);
    });

	ecs.system<HorizontalLayoutBox>("ResetHProgress").kind(flecs::PreUpdate)
	    .each([](flecs::entity e, HorizontalLayoutBox& box)
	    {
	      box.x_progress = 0.0f;
	      e.children([&](flecs::entity child)
	      {
	        Position& pos = child.ensure<Position, Local>();
	        pos.x = box.x_progress;
	        const UIElementBounds* bounds = child.try_get<UIElementBounds>();
	        box.x_progress += bounds->xmax - bounds->xmin + box.padding;
	      });
	    });

		ecs.system<VerticalLayoutBox>("ResetVProgress").kind(flecs::PreUpdate)
		    .each([](flecs::entity e, VerticalLayoutBox& box)
		    {
		      box.y_progress = 0.0f;
		      e.children([&](flecs::entity child)
		      {
		        Position& pos = child.ensure<Position, Local>();
		        pos.y = box.y_progress;
		        const UIElementBounds* bounds = child.try_get<UIElementBounds>();
		        box.y_progress += bounds->ymax - bounds->ymin + box.padding;
		      });
		    });

		ecs.system<TableLayoutBox>("ResetTProgress").kind(flecs::PreUpdate)
		    .each([&](flecs::entity e, TableLayoutBox& box)
		    {
		      box.x_progress = 0.0f;
		      box.y_progress = 0.0f;
		      int child_count = 0;
		      e.children([&](flecs::entity child)
		      {
		        Position& pos = child.ensure<Position, Local>();
		        pos.x = box.x_progress;
		        pos.y = box.y_progress;
		        const UIElementBounds& bounds = child.get<UIElementBounds>();
		        box.x_progress += bounds.xmax - bounds.xmin + box.x_padding;
		        if (child_count > 0 && (child_count + 1) % box.max_cols == 0)
		        {
		          box.x_progress = 0;
		          box.y_progress += bounds.ymax - bounds.ymin + box.y_padding;
		        }
		        child_count++;
		      });
		    });

	ecs.system<TextCreator, Position, RenderStatus*, FontStatus*, NanoVG>("RenderText").term_at(1).second<World>().term_at(2).optional().term_at(3).optional().term_at(4).src(renderer).kind(flecs::OnUpdate)
	.each([](TextCreator& text, Position& pos, RenderStatus* rstat, FontStatus* fstat, NanoVG* vg)
	{
		if (!rstat || rstat->visible)
		{
			if (fstat)
			{
				nvgFontSize(vg->ctx, fstat->font_size);
				nvgFontBlur(vg->ctx, fstat->font_blur);
			}
			nvgFontFace(vg->ctx, text.font_name.c_str());
			if (fstat)
			{
				nvgFontSize(vg->ctx, fstat->font_size);
			} else
			{
				nvgFontSize(vg->ctx, 16);
			}
			nvgFillColor(vg->ctx, rstat->color);
			nvgText(vg->ctx, pos.x, pos.y, text.word.c_str(), nullptr);
			nvgFontBlur(vg->ctx, 0.0f);
			nvgFontSize(vg->ctx, 16.0f);
		}
	});

    // TODO: Should have Inkscape to parametric NanoVG export...
		ecs.system<NanoVG, Bode, Position, RenderStatus*>("RenderBodes").term_at(0).src(renderer).term_at(2).second<World>().term_at(3).optional().kind(flecs::OnUpdate)
		.each([](flecs::entity e, NanoVG* vg, Bode* bode, Position* pos, RenderStatus* rstat) {
		  nvgClosePath(vg->ctx);
		  if (!rstat || rstat->visible)
		  {
			nvgBeginPath(vg->ctx);
			float x = pos->x;
			float y = pos->y;
			float w = bode->width;
			float h = bode->height;
			float halfw = w*0.5f;
			float halfh = h*0.5f;
			const float NVG_KAPPA90 = 0.5522847493f;
			float rxBL = std::min(bode->curve_radius, halfw), ryBL = std::min(bode->curve_radius, halfh);
			float rxTR = std::min(bode->curve_radius, halfw), ryTR = std::min(bode->curve_radius, halfh);
			float rxTL = std::min(bode->curve_radius, halfw), ryTL = std::min(bode->curve_radius, halfh);
			float rxBR = std::min(bode->curve_radius, halfw), ryBR = std::min(bode->curve_radius, halfh);
			nvgBeginPath(vg->ctx);
			nvgMoveTo(vg->ctx, x, y + ryTL);
			nvgLineTo(vg->ctx, x, y + h - ryBL);
			nvgBezierTo(vg->ctx, x, y + h - ryBL*(1 - NVG_KAPPA90), x + rxBL*(1 - NVG_KAPPA90), y + h, x + rxBL, y + h);

			// Semicircle here...
			float node_slot_x_left = (w*bode->node_edge_slider)-bode->node_slot_radius;
			nvgLineTo(vg->ctx, x + node_slot_x_left, y + h);
			// nvgLineTo(vg->ctx, x + w/2.0f, y + h + bode[i].node_slot_radius);
			float rx = bode->node_slot_radius;
			float ry = bode->node_slot_radius;
			float cx = x + node_slot_x_left + bode->node_slot_radius;
			float cy = y + h;
			nvgBezierTo(vg->ctx, cx-rx, cy+ry*NVG_KAPPA90, cx-rx*NVG_KAPPA90, cy+ry, cx, cy+ry);
			nvgBezierTo(vg->ctx, cx+rx*NVG_KAPPA90, cy+ry, cx+rx, cy+ry*NVG_KAPPA90, cx+rx, cy);

			// nvgLineTo(vg->ctx, x + (w*(1.0f-bode[i].node_edge_slider))+bode[i].node_slot_radius, y + h);

			nvgLineTo(vg->ctx, x + w - rxBR, y + h);
			nvgBezierTo(vg->ctx, x + w - rxBR*(1 - NVG_KAPPA90), y + h, x + w, y + h - ryBR*(1 - NVG_KAPPA90), x + w, y + h - ryBR);
			nvgLineTo(vg->ctx, x + w, y + ryTR);
			nvgBezierTo(vg->ctx, x + w, y + ryTR*(1 - NVG_KAPPA90), x + w - rxTR*(1 - NVG_KAPPA90), y, x + w - rxTR, y);
			nvgLineTo(vg->ctx, x + rxTL, y);
			nvgBezierTo(vg->ctx, x + rxTL*(1 - NVG_KAPPA90), y, x, y + ryTL*(1 - NVG_KAPPA90), x, y + ryTL);
			nvgClosePath(vg->ctx);

			if (!rstat || rstat->mode == RenderMode::FILL)
			{
			  nvgFillColor(vg->ctx, rstat ? rstat->color : nvgRGB(0,0,0));
			  nvgFill(vg->ctx);
			} else
			{
			  nvgStrokeColor(vg->ctx, rstat->color);
			  nvgStroke(vg->ctx);
			}
		  }
		});

		ecs.system<NanoVG, Grox, Position, RenderStatus*>("RenderGrox").term_at(0).src(renderer).term_at(2).second<World>().term_at(3).optional().kind(flecs::OnUpdate)
		.each([](NanoVG& vg, Grox& grox, Position& pos, RenderStatus* rstat) {
		  nvgClosePath(vg.ctx);
		  if (!rstat || rstat->visible)
		  {
			nvgBeginPath(vg.ctx);
			float x = pos.x;
			float y = pos.y;
			float w = grox.width;
			float h = grox.height;
			float halfw = w*0.5f;
			float halfh = h*0.5f;
			const float NVG_KAPPA90 = 0.5522847493f;
			float rxBL = std::min(grox.curve_radius, halfw), ryBL = std::min(grox.curve_radius, halfh);
			float rxTR = std::min(grox.curve_radius, halfw), ryTR = std::min(grox.curve_radius, halfh);
			float rxTL = std::min(grox.curve_radius, halfw), ryTL = std::min(grox.curve_radius, halfh);
			float rxBR = std::min(grox.curve_radius, halfw), ryBR = std::min(grox.curve_radius, halfh);

			NVGcolor stroke_colors[2];
			stroke_colors[0] = rstat ? rstat->color : grox.secondary;
			stroke_colors[1] = grox.secondary;

			int st_ind = 0;

			nvgBeginPath(vg.ctx);
			nvgMoveTo(vg.ctx, x, y + ryTL);

			// TODO: Lambda
			for (size_t t = 0; t < grox.tile_d; ++t)
			{
			  float p = (t+1.0f)/grox.tile_d;
			  float y_pos = interp(y + ryTL, y + h - ryBL, p);
			  nvgLineTo(vg.ctx, x, y_pos);
			  nvgStrokeColor(vg.ctx, stroke_colors[st_ind]);
			  if (t != grox.tile_d-1) st_ind = (st_ind+1)%2;
			  // std::cout << st_ind << " is index " << std::endl;
			  nvgStroke(vg.ctx);
			  nvgClosePath(vg.ctx);
			  nvgBeginPath(vg.ctx);
			  nvgMoveTo(vg.ctx, x, y_pos);
			}

			nvgBezierTo(vg.ctx, x, y + h - ryBL*(1 - NVG_KAPPA90), x + rxBL*(1 - NVG_KAPPA90), y + h, x + rxBL, y + h);

			for (size_t t = 0; t < grox.tile_d; ++t)
			{
			  float p = (t+1.0f)/grox.tile_d;
			  float x_pos = interp(x + rxBL, x + w - rxBR, p);
			  nvgLineTo(vg.ctx, x_pos, y + h);
			  nvgStrokeColor(vg.ctx, stroke_colors[st_ind]);
			  if (t != grox.tile_d-1) st_ind = (st_ind+1)%2;
			  nvgStroke(vg.ctx);
			  nvgClosePath(vg.ctx);
			  nvgBeginPath(vg.ctx);
			  nvgMoveTo(vg.ctx, x_pos, y + h);
			}

			nvgBezierTo(vg.ctx, x + w - rxBR*(1 - NVG_KAPPA90), y + h, x + w, y + h - ryBR*(1 - NVG_KAPPA90), x + w, y + h - ryBR);

			for (size_t t = 0; t < grox.tile_d; ++t)
			{
			  float p = (t+1.0f)/grox.tile_d;
			  float y_pos = interp(y + h - ryBR, y + ryTR, p);
			  nvgLineTo(vg.ctx, x + w, y_pos);
			  nvgStrokeColor(vg.ctx, stroke_colors[st_ind]);
			  if (t != grox.tile_d-1) st_ind = (st_ind+1)%2;
			  nvgStroke(vg.ctx);
			  nvgClosePath(vg.ctx);
			  nvgBeginPath(vg.ctx);
			  nvgMoveTo(vg.ctx, x + w, y_pos);
			}

			nvgBezierTo(vg.ctx, x + w, y + ryTR*(1 - NVG_KAPPA90), x + w - rxTR*(1 - NVG_KAPPA90), y, x + w - rxTR, y);

			for (size_t t = 0; t < grox.tile_d; ++t)
			{
			  float p = (t+1.0f)/grox.tile_d;
			  float x_pos = interp(x + w - rxTR, x + rxTL, p);
			  nvgLineTo(vg.ctx, x_pos, y);
			  nvgStrokeColor(vg.ctx, stroke_colors[st_ind]);
			  if (t != grox.tile_d-1) st_ind = (st_ind+1)%2;
			  nvgStroke(vg.ctx);
			  nvgClosePath(vg.ctx);
			  nvgBeginPath(vg.ctx);
			  nvgMoveTo(vg.ctx, x_pos, y);
			}
			// nvgLineTo(vg.ctx, x + rxTL, y);

			nvgBezierTo(vg.ctx, x + rxTL*(1 - NVG_KAPPA90), y, x, y + ryTL*(1 - NVG_KAPPA90), x, y + ryTL);
			nvgStrokeColor(vg.ctx, stroke_colors[st_ind]);
			nvgStroke(vg.ctx);
			nvgClosePath(vg.ctx);
		  }
		});

    // TODO: Debug render toggle settings...
		ecs.system<NanoVG, UIElementBounds, Position, RenderStatus*, UIDebugRender>("DebugRenderUIElementBounds").term_at(0).src(renderer).term_at(2).second<World>().term_at(3).optional().term_at(4).src(renderer).kind(flecs::OnUpdate)
		    .each([](NanoVG* vg, UIElementBounds* bounds, Position* pos, RenderStatus* rstat, UIDebugRender* ui_debug_render) {
		      if (ui_debug_render->active)
		      {
		        if (!rstat || rstat->visible)
		        {
		          nvgBeginPath(vg->ctx);
		          nvgRect(vg->ctx, bounds->xmin, bounds->ymin, bounds->xmax - bounds->xmin, bounds->ymax - bounds->ymin);
		          nvgStrokeColor(vg->ctx, nvgRGBA(255, 200, 0, 255));
		          nvgStroke(vg->ctx);
		        }
		      }
		    });

		ecs.system<Position, TrackEdge>("EdgeTelosUpdatePos").term_at(0).second<Local>()
    .each([](Position& pos, TrackEdge& track)
    {
        const Position* source_pos = track.source.try_get<flecs::pair<Position, World>>();
        const Position* target_pos = track.target.try_get<flecs::pair<Position, World>>();
        vec2 midpoint;
        vec2 s_vec = {source_pos->x, source_pos->y};
        vec2 t_vec = {target_pos->x, target_pos->y};
        get_closest_radius_point(t_vec, s_vec, glm_vec2_distance(s_vec, t_vec)/2.0f, &midpoint);
        bool unidirection = track.target.has<Edge>(track.source);
        if (unidirection)
        {
            // Assuming you have some way to define "right" relative to the vector from source to target
            vec2 direction = {t_vec[0] - s_vec[0], t_vec[1] - s_vec[1]};
            vec2 normalizedDirection;
            glm_vec2_normalize_to(direction, normalizedDirection);
            vec2 right = {-normalizedDirection[1], normalizedDirection[0]}; // Right perpendicular vector

            // Calculate amplitude offset
            float amplitude = glm_vec2_distance(s_vec, t_vec)/8.0f;
            vec2 amplitudeOffset = {right[0] * amplitude, right[1] * amplitude};

            // Apply amplitude offset to midpoint for unidirectional edges
            pos.x = midpoint[0] + amplitudeOffset[0];
            pos.y = midpoint[1] + amplitudeOffset[1];
        }
        else
        {
            pos.x = midpoint[0];
            pos.y = midpoint[1];
        }
    });

    // TODO: Better scene management than LevelHUD...zzz
    ecs.system<NanoVG, RenderStatus, EdgeRenderStatus, Edge>("RenderEdges")
    .term_at(0).src(renderer).term_at(3).second(flecs::Wildcard).kind(flecs::OnUpdate)
    .run([](flecs::iter& it, size_t i, NanoVG& vg, RenderStatus& rstat, EdgeRenderStatus& estat) 
    {
      if (rstat.visible)
      {
        // printf("Render line!");
        auto target = it.pair(3).second();
        auto source = it.entity(i);
        // std::cout << "Target: " << target.name() << "  Source: " << source.name() << std::endl;
        const Position* sourcePos = source.try_get<flecs::pair<Position, World>>();
        const Position* targetPos = target.try_get<flecs::pair<Position, World>>();
        // TODO: Render edge starting at the directional radius offset
        float sourceRadius = 0.0f;
        if (source.has<CircleShape>())
        {
          sourceRadius = source.try_get<CircleShape>()->radius;
        } else if (source.parent().has<CircleShape>())
        {
          sourceRadius = source.parent().try_get<CircleShape>()->radius;
        } else if (source.parent().has<Bode>())
        {
          sourceRadius = source.parent().try_get<Bode>()->node_slot_radius;
        }
        float targetRadius = 0.0f; 
        if (target.has<CircleShape>())
        {
          targetRadius = target.try_get<CircleShape>()->radius;
        } else if (target.parent().has<CircleShape>())
        {
          targetRadius = target.parent().try_get<CircleShape>()->radius;
        } else if (target.parent().has<Bode>())
        {
          targetRadius = target.parent().try_get<Bode>()->node_slot_radius;
        }
        float length = 12.0f; // Tailoring the triangle's tip for your world's epic.
        float width = 8.0f; // The weight of the triangle's focus.
        bool unidirection = target.has<Edge>(source);
        if (unidirection)
        {
          nvgBeginPath(vg.ctx);
          vec2 source_membrane;
          vec2 target_membrane;
          vec2 s_vec = {sourcePos->x, sourcePos->y};
          vec2 t_vec = {targetPos->x, targetPos->y};
          get_closest_radius_point(t_vec, s_vec, sourceRadius, &source_membrane);
          get_closest_radius_point(s_vec, t_vec, targetRadius, &target_membrane);

          // Adjusting the control points to arc 'right' from the source's perspective.
          float midX = (source_membrane[0] + target_membrane[0]) / 2;
          float midY = (source_membrane[1] + target_membrane[1]) / 2;
          vec2 midPoint = {midX, midY};

          // The vector from source to target.
          vec2 direction = {target_membrane[0] - source_membrane[0], target_membrane[1] - source_membrane[1]};
          // Normalizing the direction vector.
          vec2 normalizedDirection;
          glm_vec2_normalize_to(direction, normalizedDirection);

          // Creating a rightward normal from the direction vector (to arc right).
          vec2 rightNormal = {-normalizedDirection[1], normalizedDirection[0]};

          // Amplitude of the arc at its peak (middle).
          float amplitude = glm_vec2_distance(source_membrane, target_membrane) / 4; // Adjust to taste.

          // Calculating control points to push the curve rightward.
          vec2 controlPoint = {midPoint[0] + rightNormal[0] * amplitude, midPoint[1] + rightNormal[1] * amplitude};

          nvgMoveTo(vg.ctx, source_membrane[0], source_membrane[1]);
          // Single control point bezier for a quadratic curve.
          nvgQuadTo(vg.ctx, controlPoint[0], controlPoint[1], target_membrane[0], target_membrane[1]);
          nvgStrokeWidth(vg.ctx, estat.stroke_width);
          nvgStrokeColor(vg.ctx, estat.color);
          nvgStroke(vg.ctx);

          // Time parameter for our specific point of interest along the bezier.
          float t = 0.25;

          // Position on the Bezier curve at t=0.25 for the triangle.
          vec2 pointOnBezier;
          pointOnBezier[0] = (1-t) * (1-t) * source_membrane[0] + 2 * (1-t) * t * controlPoint[0] + t * t * target_membrane[0];
          pointOnBezier[1] = (1-t) * (1-t) * source_membrane[1] + 2 * (1-t) * t * controlPoint[1] + t * t * target_membrane[1];

          // Direction of the curve at t=0.25, used to orient the triangle.
          vec2 directionOnBezier;
          directionOnBezier[0] = 2 * (1-t) * (controlPoint[0] - source_membrane[0]) + 2 * t * (target_membrane[0] - controlPoint[0]);
          directionOnBezier[1] = 2 * (1-t) * (controlPoint[1] - source_membrane[1]) + 2 * t * (target_membrane[1] - controlPoint[1]);

          // Normalize this direction to get a consistent orientation for the triangle.
          vec2 normalizedDirectionOnBezier;
          glm_vec2_normalize_to(directionOnBezier, normalizedDirectionOnBezier);

          // Now, using the normalized direction for the triangle's orientation.
          vec2 triangleTip = {pointOnBezier[0] + normalizedDirectionOnBezier[0] * length, pointOnBezier[1] + normalizedDirectionOnBezier[1] * length};
          vec2 rightTriangleNormal = {-normalizedDirectionOnBezier[1], normalizedDirectionOnBezier[0]};
          vec2 base1 = {pointOnBezier[0] + rightTriangleNormal[0] * width / 2, pointOnBezier[1] + rightTriangleNormal[1] * width / 2};
          vec2 base2 = {pointOnBezier[0] - rightTriangleNormal[0] * width / 2, pointOnBezier[1] - rightTriangleNormal[1] * width / 2};

          // Drawing the triangle at the corrected position and orientation.
          nvgBeginPath(vg.ctx);
          nvgFillColor(vg.ctx, estat.color);
          nvgMoveTo(vg.ctx, triangleTip[0], triangleTip[1]);
          nvgLineTo(vg.ctx, base1[0], base1[1]);
          nvgLineTo(vg.ctx, base2[0], base2[1]);
          nvgClosePath(vg.ctx);
          nvgFill(vg.ctx);

        } 
        else
        {
          nvgBeginPath(vg.ctx);
          vec2 source_membrane;
          vec2 target_membrane;
          vec2 s_vec = {sourcePos->x, sourcePos->y};
          vec2 t_vec = {targetPos->x, targetPos->y};
          get_closest_radius_point(t_vec, s_vec, sourceRadius, &source_membrane);
          get_closest_radius_point(s_vec, t_vec, targetRadius, &target_membrane);
          nvgMoveTo(vg.ctx, source_membrane[0], source_membrane[1]);
          nvgLineTo(vg.ctx, target_membrane[0], target_membrane[1]);
          nvgStrokeWidth(vg.ctx, estat.stroke_width);
          nvgStrokeColor(vg.ctx, estat.color);
          nvgStroke(vg.ctx);
          nvgClosePath(vg.ctx);

          if (estat.show_arrow)
          {
            // TODO: Render nvg triangle pointing towards the target at 1/4 the edge distance away
            vec2 edgeVector = {target_membrane[0] - source_membrane[0], target_membrane[1] - source_membrane[1]};
            vec2 normalizedDirection;
            glm_vec2_normalize_to(edgeVector, normalizedDirection);

            vec2 normal = {normalizedDirection[1], -normalizedDirection[0]};
            vec2 baseMid = {source_membrane[0] + 0.25f * edgeVector[0], source_membrane[1] + 0.25f * edgeVector[1]}; // Beacon among the bytestream, afloat 1/4th the synaptic spell.

            // Geometric mayhem for the grand display.
            vec2 tip = {baseMid[0] + normalizedDirection[0] * length, baseMid[1] + normalizedDirection[1] * length};
            vec2 base1 = {baseMid[0] + (normal[0] * width * 0.5f), baseMid[1] + (normal[1] * width * 0.5f)};
            vec2 base2 = {baseMid[0] - (normal[0] * width * 0.5f), baseMid[1] - (normal[1] * width * 0.5f)};

            // The incantation to draw, binding the quarks of your global stem.
            nvgBeginPath(vg.ctx);
            nvgFillColor(vg.ctx, estat.color);
            nvgMoveTo(vg.ctx, tip[0], tip[1]);
            nvgLineTo(vg.ctx, base1[0], base1[1]);
            nvgLineTo(vg.ctx, base2[0], base2[1]);
            nvgClosePath(vg.ctx); // Sealing the vector echo.
            nvgFill(vg.ctx); // Fills the heart with the radiance of the road travelled.
          }

        }
      }
    });

		ecs.system<NanoVG, CapsuleShape, Position, RenderStatus*>("RenderCapsules")
    .term_at(0).src(renderer).term_at(2).second<World>().term_at(3).optional().kind(flecs::OnUpdate)
		.each([](NanoVG& vg, CapsuleShape& capsule, Position& pos, RenderStatus* rstat) {
			if (!rstat || rstat->visible)
			{
				nvgBeginPath(vg.ctx);
				nvgRoundedRect(vg.ctx, pos.x - capsule.radius, pos.y - capsule.radius, capsule.width, capsule.radius * 2.0f, capsule.radius);
				if (rstat && rstat->mode == RenderMode::FILL)
				{
					nvgFillColor(vg.ctx, rstat->color);
					nvgFill(vg.ctx);
				}
				else
				{
					if (rstat) nvgStrokeColor(vg.ctx, rstat->color);
					nvgStroke(vg.ctx);
				}
			}
		});

		ecs.system<NanoVG, CircleShape, Position, RenderStatus, ModCircle*>("RenderCircles").term_at(0).src(renderer).term_at(2).second<World>().term_at(4).optional().kind(flecs::OnUpdate)
		.each([](NanoVG& vg, CircleShape& cs, Position& pos, RenderStatus& rstat, ModCircle* mod) {
			if (rstat.visible)
			{
				if (mod)
				{
					size_t a = 0;
					size_t num_segments = mod->colors.size();
					for (NVGcolor color : mod->colors)
					{
						float start_angle = (a * 2 * M_PI) / num_segments;
						float end_angle = ((a + 1) * 2 * M_PI) / num_segments;
						nvgBeginPath(vg.ctx);
						nvgArc(vg.ctx, pos.x, pos.y, cs.radius, start_angle, end_angle, NVG_CW);
						nvgClosePath(vg.ctx);
						nvgStrokeColor(vg.ctx, color);
						nvgStroke(vg.ctx);
						a++;
					}
				}
				else
				{
					nvgBeginPath(vg.ctx);
					nvgCircle(vg.ctx, pos.x, pos.y, cs.radius);
					if (rstat.mode == RenderMode::FILL)
					{
						nvgFillColor(vg.ctx, rstat.color);
						nvgFill(vg.ctx);
					} else
					{
						nvgStrokeColor(vg.ctx, rstat.color);
						nvgStroke(vg.ctx);
					}
					nvgClosePath(vg.ctx);
				}
			}
		});

    #define NVG_KAPPA90 0.5522847493f
		ecs.system<NanoVG, RoundTriShape, Position, RenderStatus>("RenderRoundTris").term_at(0).src(renderer).term_at(2).second<World>().kind(flecs::OnUpdate)
		    .each([](NanoVG& vg, RoundTriShape& tri, Position& pos, RenderStatus& rstat) {
		      if (rstat.visible)
		      {
		          // Set the radius for rounded corners
		          float cornerRadius = tri.corner_radius;
		
		          // Set the radius for the overall size of the triangle
		          float triangleRadius = tri.triangle_radius;
		          float inner_radius = triangleRadius - cornerRadius;
		
		          // Calculate the three vertices of the triangle
		          float centerX = pos.x, centerY = pos.y; // Center position
		          float start_angle = 2*NVG_PI * (3/4.0f);
		
		          float triangle_perimeter = triangleRadius*2*NVG_PI;
		          float corner_diameter = cornerRadius*2.0f;
		          float corner_arc_angle = (corner_diameter/triangle_perimeter)*2*NVG_PI;
		          std::cout << "Corner angle arc is " << corner_arc_angle << std::endl;
		
		          Position inner_points[6] = {
		              {centerX + inner_radius * cosf(start_angle-corner_arc_angle/2 + 2*NVG_PI *  0/3.0f), centerY + inner_radius * sinf(start_angle-corner_arc_angle/2 + 2*NVG_PI * 0/3.0f)},
		              {centerX + inner_radius * cosf(start_angle+corner_arc_angle/2 + 2*NVG_PI *  0/3.0f), centerY + inner_radius * sinf(start_angle+corner_arc_angle/2 + 2*NVG_PI * 0/3.0f)},
		
		              {centerX + inner_radius * cosf(start_angle-corner_arc_angle/2 + 2*NVG_PI * 1/3.0f), centerY + inner_radius * sinf(start_angle-corner_arc_angle/2 + 2*NVG_PI * 1/3.0f)},
		              {centerX + inner_radius * cosf(start_angle+corner_arc_angle/2 + 2*NVG_PI * 1/3.0f), centerY + inner_radius * sinf(start_angle+corner_arc_angle/2 + 2*NVG_PI * 1/3.0f)},
		
		              {centerX + inner_radius * cosf(start_angle-corner_arc_angle/2 + 2*NVG_PI * 2/3.0f), centerY + inner_radius * sinf(start_angle-corner_arc_angle/2 + 2*NVG_PI * 2/3.0f)},
		              {centerX + inner_radius * cosf(start_angle+corner_arc_angle/2 + 2*NVG_PI * 2/3.0f), centerY + inner_radius * sinf(start_angle+corner_arc_angle/2 + 2*NVG_PI * 2/3.0f)}
		          };
		
		          // Move to the top vertex
		          nvgBeginPath(vg.ctx);
		          nvgMoveTo(vg.ctx, inner_points[0].x, inner_points[0].y);
		
		          nvgLineTo(vg.ctx, inner_points[1].x, inner_points[1].y);
		          nvgLineTo(vg.ctx, inner_points[2].x, inner_points[2].y);
		
		          nvgLineTo(vg.ctx, inner_points[3].x, inner_points[3].y);
		          nvgLineTo(vg.ctx, inner_points[4].x, inner_points[4].y);
		
		          nvgLineTo(vg.ctx, inner_points[5].x, inner_points[5].y);
		          nvgLineTo(vg.ctx, inner_points[0].x, inner_points[0].y);
		
		          if (rstat.mode == RenderMode::FILL)
		          {
		            nvgFillColor(vg.ctx, rstat.color);
		            nvgFill(vg.ctx);
		          }
		          else
		          {
		            nvgStrokeColor(vg.ctx, rstat.color);
		            nvgStroke(vg.ctx);
		          }
		          nvgClosePath(vg.ctx);
		      }
		    });

    // TODO: Consider just using RenderStatus color alpha? :/
		ecs.system<NanoVG, Sprite, Position, RenderStatus*, SpriteAlpha*, SpriteTint*>("RenderSprites").term_at(0).src(renderer).term_at(2).second<World>().term_at(3).optional().term_at(4).optional().term_at(5).optional().kind(flecs::PostUpdate)
		.each([](NanoVG& vg, Sprite& sprite, Position& pos, RenderStatus* rstat, SpriteAlpha* sa, SpriteTint* st) {
			if (!rstat || rstat->visible)
			{
				nvgBeginPath(vg.ctx);
				nvgRect(vg.ctx, pos.x, pos.y, sprite.c_w, sprite.c_h);
				float alpha = 1.0f;
				if (sa)
				{
					alpha = sa->alpha;
				}
				NVGpaint paint = nvgImagePattern(vg.ctx, pos.x - sprite.x, pos.y - sprite.y, sprite.w, sprite.h, 0.0f, sprite.img, alpha);

				if (st)
				{
					paint.outerColor = paint.innerColor = st->tint;
				}

				nvgFillPaint(vg.ctx, paint);
				nvgFill(vg.ctx);
			}
		});

	ecs.system<NanoVG, BoxShape, Position, RenderStatus>("RenderBoxes")
    .term_at(0).src(renderer).term_at(2).second<World>().kind(flecs::PostUpdate)
    .each([](NanoVG& vg, BoxShape& bs, Position& pos, RenderStatus& rstat) {
        if (rstat.visible) {
            nvgBeginPath(vg.ctx);
            nvgRect(vg.ctx, pos.x + 0.5f, pos.y + 0.5f, bs.width, bs.height);
            if (rstat.mode == RenderMode::FILL) {
                nvgFillColor(vg.ctx, rstat.color);
                nvgFill(vg.ctx);
            } else {
                nvgLineCap(vg.ctx, NVGlineCap::NVG_SQUARE);
                nvgStrokeWidth(vg.ctx, bs.stroke_width);
                nvgStrokeColor(vg.ctx, rstat.color);
                nvgStroke(vg.ctx);
            }
            nvgClosePath(vg.ctx);
        }
    });


	ecs.system<NanoVG, RenderStatus, LineShape, Position>("RenderLines")
      .term_at(0).src(renderer).term_at(3).second<World>().kind(flecs::OnUpdate)
      .each([](NanoVG& vg, RenderStatus& rstat, LineShape& ls, Position& pos) {
          if (rstat.visible) {
              nvgBeginPath(vg.ctx);
              nvgMoveTo(vg.ctx, pos.x, pos.y);
              nvgLineTo(vg.ctx, pos.x + ls.offset_x, pos.y + ls.offset_y);
              nvgStrokeColor(vg.ctx, rstat.color);
              nvgStrokeWidth(vg.ctx, ls.stroke_width);
              nvgStroke(vg.ctx);
              nvgClosePath(vg.ctx);
          }
      });

	ecs.system<NanoVG, DiamondShape, Position, RenderStatus*>("RenderDiamonds").term_at(0).src(renderer).term_at(2).second<World>().term_at(3).optional().kind(flecs::OnUpdate)
	.each([](NanoVG& vg, DiamondShape& ds, Position& pos, RenderStatus* rstat) {
		if (!rstat || rstat->visible) {
			float r = ds.radius;
			float x = pos.x;
			float y = pos.y;

			nvgBeginPath(vg.ctx);

			// Define the diamond shape
			nvgMoveTo(vg.ctx, x, y - r); // Top point
			nvgLineTo(vg.ctx, x + r, y); // Right point
			nvgLineTo(vg.ctx, x, y + r); // Bottom point
			nvgLineTo(vg.ctx, x - r, y); // Left point
			nvgClosePath(vg.ctx); // Back to top point

			// Fill or stroke the diamond
			if (!rstat || rstat->mode == RenderMode::FILL) {
				if (rstat) nvgFillColor(vg.ctx, rstat->color);
				nvgFill(vg.ctx);
			} else {
				nvgStrokeWidth(vg.ctx, 1.0f);
				if (rstat) nvgStrokeColor(vg.ctx, rstat->color);
				nvgStroke(vg.ctx);
			}
		}
	});

		ecs.system<GraphRail, Position, Position, ActiveGraph, Position, HorizontalBoundaryChisel, X11CursorMovement>("SnapChiselToGraph")
    .kind(flecs::PreUpdate).term_at(1).second<Local>().term_at(2).second<World>().term_at(3).parent().term_at(4).second<World>().parent().term_at(5).src(chisel).term_at(6).src(user_input)
    .each([](flecs::entity e, GraphRail& graph_rail, Position& rail_pos, Position& rail_world_pos, ActiveGraph& core, Position& graph_world_pos, HorizontalBoundaryChisel& chisel, X11CursorMovement& cursor)
    {
        // printf("(%f, %f, %f, %f\n)", graph_bounds->xmin, graph_bounds->ymin, graph_bounds->xmax,graph_bounds->ymax);
        // chisel_pos->y = graph_bounds->ymin;
        // float target_x = cursor->x;
        float x = cursor.x - rail_world_pos.x;
        // printf("X is %f\n", x);
        float target_x = x;
        // TODO: Convert from cursor screen x to graphrail local x?
        if (chisel.snap)
        {
          // Find closest word start or end boundary in speechgraph
          // or token in sengra
          flecs::entity closest_node = find_closest_node(cursor.x, e.parent());
          if (closest_node == e)
          {
            return; // if graph_root is the only node...
          }
          target_x = closest_node.try_get<Position, Local>()->x;
          // printf("Closest node x is %f\n", target_x);

          bool has_prev_node = closest_node.has<Prev>(flecs::Wildcard);
          // For speech graphs, the closest snapping point may be the prev speech end mark or closest node speech end mark
          if (closest_node.has<SpokenWord>())
          {
            const SpokenWord* speech = closest_node.try_get<SpokenWord>();
            float text_end_x = target_x + ((speech->end - speech->start) * 200.0f);
            float dist_to_text_end = abs(x - text_end_x);
            if (has_prev_node)
            {
              flecs::entity prev_node = closest_node.target<Prev>();
              float prev_node_x = prev_node.try_get<Position, Local>()->x;
              const SpokenWord* prev_speech = prev_node.try_get<SpokenWord>();
              float prev_text_end_x = prev_node_x + (prev_speech->end - prev_speech->start) * 200.0f;
              float dist_to_prev_text_end = abs(x - prev_text_end_x);
              if (dist_to_prev_text_end < abs(x - target_x))
              {
                target_x = prev_text_end_x;
              }
            }
            if (dist_to_text_end < abs(x - target_x))
            {
              target_x = text_end_x;
            }
          }
        }
        if (!chisel.lock_anchor)
        {
          chisel.anchor_x = target_x;
        }
        if (!chisel.lock_selection)
        {
          chisel.selection_x = target_x;
        }
        // TODO: Update UI canvas to graph height
    });

		ecs.system<Interpolator>("InterpolatorProgress").kind(flecs::PreUpdate)
		.each([](flecs::entity e, Interpolator& interpolator)
		{
		  float dt = e.world().delta_time();
		  interpolator.progress += dt;
		  float max = interpolator.time + interpolator.delay;
		  if (interpolator.progress > max) {
		    interpolator.progress = max;
		  }
		  if (interpolator.delay > interpolator.progress) {
		    interpolator.percent = 0.0f;
		  } else {
		    interpolator.percent = (interpolator.progress - interpolator.delay) / (interpolator.time);
		  }
		});

		ecs.system<Interpolator, ColorAnimation, RenderStatus*>("AnimateColor").kind(flecs::PreUpdate).term_at(2).optional()
			.each([](Interpolator& interpolator, ColorAnimation& color, RenderStatus* render)
			{
				float p = interpolator.percent;
				for (size_t channel = 0; channel < 4; channel++)
				{
					color.current.rgba[channel] = color.start.rgba[channel] * (1.0f - p) + color.end.rgba[channel] * p;
				}
				if (render)
				{
					render->color = color.current;
				}
			});

		ecs.system<TakeScreenshot>("TakeScreenshot").write(flecs::Wildcard).kind(flecs::PreUpdate)
		.each([ui_element](flecs::entity e) {
			auto gf = ecs.lookup("os_cursor");
			system("python3 os.py");
			gf.set<RenderStatus>({true});
			auto vo = ecs.lookup("vision_overlay");
			vo.set<SpriteCreator>({"vision.png"});
		
			// first, remove existing bounds
			auto q = ecs.query<SymbolBoundAnnotation>();
			q.each([](flecs::entity be) {
				be.destruct();
			});
		
			std::ifstream f("bounds_to_paths.json");
			json data = json::parse(f);
		
			for (auto& element : data.items()) {
				std::string key = element.key();
				std::string value = element.value();
		
				std::stringstream ss(key);
				int x, y, width, height;
				char delimiter;
				ss >> x >> delimiter >> y >> delimiter >> width >> delimiter >> height;
		
				// Find the position of the opening parenthesis
				size_t startPos = value.find("(");
				// Extract the substring containing the RGB values
				std::string rgbString = value.substr(startPos + 1);
				int r, g, b;
				// Use stringstream to parse the RGB values
				std::stringstream colorss(rgbString);
				colorss >> r >> delimiter >> g >> delimiter >> b;
		
				// Fill in the details
				// Example usage, assuming ecs, ui_element, and other variables/functions are defined elsewhere
				auto symbol_bound = ecs.entity()
					.is_a(ui_element)
					.set<Position, Local>({x, y})
					.set<BoxShape>({width, height, 1.0f})
					.add<SymbolBoundAnnotation>()
					.add<AnnotatorHUD>()
					.set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(r, g, b, 128)});
			}
			e.remove<TakeScreenshot>();
		});

    // TODO: Load multiple virt_desktops from compositor

    system("python3 get_vnc_pos.py");
    std::ifstream vnc_pos("vnc_pos.txt");
    char delim;
    int virt_x, virt_y, virt_width, virt_height;
    vnc_pos >> virt_x >> virt_y >> virt_width >> virt_height;
    std::cout << virt_x << ", " << virt_y << ", " << virt_width << ", " << virt_height << std::endl;

  // TODO: ZIndex renderer, move this config up
    // float virt_desk_width = 1240;
    // float virt_desk_height = 922;
    float hide_top = 73.0f ;
    virt_y += hide_top;
    virt_height -= hide_top;
    // float hide_bottom = 11.0f;
    // float vv_width = (width-virt_desk_width)/2;
    // float vv_height = (height-virt_desk_height)/2;
    c2AABB vv_box = {{virt_x, virt_y}, {virt_x + virt_width, virt_y + virt_height}};

    auto virt_desktop_frame = ecs.entity("virt_desktop_frame")
      .is_a(ui_element)
      .child_of(hud);

    // HUD OS view Frame
    auto frame_left = ecs.entity("frame_left")
      .is_a(ui_element)
      .set<BackgroundBoxShape>({virt_x, height})
      .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0, 0, 0, 255)})
      .set<RenderOrder>({-100})
      .add<LevelHUD>()
      .child_of(virt_desktop_frame);

    auto frame_right = ecs.entity("frame_right")
      .is_a(ui_element)
      .set<Position, Local>({virt_x+virt_width, 0.0f})
      .set<BackgroundBoxShape>({width-(virt_x+virt_width), height})
      .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0, 0, 0, 255)})
      .set<RenderOrder>({-100})
      .add<LevelHUD>()
      .child_of(virt_desktop_frame);

    auto frame_top = ecs.entity("frame_top")
      .is_a(ui_element)
      .set<BackgroundBoxShape>({width, virt_y})
      .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0, 0, 0, 255)})
      .set<RenderOrder>({-100})
      .add<LevelHUD>()
      .child_of(virt_desktop_frame);

    auto frame_bot = ecs.entity("frame_bot")
      .is_a(ui_element)
      .set<BackgroundBoxShape>({width, height-(virt_y+virt_height)})
      .set<Position, Local>({0, virt_y+virt_height})
      .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0, 0, 0, 255)})
      .set<RenderOrder>({-100})
      .add<LevelHUD>()
      .child_of(virt_desktop_frame);

		ecs.system<TakeVirtScreenshot>("VirtDesktopAnnotate").write(flecs::Wildcard).kind(flecs::PreUpdate)
		.each([ui_element, vv_box, virt_x, virt_y, virt_height](flecs::entity e, TakeVirtScreenshot& snap)
		{
		  system("python3 virt_os.py");
		  auto vo = ecs.lookup("vision_overlay");
		  vo.set<SpriteCreator>({"vision.png"});

		  // first, remove existing bounds
		  auto q = ecs.query<SymbolBoundAnnotation>();
		  q.each([](flecs::entity ent, SymbolBoundAnnotation& sboa)
		  {
		    ent.destruct();
		  });

		  std::ifstream f("bounds_to_paths.json");
		  json data = json::parse(f);

		  for (auto& element : data.items()) {
		      std::string key = element.key();
		      std::string value = element.value();

		      std::stringstream ss(key);
		      int x, y, width, height;
		      char delimiter;
		      ss >> x >> delimiter >> y >> delimiter >> width >> delimiter >> height;

		      std::string sprite_file;
		      std::stringstream sps (value);
		      sps >> sprite_file;
		      // Find the position of the opening parenthesis
		      size_t startPos = value.find("(");
		      // Extract the substring containing the RGB values
		      std::string rgbString = value.substr(startPos + 1);
		      int r, g, b;
		      // Use stringstream to parse the RGB values
		      std::stringstream colorss(rgbString);
		      colorss >> r >> delimiter >> g >> delimiter >> b;

		      c2v sym_pos = {x+virt_x, y+virt_y-51};
		      if (c2AABBtoPoint(vv_box, sym_pos))
		      {
		        auto symbol_bound = ecs.entity()
		            .is_a(ui_element)
		            .set<Position, Local>({x+virt_x-1, y-virt_height-1})
		            .set<Position, Source>({x+virt_x-1, y-virt_height-1})
		            .set<BoxShape>({width+2, height+2, 1.0f})
		            .add<SymbolBoundAnnotation>()
		            .add<LevelHUD>()
		            .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(r, g, b, 128)})
		            .set<Interpolator>({1.0f})
		            // .set<CubicMovement>({getRandomInt(250, 300)})
		            .set<CubicMovement>({getRandomInt(0, 50) + 300 + sqrt(width+height)*7.0f})
		            .set<Position, MoveTarget>({x+virt_x-1, y+virt_y-51-1});
		        auto symbol_sprite = ecs.entity()
		          .is_a(ui_element)
		          .set<SpriteCreator>({"symbols/" + sprite_file})
		          .set<Position, Local>({1, 1})
		          .child_of(symbol_bound);

		      }
		  }
		  e.remove<TakeVirtScreenshot>();
		});

		ecs.system<Sprite, SpriteAnimator>("AnimateSprite").kind(flecs::OnUpdate)
		.each([](flecs::iter& it, Sprite& sprite, SpriteAnimator& sa) {
		  sa.progress += it.delta_time();
		  while (sa.progress > sa.framerate)
		  {
		    sa.progress -= sa.framerate;
		    if (sprite.rows > 1)
		    {
		      if (sprite.y + sprite.c_h >= sprite.h)
		      {
		        sprite.y = 0;
		      }
		      else
		      {
		        sprite.y += sprite.c_h;
		      }
		    }
		    else if (sprite.cols > 1)
		    {
		      if (sprite.x + sprite.c_w >= sprite.w)
		      {
		        sprite.x = 0;
		      }
		      else
		      {
		        sprite.x += sprite.c_w;
		      }
		    }
		  }
		});

		ecs.system<ConsumeEvent>("ConsumeEvents").kind(flecs::PostUpdate).term_at(0).second(flecs::Wildcard)
			.each([](flecs::iter& it) {
				while (it.next()) {
					auto event = it.id(0).second();
					event.destruct();
				}
			});

    // ecs.system<Thread>("ResetLoadingSprite").kind(flecs::OnUpdate).iter([](flecs::iter& it, Thread* thread)
    // {
    //   int aquire_mutex = pthread_mutex_trylock(&mutex);
    //   if (aquire_mutex == 0) // success!
    //   {
    //     printf("Aquired lock!\n");
    //     auto gf = ecs.lookup("os_cursor");
    //     gf.set<SpriteCreator>({"../assets/arrow.png", 8, 1}); // move from busy... make it responsive to change again...
    //     pthread_mutex_unlock(&mutex);
    //     it.entity(0).destruct();
    //   }
    // });

    auto os_cursor = ecs.entity("os_cursor");
    os_cursor.set<SpriteCreator>({"../assets/arrow.png", 1, 8});
    // os_cursor.set<SpriteCreator>({"../assets/cutlass.png", 1, 1});
    os_cursor.set<Position, World>({512.0f, 512.0f});
    os_cursor.set<LinearMovement>({5000.0f});
    os_cursor.add<HorizontalList>();
    os_cursor.add<SymbolicTargetSeq>();
    os_cursor.set<DefaultSymbolInteraction>({SymbolInteraction::LEFT_CLICK});

    auto arc_cursor = ecs.entity("arc_cursor");
    arc_cursor.set<SpriteCreator>({"../assets/silver_arrow.png", 1, 8});
    // arc_cursor.set<SpriteCreator>({"../assets/hand.png"});
    arc_cursor.set<Position, World>({512.0f, 512.0f});

    // flecs::entity os_cursor_dialogue = ecs.entity("os_cursor_dialogue")
    //   .set<TextCreator>({"The future is pixel art", "ATARISTOCRAT"})// - LVL 0
    //   .add<Position, World>()
    //   .set<Position, Local>({32.0f, 12.0f})
    //   .add<RenderStatus>()
    // .child_of(os_cursor);

    // auto os_cursor_dialogue_bkg = ecs.entity("os_cursor_dialogue_bkg")
    //   .is_a(ui_element)
    //   .set<Position, Local>({-4.0f, -12.0f})
    //   .set<BoxShape>({164.0f, 18.0f})
    //   .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0, 0, 0, 255)})
    //   .child_of(os_cursor_dialogue);

    ecs.system<LibEvDevice, KeyboardState, CursorState, Overlay, DefaultSymbolInteraction>("PollEvents")
    .term_at(1).src(user_input)
    .term_at(2).src(user_input)
    .term_at(3).src(hud)
    .term_at(4).src(os_cursor)
    .kind(flecs::PreUpdate).write(flecs::Wildcard)
    .each([display, root](flecs::entity e, LibEvDevice& levd, KeyboardState& keyboard, CursorState& cursor_state, Overlay& sail, DefaultSymbolInteraction& def)
    {
        auto ui = e.world().lookup("user_input");
        Window root_return;
        Window child_return;
        int root_x;
        int root_y;
        int win_x;
        int win_y;
        unsigned int mask_return;
        XQueryPointer(display, root, &root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask_return);
        cursor_state.x_diff = cursor_state.x - root_x;
        cursor_state.y_diff = cursor_state.y - root_y;
        cursor_state.x = root_x;
        cursor_state.y = root_y;
        ui.set<X11CursorMovement>({(float)root_x, (float)root_y, (float)cursor_state.x_diff, (float)cursor_state.y_diff});
        
        LibEvInput input;
        while (libevdev_has_event_pending(levd.dev))
        {
          int rc = libevdev_next_event(levd.dev, LIBEVDEV_READ_FLAG_NORMAL, &input.ev);
          // print_event(&input.ev);


        // int rc = libevdev_next_event(levd.dev, LIBEVDEV_READ_FLAG_NORMAL|LIBEVDEV_READ_FLAG_BLOCKING, &input.ev);
        // int rc = libevdev_next_event(levd.dev, LIBEVDEV_READ_FLAG_NORMAL|LIBEVDEV_READ_FLAG_SYNC, &input.ev);
        // if (rc == LIBEVDEV_READ_STATUS_SYNC) {
        //   printf("::::::::::::::::::::: dropped ::::::::::::::::::::::\n");
        //   while (rc == LIBEVDEV_READ_STATUS_SYNC) {
        //     print_sync_event(&input.ev);
        //     rc = libevdev_next_event(levd.dev, LIBEVDEV_READ_FLAG_SYNC, &input.ev);
        //   }
        //   printf("::::::::::::::::::::: re-synced ::::::::::::::::::::::\n");
        // } else if (rc == LIBEVDEV_READ_STATUS_SUCCESS)
        // {
          e.set<LibEvInput>(input);
          
          ui.add<ConsumeEvent>(e);

          if (input.ev.type == EV_REL)
          {
            if (input.ev.code == REL_X)
            {
              e.set<MouseMotionEvent>({input.ev.value, 0});
            } 
            if (input.ev.code == REL_Y)
            {
              e.set<MouseMotionEvent>({0, input.ev.value});
            }
          }

          if (input.ev.type == EV_KEY)
          {
            // printf("intpu.ev.code is %i\n", input.ev.code);
            if (input.ev.code == 59 && input.ev.value == 1)
            {
                VirtRigControls& rig = ecs.lookup("hud").ensure<VirtRigControls>();
                std::string recording_update = std::string("python3 record_input.py start &");
                if (rig.recording)
                {
                  std::cout << "Stop recording rig command" << std::endl;
                  recording_update = std::string("python3 record_input.py stop &");
                  if (ecs.lookup("hud").has<Background>())
                  {
                    XGrabPointer(sail.display, sail.x11_window, GLFW_FALSE, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, sail.x11_window, NULL, CurrentTime);
                    XGrabKeyboard(sail.display, sail.x11_window, GLFW_FALSE, GrabModeAsync, GrabModeAsync, CurrentTime);
                  }
                } else
                {
                  XUngrabPointer(sail.display, CurrentTime);
                  XUngrabKeyboard(sail.display, CurrentTime);
                  std::cout << "Start recording rig command" << std::endl;
                }
                system(recording_update.c_str());
                rig.recording = !rig.recording;
            }
            if (input.ev.code == 29 || input.ev.code == 97)
            {
              if (input.ev.value == 1)
              {
                ui.ensure<GraphEditor>().edit_mode = GraphEditMode::CRAFT_TELOS;
              } else if (input.ev.value == 0)
              {
                ui.ensure<GraphEditor>().edit_mode = GraphEditMode::CREATE_GRAPH;
              }
            }
            if (input.ev.code == BTN_LEFT)
            {
              // print_event(&input.ev);

              printf("Mouse button pressed! %i\n", input.ev.value);
              if (input.ev.value == 1)
              {
                // TODO: fix evdev lag?
              }
            } else if (input.ev.code == BTN_MIDDLE)
            {
              std::cout << "Cursor middle btn pressed" << std::endl;
              cursor_state.middle = input.ev.value;
            }
            bool indicate_toggle_rpg = (keyboard.key_states[KEY_LEFTMETA] || keyboard.key_states[KEY_RIGHTMETA]) && input.ev.code == KEY_LEFTALT && input.ev.value == 1;
            bool indicate_toggle_os_annotator = (keyboard.key_states[KEY_LEFTALT] || keyboard.key_states[KEY_RIGHTALT]) && input.ev.code == KEY_LEFTMETA && input.ev.value == 1;
            if (indicate_toggle_rpg || indicate_toggle_os_annotator)
            {
              auto hud = ecs.lookup("hud");
              if (hud.has<Background>())
              {
                hud.remove<LevelHUD>();
                hud.remove<AnnotatorHUD>();
                hud.remove<Background>();
                XUngrabPointer(sail.display, CurrentTime);
                XUngrabKeyboard(sail.display, CurrentTime);
                auto gf = ecs.lookup("os_cursor");
                gf.remove<Position, MoveTarget>();
              }
              else
              {
                auto gf = ecs.lookup("os_cursor");
                hud.set<Background>({nvgRGBA(0, 0, 0, 255)});
                if (indicate_toggle_rpg)
                {
                  gf.set<Position, MoveTarget>({48.0f, 80.0f});
                  hud.add<LevelHUD>();
                  hud.set<ColorAnimation>({nvgRGBA(0, 0, 0, 0), nvgRGBA(0, 0, 0, 255)});
                  // gf.add<TakeVirtScreenshot>();
                } else if (indicate_toggle_os_annotator)
                {
                  hud.add<AnnotatorHUD>();
                  gf.set<RenderStatus>({false});
                  gf.add<TakeScreenshot>();
                  // hud.set<ColorAnimation>({nvgRGBA(0, 0, 0, 0), nvgRGBA(255, 128, 0, 64)});
                  hud.set<ColorAnimation>({nvgRGBA(0, 0, 0, 0), nvgRGBA(0, 0, 0, 0)});
                }
                hud.set<Interpolator>({0.25f});
    
                XGrabPointer(sail.display, sail.x11_window, GLFW_FALSE, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, sail.x11_window, NULL, CurrentTime);
                XGrabKeyboard(sail.display, sail.x11_window, GLFW_FALSE, GrabModeAsync, GrabModeAsync, CurrentTime);
              }
            }
            if ((keyboard.key_states[KEY_LEFTMETA] || keyboard.key_states[KEY_RIGHTMETA]) && input.ev.code == KEY_GRAVE && input.ev.value == 1)
            {
              // TODO: Trigger vision system automatically, hide cursor and graphsail overlay prior to snapshot...
              // TODO: Integrate gfos, add a shader for visualizing floodfill cluster inverse prefab matching progress in realtime...
              auto gf = ecs.lookup("os_cursor");
              gf.set<SpriteCreator>({"../assets/busy.png", 8, 1});
              system("python3 os.py");
              // TODO: On vision.png created reset cursor...

              // TODO JANE: Multithreaded C++ program...
              // Thread vision;
              // TODO JANE: Level up VISION with canny edge detection, span, hierarchy, inverse prefabs, inverse textures, and border ownership
              // vision.iret = pthread_create(&vision.pthread, NULL, segment_function, NULL);
              // flecs::entity perception = ecs.entity();
              // perception.set<Thread>(vision);
            }
            // TODO: Try something like ctrl+meta focus graphsail window, then one more key unfocuses...
            if (input.ev.code == KEY_C && input.ev.value == 1 && (keyboard.key_states[KEY_LEFTCTRL] || keyboard.key_states[KEY_RIGHTCTRL]) && (keyboard.key_states[KEY_LEFTMETA] || keyboard.key_states[KEY_RIGHTMETA]))
            {
              printf("Snip segment!\n");
              SDL_Surface* screen = IMG_Load("output.png");
              // Get bounds of all the pixels in the snippet section

              // TODO: This needs to be optimized to be integrated into the existing codebase instead of being loaded from a file via distinct executable...
              std::ifstream file("segments.txt");

              typedef Uint16 flood_color_t;
              typedef Uint32 pixel_index_t;

              printf("Cursor coords are %i, %i\n", root_x, root_y);

              std::unordered_map<flood_color_t, std::vector<pixel_index_t>> pixel_flood;
              flood_color_t selected = 0;
              pixel_index_t pixel_index = 0;
              if (file.is_open()) {
                  std::string flood_color_str;
                  while (std::getline(file, flood_color_str)) 
                  {
                    int x = pixel_index % screen->w;
                    int y = pixel_index / screen->w;
                    flood_color_t flood_color = stoi(flood_color_str);
                    if (x == root_x && y == root_y)
                    {
                      selected = flood_color;
                    }
                    pixel_flood[flood_color].push_back(pixel_index);
                    pixel_index++;
                  }
              }

              int min_x = INT_MAX;
              int min_y = INT_MAX;
              int max_x = 0;
              int max_y = 0;

              printf("Selected is %i\n", selected);
              for (size_t p = 0; p < pixel_flood[selected].size(); ++p)
              {
                pixel_index_t index = pixel_flood[selected][p];
                int x = index % screen->w;
                int y = index / screen->w;
                if (x < min_x)
                {
                  min_x = x;
                }
                if (y < min_y)
                {
                  min_y = y;
                }
                if (x > max_x)
                {
                  max_x = x;
                }
                if (y > max_y)
                {
                  max_y = y;
                }
              }
              
              int w = max_x - min_x+1;
              int h = max_y - min_y+1;

              printf("Bounds are %i, %i\n", w, h);
              // Create new image with bounds
              SDL_Surface* symbol = SDL_CreateRGBSurface(0, w, h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

              // const SDL_Rect source_rect = {min_x, min_y, w, h};
              // SDL_Rect dest_rect = {0, 0, w, h};
              // SDL_BlitSurface(screen, &source_rect, symbol, &dest_rect);

              for (size_t p = 0; p < pixel_flood[selected].size(); ++p)
              {
                pixel_index_t index = pixel_flood[selected][p];
                int x = index % screen->w;
                int y = index / screen->w;
                // printf("Screen coords (%i, %i)\n", x, y);
                Uint32* screen_pixel = (Uint32 *) ((Uint8 *) screen->pixels
                                             + y * screen->pitch
                                             + x * screen->format->BytesPerPixel);
                int local_x = x - min_x;
                int local_y = y - min_y;
                // printf("Local coords (%i, %i)\n", local_x, local_y);
                Uint32* target_pixel = (Uint32 *) ((Uint8 *) symbol->pixels
                                             + local_y * symbol->pitch
                                             + local_x * symbol->format->BytesPerPixel);
                *target_pixel = *screen_pixel;
              }

              IMG_SavePNG(symbol, "symbol.png");
              SDL_FreeSurface(symbol);
              SDL_FreeSurface(screen);
              
              // TODO: Symbol inventory UX
              flecs::entity osc = ecs.lookup("os_cursor");
              HorizontalList& hl = osc.ensure<HorizontalList>();
              float p_x = hl.x;
              hl.x += w + 4.0f;
              
              SymbolNavigator symbol_navigator = {{root_x, root_y}, def.interaction};
              osc.ensure<SymbolicTargetSeq>().navigations.push_back(symbol_navigator);
              osc.set<TargetSymbolLocation>(symbol_navigator.location);

              flecs::entity os_symbol = ecs.entity() // "os_symbol"
                .set<SpriteCreator>({"symbol.png"})
                .add<Position, World>()
                .set<Position, Local>({p_x + 32.0f, 0.0f})
                .child_of(osc);

              // TODO: Keep track of symbols added so far, append horizontal list to Local pos

              // TODO: Contextualize the symbol frame in agentic memory
              // Create a simple command which moves the mouse to the selected
              // symbol, clicks, and then returns...
              // TODO: Path along a playful bezier curve
              // TODO: Render the curve
              // TODO: Render the symbol indicator overlay
              // xdo_move_mouse(xdo, 100, 100, NULL);
              // char xdo_cmd[100];
              // snprintf(xdo_cmd, sizeof(xdo_cmd), "xdotool mousemove %i, %i", 100, 100);
              // system(xdo_cmd);

              // TODO: Render the floodfill snippet on the graphsail as a node
            }
            if (input.ev.value == 1 && (keyboard.key_states[KEY_LEFTMETA] || keyboard.key_states[KEY_RIGHTMETA]))
            {
              if (input.ev.code == KEY_1)
              {
                printf("click mode\n");
                def.interaction = SymbolInteraction::LEFT_CLICK;
              }
              if (input.ev.code == KEY_2)
              {
                printf("mousedown mode\n");
                def.interaction = SymbolInteraction::LEFT_MOUSE_DOWN;
              }
              if (input.ev.code == KEY_3)
              {
                printf("mouseup mode\n");
                def.interaction = SymbolInteraction::LEFT_MOUSE_UP;
              }
              if (input.ev.code == KEY_ESC)
              {
                e.world().quit();
              } else if (input.ev.code == KEY_SPACE)
              {
                // If os_cursor is 'thinking' about a symbol
                auto gf = ecs.lookup("os_cursor");
                auto seq = gf.ensure<SymbolicTargetSeq>();
                if (seq.navigations.size() > 0)
                {
                  SymbolNavigator sn = gf.ensure<SymbolicTargetSeq>().navigations[0];
                  // SymbolInteraction si = gf.get_mut<DefaultSymbolInteraction>()->interaction;
                  SymbolInteraction si = sn.interaction;
                  if (si == LEFT_CLICK)
                  {
                    gf.set<ClickOnTarget>({1});
                  } else if (si == LEFT_MOUSE_DOWN)
                  {
                    gf.set<MouseDownOnTarget>({1});
                  } else if (si == LEFT_MOUSE_UP)
                  {
                    gf.set<MouseUpOnTarget>({1});
                  }
                  // auto tsl = gf.try_get<TargetSymbolLocation>();
                  // TODO: SEARCH FOR SYMBOL ON SCREEN
                  // In the most basic case, let's start by searching for a floodfill
                  // with the same bounds, then check pixel by pixel if it is identital
                  // Perhaps consider ratio search with perceptual hashing?
                  // Alternatively cluster embedding
                  // Example, use MeshGraphNet by deepmind for symbolic mesh pathfinding
                  // Ex: Relative to window, open ended prefab relation neighbors
                  // TODO: Prefab inverse template matching
                  // TODO: Perception hash realtime vision system
                  auto tsl = sn.location;
                  gf.set<Position, MoveTarget>({(float)tsl.x, (float)tsl.y});
                  gf.set<SpriteCreator>({"../assets/move.png", 8, 1});
                  // TODO: Understand/cache window movement/status... (consider application favicon symbol combo origin...) 
                  // eg future simulation approval?
                }
              }
            }
            keyboard.key_states[input.ev.code] = input.ev.value;
          }

          // print_event(&input.ev);
        }
    });

    auto ui = ecs.entity("ui");
    ui.set<FontCreator>({"CharisSIL", "../assets/CharisSIL-Regular.ttf"});
    ui.set<FontCreator>({"ATARISTOCRAT", "../assets/ATARISTOCRAT.ttf"});

    // TODO: Automatically find...
    auto keyboards = find_devices({{EV_KEY, KEY_A}});
    for (size_t i = 0; i < keyboards.size(); i++)
    {
      auto keyboard = ecs.entity();
      keyboard.set<DeviceEventListener>({keyboards[i].c_str()});
    }

    auto mice = find_devices({{EV_KEY, BTN_LEFT}, {EV_REL, REL_X}});
    for (size_t i = 0; i < mice.size(); i++)
    {
      auto mouse = ecs.entity();
      mouse.set<DeviceEventListener>({mice[i].c_str()});
    }

		ecs.system<LinearMovement, Position, Position*>("MoveTowardsTarget").term_at(1).second<Local>().term_at(2).second<MoveTarget>().term_at(3).optional().kind(flecs::OnUpdate)
		.each([](flecs::iter& it, flecs::entity e, LinearMovement& movement, Position& pos, Position* target)
		{
		  // Consider Fréchet distance for evaluation of mouse move style
		  if (!target) return;
		  
		  vec2 src = {pos.x, pos.y};
		  vec2 dest = {target->x, target->y};
		  vec2 diff;
		  glm_vec2_sub(dest, src, diff);
		  vec2 zero = {0.0f, 0.0f};
		  float dist = glm_vec2_distance(zero, diff);
		  vec2 update;
		  glm_vec2_copy(src, update);
		  float am = movement.speed * it.delta_time();//*1000.0f;
		  if (am > 0.0f && dist > 0.0f)
		  {
		    glm_vec2_lerp(src, dest, am/dist, update);
		  }
		  pos.x = update[0];
		  pos.y = update[1];
		  // auto hud = ecs.lookup("hud");
		  // if (!hud.has<Background>()) // If LevelHUD is enabled, we want to disable control of the OS cursor, obviously implement better scene management...
		  // {
		  //   char xdo_cmd[100];
		  //   snprintf(xdo_cmd, sizeof(xdo_cmd), "xdotool mousemove %i, %i", (int)pos.x, (int)pos.y);
		  //   system(xdo_cmd);
		  // }
		  // if (dist == 0.0f)
		  // {
		  //   e.add<ReachedTargetEvent>();
		  // }
		});

    ecs.system<CubicMovement, Position, Position, Position>("MoveTowardsTargetCubic")
    .term_at(1).second<Local>()
    .term_at(2).second<Source>()
    .term_at(3).second<MoveTarget>()
    .kind(flecs::OnUpdate)
    .each([](flecs::iter& it, size_t i, CubicMovement& movement, Position& pos, Position& source, Position& target) {
      vec2 src = {source.x, source.y};
      vec2 dest = {target.x, target.y};
      vec2 diff;
      glm_vec2_sub(dest, src, diff);
      vec2 zero = {0.0f, 0.0f};
      float dist = glm_vec2_distance(zero, diff);
      vec2 update;
      glm_vec2_copy(src, update);

      movement.progress += movement.speed * it.delta_time() / dist;
      float t = movement.progress; // Normalized time
      if (t > 1.0f) t = 1.0f; // Clamp to ensure we don't overshoot

      // Quadratic ease in/out
      float factor;
      if (t < 0.5) {
        factor = 2 * t * t; // Ease in
      } else {
        factor = -2 * t * t + 4 * t - 1; // Ease out
      }
    
      if (factor > 0.0f && dist > 0.0f) {
        glm_vec2_lerp(src, dest, factor, update); // Use 'factor' for interpolation
      }

      pos.x = update[0];
      pos.y = update[1];
      if (t == 1.0f)
      {
        it.entity(i).children([&](flecs::entity child)
        {
          child.destruct();
        });
      }
    });


		ecs.system<Position, Position*>("CursorRadiusFollow").term_at(0).second<World>().src(os_cursor).term_at(1).second<MoveTarget>().optional().src(os_cursor).kind(flecs::PreUpdate)
			.each([display, root](Position& pos, Position* move)
			{
			  if (move) return;
			  float follow_radius = 32.0f;
	
			  Window root_return;
			  Window child_return;
			  int root_x;
			  int root_y;
			  int win_x;
			  int win_y;
			  unsigned int mask_return;
			  XQueryPointer(display, root, &root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask_return);
			  // printf("Pointer is at %i, %i\n", root_x, root_y);
	
			  vec2 a = {pos.x, pos.y};
			  vec2 b = {(float)root_x - 16.0f, (float)root_y - 16.0f}; // tmp increase  + 48
			  vec2 close;
			  get_closest_radius_point(a, b, follow_radius, &close);
	
			  pos.x = close[0];
			  pos.y = close[1];
			});

	ecs.system<Position, Position>("ArcCursorRadiusFollow").term_at(0).second<World>().src(os_cursor).term_at(1).second<World>().src(arc_cursor).kind(flecs::PreUpdate)
        .each([](Position& os_cursor_pos, Position& arc_cursor_pos)
    {

      float follow_radius = 32.0f;

      vec2 a = {arc_cursor_pos.x, arc_cursor_pos.y};
      vec2 b = {(float)os_cursor_pos.x, (float)os_cursor_pos.y};
      vec2 close;
      get_closest_radius_point(a, b, follow_radius, &close);

      arc_cursor_pos.x = close[0];// - 380.0f;
      arc_cursor_pos.y = close[1];
    });

	ecs.observer<HighlightSentence, SengraNodes, SelectedSentence*>("HighlightSentenceSubgraph").term_at(2).second(flecs::Wildcard).src(user_input).optional().write(flecs::Wildcard).event(flecs::OnAdd)
	.each([](flecs::entity select_sentence, HighlightSentence& highlight, SengraNodes& sengra, SelectedSentence* selected)
	{
		auto user_input = ecs.lookup("user_input");
		auto q = ecs.query_builder<HighlightSentence, SengraNodes>()
		  .build();
		q.each([&](flecs::entity sentence, HighlightSentence& all_sent, SengraNodes& all_sengra)
		{
		  if (select_sentence != sentence)
		  {
		    sentence.remove<HighlightSentence>();
		    user_input.remove<SelectedSentence>(sentence);
		    for (flecs::entity node : all_sengra.subgraph)
		    {
		      node.remove<Interpolator>();
		      node.remove<ColorAnimation>();
		      node.ensure<RenderStatus>().color = nvgRGBA(0x5a, 0x69, 0x88, 255);
		      auto emoji = node.parent().lookup("emoji");
		      emoji.ensure<EdgeRenderStatus>().color.a = 0.0f;
		    }
		  }
		});

		user_input.add<SelectedSentence>(select_sentence);
		std::vector<NVGcolor> colors = get_palette("../assets/palettes/highlight_2.hex");
		size_t sent_token = 0;
		for (flecs::entity node : sengra.subgraph)
		{
		  node.set<Interpolator>({0.1f, 0.0f, 0.0f, sent_token * 0.01f});
		  const RenderStatus& rs = node.get<RenderStatus>();
		  NVGcolor color = colors[sent_token % colors.size()];
		  node.set<ColorAnimation>({rs.color, nvgRGBA(color.r*255.0f, color.g*255.0f, color.b*255.0f, 255), rs.color});
		  auto emoji = node.parent().lookup("emoji");
		  emoji.ensure<EdgeRenderStatus>().color.a = 0.5f;
		  sent_token++;
		}
	});

    // TODO JANE: Use a KD tree on cursor movement for bound hover detection
		ecs.system<UIElementBounds, X11CursorMovement, SengraNode>("HighlightHoveredSentence").write(flecs::Wildcard)
    .term_at(1).src(user_input)
    .term_at(2).second(flecs::Wildcard)
    .each([](flecs::entity node, UIElementBounds& bounds, X11CursorMovement& cursor)
    {
      c2v cursor_pos {cursor.x, cursor.y};
      c2AABB box = {{bounds.xmin, bounds.ymin}, {bounds.xmax, bounds.ymax}};
      if (c2AABBtoPoint(box, cursor_pos))
      {
        flecs::entity sentence = node.target<SengraNode>();
        sentence.add<HighlightSentence>();
      }
    });

		ecs.system<UIElementBounds, X11CursorMovement, DragNodeSelector, EventOnGrab>("SelectGraphNodesWithDrag").write(flecs::Wildcard)
    .term_at(1).src(user_input)
    .term_at(2).src(user_input)
    .term_at(3).second<SelectGraphNode>()
		.each([](flecs::entity node, UIElementBounds& bounds, X11CursorMovement& cursor, DragNodeSelector& drag, EventOnGrab& on_grab)
		{
		  c2v cursor_pos {cursor.x, cursor.y};
		  c2AABB box = {{bounds.xmin, bounds.ymin}, {bounds.xmax, bounds.ymax}};
		  if (c2AABBtoPoint(box, cursor_pos))
		  {
		    node.add<NodeIsSelected>();
		  }
		});

		ecs.system<SpeechStream, GraphRail, GraphSpeak>("UpdateSpeechStream").immediate().term_at(2).up()
		    .each([ui_element](flecs::entity graph_rail, SpeechStream& stream, GraphRail& rail, GraphSpeak& speak)
    {
      flecs::entity graph_root = graph_rail.parent();

      NVGcolor ss_color = nvgRGBA(255, 85, 0, 255); // Orange Book vs Red Read speech streams...

      float start_duration_progress = speak.progress - stream.duration/2.0f;

      if (stream.start_index >= 1 && start_duration_progress < stream.words[stream.start_index-1].end)
      {
        size_t lshift = 1;
        flecs::entity output[3];
        flecs::entity node_root = graph_root.target<Next>();
        size_t iz = stream.start_index-lshift;
        SpokenWord verbal = stream.words[iz];
        float cx = (verbal.start * 200.0f);
        Position pos = {cx, 0};
        create_graph_node(pos.x, pos.y, output);

        output[0].add<Next>(node_root);
        node_root.add<Prev>(output[0]);
        node_root.remove<Head>(graph_root);
        output[1].add<Head>(graph_root);

        output[0].set<GraphNodeData>({iz});
        output[0].set<SpokenWord>(verbal);
        output[2].set<TextCreator>({verbal.word, "CharisSIL"});
        output[2].set<Position, Local>({0.0f, -24.0f});
        output[1].add<EventOnGrab, SelectGraphNode>();
        output[1].set<EmojiSlideWord>({graph_root, graph_rail});
        output[1].remove<Align>();

        auto speech_slider_background = ecs.entity()
          .is_a(ui_element)
          .set<CapsuleShape>({7.0f, 14.0f + (verbal.end - verbal.start) * 200.0f})
          .add<LevelHUD>()
          .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0, 0, 0, 255)})
          .add<SelectionStatic>()
          .child_of(output[0]);

        auto speech_slider = ecs.entity()
          .is_a(ui_element)
          .set<CapsuleShape>({7.0f, 14.0f + (verbal.end - verbal.start) * 200.0f})
          .add<LevelHUD>()
          .set<RenderStatus>({true, RenderMode::STROKE, ss_color})
          .child_of(output[0]);

        UIElementBounds& bounds = output[2].ensure<UIElementBounds>();

        output[0].child_of(graph_rail);
        output[2].remove<TextInput>();

        flecs::entity tail = node_root;
        while (tail.has<Next>(flecs::Wildcard))
        {
          tail = tail.target<Next>();
        }
        flecs::entity tail_prev = tail.target<Prev>();
        tail_prev.remove<Next>(tail);
        tail.destruct();

        // TODO: Get emoji of node_root
        flecs::entity root_emoji;
        node_root.children([&](flecs::entity child)
        {
          if (child.has<CircleShape>())
          {
            root_emoji = child; // TODO: Just use a prefab slot already sheesh
          }
        });

        output[1].add<Edge>(root_emoji);
        output[1].set<EdgeRenderStatus>({2.0f, ss_color});
        output[1].set<RenderStatus>({true, RenderMode::FILL, ss_color});
        stream.start_index--;
        stream.end_index--;
        graph_root.remove<Next>(flecs::Wildcard);
        graph_root.add<Next>(output[0]);
        return;
      }


      for (size_t iota = stream.start_index; iota < stream.end_index; ++iota)
      {
        if (start_duration_progress > stream.words[iota].end)
        {
          size_t rshift = iota - stream.start_index + 1;
          if (stream.end_index + rshift >= stream.words.size())
          {
            rshift = 0;
            return;
          }
          flecs::entity node_root = graph_root.target<Next>();
          for (size_t zeta = 0; zeta < rshift; ++zeta)
          {
            bool has_next = node_root.has<Next>(flecs::Wildcard);
            if (has_next)
            {
              flecs::entity tmp = node_root.target<Next>();
              if (node_root.is_valid())
              {
                node_root.destruct();
              }
              node_root = tmp;
            } else
            {
              break;
            }
          }
          graph_root.remove<Next>(flecs::Wildcard);
          graph_root.add<Next>(node_root);
          node_root.add<Head>(graph_root);
          stream.start_index = iota+1;
          flecs::entity tail = node_root;
          // TODO: Consider relationship to tail entity in graph_root
          while (tail.has<Next>(flecs::Wildcard))
          {
            tail = tail.target<Next>();
          }
          flecs::entity output[3];
          flecs::entity last_node = tail;
          flecs::entity last_text;
          flecs::entity last_emoji;
          tail.children([&](flecs::entity child)
          {
            if (child.has<CircleShape>())
            {
              last_emoji = child;
            }
          });
          for (size_t zeta = 0; zeta < rshift; ++zeta)
          {
            size_t iz = stream.end_index+1+zeta;
            SpokenWord verbal = stream.words[iz];
            float cx = (verbal.start * 200.0f);
            Position pos = {cx, 0};
            create_graph_node(pos.x, pos.y, output);
            if (last_node.is_valid())
            {
              output[0].add<Prev>(last_node);
              last_node.add<Next>(output[0]);
            }
            output[0].set<GraphNodeData>({iz});
            output[0].set<SpokenWord>(verbal);
            output[2].set<TextCreator>({verbal.word, "CharisSIL"});
            output[2].set<Position, Local>({0.0f, -24.0f});
            output[1].add<EventOnGrab, SelectGraphNode>();
            output[1].set<EmojiSlideWord>({graph_root, graph_rail});
            output[1].remove<Align>();

            auto speech_slider_background = ecs.entity()
              .is_a(ui_element)
              .set<CapsuleShape>({7.0f, 14.0f + (verbal.end - verbal.start) * 200.0f})
              .add<LevelHUD>()
              .set<RenderStatus>({true, RenderMode::FILL, nvgRGBA(0, 0, 0, 255)})
              .add<SelectionStatic>()
              .child_of(output[0]);

            auto speech_slider = ecs.entity()
              .is_a(ui_element)
              .set<CapsuleShape>({7.0f, 14.0f + (verbal.end - verbal.start) * 200.0f})
              .add<LevelHUD>()
              .set<RenderStatus>({true, RenderMode::STROKE, ss_color})
              .child_of(output[0]);

            UIElementBounds& bounds = output[2].ensure<UIElementBounds>();

            output[0].child_of(graph_rail);
            output[1].set<RenderStatus>({true, RenderMode::FILL, ss_color});
            output[2].remove<TextInput>();
            last_emoji.add<Edge>(output[1]);
            last_emoji.set<EdgeRenderStatus>({2.0f, ss_color});
            last_emoji = output[1];
            last_node = output[0];
            last_text = output[2];
          }
          stream.end_index += rshift;

        }
        break;
      }

    });

    // ecs.system<LibEvDevice, LibEvInput, Position, TrackpadStatus>("CursorRadiusFollow").term_at(0).src(trackpad).term_at(2).second<World>().src(os_cursor).term_at(3).src(trackpad).kind(flecs::OnUpdate).iter([](flecs::iter& it, LibEvDevice* levd, LibEvInput* input, Position* pos, TrackpadStatus* tstat) 
    // {
    //   for (size_t i : it)
    //   {
    //     input_event ev = input[i].ev;
    //     if (ev.type == EV_KEY)
    //     {
    //       if (ev.code == BTN_TOUCH)
    //       {
    //         if (ev.value == 1)
    //         {
    //           tstat->tx = true;
    //           tstat->ty = true;
    //         }
    //       }
    //     }
    //     if (ev.type == EV_ABS)
    //     {
    //       const struct input_absinfo *abs;
    //       // TODO: Update for ev.code unordered_map
    //       if (ev.code == ABS_MT_POSITION_X)
    //       {
    //         abs = libevdev_get_abs_info(levd->dev, ABS_X);
    //         if (tstat->tx)
    //         {
    //           tstat->tx = false;
    //           tstat->start.x = abs->value;
    //         } else
    //         {
    //           pos->x += abs->value - tstat->last.x;
    //         }
    //         tstat->last.x = abs->value;
    //       }
    //       if (ev.code == ABS_Y)
    //       {
    //         abs = libevdev_get_abs_info(levd->dev, ABS_Y);
    //         if (tstat->ty)
    //         {
    //           tstat->ty = false;
    //           tstat->start.y = abs->value;
    //         } else
    //         {
    //           pos->y += abs->value - tstat->last.y;
    //         }
    //         tstat->last.y = abs->value;
    //       }
    //     }
    //   }
    // });

		ecs.system<const Position*, const Position*, Position, const Position*>("CascadeTransform")
        .term_at(0).second<Local>()
        .optional()
        .term_at(1).second<World>()
        .term_at(2).second<World>()
        .term_at(1)
            // Get from the parent, in breadth-first order (cascade)
            .parent().cascade()
            // Make term component optional so we also match the root
            .optional()
        .kind(flecs::PostUpdate)
        .term_at(3).second<Relative>().optional()
    .each([](const Position *p, const Position *p_parent, Position *p_out, const Position* p_rel)
    {
      if (p)
      {
          p_out->x = p->x;
          p_out->y = p->y;
          if (p_parent) {
              p_out->x += p_parent->x;
              p_out->y += p_parent->y;
          }
          if (p_rel)
          {
            p_out->x += p_rel->x;
            p_out->y += p_rel->y;
          }
      }
    });

		ecs.system<const UIElementBounds, UIElementBounds*>("ResetBounds")
		  .term_at(1)
		      .parent().up()
		  .kind(flecs::PostFrame)
		  .each([](const UIElementBounds& bounds, UIElementBounds* parent_bounds)
		{
		    if (parent_bounds)
		    {
		      parent_bounds->xmin = 10000.0f;
		      parent_bounds->ymin = 10000.0f;
		      parent_bounds->xmax = 0;
		      parent_bounds->ymax = 0;
		    }
		});

		ecs.system<Position, UIElementBounds, TextCreator, NanoVG>("TextUpdateBounds").term_at(0).second<World>().term_at(3).src(renderer).kind(flecs::PostFrame)
    .each([](Position& pos, UIElementBounds& bounds, TextCreator& text, NanoVG& vg)
    {
      nvgFontFace(vg.ctx, text.font_name.c_str());
      nvgTextBounds(vg.ctx, pos.x, pos.y, text.word.c_str(), NULL, &bounds.xmin);
    });

		ecs.system<Position, UIElementBounds, Sprite, NanoVG>("SpriteUpdateBounds").term_at(0).second<World>().term_at(3).src(renderer).kind(flecs::PostFrame)
		.each([](Position& pos, UIElementBounds& bounds, Sprite& sprite, NanoVG& vg)
		{
		  bounds.xmin = pos.x;
		  bounds.ymin = pos.y;
		  bounds.xmax = pos.x + sprite.c_w;
		  bounds.ymax = pos.y + sprite.c_h;
		});

		ecs.system<Position, UIElementBounds, CircleShape, NanoVG>("CircleUpdateBounds").term_at(0).second<World>().term_at(3).src(renderer).kind(flecs::PostFrame)
		    .each([](Position& pos, UIElementBounds& bounds, CircleShape& circle, NanoVG& vg) {
		        bounds.xmin = pos.x + -circle.radius;
		        bounds.ymin = pos.y + -circle.radius;
		        bounds.xmax = pos.x + circle.radius;
		        bounds.ymax = pos.y + circle.radius;
		    });

		ecs.system<Position, UIElementBounds, RoundTriShape, NanoVG>("TriUpdateBounds").term_at(0).second<World>().term_at(3).src(renderer).kind(flecs::PostFrame)
			.each([](Position& pos, UIElementBounds& bounds, RoundTriShape& tri, NanoVG& vg)
			{
				bounds.xmin = pos.x + -tri.triangle_radius;
				bounds.ymin = pos.y + -tri.triangle_radius;
				bounds.xmax = pos.x + tri.triangle_radius;
				bounds.ymax = pos.y + tri.triangle_radius;
			});

		ecs.system<Position, UIElementBounds, DiamondShape, NanoVG>("DiamondUpdateBounds").term_at(0).second<World>().term_at(3).src(renderer).kind(flecs::PostFrame)
		.each([](Position& pos, UIElementBounds& bounds, DiamondShape& diamond, NanoVG& vg) {
		  bounds.xmin = pos.x + -diamond.radius;
		  bounds.ymin = pos.y + -diamond.radius;
		  bounds.xmax = pos.x + diamond.radius;
		  bounds.ymax = pos.y + diamond.radius;
		});

		ecs.system<Position, UIElementBounds, BoxShape, NanoVG>("BoxUpdateBounds").term_at(0).second<World>().term_at(3).src(renderer).kind(flecs::PostFrame)
		.each([](Position& pos, UIElementBounds& bounds, BoxShape& box, NanoVG& vg) {
		  bounds.xmin = pos.x;
		  bounds.ymin = pos.y;
		  bounds.xmax = pos.x + box.width;
		  bounds.ymax = pos.y + box.height;
		});

		ecs.system<Position, UIElementBounds, Bode, NanoVG>("BodeUpdateBounds").term_at(0).second<World>().term_at(3).src(renderer).kind(flecs::PostFrame)
		.each([](Position& pos, UIElementBounds& bounds, Bode& bode, NanoVG& vg)
		{
			bounds.xmin = pos.x;
			bounds.ymin = pos.y;
			bounds.xmax = pos.x + bode.width;
			bounds.ymax = pos.y + bode.height + bode.node_slot_radius;
		});

		ecs.system<const UIElementBounds, UIElementBounds*, RenderStatus>("BubbleUpBounds")
      .term_at(1)
          .parent().up()
      .term_at(2).optional()
      .kind(flecs::PostFrame)
      .each([](flecs::entity e, const UIElementBounds& bounds, UIElementBounds* parent_bounds, RenderStatus* render)
    {
      if (parent_bounds && (!render || render->visible))
      {
          parent_bounds->xmin = std::min(parent_bounds->xmin, bounds.xmin);
          parent_bounds->ymin = std::min(parent_bounds->ymin, bounds.ymin);
          parent_bounds->xmax = std::max(parent_bounds->xmax, bounds.xmax);
          parent_bounds->ymax = std::max(parent_bounds->ymax, bounds.ymax);
          // printf("Update parent world bounds to %f, %f, %f, %f\n", parent_bounds->xmin, parent_bounds->ymin, parent_bounds->xmax, parent_bounds->ymax);
      }
    });

    // For interpolated rendering from SFML Audio
	ecs.system<GraphRail, Position, GraphAudio*, GraphSpeak>("SpeakGraph").term_at(1).second<Local>().term_at(2).parent().term_at(3).parent().kind(flecs::OnUpdate)
	    .each([ui_element](GraphRail& rail, Position& pos, GraphAudio* graph_audio, GraphSpeak& speak)
	    {
	      if (graph_audio->music->getStatus() == sf::SoundSource::Status::Playing)
	      {
	        speak.progress = graph_audio->music->getPlayingOffset().asSeconds();
	        pos.x = rail.x_per_second * -speak.progress;
	      }
	    });

		ecs.system<EmojiSlideWord, Position, SpokenWord>("SlideEmoji").term_at(1).second<Local>().term_at(2).parent().kind(flecs::OnUpdate)
			.each([](EmojiSlideWord& emoji, Position& pos, SpokenWord& spoken)
			{
				if (emoji.graph.has<GraphSpeak>())
				{
					auto speak = emoji.graph.try_get<GraphSpeak>();
					auto rail = emoji.rail.try_get<GraphRail>();
					float duration = spoken.end - spoken.start;
					pos.x = std::clamp((speak->progress - spoken.start) * rail->x_per_second, 0.0f, 14.0f + duration * rail->x_per_second - 14.0f);
				}
			});

		ecs.system<GraphData, GraphStream, NanoVG*>("StreamGraph").immediate().term_at(2).src(renderer).kind(flecs::OnUpdate)
		    .each([ui_element](flecs::entity e, GraphData& data, GraphStream& stream, NanoVG* vg)
		    {
		      stream.progress += e.world().delta_time();
		      float tick = 1.0f/stream.nodes_per_second;
		      if (stream.progress > tick)
		      {
		        stream.progress -= tick;
		        stream_sengra_flow(e, 1, ui_element, vg);
		      }
		    });

		ecs.observer<GraphAudio>("PlayOnLoad").event(flecs::OnSet)
			.each([](flecs::entity e, GraphAudio& graph_audio) {
				std::cout << "PlayOnLoad" << std::endl;
				graph_audio.music = new sf::Music();
				graph_audio.music->openFromFile(graph_audio.filepath.c_str());
				graph_audio.music->play();
				graph_audio.music->pause();
			});
    

    // TODO: Scene management with bode graph natural language queries (to GraphSim)
    flecs::entity library = ecs.entity("library")
      .is_a(ui_element)
      .add<VerticalLayoutBox>();

    flecs::entity vision = ecs.entity("vision_overlay")
      .is_a(ui_element)
      .set<SpriteCreator>({"vision.png"})
      .add<AnnotatorHUD>();
    
	ecs.system<Button, Toggle, EventSelectBtn, GraphEditor, HorizontalBoundaryChisel, GraphEditMode>("ChangeGraphEditMode")
  .term_at(3).src("user_input")
  .term_at(4).src(chisel)
  .term_at(5).second(flecs::Wildcard)
	.each([](flecs::entity e, Button& btn, Toggle& toggle, EventSelectBtn& selected, GraphEditor& gem, HorizontalBoundaryChisel& chisel, GraphEditMode& graph_edit_mode)
	{
	  gem.edit_mode = graph_edit_mode;
	  if (graph_edit_mode == GraphEditMode::CRAFT_TELOS)
	  {
	    // Reset chisel...
	    chisel.lock_anchor = false;
	    chisel.lock_selection = false;
	    chisel.anchor_x = 0.0f;
	    chisel.selection_x = 0.0f;
	  }
	  printf("%s\n", e.type().str().c_str());
	  // e.remove<EventSelectBtn>();
	});

    // ECS default editor scene graphs ----

    ecs_script_run_file(ecs, "../assets/plecs/ecs_default.flecs");
    flecs::entity ecs_default = ecs.lookup("ecs_default");
    flecs::entity ecs_default_sg = parse_scene_graph(ecs_default, scene_graph_selection_indicator, scene_graph_node, ui_element, "prefab");

    ecs_script_run_file(ecs, "../assets/plecs/component_update.flecs");
    flecs::entity component_update = ecs.lookup("component_update");
    flecs::entity component_update_sg = parse_scene_graph(component_update, scene_graph_selection_indicator, scene_graph_node, ui_element, "int");

    ecs_script_run_file(ecs, "../assets/plecs/prefab_update.flecs");
    flecs::entity prefab_update = ecs.lookup("prefab_update");
    flecs::entity prefab_update_sg = parse_scene_graph(prefab_update, scene_graph_selection_indicator, scene_graph_node, ui_element, "prefab");

    // ----

    ecs_script_run_file(ecs, "../assets/plecs/gnn_default.flecs");
    flecs::entity gnn_default = ecs.lookup("gnn_default");
    flecs::entity gnn_default_sg = parse_scene_graph(gnn_default, scene_graph_selection_indicator, scene_graph_node, ui_element, "dataset");

    ecs_script_run_file(ecs, "../assets/plecs/os_default.flecs");
    flecs::entity os_default = ecs.lookup("os_default");
    flecs::entity os_default_sg = parse_scene_graph(os_default, scene_graph_selection_indicator, scene_graph_node, ui_element, "companion");

    // ---- Internode scene graphs
    ecs_script_run_file(ecs, "../assets/plecs/prefab_output.flecs");
    flecs::entity prefab_output = ecs.lookup("prefab_output");
    flecs::entity prefab_output_sg = parse_scene_graph(prefab_output, scene_graph_selection_indicator, scene_graph_node, ui_element, "entity");

    SceneGraphAmbLoader prefab_output_loader;
    prefab_output_loader.graphs[ECS_NodeType::STANDARD] = prefab_output_sg;

    ecs_script_run_file(ecs, "../assets/plecs/component_output.flecs");
    flecs::entity component_output = ecs.lookup("component_output");
    flecs::entity component_output_sg = parse_scene_graph(component_output, scene_graph_selection_indicator, scene_graph_node, ui_element, "plecs");

    SceneGraphAmbLoader component_output_loader;
    component_output_loader.graphs[ECS_NodeType::STANDARD] = component_output_sg;

    // ----

    flecs::entity bode_hud = ecs.entity()
      .is_a(ui_element)
      .set<Position, Local>({width/2 -128*2.5f, 0})
      .child_of(hud);

    flecs::entity bode_menu = ecs.entity()
      .is_a(ui_element)
      .add<HorizontalLayoutBox>()
      .set<Position, Local>({0.0f, -1.0f})
      .child_of(bode_hud);

    flecs::entity bode[5];
    auto read_bode = create_bode("read_graph", "Read", nvgRGBA(255, 0, 0, 255), ui_element, &bode[0], 800.0f, 0.0f, 1001);
    bode[4].add<EventOnConnect, ReadGraph>();
    read_bode.child_of(bode_menu);
    
    auto book_bode = create_bode("create_book", "Books", nvgRGBA(255, 85, 0, 255), ui_element, &bode[0], 900.0f, 0.1f, 1002);
    bode[4].add<EventOnConnect, AttemptLoadBook>();
    book_bode.child_of(bode_menu);

    // auto gnn_bode = create_bode("create_gnn", "GNN", nvgRGBA(0, 255, 134, 255), ui_element, &bode[0]);
    auto gnn_bode = create_bode("create_gnn", "GNN", nvgRGBA(32, 210, 19, 255), ui_element, &bode[0], 1000.0f, 0.2f, 1003);
    bode[4].add<EventOnConnect, OpenECS_SceneGraph>();
    SceneGraphAmbLoader gnn_loader;
    gnn_loader.graphs[ECS_NodeType::STANDARD] = gnn_default_sg;
    bode[4].set<SceneGraphAmbLoader>(gnn_loader);
    gnn_bode.child_of(bode_menu);

    // TODO: Color config plecs
    auto ecs_bode = create_bode("create_ecs", "ECS",  nvgRGBA(0, 255, 242, 255), ui_element, &bode[0], 1100.0f, 0.3f, 1004);
    bode[4].add<EventOnConnect, OpenECS_SceneGraph>();
    // bode[4].add<SceneGraphLoader>(ecs_default_sg);
    SceneGraphAmbLoader ecs_loader;
    ecs_loader.graphs[ECS_NodeType::STANDARD] = ecs_default_sg;
    ecs_loader.graphs[ECS_NodeType::PREFAB] = prefab_update_sg;
    // ecs_loader.graphs[ECS_NodeType::ENTITY] = ;
    ecs_loader.graphs[ECS_NodeType::COMPONENT] = component_update_sg;
    bode[4].set<SceneGraphAmbLoader>(ecs_loader);
    ecs_bode.child_of(bode_menu);

    auto os_bode = create_bode("os_action", "OS",  nvgRGBA(140, 22, 219, 255), ui_element, &bode[0], 1200.0f, 0.4f, 1005);
    bode[4].add<EventOnConnect, OpenECS_SceneGraph>();
    SceneGraphAmbLoader os_loader;
    os_loader.graphs[ECS_NodeType::STANDARD] = os_default_sg;
    bode[4].set<SceneGraphAmbLoader>(os_loader);
    // os_loader.graphs[ECS_NodeType::COMPANION] = ;
    // os_loader.graphs[ECS_NodeType::LINUX] = ;
    os_bode.child_of(bode_menu);

    // TODO: BOOK LOADER!
    flecs::entity book_loader = ecs.entity("book_loader");
    // TODO: Bode graph query


    flecs::entity cartridge = ecs.entity()
      .is_a(ui_element)
      .set<VerticalLayoutBox>({0.0f, 2.0f})
      .set<Position, Local>({width-16.0f, 128.0f});
      // .set<Align>(0.0f, 0.0f);
    for (size_t i = 0; i < 10; ++i)
    {
      flecs::entity save_layer = ecs.entity()
      .is_a(ui_element)
      .set<SpriteCreator>({"../assets/save_layer_" + std::to_string(i%2) + ".png"})
      .set<CartridgeBlock>({i})
      .add<OnClickEvent, SetCartridge>()
      .add<HoverEvent>()
      .set<SpriteAlpha>({0.2})
      .child_of(cartridge);
    }

		ecs.observer<ArcColorMask, RenderStatus, EventSelectBtn*>("ToggleArcColorMask").event(flecs::OnSet)
			.each([](flecs::entity e, ArcColorMask& mask, RenderStatus& render, EventSelectBtn* event)
			{
				mask.enabled = !mask.enabled;
				render.mode = mask.enabled ? RenderMode::FILL : RenderMode::STROKE;
				if (e.has<Sprite>())
				{
					if (mask.enabled)
					{
						e.set<SpriteCreator>({"../assets/wildcard.png"});
					} else
					{
						e.set<SpriteCreator>({"../assets/wildcard_unselected.png"});
					}
				}

				// auto q_arc_color_masks = ecs.query_builder<ArcColorMask>()
				//         .build();

				// std::unordered_map<int, bool> mask_states;
				// q_arc_color_masks.iter([&](flecs::iter& mask_iter, ArcColorMask* all_mask) {
				//   for (size_t m_i : mask_iter)
				//   {
				//     mask_states[all_mask[m_i].value] = all_mask[m_i].enabled;
				//   }
				// });

				ArcColorSetMask& set_mask = mask.color_mask.ensure<ArcColorSetMask>();
				if (mask.value >= 0)
				{
					set_mask.colors[mask.value] = set_mask.wildcard ? !mask.enabled : mask.enabled;
				}
				else if (mask.value == -2)
				{
					set_mask.wildcard = mask.enabled;
					for (size_t v = 0; v < 10; v++)
					{
						set_mask.colors[v] = !set_mask.colors[v];
					}
				}

				// First, we need to get all the color masks in this color set, the reason is because if the wildcard is enabled, then
				// all other masks are inverted

				// Realtime preview (disabled)

				// auto q_arc_colors = ecs.query_builder<ArcColorIndex, RenderStatus>()
				//         .build();

				// q_arc_colors.iter([&](flecs::iter& jit, ArcColorIndex* aci, RenderStatus* render) {
				//   for (size_t j : jit)
				//   {
				//     // TODO: Get second term (arc_color_entities) get name, then use arc_element_colors_map to get the color
				//     // NVGcolor color = arc_element_colors[mask[i].value];
				//     // pair is replaced by id in this older version of flecs
				//     // std::cout << std::string(jit.id(1).second().name()) << " selected" << std::endl;
				//     NVGcolor color = arc_element_colors[aci[j].value];
				//     if (mask_states[aci[j].value] && !mask_states[-2] || (mask_states[-2] && !mask_states[aci[j].value]))
				//     {
				//       render[j].color = color;
				//     } else
				//     {
				//       render[j].color =nvgRGBA(color.r*255, color.g*255, color.b*255, 32);
				//     }
				//   }

				// });
			});

		ecs.observer<LineWrapBox>("CreateLineWraps").event(flecs::OnSet)
			.each([ui_element](flecs::entity e, LineWrapBox& lwb) {
				std::cout << "CREATE LINE wraps!" << std::endl;
				int perimeter = (lwb.width + lwb.height) * 2;
				float spacing = (perimeter - (lwb.length * lwb.quantity)) / lwb.quantity;
				std::cout << "Spacing is " << spacing << std::endl;
				for (size_t j = 0; j < lwb.quantity; ++j) {
					float progress = ((int)((j + 0.75) * (lwb.length + spacing))) % perimeter;
					float next_length = 0.0f;
					wrap_line_around_box(j, e, lwb, progress, next_length, false, ui_element);
					wrap_line_around_box(j, e, lwb, ((int)(progress + (lwb.length - next_length))) % perimeter, next_length, true, ui_element);
				}
			});

		ecs.system<LineWrapBox>("RotateLineWrap").write(flecs::Wildcard)
			.each([](flecs::entity e, LineWrapBox& lwb)
			{
				lwb.progress += e.world().delta_time();
				int perimeter = (lwb.width + lwb.height) * 2;
				float spacing = (perimeter - (lwb.length * lwb.quantity)) / lwb.quantity;
				std::vector<flecs::entity> children;
				e.children([&](flecs::entity child)
				{
					children.push_back(child);
				});
				for (size_t c = 0; c < children.size(); c++)
				{
					float progress = (((int)((c + 0.75) * (lwb.length + spacing))) + ((int)((lwb.progress / lwb.time_to_rotate) * perimeter))) % perimeter;
					float next_length = 0.0f;
					update_wrap_lines(&children[c].ensure<Position, Local>(), &children[c].ensure<LineShape>(), lwb, progress, next_length, false);
					c++;
					update_wrap_lines(&children[c].ensure<Position, Local>(), &children[c].ensure<LineShape>(), lwb, ((int)(progress + (lwb.length - next_length))) % perimeter, next_length, true);
				}
			});

    // GraphSail's Software Equipped HUD is analogous to Elden Ring
    // https://eldenring.fandom.com/wiki/HUD

    float bwidth = 80;
    float bheight = 96;
    float bpad = 8;

    flecs::entity elden_ring_hud = ecs.entity()
      .is_a(ui_element)
      .set<Position, Local>({64.0f, height - (bheight*2 + bpad*2 + 48.0f)});

    // Command queue should indicate sequential hierarchical planning objectives
    // and the cognitive architecture's requests for 
    // 1. execution of neural models (typically requests to Conda servers)
    // 2. dispatch of commands to container, such as moving a mouse or typing a key
    flecs::entity command_queue = ecs.entity()
      .is_a(ui_element)
      .add<VerticalLayoutBox>()
      .child_of(elden_ring_hud);

    // for (size_t z = 0; z < 3; ++z)
    // {
    //   flecs::entity cmd = ecs.entity()
    //     .is_a(ui_element)
    //     .set<TextCreator>({"dl_ocr", "ATARISTOCRAT"})
    //     .child_of(command_queue);
    // }

    // Currently Selected Left-Hand ->
    // Continually learning cognitive architecture (ECS Graphstar)
    flecs::entity left_box = ecs.entity()
      .is_a(ui_element)
      .add<LevelHUD>()
      .set<BoxShape>({bwidth, bheight, 1})
      .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 255, 255, 255)})
      .set<Position, Local>({0.0f, bheight/2+bpad/2})
      .child_of(elden_ring_hud);

    // Currently Selected Sorcery ->
    // Neural model cloud compute burst
    flecs::entity top_box = ecs.entity()
      .is_a(ui_element)
      .add<LevelHUD>()
      .set<BoxShape>({bwidth, bheight, 1})
      .set<RenderStatus>({false, RenderMode::STROKE, nvgRGBA(255, 255, 255, 255)})
      .set<Position, Local>({bwidth+bpad, 0})
      .child_of(elden_ring_hud);

    // Currently Selected Item ->
    // Running Conda Servers/Dockers
    flecs::entity bot_box = ecs.entity()
      .is_a(ui_element)
      .add<LevelHUD>()
      .set<BoxShape>({bwidth, bheight, 1})
      .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 255, 255, 255)})
      .set<Position, Local>({bwidth+bpad, bheight+bpad})
      .child_of(elden_ring_hud);

    float bperimeter = (bwidth + bheight)*2;

    // LineWrapBoxes should indicate a temporal processes
    
    // flecs::entity python_running = ecs.entity()
    //   .is_a(ui_element)
    //   .set<LineWrapBox>({bwidth-7, bheight-7, bwidth-7, 4, 4, {nvgRGBA(0, 140, 255, 255), nvgRGBA(255, 214, 63, 255)}})
    //   .set<Position, Local>({4.0f, 4.0f})
    //   .child_of(bot_box);

    // flecs::entity fantasy_pathfinder_running = ecs.entity()
    //   .is_a(ui_element)
    //   .set<LineWrapBox>({bwidth-7, bheight-7, bwidth-7, 4, 4, {nvgRGBA(255, 255, 255, 255), nvgRGBA(238, 76, 44, 255)}})
    //   .set<Position, Local>({4.0f, 4.0f})
    //   .child_of(left_box);

    // Currently Selected Right-Hand Weapon ->
    // Software in operation
    flecs::entity right_box = ecs.entity()
      .is_a(ui_element)
      .add<LevelHUD>()
      .set<BoxShape>({bwidth, bheight, 1})
      // .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 255, 255, 255)})
      .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 255, 255, 255)})
      .set<Position, Local>({bwidth*2 + bpad*2, bheight/2+bpad/2})
      .child_of(elden_ring_hud);

    flecs::entity active_software = ecs.entity()
      .is_a(ui_element)
      .add<LevelHUD>()
      .set<SpriteCreator>({"../assets/jinja.png"})
      .set<Align>({0.5, 0.5, 0.5, 0.5})
      .child_of(right_box);

    // flecs::entity script_ops = ecs.entity()
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/python.png"})
    //   .set<Align>({0.5, 0.5, 0.5, 0.5})
    //   .child_of(bot_box);

    flecs::entity container_ops = ecs.entity()
      .is_a(ui_element)
      .add<LevelHUD>()
      .set<SpriteCreator>({"../assets/pyg.png"})
      .set<Align>({0.5, 0.5, 0.5, 0.5})
      .child_of(bot_box);

    // flecs::entity finite_cyclic_groups = ecs.entity("finite_cyclic_groups")
    //   .is_a(ui_element)
    //   .add<HorizontalLayoutBox>()
    //   .add<LevelHUD>(); 

    // flecs::entity rotate_box = ecs.entity()
    //   .is_a(ui_element)
    //   .add<VerticalLayoutBox>()
    //   .child_of(finite_cyclic_groups);

    // flecs::entity rotate_group = ecs.entity("rotate_group")
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/group_diagram_4.png"})
    //   .add<LevelHUD>()
    //   .child_of(rotate_box);

    // flecs::entity rotate_group_label = ecs.entity()
    //   .is_a(ui_element)
    //   .set<TextCreator>({"ROTATE", "ATARISTOCRAT"})
    //   .add<RenderStatus>()
    //   .add<LevelHUD>()
    //   .child_of(rotate_box);

    // flecs::entity hflip_box = ecs.entity()
    //   .is_a(ui_element)
    //   .add<VerticalLayoutBox>()
    //   .child_of(finite_cyclic_groups);

    // flecs::entity hflip_group = ecs.entity("hflip_group")
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/group_diagram_2.png"})
    //   .add<LevelHUD>()
    //   .child_of(hflip_box);

    // flecs::entity hflip_group_label = ecs.entity()
    //   .is_a(ui_element)
    //   .set<TextCreator>({"HFLIP", "ATARISTOCRAT"})
    //   .add<RenderStatus>()
    //   .add<LevelHUD>()
    //   .set<Position, Relative>({0.0f, 16.0f})
    //   .child_of(hflip_box);

    // flecs::entity vflip_box = ecs.entity()
    //   .is_a(ui_element)
    //   .add<VerticalLayoutBox>()
    //   .child_of(finite_cyclic_groups);    

    // flecs::entity vflip_group = ecs.entity("vflip_group")
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/group_diagram_2.png"})
    //   .add<LevelHUD>()
    //   .child_of(vflip_box);

    // flecs::entity vflip_group_label = ecs.entity()
    //   .is_a(ui_element)
    //   .set<TextCreator>({"VFLIP", "ATARISTOCRAT"})
    //   .add<RenderStatus>()
    //   .add<LevelHUD>()
    //   .set<Position, Relative>({0.0f, 16.0f})
    //   .child_of(vflip_box);

    flecs::entity computer_hardware = ecs.entity()
      .is_a(ui_element)
      .add<LevelHUD>()
      .set<SpriteCreator>({"../assets/sympy.png"})
      .set<Align>({0.5, 0.5, 0.5, 0.5})
      .child_of(top_box);

    flecs::entity pathfinding_system = ecs.entity()
      .is_a(ui_element)
      .add<LevelHUD>()
      .set<SpriteCreator>({"../assets/flecs_star.png"})
      .set<Align>({0.5, 0.5, 0.5, 0.5})
      .child_of(left_box);

    // flecs::entity splash = ecs.entity("splash")
    //   .is_a(ui_element)
    //   .set<BoxShape>({width, height, 0.0f})
    //   .set<RenderStatus>({false});

    // flecs::entity splash_ai = ecs.entity("splash_ai")
    //   .is_a(ui_element)
    //   .set<VerticalLayoutBox>({0.0f, 12.0f})
    //   .set<Align>({0.5, 0.5, 0.5, 0.5})
    //   .child_of(splash);
  
    // flecs::entity ecs_pathfinder = ecs.entity()
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/ecs_graph.png"})
    //   .child_of(splash_ai);

    // flecs::entity ecs_graphstar_version = ecs.entity()
    //   .is_a(ui_element)
    //   .set<TextCreator>({"ECS Graphstar v0.1", "ATARISTOCRAT"})// - LVL 0
    //   .add<LevelHUD>()
    //   .child_of(splash_ai);


    // TODO: Dynamic progress alignment
    flecs::entity pixel_perfect = ecs.entity("pixel_perfect")
      .is_a(ui_element)
      .set<Align>({0.5f, 0.0f, 0.0f, 0.0f})
      .set<HorizontalLayoutBox>({0.0f, 2.0f})
      .add<LevelHUD>()
      .set<BoxShape>({0, 14})
      .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 255, 255, 255)});

    flecs::entity coord_delta = ecs.entity("coord_delta")
      .is_a(ui_element)
      .add<LevelHUD>();

    flecs::entity grid_io_program = ecs.entity("grid_io_program")
      .is_a(ui_element)
      .set<HorizontalLayoutBox>({0.0f, 2.0f})
      .add<LevelHUD>()
      .set<BoxShape>({0, 14})
      .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 255, 255, 255)});

    flecs::entity colors_bar = ecs.entity("colors_bar")
      .is_a(ui_element)
      .set<HorizontalLayoutBox>({0.0f, 2.0f});

    flecs::entity arc_task_debug = ecs.entity("arc_task_debug")
      .is_a(ui_element)
      .set<Position, Local>({width/2-bwidth*2 - bwidth*2 - 16.0f, height - (bpad*2 + 48.0f)})
      .add<LevelHUD>()
      .set<HorizontalLayoutBox>({0.0f, 6.0f});

    flecs::entity arc_task_letters = ecs.entity("arc_task_letters")
      .is_a(ui_element)
      .set<Position, Local>({0, 0}) // Upper Left
      .add<LevelHUD>();
      // .set<VerticalLayoutBox>({0.0f, 2.0f});
      // .set<TableLayoutBox>({0, 0, 2, 2, 5});
      

    flecs::entity arc_dataset_reward = ecs.entity("arc_dataset_reward")
      .is_a(ui_element)
      .set<Position, Local>({width/2-bwidth*2 - bwidth*2 - 16.0f, height - (bpad*2 + 48.0f) - 30.0f})
      .add<LevelHUD>()
      .add<HorizontalLayoutBox>();

    flecs::entity dataset_reward = ecs.entity()
      .is_a(ui_element)
      .set<SpriteCreator>({"../assets/dataset/wooden_sword.png"})
      .add<LevelHUD>() 
      .child_of(arc_dataset_reward);

    flecs::entity dataset_key = ecs.entity()
      .is_a(ui_element)
      .set<SpriteCreator>({"../assets/dataset/wooden_key.png"})
      .add<LevelHUD>() 
      .child_of(arc_dataset_reward);

    flecs::entity spellbar = ecs.entity("spellbar")
      .is_a(ui_element)
      .set<Position, Local>({width/2-bwidth*2 - bwidth*2 - 16.0f, height - (bheight*1 + bpad*2 + 48.0f)})
      .set<BoxShape>({bwidth*4, bheight, 1})
      .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(64, 64, 64, 255)})
      .add<LevelHUD>()
      .add<HorizontalLayoutBox>();

    flecs::entity elementary_color_pairs = ecs.entity("elementary_color_pairs")
      .is_a(ui_element)
      .set<Position, Local>({width/2-bwidth*2 - bwidth*2 - 16.0f, height - (bheight*1 + bpad*2 + 48.0f) - 128.0f})
      .set<BoxShape>({bwidth*4, bheight, 1})
      .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(64, 64, 64, 255)})
      .add<LevelHUD>()
      .set<TableLayoutBox>({0, 0, 2, 2, 5});

    flecs::entity cast_status = ecs.entity("cast_status")
      .is_a(ui_element)
      .set<Position, Local>({width/2-bwidth*2 + bwidth*2 + 16.0f, height - (bheight*1 + bpad*2 + 48.0f)})
      .set<BoxShape>({bwidth*4, bheight, 1})
      .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(64, 64, 64, 255)})
      .add<LevelHUD>()
      .add<HorizontalLayoutBox>();

    // flecs::entity nauty_iso = ecs.entity("nauty_iso")
    //   .is_a(ui_element)
    //   .set<Position, Local>({width/2-bwidth*2 + bwidth*2 + 16.0f, height - (bheight*1 + bpad*2 + 48.0f) - 128.0f})
    //   .set<SpriteCreator>({"../assets/nauty_iso.png"})
    //   .add<LevelHUD>();

    flecs::entity arc_itemset_ai = ecs.entity("arc_itemset_ai")
      .is_a(ui_element)
      .set<Position, Local>({width/2-bwidth*2 - bwidth*2 - 16.0f, height - (bheight*1 + bpad*2 + 48.0f) - 128.0f - 32.0f})
      .add<LevelHUD>()
      .set<HorizontalLayoutBox>({0.0f, 1.0f});

    // flecs::entity arc_toolbar = ecs.entity("arc_advice_taker")
    //   .is_a(ui_element)
    //   .set<Position, Local>({width/2-bwidth*2 - bwidth*2 - 16.0f, height - (bheight*1 + bpad*2 + 48.0f) - 128.0f - 32.0f})
    //   .add<LevelHUD>()
    //   .set<HorizontalLayoutBox>({0.0f, 1.0f});

    flecs::entity faiss_mirror = ecs.entity("faiss_mirror")
      .is_a(ui_element)
      .set<Position, Local>({width/2-bwidth*2 - bwidth*2 - 16.0f - 256.0f, height - (bheight*1 + bpad*2 + 48.0f) - 128.0f - 32.0f})
      .set<SpriteCreator>({"../assets/faiss_mirror_isekai.png"})
      .add<LevelHUD>();

    flecs::entity query_terms_graph = ecs.entity("query_terms_graph")
      .is_a(ui_element)
      .set<Position, Local>({width/2-bwidth*2 - bwidth*2 - 16.0f - 512.0f, height - (bheight*1 + bpad*2 + 48.0f) - 128.0f - 32.0f})
      .set<SpriteCreator>({"../assets/query_terms_graph.png"})
      .add<LevelHUD>();

    for (size_t v = 0; v < 5; v++)
    {
      flecs::entity inventory_layout = ecs.entity(("inventory_layout_" + std::to_string(v)).c_str())
        .is_a(ui_element)
        .set<Position, Local>({width/2-bwidth*2 + bwidth*2 + 16.0f + 184.0f * v, height - (bheight*1 + bpad*2 + 48.0f) - 128.0f})
        .set<VerticalLayoutBox>({0.0f, 4.0f})
        .add<LevelHUD>();
    }

    flecs::entity book = ecs.entity()
        .is_a(ui_element)
        .add<LevelHUD>()
      .set<Position, Local>({0.0f, 512.0f})
        .child_of(library);

    // flecs::entity slhf = ecs.entity("sort_learning_from_human_feedback")
    //   .is_a(ui_element)
    //   // .set<Position, Local>({width - 28.0f*5, 0.0f})
    //   .set<Position, Local>({width/2, height/2})
    //   .add<RectilinearGrid>()
    //   .add<RectilinearGridSelector>()
    //   .add<SortCol>();

    // Removing ArcDataLoader for Desktop ops, ARC related things should be migrated to an external module
    flecs::entity arc = ecs.entity("arc")
      .set<ArcDataLoader>({"../assets/ARC-AGI/data/training"});

    struct HoverTarget
    {
      flecs::entity over;
    };

    struct ARC_Item
    {
      std::string name;
      bool selected;
    };

    struct EventSelectItem
    {
      bool should_select;
      bool toggle_instead;
      bool save_itemset;
    };

    struct EventHoverItem
    {
      bool hovered;
    };

		ecs.system<X11CursorMovement, EventHoverItem, ARC_Item, UIElementBounds, SpriteAlpha, NanoVG>("HoverItemEvent").term_at(0).src(user_input).term_at(5).src(renderer)
    .each([ui_element, height, bheight, bpad](flecs::entity e, X11CursorMovement* cursor, EventHoverItem* hover, ARC_Item* item, UIElementBounds* bounds, SpriteAlpha* s, NanoVG* vg)
    {
      c2v cursor_pos {cursor->x, cursor->y};

      c2AABB box = {{bounds->xmin, bounds->ymin}, {bounds->xmax, bounds->ymax}};
      if (c2AABBtoPoint(box, cursor_pos))
      {
        if (!hover->hovered)
        {
          ecs.lookup((item->name + "_label").c_str()).destruct();
          std::cout << "Hovered over " << item->name << std::endl;
          float item_desc_y_start = height - (bheight*1 + bpad*2 + 48.0f) - 128.0f -128.0f;

          // Compute layout heights assuming vg is available
          c2AABB text_bounds;
          nvgTextBounds(vg->ctx, 0, 0, "A ", NULL, &text_bounds.min.x);
          float line_height = text_bounds.max.y - text_bounds.min.y;

          std::string item_name = item->name;
          ItemDescription& item_desc = item_descriptions[item_name];

          std::string desc_text = item_desc.display_name.empty() ? item_name : item_desc.display_name;

          size_t max_params_count = std::max(item_desc.input_params.size(), item_desc.output_params.size());
          float params_h = static_cast<float>(max_params_count) * line_height;

          // Wrap description_para
          std::vector<std::string> desc_lines;
          std::string para = item_desc.description_para;
          if (!para.empty()) {
            std::vector<std::string> words;
            std::istringstream iss(para);
            std::string word;
            while (iss >> word) {
              words.push_back(word);
            }
            float max_desc_width = 384.0f - 12.0f; // Padding on sides
            std::string current_line;
            float current_width = 0.0f;
            for (const auto& w : words) {
              nvgTextBounds(vg->ctx, 0, 0, (w + " ").c_str(), NULL, &text_bounds.min.x);
              float w_width = text_bounds.max.x - text_bounds.min.x;
              if (current_width + w_width > max_desc_width && !current_line.empty()) {
                desc_lines.push_back(current_line);
                current_line = w;
                current_width = w_width;
              } else {
                if (!current_line.empty()) {
                  current_line += " ";
                }
                current_line += w;
                current_width += w_width;
              }
            }
            if (!current_line.empty()) {
              desc_lines.push_back(current_line);
            }
          }
          float desc_h = static_cast<float>(desc_lines.size()) * line_height;

          // Compute total box height
          float name_y = 16.0f;
          float params_y = 32.0f;
          float desc_y = params_y + params_h + 8.0f; // Spacing after params
          float box_h = desc_y + desc_h + 8.0f; // Bottom padding
          if (desc_lines.empty()) {
            box_h = params_y + params_h + 8.0f;
          }

          flecs::entity item_desc_box = ecs.entity((item->name + "_desc").c_str())
            .is_a(ui_element)
            .set<BoxShape>({384.0f, box_h})
            .add<LevelHUD>()
            .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 255, 255, 255)})
            .set<Position, Local>({bounds->xmin, item_desc_y_start});

          flecs::entity item_name_label = ecs.entity()
            .is_a(ui_element)
            .set<TextCreator>({desc_text, "ATARISTOCRAT"})
            .add<LevelHUD>()
            .add<RenderStatus>()
            .set<HoverTarget>({e})
            .set<Position, Local>({6, name_y})
            .child_of(item_desc_box);

          flecs::entity input_params = ecs.entity()
            .is_a(ui_element)
            .set<Position, Local>({8.0f, params_y})
            .set<VerticalLayoutBox>({0.0f, 0.0f})
            .add<LevelHUD>()
            .child_of(item_desc_box);

          for (const std::string& input_param : item_desc.input_params)
          {
            flecs::entity item_name_label = ecs.entity()
              .is_a(ui_element)
              .set<TextCreator>({input_param, "ATARISTOCRAT"})
              .add<LevelHUD>()
              .add<RenderStatus>()
              .child_of(input_params);
          }

          flecs::entity output_params = ecs.entity()
            .is_a(ui_element)
            .set<Position, Local>({192.0f, params_y})
            .set<VerticalLayoutBox>({0.0f, 0.0f})
            .add<LevelHUD>()
            .child_of(item_desc_box);

          for (const std::string& output_param : item_desc.output_params)
          {
            flecs::entity item_name_label = ecs.entity()
              .is_a(ui_element)
              .set<TextCreator>({output_param, "ATARISTOCRAT"})
              .add<LevelHUD>()
              .add<RenderStatus>()
              .child_of(output_params);
          }

          // Add description if present
          if (!desc_lines.empty()) {
            flecs::entity desc_container = ecs.entity()
              .is_a(ui_element)
              .set<Position, Local>({6.0f, desc_y})
              .set<VerticalLayoutBox>({0.0f, 0.0f})
              .add<LevelHUD>()
              .child_of(item_desc_box);

            for (const auto& line : desc_lines) {
              flecs::entity desc_line = ecs.entity()
                .is_a(ui_element)
                .set<TextCreator>({line, "ATARISTOCRAT"})
                .add<LevelHUD>()
                .add<RenderStatus>()
                .child_of(desc_container);
            }
          }

          // TODO: Render the same item sprite, in a box,
          flecs::entity line_to_item = ecs.entity()
            .is_a(ui_element)
            .set<LineShape>({0, bounds->ymin - item_desc_y_start - box_h, 1})
            .set<Position, Local>({(bounds->xmax - bounds->xmin)/2, box_h})
            .child_of(item_desc_box);
          // with a description, and input/output types
          hover->hovered = true;
        }
      } else
      {
        if (hover->hovered)
        {
          hover->hovered = false;
          flecs::entity item_name_label = ecs.lookup((item->name + "_desc").c_str());
          item_name_label.destruct();
        }
        // s->alpha = 0.2;
      }
    });

    auto q_arc_items = ecs.query_builder<ARC_Item>()
      .build();
		ecs.observer<ARC_Item, SpriteAlpha, EventSelectItem, ArcDataLoader>()
    .event(flecs::OnSet)
    .term_at(3).src(arc)
    .each([&q_arc_items](flecs::entity e, ARC_Item& item, SpriteAlpha& opacity, EventSelectItem& select_item, ArcDataLoader& arc)
    {
      bool select = select_item.should_select;

      if (select_item.toggle_instead)
      {
        select = !item.selected;
      }
      item.selected = select;
      std::cout << "Select item " << e.id() << std::endl;
      if (select)
      {
        e.set<Position, Relative>({0.0f, -6.0f});
      } else
      {
        e.set<Position, Relative>({0.0f, 0.0f});
      }
      opacity.alpha = select ? 1.0 : 0.1;

      if (select_item.save_itemset)
      {
        json saved_item_selection = json::array();
        q_arc_items.each([&](flecs::entity e2, ARC_Item& item_list)
        {
          json j_item;
          j_item["name"] = item_list.name;
          j_item["selected"] = item_list.selected;
          saved_item_selection.push_back(j_item);
        });

        fs::path output_dir = "../save/advice/" + arc.get_active_problem();

        if (!fs::exists(output_dir)) {
          fs::create_directories(output_dir);
        }

        fs::path output_file = output_dir / "itemset.json";
        std::ofstream out_file(output_file);
        if (out_file.is_open()) {
            out_file << saved_item_selection.dump(4);
            out_file.close();
        }
      }
    });

		ecs.observer<ARC_Item, Button, EventSelectBtn*>()
		    .event(flecs::OnSet)
		    .each([](flecs::entity e, ARC_Item& item, Button& btn, EventSelectBtn* select)
		    {
		      std::cout << "Select btn " << e.id() << std::endl;
		      if (select && select->mock == 1)
		      {
		        e.world().lookup("rpg_item_select").ensure<SFX>().sound.play();
		        e.set<EventSelectItem>({true, true, true});
		      }
		    });

    flecs::entity rpg_item = ecs.prefab("rpg_item")
      .is_a(ui_element)
      .add<LevelHUD>()
      .set<SpriteAlpha>({0.1})
      .set<EventSelectBtn>({0})
      .set<EventSelectItem>({false, false, false})
      .set<EventHoverItem>({false});

    flecs::entity rpg_item_select = ecs.entity("rpg_item_select")
      .add<SFX>();
    auto rpg_item_sfx = rpg_item_select.ensure<SFX>();
    if (!rpg_item_sfx.buffer.loadFromFile("../assets/item_select.wav"))
    {
      std::cout << "Failed to load sfx" << std::endl;
    }
    rpg_item_sfx.sound.setBuffer(rpg_item_sfx.buffer);

    // for (std::string& item_name : items)
    // {
    //   int item_index = 0;
    //   flecs::entity item = ecs.entity(item_name.c_str())
    //     .set<SpriteCreator>({"../assets/" + item_name + ".png"})
    //     .is_a(rpg_item)
    //     // .add((RPGItem)item_index)
    //     .set<ARC_Item>({item_name, false})
    //     .add<Button>()
    //     .child_of(arc_toolbar);
    //   item_index++;
    // }

    size_t v_index = 0;
    for (ItemCategory& item_category : item_categories)
    {
      // if (item_category.name == "STATE SPACE SEARCH" || item_category.name == "GENERALIZATION")
      if (item_category.name == "STANDARD OPERATORS" || item_category.name == "SPATIAL GRAPHS" || item_category.name == "DECOMPOSITION" || item_category.name == "CARDINALITY MODEL")
      {
        v_index++;
      }
      flecs::entity layout_entity = ecs.lookup(("inventory_layout_" + std::to_string(v_index)).c_str());
      flecs::entity category_box = ecs.entity()
        .is_a(ui_element)
        .set<BoxShape>({164.0f, 48.0f})
        .add<LevelHUD>()
        .set<VerticalLayoutBox>({0.0f, 4.0f})
        .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(64, 64, 64, 255)})
        .child_of(layout_entity);
      // This is a hack to placate flecs archetype sorting and should be refactored
      flecs::entity inventory_title_layout_row = ecs.entity()
        .is_a(ui_element)
        .set<HorizontalLayoutBox>({0.0f, 2.0f})
        .child_of(category_box);

      flecs::entity item_category_label = ecs.entity()
        .is_a(ui_element)
        .set<TextCreator>({item_category.name, "ATARISTOCRAT"})
        .add<RenderStatus>()
        .add<LevelHUD>()
        .set<Position, Relative>({0.0f, 8.0f})
        .child_of(inventory_title_layout_row);

      flecs::entity inventory_layout_row = ecs.entity()
        .is_a(ui_element)
        .set<HorizontalLayoutBox>({0.0f, 2.0f})
        .child_of(category_box);

      for (std::string& item_name : item_category.items)
      {
        flecs::entity item = ecs.entity(item_name.c_str())
          .set<SpriteCreator>({"../assets/" + item_name + ".png"})
          .is_a(rpg_item)
          .set<ARC_Item>({item_name, false})
          .add<Button>()
          .child_of(inventory_layout_row);
      }
    }

    // TODO: Destroy and recreate in prior probability sorted order when loading new task 
    for (std::string& item_name : items)
    {
      int item_index = 0;
      flecs::entity item = ecs.entity(item_name.c_str())
        .set<SpriteCreator>({"../assets/" + item_name + ".png"})
        .is_a(rpg_item)
        // .set<ARC_Item>({item_name, false})
        .set<SpriteTint>({nvgRGBA(0, 218, 173, 64)})
        .set<SpriteAlpha>({0.3f})
        .child_of(arc_itemset_ai);
      item_index++;
    }

    // flecs::entity symbol_selector = ecs.prefab("")

    // TODO: Equip sengras with a symbol!
    // flecs::entity symbol_sprite = ecs.entity()
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/pixel_symbols.png"})
    //   .set<Align>({0.0, 0.0, 0.0, 0.0});

    // flecs::entity walker_origin = ecs.entity()
    //   .set<Position, Local>({1024, 1024})
    //   .set<BoxShape>({128.0f, 128.0f})
    //   .set<RenderStatus>({true, RenderMode::STROKE})
    //   .is_a(ui_element);


    // // TODO: Bound propagation halter?
    // flecs::entity walk_up = ecs.entity()
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/walk_up.png", 3, 1})
    //   .set<Position, Relative>({0.0f, -16.0f})
    //   // .set<Align>({0.5, 0.5, 0.5, 1.0})
    //   .set<Align>({0.5, 0.5, 0.5, 0.5})
    //   .child_of(walker_origin);

    // flecs::entity walk_up_right = ecs.entity()
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/walk_up_right.png", 3, 1})
    //   .set<Position, Relative>({16.0f, -16.0f})
    //   // .set<Align>({0.5, 0.5, 0.5, 1.0})
    //   .set<Align>({0.5, 0.5, 0.5, 0.5})
    //   .child_of(walker_origin);

    // flecs::entity walk_right = ecs.entity()
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/walk_right.png", 3, 1})\
    //   .set<Position, Relative>({16.0f, 0.0f})
    //   // .set<Align>({0.5, 0.5, -1.0, 0.5})
    //   .set<Align>({0.5, 0.5, 0.5, 0.5})
    //   .child_of(walker_origin);

    // flecs::entity walk_down_right = ecs.entity()
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/walk_down_right.png", 3, 1})
    //   .set<Position, Relative>({16.0f, 16.0f})
    //   // .set<Align>({0.5, 0.5, 0.5, 1.0})
    //   .set<Align>({0.5, 0.5, 0.5, 0.5})
    //   .child_of(walker_origin);

    // flecs::entity walk_down = ecs.entity()
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/walk_down.png", 3, 1})
    //   .set<Position, Relative>({0.0f, 16.0f})
    //   // .set<Align>({0.5, 0.5, 0.5, -0.5})
    //   .set<Align>({0.5, 0.5, 0.5, 0.5})
    //   .child_of(walker_origin);

    // flecs::entity walk_down_left = ecs.entity()
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/walk_down_left.png", 3, 1})
    //   .set<Position, Relative>({-16.0f, 16.0f})
    //   // .set<Align>({0.5, 0.5, 0.5, 1.0})
    //   .set<Align>({0.5, 0.5, 0.5, 0.5})
    //   .child_of(walker_origin);

    // flecs::entity walk_left = ecs.entity()
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/walk_left.png", 3, 1})
    //   .set<Position, Relative>({-16.0f, 0.0f})
    //   // .set<Align>({0.5, 0.5, 1.0, 0.5})
    //   .set<Align>({0.5, 0.5, 0.5, 0.5})
    //   .child_of(walker_origin);

    // flecs::entity walk_up_left = ecs.entity()
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/walk_up_left.png", 3, 1})
    //   .set<Position, Relative>({-16.0f, -16.0f})
    //   // .set<Align>({0.5, 0.5, 0.5, 1.0})
    //   .set<Align>({0.5, 0.5, 0.5, 0.5})
    //   .child_of(walker_origin);


		ecs.observer<X11KeyPress, ArcDataLoader>()
		    .write(flecs::Wildcard).term_at(1).src(arc).event(flecs::OnSet)
		    .each([display, &q_arc_items](X11KeyPress& key, ArcDataLoader& loader)
		{
		  KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
		  if (symbol == XK_Right)
		  {
		    loader.active_index = (loader.active_index + 1) % loader.data.size();
		  } else if (symbol == XK_Left)
		  {
		    loader.active_index = loader.active_index - 1;
		    if (loader.active_index < 0)
		    {
		      loader.active_index = loader.data.size()-1;
		    }
		  }
		  if (symbol == XK_Right || symbol == XK_Left)
		  {
		    fs::path advice_dir = "../save/advice/" + loader.get_active_problem();
		    fs::path itemset_path = advice_dir / "itemset.json";

		    if (fs::exists(itemset_path))
		    {
		      std::ifstream items_advice_file(itemset_path);
		      json items_advice_data = json::parse(items_advice_file);
		      std::unordered_map<std::string, bool> itemset_selection;
		      for (auto& item : items_advice_data)
		      {
		        itemset_selection[item["name"]] = item["selected"];
		      }
		      q_arc_items.each([&itemset_selection](flecs::entity e, ARC_Item& item)
		      {
		        e.set<EventSelectItem>({itemset_selection[item.name], false, false});
		      });
		    }
		    else
		    {
		      q_arc_items.each([&](flecs::entity e, ARC_Item& item)
		      {
		        e.set<EventSelectItem>({false, false, false});
		      });
		    }

		    auto q_g = ecs.query_builder<SaveGraph>()
		      .build();
		    q_g.each([&](flecs::entity e, SaveGraph& save)
		    {
		      if (save.cartridge == 0)
		      {
		        e.destruct();
		      }
		    });
		    auto q = ecs.query_builder<ArcData>()
		                .build();
		    q.each([&](flecs::entity e, ArcData& arc_data)
		    {
		      e.destruct();
		    });
		    loader.viz.destruct();
		    // It seems like this is never actually... called?
		    std::cout << "Calling load arc task" << std::endl;
		    loader.viz = load_arc_task(&loader, loader.data[loader.active_index]);
		    std::string task_id = loader.get_active_problem();
		    std::string priors_path = "../save/predicted_priors/" + task_id + "_priors.txt";

		    struct ItemPrior
		    {
		      std::string item_name;
		      float probability;
		    };

		    std::vector<ItemPrior> priors;

		    flecs::entity arc_itemset_ai = ecs.lookup("arc_itemset_ai");
		    std::ifstream priors_file(priors_path);
		    if (priors_file.is_open()) {
		        std::string probability;
		        size_t item_index = 0;
		        while (std::getline(priors_file, probability))
		        {
		          float prob = stof(probability);
		          priors.push_back({items[item_index], prob});
		          item_index++;
		        }
		        arc_itemset_ai.children([&](flecs::entity child)
		        {
		          child.destruct();
		        });
		    }
		    std::sort(priors.begin(), priors.end(), [](const ItemPrior& a, const ItemPrior& b) {
		                return a.probability > b.probability;
		            });
		    flecs::entity rpg_item = ecs.lookup("rpg_item");
		    for (ItemPrior& prior : priors)
		    {
		      int item_index = 0;
		      flecs::entity item = ecs.entity(prior.item_name.c_str())
		        .set<SpriteCreator>({"../assets/" + prior.item_name + ".png"})
		        .is_a(rpg_item)
		        // .set<ARC_Item>({item_name, false})
		        .set<SpriteTint>({nvgRGBA(0, 218, 173, std::min(255, 64 + (int)(prior.probability*255.0f)))})
		        .set<SpriteAlpha>({prior.probability})
		        .child_of(arc_itemset_ai);
		      item_index++;
		    }

		    std::string annotation_dir = "../save/arc_annotation/" + task_id;
		    std::filesystem::path path(annotation_dir);

		    if (!std::filesystem::exists(path)) {
		        std::filesystem::create_directory(path);
		    }
		    std::string save_path = annotation_dir + "/save_0.txt";
		    graphs_to_load = process_scroll(save_path);
		  }
		  if (symbol == XK_Down)
		  {
		    // TODO: Store tasks vector in loader?
		    loader.active_task++;
		    loader.active_task %= loader.task_data.size()/2;
		    populate_spellcast_bar(&loader);
		    // TODO: Show something related to letter existence in task
		  }
		  else if (symbol == XK_Up)
		  {
		    loader.active_task--;
		    if (loader.active_task < 0)
		    {
		       loader.active_task = (loader.task_data.size()/2) - 1;
		    }
		    populate_spellcast_bar(&loader);
		  }
		});

    // TODO: Trigger this observer with sengra data - how?
    // Idea 1. Drag and drop edge connection to flecs module ui_element
    // ecs.observer<ArcDataLoader>("SearchForArcTask").write(flecs::Wildcard).event(flecs::OnSet).iter([](flecs::iter& it, ArcDataLoader* loader)
    // {
    //     loader->active_index = loader->get_index("673ef223");
    //     loader->viz = load_arc_task(loader->data[loader->active_index]);
    //     std::string dir = "../save/arc_annotation/" + loader->get_active_problem();
    //     std::filesystem::path path(dir);
        
    //     if (!std::filesystem::exists(path)) {
    //         std::filesystem::create_directory(path);
    //     }
    //     std::string save_path = dir + "/save_0.txt";
    //     graphs_to_load = process_scroll(save_path);
    // });

    auto q_arc_grids = ecs.query_builder<Position, ArcViz>().term_at(0).second<Local>().build();

		ecs.observer<MouseMotionEvent, CursorState>().event(flecs::OnSet).term_at(1).src(user_input)
			.each([q_arc_grids](MouseMotionEvent& motion_event, CursorState& cursor_state) {
				if (cursor_state.middle) {
					q_arc_grids
						.each([&](Position& pos, ArcViz& av) {
							pos.x += motion_event.x_diff;
							pos.y += motion_event.y_diff;
						});
				}
			});

		ecs.system<NanoVG, BoxShape, Position, SceneGraphSelection>("SetSelectorStats").term_at(0).src(renderer).term_at(2).second<Local>().kind(flecs::OnUpdate)
		.each([](NanoVG& vg, BoxShape& box, Position& pos, SceneGraphSelection& sgs)
		{
			flecs::entity selected_sg_node = sgs.selected_node;
			flecs::entity selected_sg_text = selected_sg_node.lookup("sg_viz::sg_text");
			const UIElementBounds* sg_bounds = selected_sg_text.try_get<UIElementBounds>();
			const Position* sg_text_pos = selected_sg_text.try_get<Position, World>();
			float padding = 4.0f;
			box.width = padding + (sg_bounds->xmax - sg_bounds->xmin);
			pos.x = sg_text_pos->x - padding/2.0f;
			pos.y = sg_text_pos->y - 10;
		});

		ecs.observer<X11FlecsScrollEvent, Position>("ScrollLibrary").term_at(1).second<Local>().src(library).event(flecs::OnSet)
		.each([](X11FlecsScrollEvent* scroll, Position* pos)
		{
		  pos->y += scroll->dir * 8.0f;
		});

		ecs.observer<X11HorizontalScrollEvent, Position>("ScrollBook").term_at(1).second<Local>().src(library).event(flecs::OnSet)
		.each([](X11HorizontalScrollEvent& scroll, Position& pos) {
		  pos.x += scroll.dir * 32.0f;
		});
      
		ecs.system<LoadSceneGraph, Position>("OpenECS_Menu").write(flecs::Wildcard).term_at(1).second<Local>().each([hud](flecs::entity interaction_node, LoadSceneGraph& load, Position& interaction_pos)
		{
		  flecs::entity selected_scene_graph = load.graph;
		  auto selector = selected_scene_graph.ensure<SceneGraphRoot>().selector;
		  SceneGraphSelection& sgs = selector.ensure<SceneGraphSelection>();
		  sgs.active = true;
		  selector.set<SceneGraphSelectionChanged>({0});
		  sgs.bode_telos_source = load.telos_source;
		  sgs.interaction_graph_selector = load.telos_target;
		  selected_scene_graph.set<Position, Local>({interaction_pos.x, interaction_pos.y + 32.0f});
		  selected_scene_graph.set<SceneGraphInteractionNode>({interaction_node});
		  interaction_node.remove<LoadSceneGraph>();
		});

		ecs.observer<X11KeyPress>("HideSplashscreen").write(flecs::Wildcard).event(flecs::OnSet)
		.each([display](X11KeyPress& key)
		{
		  KeySym symbol = XKeycodeToKeysym(display, key.event.keycode, 0);
		  if (symbol == XK_space)
		  {
		    ecs.lookup("splash").destruct();
		  }
		});

    // flecs::entity scope_arrow = ecs.entity("scope_arrow")
    //   .is_a(ui_element)
    //   .set<SpriteCreator>({"../assets/arrow_down.png"})
    //   .set<Position, Local>({512.0f, 512.0f})
    //   .add<LevelHUD>();
      // .child_of(scene_graph_node);

    //auto img = nvgCreateImage(vg, "../os/data/result_semi.png", 0); //NVG_IMAGE_PREMULTIPLIED
    //auto img = nvgCreateImage(vg, "../assets/arrow.png", 0); //NVG_IMAGE_PREMULTIPLIED
    // auto img = nvgCreateImage(vg, "../os/data/test.png", 0); //NVG_IMAGE_PREMULTIPLIED
    XEvent xevent;
    glViewport(0, 0, width, height);

    auto q_load_page = ecs.query_builder<GraphReader, UpdateGraphReaderPageEvent>()
        .write(flecs::Wildcard)
        .build();

    auto q_load_speech = ecs.query_builder<LoadSpeechGraphEvent, NanoVG>()
        .write(flecs::Wildcard)
        .term_at(1).src(renderer)
        .build();

    auto q_sfx = ecs.query_builder<CreateSFX>()
      .write(flecs::Wildcard)
      .build();

    auto q_prefab_sg = ecs.query_builder<SetupNodeMenuSceneGraph>()
      .write(flecs::Wildcard)
      .build();

    auto q_load_graph = ecs.query_builder<LoadGraphData, NanoVG>()
        .term_at(1).src(renderer)
        .build();

    auto sound_effect = ecs.entity()
      .add<SFX>();
    auto sfx = sound_effect.ensure<SFX>();
    if (!sfx.buffer.loadFromFile("../assets/welcome_back.wav"))
    {
      std::cout << "Failed to load sfx" << std::endl;
    }
    sfx.sound.setBuffer(sfx.buffer);
    sfx.sound.play();

    sf::SoundBuffer buffer;
    sf::Sound sound;

    // auto anchor_emoji = ecs.entity()
    //   .is_a(ui_element)
    //   .set<RoundTriShape>({10.0f, 40.0f})
    //   .set<RenderStatus>({true, RenderMode::STROKE, nvgRGBA(255, 0, 0, 255)})
    //   .set<Position, Local>({128, 128});

    // LoadGraphData suggest{ECS_NodeType::STANDARD, 1024, 1024, "Pathfinding dreams"};
    // graphs_to_load.push_back(suggest);

    while (!ecs.should_quit()) {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        nvgBeginFrame(vg, width, height, 1.0f);
        ecs.progress();
        nvgEndFrame(vg);

        // if (show_overlay)
        // {
        //   nvgBeginPath(vg);
        // //   // nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
        // nvgRect(vg, 0, 0, width, height);
        // int imgw;
        // int imgh;
        // nvgImageSize(vg, img, &imgw, &imgh);
        // NVGpaint paint = nvgImagePattern(vg, 0.0f, 0.0f, imgw, imgh, 0.0f, img, 1.0f);
        // nvgFillPaint(vg, paint);
        // nvgFill(vg);
        // }
        // Load graphs after ECS frame...
        q_load_speech.each([&](flecs::entity e, LoadSpeechGraphEvent& load, NanoVG& vg)
        {
          if (load.filepath.size() > 0)
          {
            std::cout << "Filepath is " << load.filepath << std::endl;
            stream_speech_graph(300.0f, 300.0f, parse_speech_json(load.filepath), ui_element, &vg);
            e.destruct();
          }
        });

        q_load_page.each([&](flecs::entity e, GraphReader& reader, UpdateGraphReaderPageEvent& update)
        {
          // TODO: Delete existing page sentence graph
          auto parent = reader.page_graph.parent();
          reader.page_graph.destruct();
          reader.active_page = update.page;
          flecs::entity page_graph = load_page(reader.book_data, reader.active_page, ui_element, &renderer.ensure<NanoVG>());
          reader.page_graph = page_graph;
          page_graph.child_of(parent);
          page_graph.set<Position, Local>({64.0f, 64.0f});
          e.remove<UpdateGraphReaderPageEvent>();
        });

        q_sfx.iter([&](flecs::iter& it, CreateSFX* create)
        {
          if (buffer.loadFromFile(create->filepath))
          {
            std::cout << "Failed to load sfx " << create->filepath << std::endl;
          }
          sound.setBuffer(buffer);
          sound.play();
          it.entity(0).remove<CreateSFX>();
        });

        q_prefab_sg.each([&](flecs::entity e, SetupNodeMenuSceneGraph& setup_prefab)
        {
          for (size_t i : it)
          {
            setup_prefab.telos.add<EventOnConnect, OpenECS_SceneGraph>();
            if (setup_prefab.node_type == ECS_NodeType::PREFAB)
            {
              setup_prefab.telos.set<SceneGraphAmbLoader>(prefab_output_loader);
            } 
            else if (setup_prefab.node_type == ECS_NodeType::COMPONENT)
            {
              std::cout << "SETUP COMPONENT NODE MENU" << std::endl;
              setup_prefab.telos.set<SceneGraphAmbLoader>(component_output_loader);
            }
            std::cout << "Telos updated?" << setup_prefab.telos.has<EventOnConnect>(flecs::Wildcard) << std::endl;
            setup_prefab.telos.get_mut<RenderStatus>()->color = nvgRGBA(0, 0, 255, 255);
            e.destruct();
          }
        });

        // q_load_graph.iter([&](flecs::iter& it, LoadCartridgeData* cartridge, NanoVG* vg) 
        // {
        //   for(size_t i : it)
        //   {
        //     for (LoadGraphData& load : cartridge[i].graphs)
        //     {
        //       
        //     }
        //     // flecs::entity output[6];
        //     // ;
        //     // it.entity(i).destruct();
        //   }
        // });
      
        for (LoadGraphData& load : graphs_to_load)
        {
          std::cout << "Query load graph string " << load.graph_str << std::endl;
          flecs::entity graph_root = load_graph(load.x, load.y, load.graph_str, ui_element, renderer.get_mut<NanoVG>());
          RenderMode emoji_mode = RenderMode::FILL;
          if (load.node_type == ECS_NodeType::PREFAB || load.node_type == ECS_NodeType::COMPONENT)
          {
            emoji_mode = RenderMode::STROKE;
            SetupNodeMenuSceneGraph menu;
            menu.telos = get_telos(graph_root);
            menu.node_type = (ECS_NodeType)load.node_type;
            auto setup_prefab = ecs.entity()
              .set<SetupNodeMenuSceneGraph>(menu);
          }
          set_node_type(graph_root, node_type_labels[(ECS_NodeType)load.node_type], (ECS_NodeType)load.node_type, syntax_theme[(ECS_NodeType)load.node_type], emoji_mode);
          // graph_root
        }
        graphs_to_load.clear();

        glfwSwapBuffers(window);
        // TODO: See if GLFW captures events while X11 Cursor/Keyboard are grabbed, otherwise use Linux input drivers (evdev)...
        // glfwPollEvents();
    }
    XCloseDisplay(display);
    glfwTerminate();

    // Exit program
    exit( EXIT_SUCCESS );
}
