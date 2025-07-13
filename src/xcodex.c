// This file is part of the XShell project, a simple terminal editor based on kilo

#define XCODEX_VERSION "0.1"

/* POSIX-only feature */
#if !defined(_WIN32) && !defined(_WIN64)
#define XCODEX_ENABLED 1

#ifdef __linux__
#define _POSIX_C_SOURCE 200809L
#endif

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>

/* Syntax highlight types */
#define HL_NORMAL 0
#define HL_NONPRINT 1
#define HL_COMMENT 2   /* Single line comment. */
#define HL_MLCOMMENT 3 /* Multi-line comment. */
#define HL_KEYWORD1 4
#define HL_KEYWORD2 5
#define HL_KEYWORD3 6
#define HL_STRING 7
#define HL_NUMBER 8
#define HL_MATCH 9     /* Search match. */
#define HL_FUNCTION 10
#define HL_OPERATOR 11
#define HL_PREPROCESSOR 12
#define HL_BRACKET 13
#define HL_CONSTANT 14
#define HL_VARIABLE 15
#define HL_ERROR 16 /* Error in syntax highlight. */

#define HL_HIGHLIGHT_STRINGS (1<<0)
#define HL_HIGHLIGHT_NUMBERS (1<<1)

struct editorSyntax {
    char **filematch;
    char **keywords;
    char singleline_comment_start[3];
    char multiline_comment_start[8];
    char multiline_comment_end[8];
    int flags;
};

/* Theme definitions */
typedef struct {
    char *name;
    int colors[17];  /* Colors for each highlight type */
    int bg_color;
    int cursor_color;
    int status_bg;
    int status_fg;
    int line_number_color; /* Color for line numbers */
} theme_t;

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
        240  /* line_number_color - dark grey */
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
        102  /* line_number_color - blue grey */
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
        245  /* line_number_color - grey */
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
        102  /* line_number_color - blue grey */
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
        102  /* line_number_color - blue grey */
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
        102  /* line_number_color - blue grey */
    }
};

static int current_theme = 0;
#define NUM_THEMES (sizeof(themes) / sizeof(themes[0]))

/* Forward declarations */
void editorSetStatusMessage(const char *fmt, ...);

/* This structure represents a single line of the file we are editing. */
typedef struct erow {
    int idx;            /* Row index in the file, zero-based. */
    int size;           /* Size of the row, excluding the null term. */
    int rsize;          /* Size of the rendered row. */
    char *chars;        /* Row content. */
    char *render;       /* Row content "rendered" for screen (for TABs). */
    unsigned char *hl;  /* Syntax highlight type for each character in render.*/
    int hl_oc;          /* Row had open comment at end in last syntax highlight
                           check. */
} erow;

typedef struct hlcolor {
    int r,g,b;
} hlcolor;

struct editorConfig {
    int cx,cy;  /* Cursor x and y position in characters */
    int rowoff;     /* Offset of row displayed. */
    int coloff;     /* Offset of column displayed. */
    int screenrows; /* Number of rows that we can show */
    int screencols; /* Number of cols that we can show */
    int numrows;    /* Number of rows */
    int rawmode;    /* Is terminal raw mode enabled? */
    erow *row;      /* Rows */
    int dirty;      /* File modified but not saved. */
    char *filename; /* Currently open filename */
    char statusmsg[80];
    time_t statusmsg_time;
    struct editorSyntax *syntax;    /* Current syntax highlight, or NULL. */
    int show_line_numbers; /* Show line numbers */
    int line_numbers_width; /* Width of line numbers */
};

static struct editorConfig E;

/*Maps syntax highlight token types to themed colors*/
int editorSyntaxToColor(int hl) {
    if (hl >= 0 && hl < 17) {
        return themes[current_theme].colors[hl];
    }
    return themes[current_theme].colors[HL_NORMAL];
}

/*Theme switching function*/
void editorSwitchTheme(int theme_index) {
    if (theme_index < 0 || theme_index >= NUM_THEMES) {
        editorSetStatusMessage("Invalid theme index: %d (Available: 0-%d)", 
                            theme_index, NUM_THEMES - 1);
        return;
    }
    current_theme = theme_index;
    
    /* Apply background color */
    editorSetBackgroundColor(themes[current_theme].bg_color);
    
    /* Update the status message to reflect the new theme */
    editorSetStatusMessage("Theme: %s (%d/%d) - Use Ctrl+T to cycle", 
                        themes[current_theme].name, 
                        current_theme + 1, 
                        NUM_THEMES);
}

/* Enhanced theme cycling with better feedback */
void editorCycleTheme(void) {
    current_theme = (current_theme + 1) % NUM_THEMES;
    
    /* Apply background color */
    editorSetBackgroundColor(themes[current_theme].bg_color);
    
    editorSetStatusMessage("Theme: %s (%d/%d) - Use Ctrl+T to cycle", 
                        themes[current_theme].name,
                        current_theme + 1,
                        NUM_THEMES);
}

/* Calculate line number width based on total number of rows - Memory efficient */
void editorUpdateLineNumberWidth(void) {
    if (!E.show_line_numbers) {
        E.line_numbers_width = 0;
        return;
    }
    
    int max_line = E.numrows > 0 ? E.numrows : 1;
    E.line_numbers_width = 1;
    
    /* Performance optimized: calculate width without string operations */
    while (max_line >= 10) {
        E.line_numbers_width++;
        max_line /= 10;
    }
    
    E.line_numbers_width += 2; /* Add space for padding and separator */
}

/* Toggle line numbers on/off with theme compatibility */
void editorToggleLineNumbers(void) {
    E.show_line_numbers = !E.show_line_numbers;
    editorUpdateLineNumberWidth();
    editorSetStatusMessage("Line numbers: %s (Ctrl+N to toggle)", 
                        E.show_line_numbers ? "ON" : "OFF");
}

enum KEY_ACTION{
        KEY_NULL = 0,       /* NULL */
        CTRL_C = 3,         /* Ctrl-c */
        CTRL_D = 4,         /* Ctrl-d */
        CTRL_F = 6,         /* Ctrl-f */
        CTRL_H = 8,         /* Ctrl-h */
        TAB = 9,            /* Tab */
        CTRL_L = 12,        /* Ctrl+l */
        ENTER = 13,         /* Enter */
        CTRL_N = 14,        /* Ctrl-n */
        CTRL_Q = 17,        /* Ctrl-q */
        CTRL_S = 19,        /* Ctrl-s */
        CTRL_T = 20,        /* Ctrl-t */
        CTRL_U = 21,        /* Ctrl-u */
        ESC = 27,           /* Escape */
        BACKSPACE =  127,   /* Backspace */
        /* The following are just soft codes, not really reported by the
         * terminal directly. */
        ARROW_LEFT = 1000,
        ARROW_RIGHT,
        ARROW_UP,
        ARROW_DOWN,
        DEL_KEY,
        HOME_KEY,
        END_KEY,
        PAGE_UP,
        PAGE_DOWN
};

/* Forward declarations for append buffer */
struct abuf {
    char *b;
    int len;
};

void abAppend(struct abuf *ab, const char *s, int len);

/* =========================== Syntax highlights DB =========================
 *
 * In order to add a new syntax, define two arrays with a list of file name
 * matches and keywords. The file name matches are used in order to match
 * a given syntax with a given file name: if a match pattern starts with a
 * dot, it is matched as the last past of the filename, for example ".c".
 * Otherwise the pattern is just searched inside the filenme, like "Makefile").
 *
 * The list of keywords to highlight is just a list of words, however if they
 * a trailing '|' character is added at the end, they are highlighted in
 * a different color, so that you can have two different sets of keywords.
 *
 * Finally add a stanza in the HLDB global variable with two two arrays
 * of strings, and a set of flags in order to enable highlighting of
 * comments and numbers.
 *
 * The characters for single and multi line comments must be exactly two
 * and must be provided as well (see the C language example).
 *
 * There is no support to highlight patterns currently. */

/* C / C++ */
char *C_HL_extensions[] = {".c",".h",".cpp",".hpp",".cc",NULL};
char *C_HL_keywords[] = {
    /* C Keywords (HL_KEYWORD1) */
    "auto","break","case","continue","default","do","else","enum",
    "extern","for","goto","if","register","return","sizeof","static",
    "struct","switch","typedef","union","volatile","while",
    /* C99+ Keywords */
    "const","inline","restrict","_Alignas","_Alignof","_Atomic",
    "_Bool","_Complex","_Generic","_Imaginary","_Noreturn","_Static_assert",
    "_Thread_local",

    /* C++ Keywords (HL_KEYWORD1) */
    "alignas","alignof","and","and_eq","asm","bitand","bitor","class",
    "compl","constexpr","const_cast","decltype","delete","dynamic_cast",
    "explicit","export","false","friend","mutable","namespace",
    "new","noexcept","not","not_eq","nullptr","operator","or","or_eq",
    "private","protected","public","reinterpret_cast","static_assert",
    "static_cast","template","this","thread_local","throw","true","try",
    "typeid","typename","virtual","xor","xor_eq","catch",
    /* C++11/14/17/20 Keywords */
    "concept","consteval","constinit","co_await","co_return","co_yield",
    "final","import","module","override","requires",

    /* C/C++ Types (HL_KEYWORD2) */
    "int|","long|","double|","float|","char|","unsigned|","signed|",
    "void|","short|","bool|","size_t|","wchar_t|","ptrdiff_t|",
    /* Fixed-width integer types */
    "int8_t|","int16_t|","int32_t|","int64_t|",
    "uint8_t|","uint16_t|","uint32_t|","uint64_t|",
    "int_least8_t|","int_least16_t|","int_least32_t|","int_least64_t|",
    "uint_least8_t|","uint_least16_t|","uint_least32_t|","uint_least64_t|",
    "int_fast8_t|","int_fast16_t|","int_fast32_t|","int_fast64_t|",
    "uint_fast8_t|","uint_fast16_t|","uint_fast32_t|","uint_fast64_t|",
    "intmax_t|","uintmax_t|","intptr_t|","uintptr_t|",
    /* Other C standard types */
    "FILE|","time_t|","clock_t|","va_list|","jmp_buf|",
    "sig_atomic_t|","fpos_t|","div_t|","ldiv_t|",
    "float_t|","double_t|","ssize_t|","max_align_t|",
    "char16_t|","char32_t|","mbstate_t|","locale_t|",

    /* C Standard Library Functions (HL_KEYWORD3) */
    /* I/O functions */
    "printf||","scanf||","fprintf||","fscanf||","sprintf||","sscanf||",
    "fopen||","fclose||","fread||","fwrite||","fseek||","ftell||",
    "fgets||","fputs||","fflush||","freopen||","remove||","rename||",
    "fgetc||","fputc||","getc||","putc||","getchar||","putchar||",
    /* String functions */
    "strlen||","strcpy||","strncpy||","strcat||","strncat||",
    "strcmp||","strncmp||","strchr||","strrchr||","strstr||",
    "strtok||","strpbrk||","strspn||","strcspn||","strerror||",
    /* Memory management */
    "malloc||","calloc||","realloc||","free||","memset||","memcpy||","memmove||",
    /* Character classification */
    "isalpha||","isdigit||","isalnum||","isspace||","iscntrl||",
    "isgraph||","islower||","isupper||","isprint||","ispunct||",
    "tolower||","toupper||",
    /* Conversion functions */
    "atoi||","atol||","atof||","strtol||","strtod||","strtoul||",
    /* Math functions */
    "abs||","labs||","fabs||","floor||","ceil||","sqrt||","pow||",
    "sin||","cos||","tan||","asin||","acos||","atan||","exp||","log||",
    /* Utility functions */
    "qsort||","bsearch||","div||","ldiv||","rand||","srand||",
    "time||","difftime||","clock||","mktime||","asctime||","ctime||",
    "localtime||","gmtime||","strftime||","exit||","abort||",
    "assert||","setjmp||","longjmp||","signal||",
    
    /* Important constants and macros */
    "NULL||","EOF||","SEEK_SET||","SEEK_CUR||","SEEK_END||",
    "EXIT_SUCCESS||","EXIT_FAILURE||","RAND_MAX||","INT_MIN||","INT_MAX||",
    "LONG_MIN||","LONG_MAX||","UINT_MAX||","LLONG_MIN||","LLONG_MAX||",
    "ULLONG_MAX||","CHAR_MIN||","CHAR_MAX||","CHAR_BIT||","SIZE_MAX||",
    "stdin||","stdout||","stderr||","true||","false||",
    
    /* Common preprocessor directives (highlighted as keywords) */
    "define","include","ifdef","ifndef","endif","if","elif","else",
    "error","pragma","undef","line",
    NULL
};

/* Python */
char *Python_HL_extensions[] = {".py",".pyw",".pyi",".py3",NULL};
char *Python_HL_keywords[] = {
    /* Python Keywords (HL_KEYWORD1) */
    "and","as","assert","break","class","continue","def","del","elif",
    "else","except","finally","for","from","global","if","import",
    "in","is","lambda","not","or","pass","print","raise","return","try",
    "while","with","yield","async","await","nonlocal","False","None",
    "True","match","case","_","type","self","cls",
    
    /* Python Built-in Types and Constants (HL_KEYWORD2) */
    "bool|","int|","float|","complex|","str|","bytes|","bytearray|",
    "list|","tuple|","dict|","set|","frozenset|","object|","type|",
    "function|","method|","module|","memoryview|","range|","slice|",
    "property|","classmethod|","staticmethod|","Ellipsis|","NotImplemented|",
    "NoneType|","generator|","coroutine|","iterator|","sequence|","mapping|",
    "__annotations__|","__dict__|","__doc__|","__file__|","__name__|",
    "__package__|","__spec__|","__loader__|","__path__|","__class__|",
    
    /* Python Built-in Functions and Methods (HL_KEYWORD3) */
    "abs||","all||","any||","ascii||","bin||","bool||","breakpoint||",
    "bytearray||","bytes||","callable||","chr||","classmethod||","compile||",
    "complex||","delattr||","dict||","dir||","divmod||","enumerate||","eval||",
    "exec||","filter||","float||","format||","frozenset||","getattr||",
    "globals||","hasattr||","hash||","help||","hex||","id||","input||","int||",
    "isinstance||","issubclass||","iter||","len||","list||","locals||","map||",
    "max||","memoryview||","min||","next||","object||","oct||","open||","ord||",
    "pow||","print||","property||","range||","repr||","reversed||","round||",
    "set||","setattr||","slice||","sorted||","staticmethod||","str||","sum||",
    "super||","tuple||","type||","vars||","zip||","__import__||",
    
    /* Common Python Modules and Their Functions */
    "os||","sys||","re||","math||","datetime||","json||","random||",
    "collections||","itertools||","functools||","pathlib||","time||",
    "os.path||","numpy||","pandas||","matplotlib||","requests||",
    "unittest||","pytest||","logging||","argparse||","csv||","pickle||",
    "sqlite3||","socket||","subprocess||","threading||","multiprocessing||",
    
    /* Common Methods */
    "append||","extend||","insert||","remove||","pop||","clear||","index||",
    "count||","sort||","reverse||","copy||","deepcopy||","keys||","values||",
    "items||","get||","update||","add||","discard||","join||","split||",
    "strip||","lstrip||","rstrip||","replace||","format||","startswith||",
    "endswith||","find||","rfind||","lower||","upper||","title||","read||",
    "write||","close||","seek||","tell||","readline||","readlines||",
    
    /* Common Exception Types */
    "Exception||","ArithmeticError||","AssertionError||","AttributeError||",
    "BaseException||","BlockingIOError||","BrokenPipeError||","BufferError||",
    "ChildProcessError||","ConnectionError||","EOFError||","FileExistsError||",
    "FileNotFoundError||","FloatingPointError||","ImportError||",
    "IndentationError||","IndexError||","InterruptedError||","IsADirectoryError||",
    "KeyError||","KeyboardInterrupt||","LookupError||","MemoryError||",
    "ModuleNotFoundError||","NameError||","NotADirectoryError||",
    "NotImplementedError||","OSError||","OverflowError||","PermissionError||",
    "ProcessLookupError||","RecursionError||","ReferenceError||","RuntimeError||",
    "StopIteration||","StopAsyncIteration||","SyntaxError||","SystemError||",
    "SystemExit||","TabError||","TimeoutError||","TypeError||",
    "UnboundLocalError||","UnicodeError||","UnicodeDecodeError||",
    "UnicodeEncodeError||","UnicodeTranslateError||","ValueError||",
    "ZeroDivisionError||",
    NULL
};

/* JavaScript */
char *JS_HL_extensions[] = {".js",".jsx",".mjs",".cjs",NULL};
char *JS_HL_keywords[] = {
    /* JavaScript Keywords (HL_KEYWORD1) */
    "break","case","catch","class","const","continue","debugger","default",
    "delete","do","else","export","extends","finally","for","function",
    "if","import","in","instanceof","let","new","return","super","switch",
    "this","throw","try","typeof","var","void","while","with","yield",
    "async","await","of","static","get","set","from","as","enum",
    "implements","interface","package","private","protected","public",
    "arguments","eval","globalThis",
    
    /* JavaScript Built-in Types (HL_KEYWORD2) */
    "undefined|","null|","boolean|","number|","string|","symbol|","object|","bigint|",
    "Array|","Object|","Function|","String|","Number|","Boolean|","Date|",
    "RegExp|","Error|","Promise|","Map|","Set|","WeakMap|","WeakSet|","Symbol|",
    "Int8Array|","Uint8Array|","Uint8ClampedArray|","Int16Array|","Uint16Array|",
    "Int32Array|","Uint32Array|","Float32Array|","Float64Array|","BigInt64Array|",
    "BigUint64Array|","ArrayBuffer|","SharedArrayBuffer|","DataView|","Proxy|",
    "Reflect|","Intl|","WebAssembly|","Generator|","GeneratorFunction|","AsyncFunction|",
    "HTMLElement|","Element|","Node|","Document|","Window|","Event|","File|","Blob|",
    
    /* JavaScript Built-in Functions and Objects (HL_KEYWORD3) */
    "console||","console.log||","console.error||","console.warn||","console.info||",
    "console.debug||","console.table||","console.time||","console.timeEnd||",
    "parseInt||","parseFloat||","isNaN||","isFinite||","isInteger||",
    "encodeURI||","decodeURI||","encodeURIComponent||","decodeURIComponent||",
    "setTimeout||","setInterval||","clearTimeout||","clearInterval||","requestAnimationFrame||",
    "JSON||","JSON.parse||","JSON.stringify||","Math||","Math.abs||","Math.floor||",
    "Math.ceil||","Math.round||","Math.max||","Math.min||","Math.random||",
    "Object.keys||","Object.values||","Object.entries||","Object.assign||","Object.create||",
    "Array.isArray||","Array.from||","Array.of||","Promise.all||","Promise.race||",
    "Promise.resolve||","Promise.reject||","Promise.allSettled||","String.fromCharCode||",
    "document||","window||","location||","history||","localStorage||","sessionStorage||",
    "navigator||","fetch||","XMLHttpRequest||","WebSocket||","Worker||","FormData||",
    "URL||","URLSearchParams||","alert||","confirm||","prompt||","performance||",
    "addEventListener||","removeEventListener||","querySelector||","querySelectorAll||",
    NULL
};

/* TypeScript */
char *TS_HL_extensions[] = {".ts",".tsx",".d.ts",NULL};
char *TS_HL_keywords[] = {
    /* TypeScript Keywords (HL_KEYWORD1) */
    "abstract","any","as","asserts","bigint","boolean","break","case","catch",
    "class","const","continue","debugger","declare","default","delete","do",
    "else","enum","export","extends","false","finally","for","from","function",
    "get","if","implements","import","in","infer","instanceof","interface",
    "is","keyof","let","module","namespace","never","new","null","number",
    "object","package","private","protected","public","readonly","require",
    "return","set","static","string","super","switch","symbol","this","throw",
    "true","try","type","typeof","undefined","unique","unknown","var","void",
    "while","with","yield","async","await","of","satisfies","override",
    "accessor","global","out","using","intrinsic","as const","in keyof",

    /* TypeScript Built-in Types (HL_KEYWORD2) */
    "string|","number|","boolean|","object|","undefined|","null|","void|",
    "never|","unknown|","any|","bigint|","symbol|","Array|","Promise|",
    "Record|","Partial|","Required|","Pick|","Omit|","Exclude|","Extract|",
    "Readonly|","ReadonlyArray|","NonNullable|","ReturnType|","Parameters|",
    "InstanceType|","ThisParameterType|","OmitThisParameter|","ThisType|",
    "Uppercase|","Lowercase|","Capitalize|","Uncapitalize|","Map|","Set|",
    "WeakMap|","WeakSet|","Date|","RegExp|","Error|","Function|","Tuple|",
    "Awaited|","ArrayLike|","Iterator|","IterableIterator|","PropertyKey|",
    "ConstructorParameters|","CallableFunction|","NewableFunction|",
    "ConcatArray|","ReadonlyMap|","ReadonlySet|","ClassDecorator|",
    "PropertyDecorator|","MethodDecorator|","ParameterDecorator|",
    "Iterable|","AsyncIterable|","Generator|","AsyncGenerator|",

    /* TypeScript DOM Types (HL_KEYWORD2) */
    "HTMLElement|","Node|","Document|","Window|","Event|","EventTarget|",
    "MouseEvent|","KeyboardEvent|","TouchEvent|","NodeList|","Element|",
    "CSSStyleDeclaration|","DOMParser|","FileReader|","Blob|","File|",
    "URL|","URLSearchParams|","Request|","Response|","Headers|",
    "FormData|","WebSocket|","Worker|","MutationObserver|","IntersectionObserver|",
    "ResizeObserver|","Performance|","SVGElement|","Canvas|","CanvasRenderingContext2D|",

    /* TypeScript Utility Functions and Objects (HL_KEYWORD3) */
    "console||","parseInt||","parseFloat||","isNaN||","isFinite||",
    "JSON||","Math||","Object||","Array||","String||","Number||","Boolean||",
    "Promise||","Date||","Map||","Set||","WeakMap||","WeakSet||","Proxy||",
    "Reflect||","Symbol||","RegExp||","Error||","encodeURI||","decodeURI||",
    "encodeURIComponent||","decodeURIComponent||","setTimeout||","clearTimeout||",
    "setInterval||","clearInterval||","requestAnimationFrame||","cancelAnimationFrame||",
    "localStorage||","sessionStorage||","navigator||","location||","history||",
    "document||","window||","globalThis||","fetch||","XMLHttpRequest||",
    "alert||","confirm||","prompt||","Error||","SyntaxError||","TypeError||",
    "RangeError||","ReferenceError||","EvalError||","URIError||","AggregateError||",
    "Intl||","structuredClone||","crypto||","performance||","console.log||",
    "console.error||","console.warn||","console.info||","console.debug||",
    "console.table||","console.time||","console.timeEnd||","console.trace||",
    "Array.isArray||","Object.keys||","Object.values||","Object.entries||",
    "Object.assign||","Object.create||","Object.defineProperty||",
    "Promise.all||","Promise.race||","Promise.resolve||","Promise.reject||",
    "Promise.allSettled||","Promise.any||",
    NULL
};

/* HTML */
char *HTML_HL_extensions[] = {".html",".htm",".xhtml",".shtml",NULL};
char *HTML_HL_keywords[] = {
    /* HTML Structure Tags (HL_KEYWORD1) */
    "html","head","body","title","meta","link","script","style","template",
    "slot","shadow","base","noscript","iframe","object","embed","param",
    
    /* HTML Content Sectioning (HL_KEYWORD1) */
    "div","span","h1","h2","h3","h4","h5","h6","p","br","hr","header",
    "footer","section","article","aside","main","nav","dialog","details",
    "summary","figure","figcaption","address","blockquote","pre","code",
    
    /* HTML Lists and Tables (HL_KEYWORD1) */
    "ul","ol","li","dl","dt","dd","table","tr","td","th","thead","tbody",
    "tfoot","caption","col","colgroup",
    
    /* HTML Form Elements (HL_KEYWORD1) */
    "form","input","button","select","option","optgroup","textarea","label",
    "fieldset","legend","datalist","output","progress","meter","keygen",
    
    /* HTML Media Elements (HL_KEYWORD1) */
    "a","img","audio","video","source","track","canvas","svg","picture",
    "map","area","portal",
    
    /* HTML Common Attributes (HL_KEYWORD2) */
    "id|","class|","style|","src|","href|","alt|","title|","width|","height|",
    "lang|","dir|","hidden|","tabindex|","accesskey|","draggable|","translate|",
    "contenteditable|","spellcheck|","autocapitalize|","enterkeyhint|","inputmode|",
    
    /* HTML Form Attributes (HL_KEYWORD2) */
    "type|","value|","name|","placeholder|","required|","disabled|",
    "readonly|","checked|","selected|","multiple|","autofocus|","pattern|",
    "min|","max|","step|","maxlength|","minlength|","size|","autocomplete|",
    "action|","method|","enctype|","novalidate|","for|","form|","formaction|",
    "formmethod|","formenctype|","formnovalidate|","formtarget|",
    
    /* HTML Link & Resource Attributes (HL_KEYWORD2) */
    "target|","rel|","download|","media|","crossorigin|","integrity|",
    "referrerpolicy|","loading|","decoding|","importance|","fetchpriority|",
    
    /* HTML Metadata Attributes (HL_KEYWORD2) */
    "charset|","content|","http-equiv|","property|","itemprop|","itemscope|",
    "itemtype|","itemid|","async|","defer|","nonce|","data-*|",
    
    /* WAI-ARIA Accessibility Attributes (HL_KEYWORD2) */
    "role|","aria-label|","aria-labelledby|","aria-describedby|","aria-hidden|",
    "aria-expanded|","aria-controls|","aria-live|","aria-atomic|","aria-relevant|",
    "aria-disabled|","aria-haspopup|","aria-pressed|","aria-checked|","aria-selected|",
    "aria-current|","aria-invalid|","aria-required|","aria-orientation|","aria-level|",
    
    /* HTML5 Semantic & Special Elements (HL_KEYWORD3) */
    "article||","section||","nav||","aside||","header||","footer||","main||",
    "mark||","time||","ruby||","rt||","rp||","bdi||","wbr||","data||",
    "abbr||","cite||","dfn||","em||","strong||","small||","sub||","sup||",
    "samp||","kbd||","var||","q||","u||","b||","i||","s||","del||","ins||",
    "dialog||","slot||","template||","picture||","portal||","search||",
    
    /* HTML5 Interactive & Media Elements (HL_KEYWORD3) */
    "audio||","video||","canvas||","svg||","math||","progress||","meter||",
    "details||","summary||","dialog||","datalist||","output||","track||",
    
    /* HTML Global Event Attributes (HL_KEYWORD3) */
    "onclick||","onchange||","onsubmit||","onload||","oninput||","onfocus||",
    "onblur||","onkeydown||","onkeyup||","onmouseover||","onmouseout||",
    "ondragstart||","ondrop||","onscroll||","ontouchstart||","ontouchend||",
    NULL
};

/* CSS */
char *CSS_HL_extensions[] = {".css",".scss",".sass",".less",NULL};
char *CSS_HL_keywords[] = {
    /* CSS Layout Properties (HL_KEYWORD1) */
    "display","position","top","right","bottom","left","z-index","float","clear",
    "width","height","max-width","max-height","min-width","min-height",
    "overflow","overflow-x","overflow-y","resize","clip","visibility",
    "margin","margin-top","margin-right","margin-bottom","margin-left",
    "padding","padding-top","padding-right","padding-bottom","padding-left",
    "box-sizing","object-fit","object-position","aspect-ratio",
    
    /* CSS Flexbox Properties */
    "flex","flex-direction","flex-wrap","flex-flow","flex-grow","flex-shrink",
    "flex-basis","justify-content","align-items","align-self","align-content",
    "gap","row-gap","column-gap","order",
    
    /* CSS Grid Properties */
    "grid","grid-template","grid-template-rows","grid-template-columns","grid-template-areas",
    "grid-auto-rows","grid-auto-columns","grid-auto-flow","grid-row","grid-column",
    "grid-area","grid-row-start","grid-row-end","grid-column-start","grid-column-end",
    
    /* CSS Background & Border Properties */
    "color","background","background-color","background-image","background-repeat",
    "background-attachment","background-position","background-size","background-origin",
    "background-clip","background-blend-mode","border","border-width","border-style",
    "border-color","border-top","border-right","border-bottom","border-left",
    "border-radius","border-top-left-radius","border-top-right-radius",
    "border-bottom-right-radius","border-bottom-left-radius","border-image",
    "border-collapse","outline","outline-width","outline-style","outline-color",
    "outline-offset","box-shadow","mask","mask-image","mask-position",
    
    /* CSS Typography Properties */
    "font","font-family","font-size","font-weight","font-style","font-variant",
    "font-stretch","line-height","letter-spacing","word-spacing","text-align",
    "text-decoration","text-decoration-line","text-decoration-style","text-decoration-color",
    "text-transform","text-indent","text-overflow","text-shadow","white-space",
    "vertical-align","word-break","word-wrap","overflow-wrap","hyphens",
    "direction","unicode-bidi","writing-mode","text-orientation","quotes",
    
    /* CSS Transform & Animation Properties */
    "transform","transform-origin","transform-style","backface-visibility",
    "perspective","perspective-origin","transition","transition-property",
    "transition-duration","transition-timing-function","transition-delay",
    "animation","animation-name","animation-duration","animation-timing-function",
    "animation-delay","animation-iteration-count","animation-direction",
    "animation-fill-mode","animation-play-state",
    
    /* CSS Other Properties */
    "opacity","filter","backdrop-filter","cursor","pointer-events","user-select",
    "list-style","list-style-type","list-style-position","list-style-image",
    "table-layout","caption-side","empty-cells","content","counter-reset",
    "counter-increment","will-change","scroll-behavior","overscroll-behavior",
    "contain","isolation","mix-blend-mode","appearance","touch-action",
    "color-scheme","accent-color","caret-color","scrollbar-color","scrollbar-width",
    "place-items","place-content","place-self","all","container","container-type",
    
    /* CSS Values (HL_KEYWORD2) */
    "auto|","none|","inherit|","initial|","unset|","revert|","revert-layer|",
    "block|","inline|","inline-block|","flex|","grid|","contents|","flow-root|",
    "absolute|","relative|","fixed|","static|","sticky|","hidden|","visible|",
    "scroll|","auto|","clip|","ellipsis|","nowrap|","break-word|","normal|",
    "bold|","bolder|","lighter|","italic|","oblique|","underline|","overline|",
    "line-through|","solid|","dashed|","dotted|","double|","groove|","ridge|",
    "inset|","outset|","center|","left|","right|","justify|","top|","bottom|",
    "middle|","transparent|","currentcolor|","repeat|","no-repeat|","repeat-x|","repeat-y|",
    "cover|","contain|","pointer|","default|","move|","grab|","zoom-in|","zoom-out|",
    "row|","column|","row-reverse|","column-reverse|","wrap|","nowrap|","wrap-reverse|",
    "start|","end|","flex-start|","flex-end|","space-between|","space-around|","space-evenly|",
    "stretch|","baseline|","first|","last|","ease|","ease-in|","ease-out|","ease-in-out|",
    "linear|","step-start|","step-end|","forwards|","backwards|","both|","infinite|",
    "paused|","running|","alternate|","alternate-reverse|","normal|","reverse|",
    "uppercase|","lowercase|","capitalize|","small-caps|","subgrid|","masonry|",
    
    /* CSS Color Values */
    "black|","white|","red|","green|","blue|","yellow|","magenta|","cyan|",
    "gray|","grey|","silver|","maroon|","olive|","navy|","purple|","teal|",
    "aqua|","fuchsia|","lime|","orange|","brown|","pink|","violet|","indigo|",
    
    /* CSS Units */
    "px|","em|","rem|","vh|","vw|","vmin|","vmax|","dvh|","svh|","lvh|",
    "ex|","ch|","%|","pt|","pc|","in|","cm|","mm|","fr|","s|","ms|","deg|",
    "rad|","grad|","turn|","dpi|","dpcm|","dppx|",
    
    /* CSS Functions (HL_KEYWORD3) */
    "rgb||","rgba||","hsl||","hsla||","hwb||","lab||","lch||","color||",
    "url||","attr||","calc||","clamp||","min||","max||","var||","env||",
    "linear-gradient||","radial-gradient||","conic-gradient||","repeating-linear-gradient||",
    "repeating-radial-gradient||","repeating-conic-gradient||","image-set||",
    "translate||","translateX||","translateY||","translateZ||","translate3d||",
    "scale||","scaleX||","scaleY||","scaleZ||","scale3d||",
    "rotate||","rotateX||","rotateY||","rotateZ||","rotate3d||",
    "skew||","skewX||","skewY||","matrix||","matrix3d||","perspective||",
    "blur||","brightness||","contrast||","drop-shadow||","grayscale||",
    "hue-rotate||","invert||","opacity||","saturate||","sepia||",
    "cubic-bezier||","steps||","counter||","counters||","element||",
    "not||","is||","where||","has||","nth-child||","nth-of-type||",
    "cross-fade||","fit-content||","minmax||","repeat||","symbols||",
    "supports||","theme||","format||","local||","from||","to||",
    NULL
};

/* Lua */
char *Lua_HL_extensions[] = {".lua",NULL};
char *Lua_HL_keywords[] = {
    /* Lua Keywords (HL_KEYWORD1) */
    "and","break","do","else","elseif","end","false","for","function",
    "goto","if","in","local","nil","not","or","repeat","return","then",
    "true","until","while","_ENV","_G",
    
    /* Lua Built-in Types and Values (HL_KEYWORD2) */
    "nil|","boolean|","number|","string|","function|","userdata|","thread|","table|",
    "integer|","float|","true|","false|","...","self|","_VERSION|",
    
    /* Lua Built-in Functions and Libraries (HL_KEYWORD3) */
    /* Global functions */
    "assert||","collectgarbage||","dofile||","error||","getmetatable||",
    "ipairs||","load||","loadfile||","next||","pairs||","pcall||","print||",
    "rawequal||","rawget||","rawlen||","rawset||","require||","select||",
    "setmetatable||","tonumber||","tostring||","type||","xpcall||",
    /* Standard libraries */
    "coroutine||","debug||","io||","math||","os||","package||","string||","table||","utf8||",
    /* String methods */
    "string.byte||","string.char||","string.dump||","string.find||","string.format||",
    "string.gmatch||","string.gsub||","string.len||","string.lower||","string.match||",
    "string.rep||","string.reverse||","string.sub||","string.upper||",
    /* Table methods */
    "table.concat||","table.insert||","table.move||","table.pack||","table.remove||",
    "table.sort||","table.unpack||",
    /* Math functions */
    "math.abs||","math.acos||","math.asin||","math.atan||","math.ceil||",
    "math.cos||","math.deg||","math.exp||","math.floor||","math.fmod||",
    "math.huge||","math.log||","math.max||","math.min||","math.modf||",
    "math.pi||","math.rad||","math.random||","math.randomseed||","math.sin||",
    "math.sqrt||","math.tan||",
    /* IO functions */
    "io.close||","io.flush||","io.input||","io.lines||","io.open||",
    "io.output||","io.popen||","io.read||","io.tmpfile||","io.type||","io.write||",
    /* OS functions */
    "os.clock||","os.date||","os.difftime||","os.execute||","os.exit||",
    "os.getenv||","os.remove||","os.rename||","os.setlocale||","os.time||",
    /* Coroutine functions */
    "coroutine.create||","coroutine.isyieldable||","coroutine.resume||",
    "coroutine.running||","coroutine.status||","coroutine.wrap||","coroutine.yield||",
    NULL
};

/* Go */
char *Go_HL_extensions[] = {".go",NULL};
char *Go_HL_keywords[] = {
    /* Go Keywords (HL_KEYWORD1) */
    "break","case","chan","const","continue","default","defer","else",
    "fallthrough","for","func","go","goto","if","import","interface",
    "map","package","range","return","select","struct","switch","type","var",
    "iota","nil","true","false","_",

    /* Go Built-in Types (HL_KEYWORD2) */
    "bool|","byte|","complex64|","complex128|","error|","float32|","float64|",
    "int|","int8|","int16|","int32|","int64|","rune|","string|","uint|",
    "uint8|","uint16|","uint32|","uint64|","uintptr|","any|","comparable|",
    "Context|","Reader|","Writer|","ReadWriter|","ReadCloser|","WriteCloser|",
    "ReadWriteCloser|","Error|","Handler|","ResponseWriter|","Request|",
    "Time|","Duration|","WaitGroup|","Mutex|","RWMutex|","Cond|","Once|",
    
    /* Go Built-in Functions and Common Methods (HL_KEYWORD3) */
    /* Built-in functions */
    "append||","cap||","close||","complex||","copy||","delete||","imag||",
    "len||","make||","new||","panic||","print||","println||","real||","recover||",
    /* Common standard library packages */
    "fmt||","log||","os||","io||","time||","context||","sync||","net||","http||",
    "strings||","bytes||","strconv||","math||","sort||","encoding||","json||",
    "xml||","crypto||","database||","sql||","regexp||","reflect||","path||",
    "filepath||","bufio||","flag||","errors||","testing||",
    /* Common methods */
    "Printf||","Sprintf||","Fprintf||","Println||","Sprint||","Fprintln||",
    "Error||","String||","MarshalJSON||","UnmarshalJSON||","Scan||","Exec||",
    "Query||","QueryRow||","Open||","Close||","Read||","Write||","ReadFrom||",
    "WriteTo||","Marshal||","Unmarshal||","New||","Add||","Done||","Wait||",
    "Lock||","Unlock||","RLock||","RUnlock||","Listen||","ListenAndServe||",
    "Handle||","HandleFunc||","Get||","Post||","Set||","Do||","Parse||",
    "Execute||","Sleep||","Now||","Since||","Until||","Format||","Join||",
    "Split||","Replace||","Contains||","HasPrefix||","HasSuffix||","TrimSpace||",
    "NewReader||","NewWriter||","NewDecoder||","NewEncoder||","Fatal||",
    "Fatalf||","Copy||","CopyN||","Create||","Remove||","Mkdir||","MkdirAll||",
    NULL
};

/* Rust */
char *Rust_HL_extensions[] = {".rs",NULL};
char *Rust_HL_keywords[] = {
    /* Rust Keywords (HL_KEYWORD1) */
    "as","async","await","break","const","continue","crate","dyn","else",
    "enum","extern","false","fn","for","if","impl","in","let","loop",
    "match","mod","move","mut","pub","ref","return","self","Self","static",
    "struct","super","trait","true","type","unsafe","use","where","while",
    "abstract","become","box","do","final","macro","override","priv","typeof",
    "unsized","virtual","yield","try","union","catch","default","macro_rules",
    
    /* Rust Built-in Types (HL_KEYWORD2) */
    "bool|","char|","str|","i8|","i16|","i32|","i64|","i128|","isize|",
    "u8|","u16|","u32|","u64|","u128|","usize|","f32|","f64|","String|",
    "Vec|","Option|","Result|","Box|","Rc|","Arc|","RefCell|","Mutex|",
    "HashMap|","HashSet|","BTreeMap|","BTreeSet|","VecDeque|","LinkedList|",
    "BinaryHeap|","Cell|","RwLock|","Cow|","Path|","PathBuf|","OsString|",
    "Ordering|","Range|","RangeInclusive|","RangeTo|","RangeFrom|","Duration|",
    "Instant|","SystemTime|","PhantomData|","Pin|","Future|","Stream|","Iterator|",
    "Send|","Sync|","Copy|","Clone|","Debug|","Display|","Error|","From|","Into|",
    
    /* Rust Built-in Functions and Macros (HL_KEYWORD3) */
    "println||","print||","panic||","assert||","assert_eq||","assert_ne||",
    "debug_assert||","unreachable||","unimplemented||","todo||","compile_error||",
    "format||","vec||","Some||","None||","Ok||","Err||","Default||","Clone||",
    "include||","include_str||","include_bytes||","concat||","env||","option_env||",
    "file||","line||","column||","module_path||","cfg||","stringify||","dbg||",
    "eprint||","eprintln||","write||","writeln||","format_args||","from_iter||",
    "iter||","into_iter||","collect||","map||","filter||","fold||","reduce||",
    "find||","any||","all||","count||","enumerate||","zip||","rev||","sorted||",
    "to_string||","to_owned||","as_ref||","as_mut||","unwrap||","expect||",
    "unwrap_or||","unwrap_or_else||","unwrap_or_default||","is_some||","is_none||",
    "is_ok||","is_err||","and_then||","or_else||","map_err||","new||","default||",
    "len||","is_empty||","contains||","insert||","remove||","get||","set||",
    NULL
};

/* Java */
char *Java_HL_extensions[] = {".java",NULL};
char *Java_HL_keywords[] = {
    /* Java Keywords (HL_KEYWORD1) */
    "abstract","assert","boolean","break","byte","case","catch","char","class",
    "const","continue","default","do","double","else","enum","extends","final",
    "finally","float","for","goto","if","implements","import","instanceof",
    "int","interface","long","native","new","package","private","protected",
    "public","return","short","static","strictfp","super","switch","synchronized",
    "this","throw","throws","transient","try","void","volatile","while",
    /* Java 8+ Keywords */
    "var","module","requires","exports","opens","uses","provides","with","to",
    "yield","sealed","permits","record","non-sealed",
    
    /* Java Built-in Types (HL_KEYWORD2) */
    "boolean|","byte|","char|","double|","float|","int|","long|","short|",
    "String|","Object|","Class|","Integer|","Double|","Float|","Boolean|",
    "Character|","Byte|","Short|","Long|","BigInteger|","BigDecimal|",
    /* Collections */
    "ArrayList|","HashMap|","HashSet|","LinkedList|","TreeMap|","TreeSet|",
    "Queue|","Deque|","LinkedHashMap|","LinkedHashSet|","ConcurrentHashMap|",
    "PriorityQueue|","Vector|","Stack|","CopyOnWriteArrayList|",
    /* Functional Interfaces */
    "Function|","Consumer|","Supplier|","Predicate|","BiFunction|","BiConsumer|",
    "BiPredicate|","UnaryOperator|","BinaryOperator|","Runnable|","Callable|",
    /* Utility Types */
    "Optional|","Stream|","Collector|","Collectors|","Arrays|","Collections|",
    "Comparator|","Iterable|","Iterator|","Enum|","Thread|","ThreadLocal|",
    "Future|","CompletableFuture|","Path|","Files|","Instant|","Duration|",
    "LocalDate|","LocalTime|","LocalDateTime|","ZonedDateTime|","Period|",
    
    /* Java Built-in Functions (HL_KEYWORD3) */
    "System||","out||","in||","err||","println||","print||","printf||",
    "format||","valueOf||","toString||","equals||","hashCode||","compareTo||",
    "length||","size||","get||","set||","put||","remove||","add||","contains||",
    "stream||","forEach||","map||","filter||","reduce||","collect||","sorted||",
    "of||","join||","split||","replace||","substring||","parseInt||","parseDouble||",
    /* Common Classes */
    "String||","Integer||","Double||","Float||","Boolean||","Character||",
    "Math||","Scanner||","Arrays||","Collections||","List||","Map||","Set||",
    "Optional||","Stream||","Files||","Paths||","Pattern||","Matcher||",
    /* Exceptions */
    "Exception||","RuntimeException||","IOException||","SQLException||",
    "NullPointerException||","IllegalArgumentException||","ClassNotFoundException||",
    "IndexOutOfBoundsException||","NumberFormatException||","ArithmeticException||",
    "UnsupportedOperationException||","ConcurrentModificationException||",
    NULL
};

/* Here we define an array of syntax highlights by extensions, keywords,
 * comments delimiters and flags. */
struct editorSyntax HLDB[] = {
    {
        /* C / C++ */
        C_HL_extensions,
        C_HL_keywords,
        "//","/*","*/",
        HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
    },
    {
        /* Python */
        Python_HL_extensions,
        Python_HL_keywords,
        "#","","",
        HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
    },
    {
        /* JavaScript */
        JS_HL_extensions,
        JS_HL_keywords,
        "//","/*","*/",
        HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
    },
    {
        /* TypeScript */
        TS_HL_extensions,
        TS_HL_keywords,
        "//","/*","*/",
        HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
    },
    {
        /* HTML */
        HTML_HL_extensions,
        HTML_HL_keywords,
        "","<!--","-->",
        HL_HIGHLIGHT_STRINGS
    },
    {
        /* CSS */
        CSS_HL_extensions,
        CSS_HL_keywords,
        "","/*","*/",
        HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
    },
    {
        /* Lua */
        Lua_HL_extensions,
        Lua_HL_keywords,
        "--","--[[","]]",
        HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
    },
    {
        /* Go */
        Go_HL_extensions,
        Go_HL_keywords,
        "//","/*","*/",
        HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
    },
    {
        /* Rust */
        Rust_HL_extensions,
        Rust_HL_keywords,
        "//","/*","*/",
        HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
    },
    {
        /* Java */
        Java_HL_extensions,
        Java_HL_keywords,
        "//","/*","*/",
        HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
    }
};

#define HLDB_ENTRIES (sizeof(HLDB)/sizeof(HLDB[0]))

/* ======================= Low level terminal handling ====================== */

static struct termios orig_termios; /* In order to restore at exit.*/

void disableRawMode(int fd) {
    /* Don't even check the return value as it's too late. */
    if (E.rawmode) {
        tcsetattr(fd,TCSAFLUSH,&orig_termios);
        E.rawmode = 0;
    }
}

/* Called at exit to avoid remaining in raw mode. */
void editorAtExit(void) {
    disableRawMode(STDIN_FILENO);
    /* Reset background color */
    editorSetBackgroundColor(-1);
    /* Clear the screen and reposition cursor at top-left on exit. */
    if (write(STDOUT_FILENO, "\x1b[2J", 4) == -1) {
        perror("write");
    }
    if (write(STDOUT_FILENO, "\x1b[H", 3) == -1) {
        perror("write");
    }
}

/* Raw mode: 1960 magic shit. */
int enableRawMode(int fd) {
    struct termios raw;

    if (E.rawmode) return 0; /* Already enabled. */
    if (!isatty(STDIN_FILENO)) goto fatal;
    atexit(editorAtExit);
    if (tcgetattr(fd,&orig_termios) == -1) goto fatal;

    raw = orig_termios;  /* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    /* local modes - choing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer. */
    raw.c_cc[VMIN] = 0; /* Return each byte, or zero for timeout. */
    raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(fd,TCSAFLUSH,&raw) < 0) goto fatal;
    E.rawmode = 1;
    return 0;

fatal:
    errno = ENOTTY;
    return -1;
}

/* Read a key from the terminal put in raw mode, trying to handle
 * escape sequences. */
int editorReadKey(int fd) {
    int nread;
    char c, seq[3];
    while ((nread = read(fd,&c,1)) == 0);
    if (nread == -1) exit(1);

    while(1) {
        switch(c) {
        case ESC:    /* escape sequence */
            /* If this is just an ESC, we'll timeout here. */
            if (read(fd,seq,1) == 0) return ESC;
            if (read(fd,seq+1,1) == 0) return ESC;

            /* ESC [ sequences. */
            if (seq[0] == '[') {
                if (seq[1] >= '0' && seq[1] <= '9') {
                    /* Extended escape, read additional byte. */
                    if (read(fd,seq+2,1) == 0) return ESC;
                    if (seq[2] == '~') {
                        switch(seq[1]) {
                        case '3': return DEL_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        }
                    }
                } else {
                    switch(seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                    }
                }
            }

            /* ESC O sequences. */
            else if (seq[0] == 'O') {
                switch(seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
                }
            }
            break;
        default:
            return c;
        }
    }
}

/* Use the ESC [6n escape sequence to query the horizontal cursor position
 * and return it. On error -1 is returned, on success the position of the
 * cursor is stored at *rows and *cols and 0 is returned. */
int getCursorPosition(int ifd, int ofd, int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    /* Report cursor location */
    if (write(ofd, "\x1b[6n", 4) != 4) return -1;

    /* Read the response: ESC [ rows ; cols R */
    while (i < sizeof(buf)-1) {
        if (read(ifd,buf+i,1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    /* Parse it. */
    if (buf[0] != ESC || buf[1] != '[') return -1;
    if (sscanf(buf+2,"%d;%d",rows,cols) != 2) return -1;
    return 0;
}

/* Try to get the number of columns in the current terminal. If the ioctl()
 * call fails the function will try to query the terminal itself.
 * Returns 0 on success, -1 on error. */
int getWindowSize(int ifd, int ofd, int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        /* ioctl() failed. Try to query the terminal itself. */
        int orig_row, orig_col, retval;

        /* Get the initial position so we can restore it later. */
        retval = getCursorPosition(ifd,ofd,&orig_row,&orig_col);
        if (retval == -1) goto failed;

        /* Go to right/bottom margin and get position. */
        if (write(ofd,"\x1b[999C\x1b[999B",12) != 12) goto failed;
        retval = getCursorPosition(ifd,ofd,rows,cols);
        if (retval == -1) goto failed;

        /* Restore position. */
        char seq[32];
        snprintf(seq,32,"\x1b[%d;%dH",orig_row,orig_col);
        if (write(ofd,seq,strlen(seq)) == -1) {
            /* Can't recover... */
        }
        return 0;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }

failed:
    return -1;
}

/* ====================== Syntax highlight color scheme  ==================== */

int is_separator(int c) {
    return c == '\0' || isspace(c) || strchr(",.()+-/*=~%[];",c) != NULL;
}

/* Return true if the specified row last char is part of a multi line comment
 * that starts at this row or at one before, and does not end at the end
 * of the row but spawns to the next row. */
int editorRowHasOpenComment(erow *row) {
    if (row->hl && row->rsize && row->hl[row->rsize-1] == HL_MLCOMMENT &&
        (row->rsize < 2 || (row->render[row->rsize-2] != '*' ||
                            row->render[row->rsize-1] != '/'))) return 1;
    return 0;
}

/* Set every byte of row->hl (that corresponds to every character in the line)
 * to the right syntax highlight type (HL_* defines). */
void editorUpdateSyntax(erow *row) {
    row->hl = realloc(row->hl,row->rsize);
    memset(row->hl,HL_NORMAL,row->rsize);

    if (E.syntax == NULL) return;

    int i, prev_sep, in_string, in_comment, in_char;
    char *p;
    char **keywords = E.syntax->keywords;
    char *scs = E.syntax->singleline_comment_start;
    char *mcs = E.syntax->multiline_comment_start;
    char *mce = E.syntax->multiline_comment_end;

    p = row->render;
    i = 0;
    while(*p && isspace(*p)) {
        p++;
        i++;
    }
    prev_sep = 1;
    in_string = 0;
    in_char = 0;
    in_comment = 0;

    if (row->idx > 0 && editorRowHasOpenComment(&E.row[row->idx-1]))
        in_comment = 1;

    while(*p) {
        /* Handle preprocessor directives */
        if (i == 0 && *p == '#') {
            while(*p && *p != '\n') {
                row->hl[i] = HL_PREPROCESSOR;
                p++; i++;
            }
            continue;
        }

        /* Handle single line comments */
        if (prev_sep && *p == scs[0] && *(p+1) == scs[1]) {
            memset(row->hl+i,HL_COMMENT,row->size-i);
            return;
        }

        /* Handle multi line comments */
        if (in_comment) {
            row->hl[i] = HL_MLCOMMENT;
            if (*p == mce[0] && *(p+1) == mce[1]) {
                row->hl[i+1] = HL_MLCOMMENT;
                p += 2; i += 2;
                in_comment = 0;
                prev_sep = 1;
                continue;
            } else {
                prev_sep = 0;
                p++; i++;
                continue;
            }
        } else if (*p == mcs[0] && *(p+1) == mcs[1]) {
            row->hl[i] = HL_MLCOMMENT;
            row->hl[i+1] = HL_MLCOMMENT;
            p += 2; i += 2;
            in_comment = 1;
            prev_sep = 0;
            continue;
        }

        /* Handle strings and character literals */
        if (in_string || in_char) {
            row->hl[i] = HL_STRING;
            if (*p == '\\') {
                row->hl[i+1] = HL_STRING;
                p += 2; i += 2;
                prev_sep = 0;
                continue;
            }
            if (*p == in_string || *p == in_char) {
                in_string = in_char = 0;
            }
            p++; i++;
            continue;
        } else {
            if (*p == '"') {
                in_string = *p;
                row->hl[i] = HL_STRING;
                p++; i++;
                prev_sep = 0;
                continue;
            } else if (*p == '\'') {
                in_char = *p;
                row->hl[i] = HL_STRING;
                p++; i++;
                prev_sep = 0;
                continue;
            }
        }

        /* Handle operators */
        if (strchr("+-*/%=<>!&|^~?:", *p)) {
            row->hl[i] = HL_OPERATOR;
            p++; i++;
            prev_sep = 1;
            continue;
        }

        /* Handle brackets */
        if (strchr("(){}[]", *p)) {
            row->hl[i] = HL_BRACKET;
            p++; i++;
            prev_sep = 1;
            continue;
        }

        /* Handle non printable chars */
        if (!isprint(*p)) {
            row->hl[i] = HL_NONPRINT;
            p++; i++;
            prev_sep = 0;
            continue;
        }

        /* Handle numbers */
        if ((isdigit(*p) && (prev_sep || row->hl[i-1] == HL_NUMBER)) ||
            (*p == '.' && i >0 && row->hl[i-1] == HL_NUMBER)) {
            row->hl[i] = HL_NUMBER;
            p++; i++;
            prev_sep = 0;
            continue;
        }

        /* Handle function calls */
        if (prev_sep && isalpha(*p)) {
            int j = i;
            while (j < row->rsize && (isalnum(row->render[j]) || row->render[j] == '_')) {
                j++;
            }
            if (j < row->rsize && row->render[j] == '(') {
                memset(row->hl+i, HL_FUNCTION, j-i);
                p += (j-i);
                i = j;
                prev_sep = 0;
                continue;
            }
        }

        /* Handle keywords */
        if (prev_sep) {
            int j;
            for (j = 0; keywords[j]; j++) {
                int klen = strlen(keywords[j]);
                int kw2 = keywords[j][klen-1] == '|';
                int kw3 = kw2 && keywords[j][klen-2] == '|';
                
                if (kw3) klen -= 2;
                else if (kw2) klen--;

                if (!memcmp(p,keywords[j],klen) &&
                    is_separator(*(p+klen)))
                {
                    int hl_type = kw3 ? HL_KEYWORD3 : (kw2 ? HL_KEYWORD2 : HL_KEYWORD1);
                    memset(row->hl+i, hl_type, klen);
                    p += klen;
                    i += klen;
                    break;
                }
            }
            if (keywords[j] != NULL) {
                prev_sep = 0;
                continue;
            }
        }

        prev_sep = is_separator(*p);
        p++; i++;
    }

    int oc = editorRowHasOpenComment(row);
    if (row->hl_oc != oc && row->idx+1 < E.numrows)
        editorUpdateSyntax(&E.row[row->idx+1]);
    row->hl_oc = oc;
}

/* Maps syntax highlight token types to terminal colors. */
void editorSelectSyntaxHighlight(char *filename) {
    for (unsigned int j = 0; j < HLDB_ENTRIES; j++) {
        struct editorSyntax *s = HLDB+j;
        unsigned int i = 0;
        while(s->filematch[i]) {
            char *p;
            int patlen = strlen(s->filematch[i]);
            if ((p = strstr(filename,s->filematch[i])) != NULL) {
                if (s->filematch[i][0] != '.' || p[patlen] == '\0') {
                    E.syntax = s;
                    return;
                }
            }
            i++;
        }
    }
}

/* Set terminal background color */
void editorSetBackgroundColor(int color) {
    if (color == -1) {
        /* Reset to default background */
        printf("\x1b[49m");
    } else {
        /* Set 256-color background */
        printf("\x1b[48;5;%dm", color);
    }
    fflush(stdout);
}

/* Enhanced status bar with theme colors */
void editorDrawStatusBar(struct abuf *ab) {
    abAppend(ab,"\x1b[0K",4);
    
    /* Set status bar background */
    char status_bg[16];
    snprintf(status_bg, sizeof(status_bg), "\x1b[48;5;%dm", themes[current_theme].status_bg);
    abAppend(ab, status_bg, strlen(status_bg));
    
    /* Set status bar foreground */
    char status_fg[16];
    snprintf(status_fg, sizeof(status_fg), "\x1b[38;5;%dm", themes[current_theme].status_fg);
    abAppend(ab, status_fg, strlen(status_fg));

    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), " %.15s - %d lines %s | Theme: %s | Line#: %s",
        E.filename ? E.filename : "[No Name]", E.numrows, 
        E.dirty ? "(modified)" : "", themes[current_theme].name,
        E.show_line_numbers ? "ON" : "OFF");
    
    int rlen = snprintf(rstatus, sizeof(rstatus),
        "%d/%d ", E.rowoff+E.cy+1, E.numrows);
    
    if (len > E.screencols) len = E.screencols;
    abAppend(ab, status, len);
    
    while(len < E.screencols) {
        if (E.screencols - len == rlen) {
            abAppend(ab, rstatus, rlen);
            break;
        } else {
            abAppend(ab, " ", 1);
            len++;
        }
    }
    abAppend(ab, "\x1b[0m\r\n", 6);
}

/* ======================= Editor rows implementation ======================= */

/* Update the rendered version and the syntax highlight of a row. */
void editorUpdateRow(erow *row) {
    unsigned int tabs = 0, nonprint = 0;
    int j, idx;

/* Create a version of the row we can directly print on the screen,
     * respecting tabs, substituting non printable characters with '?'. */
    free(row->render);
    for (j = 0; j < row->size; j++)
        if (row->chars[j] == TAB) tabs++;

    unsigned long long allocsize =
        (unsigned long long) row->size + tabs*8 + nonprint*9 + 1;
    if (allocsize > UINT32_MAX) {
        printf("Some line of the edited file is too long for xcodex\n");
        exit(1);
    }

    row->render = malloc(row->size + tabs*8 + nonprint*9 + 1);
    idx = 0;
    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == TAB) {
            row->render[idx++] = ' ';
            while((idx+1) % 8 != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->rsize = idx;
    row->render[idx] = '\0';

    /* Update the syntax highlighting attributes of the row. */
    editorUpdateSyntax(row);
}

/* Insert a row at the specified position, shifting the other rows on the bottom
 * if required. */
void editorInsertRow(int at, char *s, size_t len) {
    if (at > E.numrows) return;
    E.row = realloc(E.row,sizeof(erow)*(E.numrows+1));
    if (at != E.numrows) {
        memmove(E.row+at+1,E.row+at,sizeof(E.row[0])*(E.numrows-at));
        for (int j = at+1; j <= E.numrows; j++) E.row[j].idx++;
    }
    E.row[at].size = len;
    E.row[at].chars = malloc(len+1);
    memcpy(E.row[at].chars,s,len+1);
    E.row[at].hl = NULL;
    E.row[at].hl_oc = 0;
    E.row[at].render = NULL;
    E.row[at].rsize = 0;
    E.row[at].idx = at;
    editorUpdateRow(E.row+at);
    E.numrows++;
    editorUpdateLineNumberWidth(); /* Update line number width */
    E.dirty++;
}

/* Free row's heap allocated stuff. */
void editorFreeRow(erow *row) {
    free(row->render);
    free(row->chars);
    free(row->hl);
}

/* Remove the row at the specified position, shifting the remainign on the
 * top. */
void editorDelRow(int at) {
    erow *row;

    if (at >= E.numrows) return;
    row = E.row+at;
    editorFreeRow(row);
    memmove(E.row+at,E.row+at+1,sizeof(E.row[0])*(E.numrows-at-1));
    for (int j = at; j < E.numrows-1; j++) E.row[j].idx++;
    E.numrows--;
    editorUpdateLineNumberWidth(); /* Update line number width */
    E.dirty++;
}

/* Turn the editor rows into a single heap-allocated string.
 * Returns the pointer to the heap-allocated string and populate the
 * integer pointed by 'buflen' with the size of the string, escluding
 * the final nulterm. */
char *editorRowsToString(int *buflen) {
    char *buf = NULL, *p;
    int totlen = 0;
    int j;

    /* Compute count of bytes */
    for (j = 0; j < E.numrows; j++)
        totlen += E.row[j].size+1; /* +1 is for "\n" at end of every row */
    *buflen = totlen;
    totlen++; /* Also make space for nulterm */

    p = buf = malloc(totlen);
    for (j = 0; j < E.numrows; j++) {
        memcpy(p,E.row[j].chars,E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    *p = '\0';
    return buf;
}

/* Insert a character at the specified position in a row, moving the remaining
 * chars on the right if needed. */
void editorRowInsertChar(erow *row, int at, int c) {
    if (at > row->size) {
        /* Pad the string with spaces if the insert location is outside the
         * current length by more than a single character. */
        int padlen = at-row->size;
        /* In the next line +2 means: new char and null term. */
        row->chars = realloc(row->chars,row->size+padlen+2);
        memset(row->chars+row->size,' ',padlen);
        row->chars[row->size+padlen+1] = '\0';
        row->size += padlen+1;
    } else {
        /* If we are in the middle of the string just make space for 1 new
         * char plus the (already existing) null term. */
        row->chars = realloc(row->chars,row->size+2);
        memmove(row->chars+at+1,row->chars+at,row->size-at+1);
        row->size++;
    }
    row->chars[at] = c;
    editorUpdateRow(row);
    E.dirty++;
}

/* Append the string 's' at the end of a row */
void editorRowAppendString(erow *row, char *s, size_t len) {
    row->chars = realloc(row->chars,row->size+len+1);
    memcpy(row->chars+row->size,s,len);
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
    E.dirty++;
}

/* Delete the character at offset 'at' from the specified row. */
void editorRowDelChar(erow *row, int at) {
    if (row->size <= at) return;
    memmove(row->chars+at,row->chars+at+1,row->size-at);
    editorUpdateRow(row);
    row->size--;
    E.dirty++;
}

/* Insert the specified char at the current prompt position. */
void editorInsertChar(int c) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];
    int effective_screencols = E.screencols - E.line_numbers_width;

    /* If the row where the cursor is currently located does not exist in our
     * logical representaion of the file, add enough empty rows as needed. */
    if (!row) {
        while(E.numrows <= filerow)
            editorInsertRow(E.numrows,"",0);
    }
    row = &E.row[filerow];
    editorRowInsertChar(row,filecol,c);
    if (E.cx == effective_screencols-1)
        E.coloff++;
    else
        E.cx++;
    E.dirty++;
}

/* Inserting a newline is slightly complex as we have to handle inserting a
 * newline in the middle of a line, splitting the line as needed. */
void editorInsertNewline(void) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (!row) {
        if (filerow == E.numrows) {
            editorInsertRow(filerow,"",0);
            goto fixcursor;
        }
        return;
    }
    /* If the cursor is over the current line size, we want to conceptually
     * think it's just over the last character. */
    if (filecol >= row->size) filecol = row->size;
    if (filecol == 0) {
        editorInsertRow(filerow,"",0);
    } else {
        /* We are in the middle of a line. Split it between two rows. */
        editorInsertRow(filerow+1,row->chars+filecol,row->size-filecol);
        row = &E.row[filerow];
        row->chars[filecol] = '\0';
        row->size = filecol;
        editorUpdateRow(row);
    }
fixcursor:
    if (E.cy == E.screenrows-1) {
        E.rowoff++;
    } else {
        E.cy++;
    }
    E.cx = 0;
    E.coloff = 0;
}

/* Delete the char at the current prompt position. */
void editorDelChar(void) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (!row || (filecol == 0 && filerow == 0)) return;
    if (filecol == 0) {
        /* Handle the case of column 0, we need to move the current line
         * on the right of the previous one. */
        filecol = E.row[filerow-1].size;
        editorRowAppendString(&E.row[filerow-1],row->chars,row->size);
        editorDelRow(filerow);
        row = NULL;
        if (E.cy == 0)
            E.rowoff--;
        else
            E.cy--;
        E.cx = filecol;
        if (E.cx >= E.screencols) {
            int shift = (E.screencols-E.cx)+1;
            E.cx -= shift;
            E.coloff += shift;
        }
    } else {
        editorRowDelChar(row,filecol-1);
        if (E.cx == 0 && E.coloff)
            E.coloff--;
        else
            E.cx--;
    }
    if (row) editorUpdateRow(row);
    E.dirty++;
}

/* Load the specified program in the editor memory and returns 0 on success
 * or 1 on error. */
int editorOpen(char *filename) {
    FILE *fp;

    E.dirty = 0;
    free(E.filename);
    size_t fnlen = strlen(filename)+1;
    E.filename = malloc(fnlen);
    memcpy(E.filename,filename,fnlen);

    fp = fopen(filename,"r");
    if (!fp) {
        if (errno != ENOENT) {
            perror("Opening file");
            exit(1);
        }
        return 1;
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while((linelen = getline(&line,&linecap,fp)) != -1) {
        if (linelen && (line[linelen-1] == '\n' || line[linelen-1] == '\r'))
            line[--linelen] = '\0';
        editorInsertRow(E.numrows,line,linelen);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
    return 0;
}

/* Save the current file on disk. Return 0 on success, 1 on error. */
int editorSave(void) {
    int len;
    char *buf = editorRowsToString(&len);
    int fd = open(E.filename,O_RDWR|O_CREAT,0644);
    if (fd == -1) goto writeerr;

    /* Use truncate + a single write(2) call in order to make saving
     * a bit safer, under the limits of what we can do in a small editor. */
    if (ftruncate(fd,len) == -1) goto writeerr;
    if (write(fd,buf,len) != len) goto writeerr;

    close(fd);
    free(buf);
    E.dirty = 0;
    editorSetStatusMessage("%d bytes written on disk", len);
    return 0;

writeerr:
    free(buf);
    if (fd != -1) close(fd);
    editorSetStatusMessage("Can't save! I/O error: %s",strerror(errno));
    return 1;
}

/* ============================= Terminal update ============================ */

#define ABUF_INIT {NULL,0}

void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b,ab->len+len);

    if (new == NULL) return;
    memcpy(new+ab->len,s,len);
    ab->b = new;
    ab->len += len;
}

void abFree(struct abuf *ab) {
    free(ab->b);
}

/* This function writes the whole screen using VT100 escape characters
 * starting from the logical state of the editor in the global state 'E'. */
void editorRefreshScreen(void) {
    int y;
    erow *r;
    char buf[32];
    struct abuf ab = ABUF_INIT;

    abAppend(&ab,"\x1b[?25l",6); /* Hide cursor. */
    abAppend(&ab,"\x1b[H",3); /* Go home. */
    for (y = 0; y < E.screenrows; y++) {
        int filerow = E.rowoff+y;

        if (filerow >= E.numrows) {
            /* Draw line numbers for empty lines */
            if (E.show_line_numbers) {
                char line_num[16];
                int line_num_len = snprintf(line_num, sizeof(line_num), 
                    "%*s ", E.line_numbers_width - 1, "");
                
                /* Set line number color from theme */
                char line_color[16];
                snprintf(line_color, sizeof(line_color), "\x1b[38;5;%dm", themes[current_theme].line_number_color);
                abAppend(&ab, line_color, strlen(line_color));
                abAppend(&ab, line_num, line_num_len);
                abAppend(&ab, "\x1b[39m", 5); /* Reset color */
            }
            
            if (E.numrows == 0 && y == E.screenrows/3) {
                char welcome[80];
                int welcomelen = snprintf(welcome,sizeof(welcome),
                    "XCodex editor -- version %s -- Theme: %s\x1b[0K\r\n", 
                    XCODEX_VERSION, themes[current_theme].name);
                int effective_cols = E.screencols - E.line_numbers_width;
                int padding = (effective_cols - welcomelen)/2;
                if (padding) {
                    abAppend(&ab,"~",1);
                    padding--;
                }
                while(padding--) abAppend(&ab," ",1);
                abAppend(&ab,welcome,welcomelen);
            } else {
                abAppend(&ab,"~\x1b[0K\r\n",7);
            }
            continue;
        }

        r = &E.row[filerow];

        /* Draw line numbers */
        if (E.show_line_numbers) {
            char line_num[16];
            int current_line = filerow + 1;
            int line_num_len = snprintf(line_num, sizeof(line_num), 
                "%*d ", E.line_numbers_width - 1, current_line);
            
            /* Use theme-specific color for line numbers */
            char line_color[16];
            snprintf(line_color, sizeof(line_color), "\x1b[38;5;%dm", themes[current_theme].line_number_color);
            abAppend(&ab, line_color, strlen(line_color));
            abAppend(&ab, line_num, line_num_len);
            abAppend(&ab, "\x1b[0m", 4); /* Reset formatting */
        }

        int len = r->rsize - E.coloff;
        int effective_screencols = E.screencols - E.line_numbers_width;
        int current_color = -1;
        if (len > 0) {
            if (len > effective_screencols) len = effective_screencols;
            char *c = r->render+E.coloff;
            unsigned char *hl = r->hl+E.coloff;
            int j;
            for (j = 0; j < len; j++) {
                if (hl[j] == HL_NONPRINT) {
                    char sym;
                    abAppend(&ab,"\x1b[7m",4);
                    if (c[j] <= 26)
                        sym = '@'+c[j];
                    else
                        sym = '?';
                    abAppend(&ab,&sym,1);
                    abAppend(&ab,"\x1b[0m",4);
                } else if (hl[j] == HL_NORMAL) {
                    if (current_color != -1) {
                        abAppend(&ab,"\x1b[39m",5);
                        current_color = -1;
                    }
                    abAppend(&ab,c+j,1);
                } else {
                    int color = editorSyntaxToColor(hl[j]);
                    if (color != current_color) {
                        char buf[32];
                        int clen = snprintf(buf,sizeof(buf),"\x1b[38;5;%dm",color);
                        current_color = color;
                        abAppend(&ab,buf,clen);
                    }
                    abAppend(&ab,c+j,1);
                }
            }
        }
        abAppend(&ab,"\x1b[39m",5);
        abAppend(&ab,"\x1b[0K",4);
        abAppend(&ab,"\r\n",2);
    }

    /* Use the new enhanced status bar function */
    editorDrawStatusBar(&ab);

    /* Second row depends on E.statusmsg and the status message update time. */
    abAppend(&ab,"\x1b[0K",4);
    int msglen = strlen(E.statusmsg);
    if (msglen && time(NULL)-E.statusmsg_time < 5)
        abAppend(&ab,E.statusmsg,msglen <= E.screencols ? msglen : E.screencols);

    /* Put cursor at its current position. Note that the horizontal position
     * at which the cursor is displayed may be different compared to 'E.cx'
     * because of TABs. */
    int j;
    int cx = 1; /* Start at position 1 */
    
    /* Add line number width offset if line numbers are enabled */
    if (E.show_line_numbers) {
        cx += E.line_numbers_width;
    }
    
    int filerow = E.rowoff+E.cy;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];
    if (row) {
        for (j = E.coloff; j < (E.cx+E.coloff); j++) {
            if (j < row->size && row->chars[j] == TAB) {
                cx += 7-((cx-1)%8); /* Fix tab calculation by accounting for 0-based indexing */
            }
            cx++;
        }
    }
    snprintf(buf,sizeof(buf),"\x1b[%d;%dH",E.cy+1,cx);
    abAppend(&ab,buf,strlen(buf));
    abAppend(&ab,"\x1b[?25h",6); /* Show cursor. */
    write(STDOUT_FILENO,ab.b,ab.len);
    abFree(&ab);
}

/* Set an editor status message for the second line of the status, at the
 * end of the screen. */
void editorSetStatusMessage(const char *fmt, ...) {
    va_list ap;
    va_start(ap,fmt);
    vsnprintf(E.statusmsg,sizeof(E.statusmsg),fmt,ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}

/* =============================== Find mode ================================ */

#define XCODEX_QUERY_LEN 256

void editorFind(int fd) {
    char query[XCODEX_QUERY_LEN+1] = {0};
    int qlen = 0;
    int last_match = -1; /* Last line where a match was found. -1 for none. */
    int find_next = 0; /* if 1 search next, if -1 search prev. */
    int saved_hl_line = -1;  /* No saved HL */
    char *saved_hl = NULL;

#define FIND_RESTORE_HL do { \
    if (saved_hl) { \
        memcpy(E.row[saved_hl_line].hl,saved_hl, E.row[saved_hl_line].rsize); \
        free(saved_hl); \
        saved_hl = NULL; \
    } \
} while (0)

    /* Save the cursor position in order to restore it later. */
    int saved_cx = E.cx, saved_cy = E.cy;
    int saved_coloff = E.coloff, saved_rowoff = E.rowoff;

    while(1) {
        editorSetStatusMessage(
            "Search: %s (Use ESC/Arrows/Enter)", query);
        editorRefreshScreen();

        int c = editorReadKey(fd);
        if (c == DEL_KEY || c == CTRL_H || c == BACKSPACE) {
            if (qlen != 0) query[--qlen] = '\0';
            last_match = -1;
        } else if (c == ESC || c == ENTER) {
            if (c == ESC) {
                E.cx = saved_cx; E.cy = saved_cy;
                E.coloff = saved_coloff; E.rowoff = saved_rowoff;
            }
            FIND_RESTORE_HL;
            editorSetStatusMessage("");
            return;
        } else if (c == ARROW_RIGHT || c == ARROW_DOWN) {
            find_next = 1;
        } else if (c == ARROW_LEFT || c == ARROW_UP) {
            find_next = -1;
        } else if (isprint(c)) {
            if (qlen < XCODEX_QUERY_LEN) {
                query[qlen++] = c;
                query[qlen] = '\0';
                last_match = -1;
            }
        }

        /* Search occurrence. */
        if (last_match == -1) find_next = 1;
        if (find_next) {
            char *match = NULL;
            int match_offset = 0;
            int i, current = last_match;

            for (i = 0; i < E.numrows; i++) {
                current += find_next;
                if (current == -1) current = E.numrows-1;
                else if (current == E.numrows) current = 0;
                match = strstr(E.row[current].render,query);
                if (match) {
                    match_offset = match-E.row[current].render;
                    break;
                }
            }
            find_next = 0;

            /* Highlight */
            FIND_RESTORE_HL;

            if (match) {
                erow *row = &E.row[current];
                last_match = current;
                if (row->hl) {
                    saved_hl_line = current;
                    saved_hl = malloc(row->rsize);
                    memcpy(saved_hl,row->hl,row->rsize);
                    memset(row->hl+match_offset,HL_MATCH,qlen);
                }
                E.cy = 0;
                E.cx = match_offset;
                E.rowoff = current;
                E.coloff = 0;
                /* Scroll horizontally as needed. */
                if (E.cx > E.screencols) {
                    int diff = E.cx - E.screencols;
                    E.cx -= diff;
                    E.coloff += diff;
                }
            }
        }
    }
}

/* ========================= Editor events handling  ======================== */

/* Handle cursor position change because arrow keys were pressed. */
void editorMoveCursor(int key) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    int rowlen;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];
    int effective_screencols = E.screencols - E.line_numbers_width;

    switch(key) {
    case ARROW_LEFT:
        if (E.cx == 0) {
            if (E.coloff) {
                E.coloff--;
            } else {
                if (filerow > 0) {
                    E.cy--;
                    E.cx = E.row[filerow-1].size;
                    if (E.cx > effective_screencols-1) {
                        E.coloff = E.cx-effective_screencols+1;
                        E.cx = effective_screencols-1;
                    }
                }
            }
        } else {
            E.cx -= 1;
        }
        break;
    case ARROW_RIGHT:
        if (row && filecol < row->size) {
            if (E.cx == effective_screencols-1) {
                E.coloff++;
            } else {
                E.cx += 1;
            }
        } else if (row && filecol == row->size) {
            E.cx = 0;
            E.coloff = 0;
            if (E.cy == E.screenrows-1) {
                E.rowoff++;
            } else {
                E.cy += 1;
            }
        }
        break;
    case ARROW_UP:
        if (E.cy == 0) {
            if (E.rowoff) E.rowoff--;
        } else {
            E.cy -= 1;
        }
        break;
    case ARROW_DOWN:
        if (filerow < E.numrows) {
            if (E.cy == E.screenrows-1) {
                E.rowoff++;
            } else {
                E.cy += 1;
            }
        }
        break;
    }
    /* Fix cx if the current line has not enough chars. */
    filerow = E.rowoff+E.cy;
    filecol = E.coloff+E.cx;
    row = (filerow >= E.numrows) ? NULL : &E.row[filerow];
    rowlen = row ? row->size : 0;
    if (filecol > rowlen) {
        E.cx -= filecol-rowlen;
        if (E.cx < 0) {
            E.coloff += E.cx;
            E.cx = 0;
        }
    }
}

/* Process events arriving from the standard input, which is, the user
 * is typing stuff on the terminal. */
#define XCODEX_QUIT_TIMES 3
void editorProcessKeypress(int fd) {
    /* When the file is modified, requires Ctrl-q to be pressed N times
     * before actually quitting. */
    static int quit_times = XCODEX_QUIT_TIMES;

    int c = editorReadKey(fd);
    switch(c) {
    case ENTER:         /* Enter */
        editorInsertNewline();
        break;
    case CTRL_C:        /* Ctrl-c */
        /* We ignore ctrl-c, it can't be so simple to lose the changes
         * to the edited file. */
        break;
    case CTRL_Q:        /* Ctrl-q */
        /* Quit if the file was already saved. */
        if (E.dirty && quit_times) {
            editorSetStatusMessage("WARNING!!! File has unsaved changes. "
                "Press Ctrl-Q %d more times to quit.", quit_times);
            quit_times--;
            return;
        }
        exit(0);
        break;
    case CTRL_S:        /* Ctrl-s */
        editorSave();
        break;
    case CTRL_F:
        editorFind(fd);
        break;
    case CTRL_T:        /* Ctrl-t - cycle themes */
        editorCycleTheme(); /* Use the enhanced function instead */
        break;
    case CTRL_N:        /* Ctrl-n - toggle line numbers */
        editorToggleLineNumbers();
        break;
    case BACKSPACE:     /* Backspace */
    case CTRL_H:        /* Ctrl-h */
    case DEL_KEY:
        editorDelChar();
        break;
    case PAGE_UP:
    case PAGE_DOWN:
        if (c == PAGE_UP && E.cy != 0)
            E.cy = 0;
        else if (c == PAGE_DOWN && E.cy != E.screenrows-1)
            E.cy = E.screenrows-1;
        {
        int times = E.screenrows;
        while(times--)
            editorMoveCursor(c == PAGE_UP ? ARROW_UP:
                                            ARROW_DOWN);
        }
        break;

    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
        editorMoveCursor(c);
        break;
    case CTRL_L: /* ctrl+l, clear screen */
        /* Just refresht the line as side effect. */
        break;
    case ESC:
        /* Nothing to do for ESC in this mode. */
        break;
    default:
        editorInsertChar(c);
        break;
    }

    quit_times = XCODEX_QUIT_TIMES; /* Reset it to the original value. */
}

int editorFileWasModified(void) {
    return E.dirty;
}

void updateWindowSize(void) {
    if (getWindowSize(STDIN_FILENO,STDOUT_FILENO,
                    &E.screenrows,&E.screencols) == -1) {
        perror("Unable to query the screen for size (columns / rows)");
        exit(1);
    }
    E.screenrows -= 2; /* Get room for status bar. */
}

void handleSigWinCh(int unused __attribute__((unused))) {
    updateWindowSize();
    if (E.cy > E.screenrows) E.cy = E.screenrows - 1;
    int effective_screencols = E.screencols - E.line_numbers_width;
    if (E.cx > effective_screencols) E.cx = effective_screencols - 1;
    editorRefreshScreen();
}

void initEditor(void) {
    E.cx = 0;
    E.cy = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.syntax = NULL;
    E.show_line_numbers = 1;        /* Line numbers ON by default */
    E.line_numbers_width = 0;
    updateWindowSize();
    editorUpdateLineNumberWidth();  /* Initialize line number width */
    signal(SIGWINCH, handleSigWinCh);
    
    /* Set initial background color */
    editorSetBackgroundColor(themes[current_theme].bg_color);
}

int xcodex_main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr,"Usage: xcodex <filename>\n");
        return 1;
    }

    initEditor();
    editorSelectSyntaxHighlight(argv[1]);
    editorOpen(argv[1]);
    if (enableRawMode(STDIN_FILENO) == -1) return 1;
    editorSetStatusMessage(
        "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find | Ctrl-T = theme | Ctrl-N = line numbers");
    while(1) {
        editorRefreshScreen();
        editorProcessKeypress(STDIN_FILENO);
    }
    
    return 1;
}

#else

#include <stdio.h>

int XcodexMain(int argc, char **argv) {
    printf("XCodex editor is only available on POSIX systems.\n");
    return 1;
}

#endif
