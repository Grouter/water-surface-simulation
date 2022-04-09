#if !defined(INPUT_H)
#define INPUT_H

enum MouseButton : u8 {
    MouseButton_L = 0,
    MouseButton_M = 1,
    MouseButton_R = 2,
};

struct KeyInput {
    u8       scan_code;
    u16      virtual_code;
    wchar_t  character;
    bool32   alt_down;  // @Todo: bitfield with other modifiers
    bool32   pressed;
};

struct InputState {
    bool16 mouse_input_blocked;
    bool16 keyboard_input_blocked;

    bool32 mouse_locked;

    Vector2i mouse;
    Vector2i mouse_delta;
    Vector2i mouse_old;
    Vector2i mouse_viewport;
    Vector2  mouse_unit;
    Vector2i mouse_grid;

    bool wasd_pressed[4];
    bool32 shift_pressed;
};

internal void handle_mouse_left_click();
internal void handle_mouse_right_click();

#endif
