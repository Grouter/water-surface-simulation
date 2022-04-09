#define DEVELOPER

#if defined(DEVELOPER)

#else
    #define NDEBUG
#endif

// Libraries
#define STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM

#include <windows.h>
#include <Windowsx.h>
#include <glew.h>
#include <wglew.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <cmath>
#include <assert.h>
#include <algorithm>
#include <time.h>
#include <random>

#include <fftw/fftw3.h>

#include <stb_rect_pack.h>
#include <stb_truetype.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui.cpp>
#include <imgui/imgui_impl_win32.cpp>
#include <imgui/imgui_impl_opengl3.cpp>
#include <imgui/imgui_tables.cpp>
#include <imgui/imgui_widgets.cpp>
#include <imgui/imgui_draw.cpp>

#include "types.h"

const u32 INITIAL_WINDOW_W = 1920 / 2;
const u32 INITIAL_WINDOW_H = 1080 / 2;

const u32 VIRTUAL_WINDOW_W = 32;
const u32 VIRTUAL_WINDOW_H = 18;

const u32 TARGET_ASPECT_W = 16;
const u32 TARGET_ASPECT_H = 9;

#include "utils.h"
#include "memory.h"
#include "array.h"
#include "math/math.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "math/shapes.h"
#include "color.h"

#include "parse_utils.h"
#include "camera.h"
#include "graphics.h"
#include "resources.h"
#include "render.h"
#include "input.h"
#include "game.h"

#include "shared.h"

#include "camera.cpp"
#include "graphics.cpp"
#include "resources.cpp"
#include "input.cpp"
#include "render.cpp"
#include "water.cpp"
#include "game.cpp"

#include "win32.cpp"
