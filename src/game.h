#if !defined(GAME_H)
#define GAME_H

#define GRAVITY 9.81f
#define NORM 10000.0f

struct GameState {
    //
    // Window and rendering
    //
    u32 window_width_pixels;
    u32 window_height_pixels;

    Viewport viewport;  // pixels

    f32 unit_to_pixels = 1.0f;
    f32 pixels_to_units = 1.0f;

    //
    // Memory
    //
    MemoryArena permanent_memory;   // For runtime initialized permanent arrays and structs
    MemoryArena transient_memory;   // For runtime initialized arrays and structs that do not persist throught the game

    //
    // Game
    //
    f32 time;

    Camera camera;

    std::default_random_engine random_engine;
    std::normal_distribution<f32> normal_dist;

    Mesh water_plane;

    Texture water_displacement;
    StaticArray<Vector3> water_displacement_data;

    Texture water_normals;
    StaticArray<Vector3> water_normals_data;

    u32 resolution = 256;

    f32 amplitude = 8.0f;
    f32 wind_speed = 20.0f;
    f32 length = 300.0f;
    f32 lambda = -0.5f;
    f32 smoothing = 1.0f;
    Vector2 wind_direction = make_vector2(1.0f, 0.0f);

    StaticArray<Vector2> h0tk = {};
    StaticArray<Vector2> h0tmk = {};

    StaticArray<Vector2> h_tilde = {};
    StaticArray<Vector2> h_tilde_i = {};

    StaticArray<Vector2> dx = {};
    StaticArray<Vector2> dx_i = {};

    StaticArray<Vector2> dz = {};
    StaticArray<Vector2> dz_i = {};

    fftwf_plan p_h_tilde;
    fftwf_plan p_dx;
    fftwf_plan p_dz;
};

#endif // GAME_H
