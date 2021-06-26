//
// Created by Mike Smith on 2021/6/27.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace luisa::gui {

enum Key : uint32_t {
    KEY_SPACE = 32u,
    KEY_APOSTROPHE = 39u, /* ' */
    KEY_COMMA = 44u,      /* , */
    KEY_MINUS = 45u,      /* - */
    KEY_PERIOD = 46u,     /* . */
    KEY_SLASH = 47u,      /* / */
    KEY_0 = 48u,
    KEY_1 = 49u,
    KEY_2 = 50u,
    KEY_3 = 51u,
    KEY_4 = 52u,
    KEY_5 = 53u,
    KEY_6 = 54u,
    KEY_7 = 55u,
    KEY_8 = 56u,
    KEY_9 = 57u,
    KEY_SEMICOLON = 59u, /* ; */
    KEY_EQUAL = 61u,     /* = */
    KEY_A = 65u,
    KEY_B = 66u,
    KEY_C = 67u,
    KEY_D = 68u,
    KEY_E = 69u,
    KEY_F = 70u,
    KEY_G = 71u,
    KEY_H = 72u,
    KEY_I = 73u,
    KEY_J = 74u,
    KEY_K = 75u,
    KEY_L = 76u,
    KEY_M = 77u,
    KEY_N = 78u,
    KEY_O = 79u,
    KEY_P = 80u,
    KEY_Q = 81u,
    KEY_R = 82u,
    KEY_S = 83u,
    KEY_T = 84u,
    KEY_U = 85u,
    KEY_V = 86u,
    KEY_W = 87u,
    KEY_X = 88u,
    KEY_Y = 89u,
    KEY_Z = 90u,
    KEY_LEFT_BRACKET = 91u,  /* [ */
    KEY_BACKSLASH = 92u,     /* \ */
    KEY_RIGHT_BRACKET = 93u, /* ] */
    KEY_GRAVE_ACCENT = 96u,  /* ` */
    KEY_WORLD_1 = 161u,      /* non-US #1 */
    KEY_WORLD_2 = 162u,      /* non-US #2 */
    KEY_ESCAPE = 256u,
    KEY_ENTER = 257u,
    KEY_TAB = 258u,
    KEY_BACKSPACE = 259u,
    KEY_INSERT = 260u,
    KEY_DELETE = 261u,
    KEY_RIGHT = 262u,
    KEY_LEFT = 263u,
    KEY_DOWN = 264u,
    KEY_UP = 265u,
    KEY_PAGE_UP = 266u,
    KEY_PAGE_DOWN = 267u,
    KEY_HOME = 268u,
    KEY_END = 269u,
    KEY_CAPS_LOCK = 280u,
    KEY_SCROLL_LOCK = 281u,
    KEY_NUM_LOCK = 282u,
    KEY_PRINT_SCREEN = 283u,
    KEY_PAUSE = 284u,
    KEY_F1 = 290u,
    KEY_F2 = 291u,
    KEY_F3 = 292u,
    KEY_F4 = 293u,
    KEY_F5 = 294u,
    KEY_F6 = 295u,
    KEY_F7 = 296u,
    KEY_F8 = 297u,
    KEY_F9 = 298u,
    KEY_F10 = 299u,
    KEY_F11 = 300u,
    KEY_F12 = 301u,
    KEY_F13 = 302u,
    KEY_F14 = 303u,
    KEY_F15 = 304u,
    KEY_F16 = 305u,
    KEY_F17 = 306u,
    KEY_F18 = 307u,
    KEY_F19 = 308u,
    KEY_F20 = 309u,
    KEY_F21 = 310u,
    KEY_F22 = 311u,
    KEY_F23 = 312u,
    KEY_F24 = 313u,
    KEY_F25 = 314u,
    KEY_KP_0 = 320u,
    KEY_KP_1 = 321u,
    KEY_KP_2 = 322u,
    KEY_KP_3 = 323u,
    KEY_KP_4 = 324u,
    KEY_KP_5 = 325u,
    KEY_KP_6 = 326u,
    KEY_KP_7 = 327u,
    KEY_KP_8 = 328u,
    KEY_KP_9 = 329u,
    KEY_KP_DECIMAL = 330u,
    KEY_KP_DIVIDE = 331u,
    KEY_KP_MULTIPLY = 332u,
    KEY_KP_SUBTRACT = 333u,
    KEY_KP_ADD = 334u,
    KEY_KP_ENTER = 335u,
    KEY_KP_EQUAL = 336u,
    KEY_LEFT_SHIFT = 340u,
    KEY_LEFT_CONTROL = 341u,
    KEY_LEFT_ALT = 342u,
    KEY_LEFT_SUPER = 343u,
    KEY_RIGHT_SHIFT = 344u,
    KEY_RIGHT_CONTROL = 345u,
    KEY_RIGHT_ALT = 346u,
    KEY_RIGHT_SUPER = 347u,
    KEY_MENU = 348u
};

enum Mouse : uint32_t {
    MOUSE_LEFT = 0u,
    MOUSE_RIGHT = 1u,
    MOUSE_MIDDLE = 2u
};

}// namespace luisa::gui
