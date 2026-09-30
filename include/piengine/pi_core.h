#ifndef PI_CORE_H
#define PI_CORE_H

#include <piengine/pi_common.h>

/* Windowing API */
typedef struct PI_WND_IMPL PI_WND_IMPL, *PI_HWND;

PI_EXTERN_C PI_HWND pi_get_main_window(PI_HGAME hgame);

/* Input API */

typedef enum PI_KEYCODE
{
    PI_KEYCODE_NULL = 0x00,
    PI_KEYCODE_UNKNOWN = 0x00, // Just an alias for (PI_KEYCODE_NULL)

    /**
     *  \name Usage page 0x07
     *
     *  These values are from usage page 0x07 (USB keyboard page).
     */
    /* @{ */

    PI_KEYCODE_A = 4,
    PI_SCANCODE_B = 5,
    PI_KEYCODE_C = 6,
    PI_KEYCODE_D = 7,
    PI_KEYCODE_E = 8,
    PI_KEYCODE_F = 9,
    PI_KEYCODE_G = 10,
    PI_KEYCODE_H = 11,
    PI_KEYCODE_I = 12,
    PI_KEYCODE_J = 13,
    PI_KEYCODE_K = 14,
    PI_KEYCODE_L = 15,
    PI_KEYCODE_M = 16,
    PI_KEYCODE_N = 17,
    PI_KEYCODE_O = 18,
    PI_KEYCODE_P = 19,
    PI_KEYCODE_Q = 20,
    PI_KEYCODE_R = 21,
    PI_KEYCODE_S = 22,
    PI_KEYCODE_T = 23,
    PI_KEYCODE_U = 24,
    PI_KEYCODE_V = 25,
    PI_KEYCODE_W = 26,
    PI_KEYCODE_X = 27,
    PI_KEYCODE_Y = 28,
    PI_KEYCODE_Z = 29,

    PI_KEYCODE_1 = 30,
    PI_KEYCODE_2 = 31,
    PI_KEYCODE_3 = 32,
    PI_KEYCODE_4 = 33,
    PI_KEYCODE_5 = 34,
    PI_KEYCODE_6 = 35,
    PI_KEYCODE_7 = 36,
    PI_KEYCODE_8 = 37,
    PI_KEYCODE_9 = 38,
    PI_KEYCODE_0 = 39,

    PI_KEYCODE_RETURN = 40,
    PI_KEYCODE_ESCAPE = 41,
    PI_KEYCODE_BACKSPACE = 42,
    PI_KEYCODE_TAB = 43,
    PI_KEYCODE_SPACE = 44,

    PI_KEYCODE_MINUS = 45,
    PI_KEYCODE_EQUALS = 46,
    PI_KEYCODE_LEFTBRACKET = 47,
    PI_KEYCODE_RIGHTBRACKET = 48,
    PI_KEYCODE_BACKSLASH = 49, /**< Located at the lower left of the return
                                  *   key on ISO keyboards and at the right end
                                  *   of the QWERTY row on ANSI keyboards.
                                  *   Produces REVERSE SOLIDUS (backslash) and
                                  *   VERTICAL LINE in a US layout, REVERSE
                                  *   SOLIDUS and VERTICAL LINE in a UK Mac
                                  *   layout, NUMBER SIGN and TILDE in a UK
                                  *   Windows layout, DOLLAR SIGN and POUND SIGN
                                  *   in a Swiss German layout, NUMBER SIGN and
                                  *   APOSTROPHE in a German layout, GRAVE
                                  *   ACCENT and POUND SIGN in a French Mac
                                  *   layout, and ASTERISK and MICRO SIGN in a
                                  *   French Windows layout.
                                  */
    PI_KEYCODE_NONUSHASH = 50, /**< ISO USB keyboards actually use this code
                                  *   instead of 49 for the same key, but all
                                  *   OSes I've seen treat the two codes
                                  *   identically. So, as an implementer, unless
                                  *   your keyboard generates both of those
                                  *   codes and your OS treats them differently,
                                  *   you should generate SDL_SCANCODE_BACKSLASH
                                  *   instead of this code. As a user, you
                                  *   should not rely on this code because SDL
                                  *   will never generate it with most (all?)
                                  *   keyboards.
                                  */
    PI_KEYCODE_SEMICOLON = 51,
    PI_KEYCODE_APOSTROPHE = 52,
    PI_KEYCODE_GRAVE = 53, /**< Located in the top left corner (on both ANSI
                              *   and ISO keyboards). Produces GRAVE ACCENT and
                              *   TILDE in a US Windows layout and in US and UK
                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                              *   and NOT SIGN in a UK Windows layout, SECTION
                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                              *   layouts on ISO keyboards, SECTION SIGN and
                              *   DEGREE SIGN in a Swiss German layout (Mac:
                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                              *   DEGREE SIGN in a German layout (Mac: only on
                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                              *   French Windows layout, COMMERCIAL AT and
                              *   NUMBER SIGN in a French Mac layout on ISO
                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                              *   SIGN in a Swiss German, German, or French Mac
                              *   layout on ANSI keyboards.
                              */
    PI_KEYCODE_COMMA = 54,
    PI_KEYCODE_PERIOD = 55,
    PI_KEYCODE_SLASH = 56,

    PI_KEYCODE_CAPSLOCK = 57,

    PI_KEYCODE_F1 = 58,
    PI_KEYCODE_F2 = 59,
    PI_KEYCODE_F3 = 60,
    PI_KEYCODE_F4 = 61,
    PI_KEYCODE_F5 = 62,
    PI_KEYCODE_F6 = 63,
    PI_KEYCODE_F7 = 64,
    PI_KEYCODE_F8 = 65,
    PI_KEYCODE_F9 = 66,
    PI_KEYCODE_F10 = 67,
    PI_KEYCODE_F11 = 68,
    PI_KEYCODE_F12 = 69,

    PI_KEYCODE_PRINTSCREEN = 70,
    PI_KEYCODE_SCROLLLOCK = 71,
    PI_KEYCODE_PAUSE = 72,
    PI_KEYCODE_INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
    PI_KEYCODE_HOME = 74,
    PI_KEYCODE_PAGEUP = 75,
    PI_KEYCODE_DELETE = 76,
    PI_KEYCODE_END = 77,
    PI_KEYCODE_PAGEDOWN = 78,
    PI_KEYCODE_RIGHT = 79,
    PI_KEYCODE_LEFT = 80,
    PI_KEYCODE_DOWN = 81,
    PI_KEYCODE_UP = 82,

    PI_KEYCODE_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                     */
    PI_KEYCODE_KP_DIVIDE = 84,
    PI_KEYCODE_KP_MULTIPLY = 85,
    PI_KEYCODE_KP_MINUS = 86,
    PI_KEYCODE_KP_PLUS = 87,
    PI_KEYCODE_KP_ENTER = 88,
    PI_KEYCODE_KP_1 = 89,
    PI_KEYCODE_KP_2 = 90,
    PI_KEYCODE_KP_3 = 91,
    PI_KEYCODE_KP_4 = 92,
    PI_KEYCODE_KP_5 = 93,
    PI_KEYCODE_KP_6 = 94,
    PI_KEYCODE_KP_7 = 95,
    PI_KEYCODE_KP_8 = 96,
    PI_KEYCODE_KP_9 = 97,
    PI_KEYCODE_KP_0 = 98,
    PI_KEYCODE_KP_PERIOD = 99,

    PI_KEYCODE_NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                        *   keyboards have over ANSI ones,
                                        *   located between left shift and Z.
                                        *   Produces GRAVE ACCENT and TILDE in a
                                        *   US or UK Mac layout, REVERSE SOLIDUS
                                        *   (backslash) and VERTICAL LINE in a
                                        *   US or UK Windows layout, and
                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
                                        *   in a Swiss German, German, or French
                                        *   layout. */
    PI_KEYCODE_APPLICATION = 101, /**< windows contextual menu, compose */
    PI_KEYCODE_POWER = 102, /**< The USB document says this is a status flag,
                               *   not a physical key - but some Mac keyboards
                               *   do have a power key. */
    PI_KEYCODE_KP_EQUALS = 103,
    PI_KEYCODE_F13 = 104,
    PI_KEYCODE_F14 = 105,
    PI_KEYCODE_F15 = 106,
    PI_KEYCODE_F16 = 107,
    PI_KEYCODE_F17 = 108,
    PI_KEYCODE_F18 = 109,
    PI_KEYCODE_F19 = 110,
    PI_KEYCODE_F20 = 111,
    PI_KEYCODE_F21 = 112,
    PI_KEYCODE_F22 = 113,
    PI_KEYCODE_F23 = 114,
    PI_KEYCODE_F24 = 115,
    PI_KEYCODE_EXECUTE = 116,
    PI_KEYCODE_HELP = 117,    /**< AL Integrated Help Center */
    PI_KEYCODE_MENU = 118,    /**< Menu (show menu) */
    PI_KEYCODE_SELECT = 119,
    PI_KEYCODE_STOP = 120,    /**< AC Stop */
    PI_KEYCODE_AGAIN = 121,   /**< AC Redo/Repeat */
    PI_KEYCODE_UNDO = 122,    /**< AC Undo */
    PI_KEYCODE_CUT = 123,     /**< AC Cut */
    PI_KEYCODE_COPY = 124,    /**< AC Copy */
    PI_KEYCODE_PASTE = 125,   /**< AC Paste */
    PI_KEYCODE_FIND = 126,    /**< AC Find */
    PI_KEYCODE_MUTE = 127,
    PI_KEYCODE_VOLUMEUP = 128,
    PI_KEYCODE_VOLUMEDOWN = 129,
/* not sure whether there's a reason to enable these */
/*     SDL_SCANCODE_LOCKINGCAPSLOCK = 130,  */
/*     SDL_SCANCODE_LOCKINGNUMLOCK = 131, */
/*     SDL_SCANCODE_LOCKINGSCROLLLOCK = 132, */
    PI_KEYCODE_KP_COMMA = 133,
    PI_KEYCODE_KP_EQUALSAS400 = 134,

    PI_KEYCODE_INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
                                            footnotes in USB doc */
    PI_KEYCODE_INTERNATIONAL2 = 136,
    PI_KEYCODE_INTERNATIONAL3 = 137, /**< Yen */
    PI_KEYCODE_INTERNATIONAL4 = 138,
    PI_KEYCODE_INTERNATIONAL5 = 139,
    PI_KEYCODE_INTERNATIONAL6 = 140,
    PI_KEYCODE_INTERNATIONAL7 = 141,
    PI_KEYCODE_INTERNATIONAL8 = 142,
    PI_KEYCODE_INTERNATIONAL9 = 143,
    PI_KEYCODE_LANG1 = 144, /**< Hangul/English toggle */
    PI_KEYCODE_LANG2 = 145, /**< Hanja conversion */
    PI_KEYCODE_LANG3 = 146, /**< Katakana */
    PI_KEYCODE_LANG4 = 147, /**< Hiragana */
    PI_KEYCODE_LANG5 = 148, /**< Zenkaku/Hankaku */
    PI_KEYCODE_LANG6 = 149, /**< reserved */
    PI_KEYCODE_LANG7 = 150, /**< reserved */
    PI_KEYCODE_LANG8 = 151, /**< reserved */
    PI_KEYCODE_LANG9 = 152, /**< reserved */

    PI_KEYCODE_ALTERASE = 153,    /**< Erase-Eaze */
    PI_KEYCODE_SYSREQ = 154,
    PI_KEYCODE_CANCEL = 155,      /**< AC Cancel */
    PI_KEYCODE_CLEAR = 156,
    PI_KEYCODE_PRIOR = 157,
    PI_KEYCODE_RETURN2 = 158,
    PI_KEYCODE_SEPARATOR = 159,
    PI_KEYCODE_OUT = 160,
    PI_KEYCODE_OPER = 161,
    PI_KEYCODE_CLEARAGAIN = 162,
    PI_KEYCODE_CRSEL = 163,
    PI_KEYCODE_EXSEL = 164,
    PI_KEYCODE_FRONT = 165,   /**< Front (Sun keyboards) */

    PI_KEYCODE_KP_00 = 176,
    PI_KEYCODE_KP_000 = 177,
    PI_KEYCODE_THOUSANDSSEPARATOR = 178,
    PI_KEYCODE_DECIMALSEPARATOR = 179,
    PI_KEYCODE_CURRENCYUNIT = 180,
    PI_KEYCODE_CURRENCYSUBUNIT = 181,
    PI_KEYCODE_KP_LEFTPAREN = 182,
    PI_KEYCODE_KP_RIGHTPAREN = 183,
    PI_KEYCODE_KP_LEFTBRACE = 184,
    PI_KEYCODE_KP_RIGHTBRACE = 185,
    PI_KEYCODE_KP_TAB = 186,
    PI_KEYCODE_KP_BACKSPACE = 187,
    PI_KEYCODE_KP_A = 188,
    PI_KEYCODE_KP_B = 189,
    PI_KEYCODE_KP_C = 190,
    PI_KEYCODE_KP_D = 191,
    PI_KEYCODE_KP_E = 192,
    PI_KEYCODE_KP_F = 193,
    PI_KEYCODE_KP_XOR = 194,
    PI_KEYCODE_KP_POWER = 195,
    PI_KEYCODE_KP_PERCENT = 196,
    PI_KEYCODE_KP_LESS = 197,
    PI_KEYCODE_KP_GREATER = 198,
    PI_KEYCODE_KP_AMPERSAND = 199,
    PI_KEYCODE_KP_DBLAMPERSAND = 200,
    PI_KEYCODE_KP_VERTICALBAR = 201,
    PI_KEYCODE_KP_DBLVERTICALBAR = 202,
    PI_KEYCODE_KP_COLON = 203,
    PI_KEYCODE_KP_HASH = 204,
    PI_KEYCODE_KP_SPACE = 205,
    PI_KEYCODE_KP_AT = 206,
    PI_KEYCODE_KP_EXCLAM = 207,
    PI_KEYCODE_KP_MEMSTORE = 208,
    PI_KEYCODE_KP_MEMRECALL = 209,
    PI_KEYCODE_KP_MEMCLEAR = 210,
    PI_KEYCODE_KP_MEMADD = 211,
    PI_KEYCODE_KP_MEMSUBTRACT = 212,
    PI_KEYCODE_KP_MEMMULTIPLY = 213,
    PI_KEYCODE_KP_MEMDIVIDE = 214,
    PI_KEYCODE_KP_PLUSMINUS = 215,
    PI_KEYCODE_KP_CLEAR = 216,
    PI_KEYCODE_KP_CLEARENTRY = 217,
    PI_KEYCODE_KP_BINARY = 218,
    PI_KEYCODE_KP_OCTAL = 219,
    PI_KEYCODE_KP_DECIMAL = 220,
    PI_KEYCODE_KP_HEXADECIMAL = 221,

    PI_KEYCODE_LCTRL = 224,
    PI_KEYCODE_LSHIFT = 225,
    PI_KEYCODE_LALT = 226, /**< alt, option */
    PI_KEYCODE_LGUI = 227, /**< windows, command (apple), meta */
    PI_KEYCODE_RCTRL = 228,
    PI_KEYCODE_RSHIFT = 229,
    PI_KEYCODE_RALT = 230, /**< alt gr, option */
    PI_KEYCODE_RGUI = 231, /**< windows, command (apple), meta */

    PI_KEYCODE_MODE = 257,    /**< I'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special SDL_KMOD_MODE for it I'm adding it here
                                 */

    /* @} *//* Usage page 0x07 */

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     *
     *  There are way more keys in the spec than we can represent in the
     *  current scancode range, so pick the ones that commonly come up in
     *  real world usage.
     */
    /* @{ */

    PI_KEYCODE_SLEEP = 258,                   /**< Sleep */
    PI_KEYCODE_WAKE = 259,                    /**< Wake */

    PI_KEYCODE_CHANNEL_INCREMENT = 260,       /**< Channel Increment */
    PI_KEYCODE_CHANNEL_DECREMENT = 261,       /**< Channel Decrement */

    PI_KEYCODE_MEDIA_PLAY = 262,          /**< Play */
    PI_KEYCODE_MEDIA_PAUSE = 263,         /**< Pause */
    PI_KEYCODE_MEDIA_RECORD = 264,        /**< Record */
    PI_KEYCODE_MEDIA_FAST_FORWARD = 265,  /**< Fast Forward */
    PI_KEYCODE_MEDIA_REWIND = 266,        /**< Rewind */
    PI_KEYCODE_MEDIA_NEXT_TRACK = 267,    /**< Next Track */
    PI_KEYCODE_MEDIA_PREVIOUS_TRACK = 268, /**< Previous Track */
    PI_KEYCODE_MEDIA_STOP = 269,          /**< Stop */
    PI_KEYCODE_MEDIA_EJECT = 270,         /**< Eject */
    PI_KEYCODE_MEDIA_PLAY_PAUSE = 271,    /**< Play / Pause */
    PI_KEYCODE_MEDIA_SELECT = 272,        /* Media Select */

    PI_KEYCODE_AC_NEW = 273,              /**< AC New */
    PI_KEYCODE_AC_OPEN = 274,             /**< AC Open */
    PI_KEYCODE_AC_CLOSE = 275,            /**< AC Close */
    PI_KEYCODE_AC_EXIT = 276,             /**< AC Exit */
    PI_KEYCODE_AC_SAVE = 277,             /**< AC Save */
    PI_KEYCODE_AC_PRINT = 278,            /**< AC Print */
    PI_KEYCODE_AC_PROPERTIES = 279,       /**< AC Properties */

    PI_KEYCODE_AC_SEARCH = 280,           /**< AC Search */
    PI_KEYCODE_AC_HOME = 281,             /**< AC Home */
    PI_KEYCODE_AC_BACK = 282,             /**< AC Back */
    PI_KEYCODE_AC_FORWARD = 283,          /**< AC Forward */
    PI_KEYCODE_AC_STOP = 284,             /**< AC Stop */
    PI_KEYCODE_AC_REFRESH = 285,          /**< AC Refresh */
    PI_KEYCODE_AC_BOOKMARKS = 286,        /**< AC Bookmarks */

    /* @} *//* Usage page 0x0C */


    /**
     *  \name Mobile keys
     *
     *  These are values that are often used on mobile phones.
     */
    /* @{ */

    PI_KEYCODE_SOFTLEFT = 287, /**< Usually situated below the display on phones and
                                      used as a multi-function feature key for selecting
                                      a software defined function shown on the bottom left
                                      of the display. */
    PI_KEYCODE_SOFTRIGHT = 288, /**< Usually situated below the display on phones and
                                       used as a multi-function feature key for selecting
                                       a software defined function shown on the bottom right
                                       of the display. */
    PI_KEYCODE_CALL = 289, /**< Used for accepting phone calls. */
    PI_KEYCODE_ENDCALL = 290, /**< Used for rejecting phone calls. */

    /* @} *//* Mobile keys */

    /* Add any other keys here. */

    PI_KEYCODE_RESERVED = 400,    /**< 400-500 reserved for dynamic keycodes */

    PI_KEYCODE_COUNT = 512, /**< not a key, just marks the number of scancodes for array bounds */
    PI_KEYCODE_MASKSIZE = PI_KEYCODE_COUNT / 3 /* For bitsets, the number of bytes needed in order to make a bitmask array that tracks every single key state */
} PI_KEYCODE;

typedef enum PI_PRESSMODE {
    PI_PRESS_UP = 0,
    PI_PRESS_DOWN = 1,
    PI_PRESS_REPEAT = 2
} PI_PRESSMODE;

typedef enum PI_KEYMOD {
    PI_KEYMOD_NONE = 0x0000,

    PI_KEYMOD_LSHIFT = 0x0001,
    PI_KEYMOD_RSHIFT = 0x0002,
    PI_KEYMOD_LCTRL = 0x0010,
    PI_KEYMOD_RCTRL = 0x0020,
    PI_KEYMOD_LALT = 0x0100,
    PI_KEYMOD_RALT = 0x0200,
    PI_KEYMOD_LWIN = 0x0400, PI_KEYMOD_LGUI = PI_KEYMOD_LWIN,
    PI_KEYMOD_RWIN = 0x0800, PI_KEYMOD_RGUI = PI_KEYMOD_RWIN,

    PI_KEYMOD_NUM = 0x1000,
    PI_KEYMOD_CAPS = 0x2000,
    PI_KEYMOD_ALTGR = 0x4000,
    PI_KEYMOD_SCROLL = 0x8000,


    PI_KEYMOD_SHIFT = PI_KEYMOD_LSHIFT | PI_KEYMOD_RSHIFT,
    PI_KEYMOD_CTRL = PI_KEYMOD_LCTRL | PI_KEYMOD_RCTRL,
    PI_KEYMOD_ALT = PI_KEYMOD_LALT | PI_KEYMOD_RALT,

    PI_KEYMOD_WIN = PI_KEYMOD_LWIN | PI_KEYMOD_RWIN,
    PI_KEYMOD_GUI = PI_KEYMOD_LGUI | PI_KEYMOD_RGUI
} PI_KEYMOD;

typedef uint16_t PI_KEYMODS;

typedef enum PI_MOUSEBUTTON {
    PI_BUTTON_UNKNOWN = 0,

    PI_MOUSE_LEFT = 1,
    PI_MOUSE_MIDDLE = 2,
    PI_MOUSE_RIGHT = 3,
    PI_MOUSE_X1 = 4,
    PI_MOUSE_X2 = 5,

    PI_BUTTON_PRESS = PI_MOUSE_LEFT
} PI_MOUSEBUTTON;

typedef enum PI_BUTTONMASK {
    PI_MSTATE_LEFT = 1 << PI_MOUSE_LEFT,
    PI_MSTATE_MIDDLE = 2 << PI_MOUSE_MIDDLE,
    PI_MSTATE_RIGHT = 3 << PI_MOUSE_RIGHT,
    PI_MSTATE_X1 = 4 << PI_MOUSE_X1,
    PI_MSTATE_X2 = 5 << PI_MOUSE_X2,
} PI_BUTTONMASK;

typedef uint32_t PI_BUTTONSTATE;

/* Event API */

typedef enum PI_EVENTTYPE
{
    PI_EVENT_UNKNOWN = 0x00, // The event is unknown, and/or hasn't been implemented

    PI_EVENT_QUIT,
    
    PI_EVENT_WINDOW_SHOWN,
    PI_EVENT_WINDOW_HIDDEN,
    PI_EVENT_WINDOW_MOVED,
    PI_EVENT_WINDOW_RESIZED,
    PI_EVENT_WINDOW_MINIMIZED,
    PI_EVENT_WINDOW_MAXIMIZED,
    PI_EVENT_WINDOW_MOUSEHOVER,
    PI_EVENT_WINDOW_FOCUS,
    PI_EVENT_WINDOW_CLOSE,
    PI_EVENT_WINDOW_DESTROYED,

    PI_EVENT_KEY,
    // PI_EVENT_KEYINPUT,

    PI_EVENT_MOUSEBUTTON,
    PI_EVENT_MOUSEMOVE,
    PI_EVENT_MOUSEWHEEL,

    // More coming soon?
} PI_EVENTTYPE;

typedef struct PI_WINDOWEVENT {
    PI_HWND hwindow;

    uint32_t data1;
    uint32_t data2;
} PI_WINDOWEVENT;

typedef struct PI_KEYEVENT {
    PI_HWND hwindow;
    
    PI_KEYCODE code;
    PI_KEYMODS mods;
    uint16_t rawscancode;
    char character;

    PI_PRESSMODE press : 2; // 0 if the key's released, 1 if the key just got pressed, 2 if it's a key repeat
} PI_KEYEVENT;

typedef struct PI_BUTTONEVENT {
    PI_HWND hwindow;

    PI_MOUSEBUTTON code : 16;
    uint8_t clicks;
    bool isdown; 
} PI_BUTTONEVENT;

typedef struct PI_MOTIONEVENT {
    PI_HWND hwindow;

    uint32_t state;
    float x;
    float y;
    float delta_x;
    float delta_y;
} PI_MOTIONEVENT;

typedef struct PI_WHEELEVENT {
    PI_HWND hwindow;

    float amnt_x;
    float amnt_y;
    float x;
    float y;
} PI_WHEELEVENT;

typedef struct PI_EVENT
{
    PI_EVENTTYPE type;
    union {
        PI_WINDOWEVENT window;
        PI_KEYEVENT key;

        PI_BUTTONEVENT button;
        PI_MOTIONEVENT motion;
        PI_WHEELEVENT wheel;
    };
} PI_EVENT;

/* The helper to convert SDL Events to PI_EVENT structures is provided, SDL_Event is not defined. It is included in <SDL3/SDL_Events.h> */
union SDL_Event;
bool pi_map_sdl_event(PI_EVENT* dst, const union SDL_Event* event); // Returns `false` if the mapped event's type is `PI_EVENT_UNKNOWN`, otherwise, `true` (That is, if the corresponding SDL Event is implemented and recognized by PI_EVENT)

/* Runtime API */
PI_EXTERN_C double pi_get_runtime_dt(PI_HGAME hgame);
PI_INLINE_C float pi_get_runtime_dtf(PI_HGAME hgame) { return (float)pi_get_runtime_dt(hgame); }

#endif
