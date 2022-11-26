#include "keyboard.h"

bool is_pressed(VirtualKeyEvent e) {
    return (e.status & 0x01) == 0x01;
}

bool is_shift_modifier(VirtualKeyEvent e) {
    return (e.status & 0b00000010);
}
bool is_ctrl_modifier(VirtualKeyEvent e) {
    return (e.status & 0b00000100);
}

bool keyevent_to_char(VirtualKeyEvent event, char* result) {
    if (!is_pressed(event)) {
        return false;
    }

    if (event.vkey == ENTER) {
        *result = '\n';
    } else if (event.vkey >= A && event.vkey <= Z) {
        if (event.status & 0b00000010) {
            char key = event.vkey - A;
            *result = 'A' + key;
        } else {
            char key = event.vkey - A;
            *result = 'a' + key;
        }
    } else if (event.vkey == TAB) {
        *result = '\t;';
    } else if (event.vkey == OPEN_SQUARE) {
        *result = event.status & 0b00000010 ? '{' : '[';
    } else if (event.vkey == CLOSE_SQUARE) {
        *result = event.status & 0b00000010 ? '}' : ']';
    } else if (event.vkey == TILDE) {
        *result = event.status & 0b00000010 ? '~' : '`';
    } else if (event.vkey == COLON) {
        *result = event.status & 0b00000010 ? ':' : ';';
    } else if (event.vkey == QUOTE) {
        *result = event.status & 0b00000010 ? '"' : '\'';
    } else if (event.vkey == COMMA) {
        *result = event.status & 0b00000010 ? '<' : ',';
    } else if (event.vkey == DOT) {
        *result = event.status & 0b00000010 ? '>' : '.';
    } else if (event.vkey == BACKSLASH) {
        *result = event.status & 0b00000010 ? '|' : '\\';
    } else if (event.vkey == TILDE) {
        *result = event.status & 0b00000010 ? '~' : '`';
    } else if (event.vkey >= T_0 && event.vkey <= T_11) {
        if (event.status & 0b00000010) {
            const char* keys = ")!@#$%^&*(_+";
            *result = keys[(event.vkey - T_0)];
        } else {
            const char* keys = "0123456789-=";
            *result = keys[(event.vkey - T_0)];
        }
    }else if (event.vkey == SPACE) {
        *result = ' ';
    } else if (event.vkey == N_SLASH || event.vkey == SLASH) {
        *result = '/';
    } else if (event.vkey == N_MINUS) {
        *result = '-';
    } else if (event.vkey == N_DOT) {
        *result = '.';
    } else {
		return false;
	}

    return true;
}