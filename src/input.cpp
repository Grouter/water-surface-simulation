internal void handle_key_down(u8 scan_code, u16 virtual_code, bool alt_down) {
    if (input_state->keyboard_input_blocked) return;

    KeyInput key = {};
    key.scan_code    = scan_code;
    key.virtual_code = virtual_code;
    key.alt_down     = alt_down;
    key.pressed      = true;

    if (virtual_code == VK_ESCAPE) {
        platform_quit_program();
        return;
    }

    switch (virtual_code) {
        case 'W' : {
            input_state->wasd_pressed[0] = true;
        } break;
        case 'A' : {
            input_state->wasd_pressed[1] = true;
        } break;
        case 'S' : {
            input_state->wasd_pressed[2] = true;
        } break;
        case 'D' : {
            input_state->wasd_pressed[3] = true;
        } break;
        case VK_SHIFT : {
            input_state->shift_pressed = true;
        } break;
    }

    game_handle_key_down(key);
}

internal void handle_key_up(u8 scan_code, u16 virtual_code, bool alt_down) {
    if (input_state->keyboard_input_blocked) return;

    KeyInput key = {};
    key.scan_code    = scan_code;
    key.virtual_code = virtual_code;
    key.alt_down     = alt_down;
    key.pressed      = false;

    switch (virtual_code) {
        case 'W' : {
            input_state->wasd_pressed[0] = false;
        } break;
        case 'A' : {
            input_state->wasd_pressed[1] = false;
        } break;
        case 'S' : {
            input_state->wasd_pressed[2] = false;
        } break;
        case 'D' : {
            input_state->wasd_pressed[3] = false;
        } break;
        case VK_SHIFT : {
            input_state->shift_pressed = false;
        } break;
    }
}

internal void handle_char(wchar_t c) {
    if (input_state->keyboard_input_blocked) return;

    KeyInput key = {};
    key.character = c;
    key.pressed   = true;
}

internal void handle_mouse_input() {
    input_state->mouse_delta.x = input_state->mouse.x - input_state->mouse_old.x;
    input_state->mouse_delta.y = input_state->mouse.y - input_state->mouse_old.y;

    input_state->mouse_old.x = input_state->mouse.x;
    input_state->mouse_old.y = input_state->mouse.y;

    input_state->mouse_viewport.x = max(min(input_state->mouse.x, game_state->viewport.width ) - game_state->viewport.left, 0);
    input_state->mouse_viewport.y = max(min(input_state->mouse.y, game_state->viewport.height) - game_state->viewport.bottom, 0);

    // Mouse unit position
    {
        input_state->mouse_unit.x = (f32)input_state->mouse_viewport.x;
        input_state->mouse_unit.y = (f32)game_state->viewport.height - (f32)input_state->mouse_viewport.y;

        input_state->mouse_unit *= game_state->pixels_to_units;
    }
}

internal void handle_mouse_left_click() {
    if (input_state->mouse_input_blocked) return;
}

internal void handle_mouse_right_click() {
    if (input_state->mouse_input_blocked) return;
}
