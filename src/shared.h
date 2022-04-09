#if !defined(SHARED_H)
#define SHARED_H

//
// Globals
//
global bool running = true;

global MemoryArena game_memory;

global Resources *resources;
global InputState *input_state;
global GameState *game_state;
global RenderState *render_state;

//
// Procs
//

internal void game_handle_key_down(KeyInput key);
internal void update_and_render_game(f32 delta_time);

internal void platform_quit_program();
internal void platform_show_cursor(bool show);

#endif
