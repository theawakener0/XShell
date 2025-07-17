#include "themes.h"

/* Neovim-inspired themes */
theme_t themes[] = {
    {
        "xcodex_dark",
        {
            37,  /* HL_NORMAL - light grey */
            90,  /* HL_NONPRINT - dark grey */
            244, /* HL_COMMENT - grey */
            244, /* HL_MLCOMMENT - grey */
            207, /* HL_KEYWORD1 - magenta */
            108, /* HL_KEYWORD2 - green */
            81,  /* HL_KEYWORD3 - cyan */
            114, /* HL_STRING - light green */
            209, /* HL_NUMBER - orange */
            220, /* HL_MATCH - yellow */
            81,  /* HL_FUNCTION - cyan */
            204, /* HL_OPERATOR - red */
            175, /* HL_PREPROCESSOR - purple */
            248, /* HL_BRACKET - white */
            209, /* HL_CONSTANT - orange */
            146, /* HL_VARIABLE - light purple */
            196  /* HL_ERROR - bright red */
        },
        0,   /* bg_color */
        15,  /* cursor_color */
        236, /* status_bg */
        248, /* status_fg */
        240, /* line_number_color - dark grey */
        236  /* cursor_line_color - dark background for current line */
    },
    {
        "xcodex_light",
        {
            16,  /* HL_NORMAL - black */
            240, /* HL_NONPRINT - dark grey */
            102, /* HL_COMMENT - blue grey */
            102, /* HL_MLCOMMENT - blue grey */
            127, /* HL_KEYWORD1 - purple */
            28,  /* HL_KEYWORD2 - dark green */
            30,  /* HL_KEYWORD3 - dark cyan */
            22,  /* HL_STRING - dark green */
            166, /* HL_NUMBER - orange */
            220, /* HL_MATCH - yellow */
            30,  /* HL_FUNCTION - dark cyan */
            124, /* HL_OPERATOR - dark red */
            90,  /* HL_PREPROCESSOR - purple */
            16,  /* HL_BRACKET - black */
            166, /* HL_CONSTANT - orange */
            55,  /* HL_VARIABLE - purple */
            196  /* HL_ERROR - red */
        },
        15,  /* bg_color */
        0,   /* cursor_color */
        252, /* status_bg */
        16,  /* status_fg */
        102, /* line_number_color - blue grey */
        254  /* cursor_line_color - light grey for current line */
    },
    {
        "gruvbox_dark",
        {
            223, /* HL_NORMAL - light cream */
            245, /* HL_NONPRINT - grey */
            245, /* HL_COMMENT - grey */
            245, /* HL_MLCOMMENT - grey */
            167, /* HL_KEYWORD1 - red */
            142, /* HL_KEYWORD2 - green */
            108, /* HL_KEYWORD3 - blue */
            214, /* HL_STRING - yellow */
            208, /* HL_NUMBER - orange */
            172, /* HL_MATCH - orange */
            108, /* HL_FUNCTION - blue */
            208, /* HL_OPERATOR - orange */
            175, /* HL_PREPROCESSOR - purple */
            223, /* HL_BRACKET - cream */
            208, /* HL_CONSTANT - orange */
            142, /* HL_VARIABLE - green */
            167  /* HL_ERROR - red */
        },
        235, /* bg_color */
        223, /* cursor_color */
        237, /* status_bg */
        223, /* status_fg */
        245, /* line_number_color - grey */
        237  /* cursor_line_color - slightly lighter dark */
    },
    {
        "tokyo_night_dark",
        {
            169, /* HL_NORMAL - light purple/blue */
            241, /* HL_NONPRINT - dark grey */
            102, /* HL_COMMENT - blue grey */
            102, /* HL_MLCOMMENT - blue grey */
            175, /* HL_KEYWORD1 - light purple */
            150, /* HL_KEYWORD2 - light green */
            110, /* HL_KEYWORD3 - blue */
            158, /* HL_STRING - light green */
            215, /* HL_NUMBER - orange */
            220, /* HL_MATCH - yellow */
            110, /* HL_FUNCTION - blue */
            204, /* HL_OPERATOR - red */
            176, /* HL_PREPROCESSOR - purple */
            248, /* HL_BRACKET - white */
            215, /* HL_CONSTANT - orange */
            176, /* HL_VARIABLE - purple */
            203  /* HL_ERROR - red */
        },
        234, /* bg_color - dark blue */
        169, /* cursor_color */
        235, /* status_bg */
        169, /* status_fg */
        102, /* line_number_color - blue grey */
        235  /* cursor_line_color - slightly lighter blue */
    },
    {
        "tokyo_night_light",
        {
            52,  /* HL_NORMAL - dark purple */
            241, /* HL_NONPRINT - grey */
            102, /* HL_COMMENT - blue grey */
            102, /* HL_MLCOMMENT - blue grey */
            90,  /* HL_KEYWORD1 - purple */
            29,  /* HL_KEYWORD2 - dark green */
            24,  /* HL_KEYWORD3 - dark blue */
            28,  /* HL_STRING - dark green */
            166, /* HL_NUMBER - orange */
            220, /* HL_MATCH - yellow */
            24,  /* HL_FUNCTION - dark blue */
            124, /* HL_OPERATOR - dark red */
            90,  /* HL_PREPROCESSOR - purple */
            52,  /* HL_BRACKET - dark purple */
            166, /* HL_CONSTANT - orange */
            90,  /* HL_VARIABLE - purple */
            196  /* HL_ERROR - red */
        },
        255, /* bg_color - white */
        52,  /* cursor_color */
        252, /* status_bg */
        52,  /* status_fg */
        102, /* line_number_color - blue grey */
        253  /* cursor_line_color - very light grey */
    },
    {
        "tokyo_night_storm",
        {
            188, /* HL_NORMAL - light grey */
            241, /* HL_NONPRINT - dark grey */
            102, /* HL_COMMENT - blue grey */
            102, /* HL_MLCOMMENT - blue grey */
            141, /* HL_KEYWORD1 - light purple */
            114, /* HL_KEYWORD2 - green */
            74,  /* HL_KEYWORD3 - blue */
            143, /* HL_STRING - light green */
            215, /* HL_NUMBER - orange */
            220, /* HL_MATCH - yellow */
            74,  /* HL_FUNCTION - blue */
            203, /* HL_OPERATOR - red */
            141, /* HL_PREPROCESSOR - purple */
            188, /* HL_BRACKET - light grey */
            215, /* HL_CONSTANT - orange */
            141, /* HL_VARIABLE - purple */
            203  /* HL_ERROR - red */
        },
        236, /* bg_color - storm dark */
        188, /* cursor_color */
        237, /* status_bg */
        188, /* status_fg */
        102, /* line_number_color - blue grey */
        235  /* cursor_line_color - dark grey */
    }
};

const int NUM_THEMES = sizeof(themes) / sizeof(themes[0]);
