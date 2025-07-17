#include "syntax.h"
#include <stddef.h>



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

/* ========================= Markdown Syntax Highlighting ========================= */

char *Markdown_HL_extensions[] = {".md",".markdown",".mdown",".mkd",".mkdn",NULL};
char *Markdown_HL_keywords[] = {
    /* Markdown Headers (HL_KEYWORD1) */
    "#","##","###","####","#####","######",
    
    /* Markdown Emphasis and Formatting (HL_KEYWORD2) */
    "**|","__","*|","_|","`|","```|","~~|","==|",
    "***|","___","^|","~|",
    
    /* Markdown Code Blocks and Languages (HL_KEYWORD2) */
    "```c|","```cpp|","```python|","```javascript|","```typescript|",
    "```html|","```css|","```java|","```rust|","```go|","```php|",
    "```ruby|","```swift|","```kotlin|","```scala|","```shell|",
    "```bash|","```sql|","```json|","```xml|","```yaml|","```toml|",
    "```dockerfile|","```makefile|","```lua|","```perl|","```r|",
    "```matlab|","```latex|","```markdown|","```diff|","```vim|",
    
    /* Markdown Links, Images, and References (HL_KEYWORD3) */
    "http://||","https://||","ftp://||","mailto://||","file://||",
    "www.||","[||","]||","(||",")||","<||",">||","![||",
    
    /* Markdown Lists and Structure (HL_KEYWORD3) */
    "-||","+||","*||","1.||","2.||","3.||","4.||","5.||","6.||",
    "7.||","8.||","9.||","10.||",">||",">>||",">>>||",
    
    /* Markdown Tables and Separators (HL_KEYWORD3) */
    "||","---||","***||","___||",":::||","---|",":-:|",":--|","--:|",
    
    /* Markdown Task Lists (HL_KEYWORD3) */
    "- [ ]||","- [x]||","- [X]||","+ [ ]||","+ [x]||","+ [X]||",
    "* [ ]||","* [x]||","* [X]||",
    
    /* Markdown Footnotes and References (HL_KEYWORD3) */
    "[^||","^]||","[1]||","[2]||","[3]||","[4]||","[5]||",
    
    /* Markdown HTML Tags (HL_KEYWORD3) */
    "<br>||","<hr>||","<div>||","</div>||","<span>||","</span>||",
    "<p>||","</p>||","<pre>||","</pre>||","<code>||","</code>||",
    "<kbd>||","</kbd>||","<sub>||","</sub>||","<sup>||","</sup>||",
    "<mark>||","</mark>||","<del>||","</del>||","<ins>||","</ins>||",
    
    /* Markdown Metadata and Front Matter (HL_KEYWORD3) */
    "---||","+++||","title:||","author:||","date:||","tags:||",
    "categories:||","description:||","draft:||","slug:||","weight:||",
    "layout:||","template:||","permalink:||","published:||","updated:||",
    
    /* Common Markdown Keywords and Annotations (HL_KEYWORD3) */
    "TODO||","FIXME||","NOTE||","WARNING||","IMPORTANT||","DANGER||",
    "TIP||","INFO||","BUG||","HACK||","REVIEW||","OPTIMIZE||",
    "DEPRECATED||","REMOVED||","ADDED||","CHANGED||","SECURITY||",
    
    /* Mathematical Notation (HL_KEYWORD3) */
    "$$||","$||","\\(||","\\)||","\\[||","\\]||","\\begin||","\\end||",
    
    /* Markdown Extensions (HL_KEYWORD3) */
    ":::||","!!!||","???||","!!!note||","!!!warning||","!!!danger||",
    "!!!tip||","!!!info||","!!!example||","!!!quote||","!!!success||",
    "!!!failure||","!!!bug||","!!!abstract||","!!!summary||",
    
    /* Mermaid Diagrams (HL_KEYWORD3) */
    "```mermaid||","graph||","flowchart||","sequenceDiagram||","classDiagram||",
    "stateDiagram||","gantt||","pie||","journey||","gitgraph||",
    
    /* Common File Extensions in Code Blocks (HL_KEYWORD2) */
    "```txt|","```log|","```ini|","```conf|","```cfg|","```properties|",
    "```env|","```gitignore|","```gitattributes|","```editorconfig|",
    "```tsconfig|","```webpack|","```package|","```composer|","```gemfile|",
    "```requirements|","```poetry|","```cargo|","```gradle|","```maven|",
    
    /* Markdown Link Definitions (HL_KEYWORD3) */
    "[def]:||","[link]:||","[image]:||","[reference]:||","[anchor]:||",
    
    /* Admonition Types (HL_KEYWORD3) */
    "!!! note||","!!! warning||","!!! danger||","!!! tip||","!!! info||",
    "!!! example||","!!! quote||","!!! success||","!!! failure||",
    "!!! bug||","!!! abstract||","!!! summary||","!!! question||",
    
    NULL
};

/* ========================= LaTeX Syntax Highlighting ========================= */

char *LaTeX_HL_extensions[] = {".tex",".latex",".sty",".cls",".bib",NULL};
char *LaTeX_HL_keywords[] = {
    /* LaTeX Document Structure Commands (HL_KEYWORD1) */
    "\\documentclass","\\usepackage","\\begin","\\end","\\section",
    "\\subsection","\\subsubsection","\\paragraph","\\subparagraph",
    "\\chapter","\\part","\\title","\\author","\\date","\\thanks",
    "\\maketitle","\\tableofcontents","\\listoffigures","\\listoftables",
    "\\bibliography","\\bibliographystyle","\\cite","\\nocite","\\bibitem",
    "\\appendix","\\frontmatter","\\mainmatter","\\backmatter",
    "\\include","\\input","\\includeonly","\\includegraphics",
    
    /* LaTeX Page Layout Commands (HL_KEYWORD1) */
    "\\newpage","\\clearpage","\\cleardoublepage","\\pagebreak","\\nopagebreak",
    "\\newline","\\linebreak","\\nolinebreak","\\\\","\\hfill","\\vfill",
    "\\hspace","\\vspace","\\hskip","\\vskip","\\smallskip","\\medskip","\\bigskip",
    "\\indent","\\noindent","\\par","\\parskip","\\parindent","\\baselineskip",
    "\\linespread","\\onehalfspacing","\\doublespacing","\\singlespacing",
    "\\pagestyle","\\thispagestyle","\\pagenumbering","\\setcounter",
    "\\addtocounter","\\stepcounter","\\refstepcounter","\\value",
    
    /* LaTeX Environments (HL_KEYWORD2) */
    "document|","figure|","table|","equation|","align|","align*|","eqnarray|",
    "eqnarray*|","gather|","gather*|","multline|","multline*|","split|",
    "itemize|","enumerate|","description|","list|","trivlist|","quote|",
    "quotation|","verse|","verbatim|","verbatim*|","flushleft|","flushright|",
    "center|","abstract|","titlepage|","thebibliography|","theindex|",
    "minipage|","parbox|","picture|","tabbing|","tabular|","tabular*|",
    "array|","matrix|","pmatrix|","bmatrix|","vmatrix|","Vmatrix|",
    "cases|","proof|","theorem|","lemma|","corollary|","proposition|",
    "definition|","example|","remark|","note|","problem|","solution|",
    "algorithmic|","algorithm|","lstlisting|","minted|","longtable|",
    "sidewaystable|","sidewaysfigure|","multicols|","subequations|",
    "subfigure|","subtable|","tikzpicture|","pgfpicture|","forest|",
    
    /* LaTeX Math Commands and Symbols (HL_KEYWORD3) */
    "\\frac||","\\dfrac||","\\tfrac||","\\cfrac||","\\binom||","\\sqrt||",
    "\\sum||","\\prod||","\\int||","\\iint||","\\iiint||","\\oint||",
    "\\lim||","\\sup||","\\inf||","\\max||","\\min||","\\arg||",
    "\\sin||","\\cos||","\\tan||","\\sec||","\\csc||","\\cot||",
    "\\arcsin||","\\arccos||","\\arctan||","\\sinh||","\\cosh||","\\tanh||",
    "\\log||","\\lg||","\\ln||","\\exp||","\\det||","\\gcd||","\\deg||",
    "\\dim||","\\ker||","\\hom||","\\Pr||","\\mod||","\\pmod||","\\bmod||",
    
    /* Greek Letters */
    "\\alpha||","\\beta||","\\gamma||","\\delta||","\\epsilon||","\\varepsilon||",
    "\\zeta||","\\eta||","\\theta||","\\vartheta||","\\iota||","\\kappa||",
    "\\lambda||","\\mu||","\\nu||","\\xi||","\\pi||","\\varpi||","\\rho||",
    "\\varrho||","\\sigma||","\\varsigma||","\\tau||","\\upsilon||","\\phi||",
    "\\varphi||","\\chi||","\\psi||","\\omega||","\\Gamma||","\\Delta||",
    "\\Theta||","\\Lambda||","\\Xi||","\\Pi||","\\Sigma||","\\Upsilon||",
    "\\Phi||","\\Psi||","\\Omega||",
    
    /* Math Operators and Relations */
    "\\pm||","\\mp||","\\times||","\\div||","\\cdot||","\\bullet||",
    "\\circ||","\\ast||","\\star||","\\dagger||","\\ddagger||","\\amalg||",
    "\\cap||","\\cup||","\\uplus||","\\sqcap||","\\sqcup||","\\wedge||",
    "\\vee||","\\setminus||","\\wr||","\\diamond||","\\bigtriangleup||",
    "\\bigtriangledown||","\\triangleleft||","\\triangleright||","\\lhd||",
    "\\rhd||","\\unlhd||","\\unrhd||","\\oplus||","\\ominus||","\\otimes||",
    "\\oslash||","\\odot||","\\bigcirc||","\\bigodot||","\\bigoplus||",
    "\\bigotimes||","\\biguplus||","\\bigwedge||","\\bigvee||","\\bigcap||",
    "\\bigcup||","\\bigsqcup||",
    
    /* Relations */
    "\\leq||","\\geq||","\\equiv||","\\models||","\\prec||","\\succ||",
    "\\sim||","\\perp||","\\preceq||","\\succeq||","\\simeq||","\\mid||",
    "\\ll||","\\gg||","\\asymp||","\\parallel||","\\subset||","\\supset||",
    "\\approx||","\\bowtie||","\\subseteq||","\\supseteq||","\\cong||",
    "\\neq||","\\smile||","\\sqsubseteq||","\\sqsupseteq||","\\doteq||",
    "\\frown||","\\in||","\\ni||","\\propto||","\\vdash||","\\dashv||",
    "\\notin||","\\not||","\\exists||","\\nexists||","\\forall||",
    
    /* Arrows */
    "\\leftarrow||","\\rightarrow||","\\uparrow||","\\downarrow||",
    "\\leftrightarrow||","\\updownarrow||","\\Leftarrow||","\\Rightarrow||",
    "\\Uparrow||","\\Downarrow||","\\Leftrightarrow||","\\Updownarrow||",
    "\\mapsto||","\\longmapsto||","\\hookleftarrow||","\\hookrightarrow||",
    "\\leftharpoonup||","\\leftharpoondown||","\\rightharpoonup||",
    "\\rightharpoondown||","\\rightleftharpoons||","\\iff||","\\to||",
    "\\gets||","\\longleftarrow||","\\longrightarrow||","\\longleftrightarrow||",
    "\\Longleftarrow||","\\Longrightarrow||","\\Longleftrightarrow||",
    "\\nearrow||","\\searrow||","\\swarrow||","\\nwarrow||",
    
    /* Delimiters */
    "\\left||","\\right||","\\bigl||","\\bigr||","\\Bigl||","\\Bigr||",
    "\\biggl||","\\biggr||","\\Biggl||","\\Biggr||","\\langle||","\\rangle||",
    "\\lceil||","\\rceil||","\\lfloor||","\\rfloor||","\\ulcorner||",
    "\\urcorner||","\\llcorner||","\\lrcorner||",
    
    /* Math Fonts and Styles */
    "\\mathrm||","\\mathbf||","\\mathit||","\\mathsf||","\\mathtt||",
    "\\mathcal||","\\mathfrak||","\\mathbb||","\\boldsymbol||","\\pmb||",
    "\\text||","\\mbox||","\\hbox||","\\vbox||","\\makebox||","\\framebox||",
    "\\fbox||","\\boxed||","\\underline||","\\overline||","\\widehat||",
    "\\widetilde||","\\overleftarrow||","\\overrightarrow||","\\overleftrightarrow||",
    "\\underleftarrow||","\\underrightarrow||","\\underleftrightarrow||",
    "\\overbrace||","\\underbrace||","\\overset||","\\underset||",
    
    /* Spacing Commands */
    "\\quad||","\\qquad||","\\,||","\\:||","\\;||","\\!||","\\enspace||",
    "\\thinspace||","\\negthickspace||","\\negthinspace||","\\negmedspace||",
    "\\phantom||","\\vphantom||","\\hphantom||","\\smash||","\\mathstrut||",
    "\\strut||","\\rule||","\\hline||","\\cline||","\\vline||",
    
    /* Text Formatting Commands */
    "\\textbf||","\\textit||","\\textsl||","\\textsc||","\\texttt||",
    "\\textsf||","\\textrm||","\\textup||","\\textmd||","\\emph||",
    "\\em||","\\bf||","\\it||","\\sl||","\\sc||","\\tt||","\\sf||",
    "\\rm||","\\up||","\\md||","\\normalfont||","\\cal||","\\mit||",
    
    /* Font Size Commands */
    "\\tiny||","\\scriptsize||","\\footnotesize||","\\small||","\\normalsize||",
    "\\large||","\\Large||","\\LARGE||","\\huge||","\\Huge||",
    
    /* Alignment and Positioning */
    "\\centering||","\\raggedright||","\\raggedleft||","\\raggedleft||",
    "\\flushleft||","\\flushright||","\\center||","\\justify||",
    "\\hfil||","\\hfill||","\\hfilneg||","\\vfil||","\\vfill||","\\vfilneg||",
    "\\stretch||","\\dotfill||","\\hrulefill||","\\leaders||","\\cleaders||",
    
    /* Cross-references and Citations */
    "\\label||","\\ref||","\\pageref||","\\eqref||","\\autoref||",
    "\\nameref||","\\hyperref||","\\cite||","\\citep||","\\citet||",
    "\\citeauthor||","\\citeyear||","\\citealt||","\\citealp||",
    "\\citetext||","\\bibentry||","\\fullcite||","\\footcite||",
    "\\textcite||","\\parencite||","\\smartcite||","\\autocite||",
    
    /* Footnotes and Marginalia */
    "\\footnote||","\\footnotemark||","\\footnotetext||","\\marginpar||",
    "\\marginparpush||","\\marginnote||","\\sidenote||","\\sidenotemark||",
    "\\sidenotetext||","\\footnotesize||","\\footnoterule||",
    
    /* Lists and Numbering */
    "\\item||","\\setlength||","\\addtolength||","\\settowidth||",
    "\\settoheight||","\\settodepth||","\\usecounter||","\\newcounter||",
    "\\arabic||","\\roman||","\\Roman||","\\alph||","\\Alph||",
    "\\fnsymbol||","\\labelitemi||","\\labelitemii||","\\labelitemiii||",
    "\\labelitemiv||","\\labelenumi||","\\labelenumii||","\\labelenumiii||",
    "\\labelenumiv||","\\theenumi||","\\theenumii||","\\theenumiii||",
    "\\theenumiv||","\\theitemi||","\\theitemii||","\\theitemiii||",
    "\\theitemiv||",
    
    /* Boxes and Frames */
    "\\fbox||","\\framebox||","\\makebox||","\\parbox||","\\minipage||",
    "\\raisebox||","\\savebox||","\\sbox||","\\usebox||","\\newsavebox||",
    "\\colorbox||","\\fcolorbox||","\\shadowbox||","\\doublebox||",
    "\\ovalbox||","\\Ovalbox||","\\roundedbox||","\\dashedbox||",
    
    /* Tables and Arrays */
    "\\multicolumn||","\\multirow||","\\cline||","\\hline||","\\toprule||",
    "\\midrule||","\\bottomrule||","\\addlinespace||","\\morecmidrules||",
    "\\specialrule||","\\cmidrule||","\\arrayrulewidth||","\\arraycolsep||",
    "\\tabcolsep||","\\arraystretch||","\\extrarowheight||","\\belowrulesep||",
    "\\aboverulesep||","\\doublerulesep||","\\heavyrulewidth||",
    "\\lightrulewidth||","\\cmidrulewidth||","\\abovetopsep||","\\belowbottomsep||",
    
    /* Graphics and Figures */
    "\\includegraphics||","\\graphicspath||","\\DeclareGraphicsExtensions||",
    "\\rotatebox||","\\scalebox||","\\resizebox||","\\reflectbox||",
    "\\caption||","\\subcaption||","\\captionof||","\\listoffigures||",
    "\\listoftables||","\\newfloat||","\\floatname||","\\floatplacement||",
    "\\suppressfloats||","\\clearpage||","\\afterpage||","\\FloatBarrier||",
    
    /* Colors */
    "\\color||","\\textcolor||","\\colorbox||","\\fcolorbox||",
    "\\pagecolor||","\\definecolor||","\\colorlet||","\\xcolor||",
    
    /* Hyperlinks and URLs */
    "\\href||","\\url||","\\hyperlink||","\\hypertarget||","\\hypersetup||",
    "\\autoref||","\\nameref||","\\hyperref||","\\nolinkurl||",
    
    /* Special Characters and Symbols */
    "\\LaTeX||","\\TeX||","\\&||","\\%||","\\#||","\\$||","\\{||","\\}||",
    "\\textbackslash||","\\textasciicircum||","\\textasciitilde||",
    "\\textbar||","\\textless||","\\textgreater||","\\textunderscore||",
    "\\textregistered||","\\texttrademark||","\\textcopyright||",
    "\\textdegree||","\\textcelsius||","\\textmu||","\\textohm||",
    "\\texteuro||","\\textsterling||","\\textyen||","\\textcent||",
    "\\textflorin||","\\textpound||","\\textdollar||","\\textquotedblleft||",
    "\\textquotedblright||","\\textquoteleft||","\\textquoteright||",
    "\\guillemotleft||","\\guillemotright||","\\quotedblbase||","\\quotesinglbase||",
    "\\textellipsis||","\\textendash||","\\textemdash||","\\textvisiblespace||",
    
    /* Math Accents and Decorations */
    "\\hat||","\\check||","\\breve||","\\acute||","\\grave||","\\tilde||",
    "\\bar||","\\vec||","\\dot||","\\ddot||","\\dddot||","\\ddddot||",
    "\\mathring||","\\widehat||","\\widetilde||","\\widecheck||",
    "\\widebreve||","\\widebar||","\\widevec||",
    
    /* Theorem-like Environments */
    "\\newtheorem||","\\theoremstyle||","\\newtheoremstyle||","\\qed||",
    "\\qedsymbol||","\\proof||","\\endproof||","\\proofname||",
    
    /* Special Math Constants */
    "\\infty||","\\partial||","\\nabla||","\\Box||","\\Diamond||",
    "\\triangle||","\\angle||","\\measuredangle||","\\sphericalangle||",
    "\\top||","\\bot||","\\flat||","\\natural||","\\sharp||",
    "\\clubsuit||","\\diamondsuit||","\\heartsuit||","\\spadesuit||",
    "\\Re||","\\Im||","\\wp||","\\ell||","\\hbar||","\\imath||","\\jmath||",
    "\\aleph||","\\beth||","\\gimel||","\\daleth||","\\backslash||",
    "\\prime||","\\emptyset||","\\varnothing||","\\complement||",
    
    /* Units and Measurements (if using siunitx) */
    "\\si||","\\SI||","\\num||","\\ang||","\\unit||","\\qty||",
    "\\qtyrange||","\\qtylist||","\\numrange||","\\numlist||",
    "\\sisetup||","\\DeclareSIUnit||","\\DeclareSIPrefix||",
    
    /* Chemistry (if using mhchem) */
    "\\ce||","\\cee||","\\cf||","\\cfe||","\\isotope||",
    
    /* Algorithms and Code */
    "\\algorithm||","\\algstore||","\\algrestore||","\\algorithmic||",
    "\\State||","\\Statex||","\\Require||","\\Ensure||","\\While||",
    "\\EndWhile||","\\For||","\\EndFor||","\\If||","\\ElsIf||","\\Else||",
    "\\EndIf||","\\Function||","\\EndFunction||","\\Procedure||",
    "\\EndProcedure||","\\Call||","\\Comment||","\\Return||",
    "\\lstset||","\\lstinline||","\\lstinputlisting||","\\lstlistoflistings||",
    "\\mintinline||","\\inputminted||","\\usemintedstyle||",
    
    /* Bibliography and Index */
    "\\printbibliography||","\\addbibresource||","\\bibliography||",
    "\\bibliographystyle||","\\thebibliography||","\\bibitem||",
    "\\newblock||","\\index||","\\makeindex||","\\printindex||",
    "\\indexentry||","\\see||","\\seealso||",
    
    /* Miscellaneous Commands */
    "\\newcommand||","\\renewcommand||","\\providecommand||","\\def||",
    "\\newenvironment||","\\renewenvironment||","\\newlength||",
    "\\newdimen||","\\newskip||","\\newmuskip||","\\newbox||",
    "\\newtoks||","\\newread||","\\newwrite||","\\newcount||",
    "\\newfam||","\\newif||","\\csname||","\\endcsname||",
    "\\expandafter||","\\noexpand||","\\protect||","\\string||",
    "\\meaning||","\\the||","\\number||","\\romannumeral||",
    "\\jobname||","\\today||","\\TeX||","\\LaTeX||","\\LaTeXe||",
    
    NULL
};

/* JSON */
char *JSON_HL_extensions[] = {".json",".jsonl",".geojson",".topojson",NULL};
char *JSON_HL_keywords[] = {
    /* JSON Keywords (HL_KEYWORD1) */
    "true","false","null",
    
    /* JSON Structure Keywords (HL_KEYWORD2) */
    "object|","array|","string|","number|","boolean|",
    
    /* Common JSON Properties (HL_KEYWORD3) */
    "\"name\"||","\"id\"||","\"type\"||","\"value\"||","\"data\"||",
    "\"config\"||","\"settings\"||","\"options\"||","\"properties\"||",
    "\"items\"||","\"list\"||","\"array\"||","\"object\"||",
    "\"title\"||","\"description\"||","\"url\"||","\"path\"||",
    "\"version\"||","\"author\"||","\"license\"||","\"dependencies\"||",
    "\"devDependencies\"||","\"scripts\"||","\"main\"||","\"module\"||",
    "\"exports\"||","\"imports\"||","\"repository\"||","\"homepage\"||",
    "\"bugs\"||","\"keywords\"||","\"files\"||","\"bin\"||",
    "\"engines\"||","\"os\"||","\"cpu\"||","\"private\"||",
    "\"workspaces\"||","\"peerDependencies\"||","\"optionalDependencies\"||",
    
    /* Common Data Properties */
    "\"status\"||","\"state\"||","\"code\"||","\"message\"||","\"error\"||",
    "\"success\"||","\"result\"||","\"response\"||","\"request\"||",
    "\"payload\"||","\"body\"||","\"headers\"||","\"method\"||",
    "\"timestamp\"||","\"date\"||","\"time\"||","\"created\"||","\"updated\"||",
    "\"modified\"||","\"expires\"||","\"ttl\"||","\"duration\"||",
    
    /* User/Auth Properties */
    "\"user\"||","\"username\"||","\"email\"||","\"password\"||",
    "\"token\"||","\"apiKey\"||","\"secret\"||","\"key\"||",
    "\"auth\"||","\"authorization\"||","\"session\"||","\"cookie\"||",
    "\"role\"||","\"permissions\"||","\"scope\"||","\"access\"||",
    
    /* Database/API Properties */
    "\"database\"||","\"collection\"||","\"table\"||","\"schema\"||",
    "\"query\"||","\"filter\"||","\"sort\"||","\"limit\"||","\"offset\"||",
    "\"page\"||","\"pageSize\"||","\"total\"||","\"count\"||",
    "\"index\"||","\"primary\"||","\"foreign\"||","\"reference\"||",
    
    /* Configuration Properties */
    "\"host\"||","\"port\"||","\"protocol\"||","\"domain\"||",
    "\"baseUrl\"||","\"endpoint\"||","\"api\"||","\"service\"||",
    "\"timeout\"||","\"retries\"||","\"interval\"||","\"delay\"||",
    "\"ssl\"||","\"tls\"||","\"certificate\"||","\"secure\"||",
    
    /* File/Resource Properties */
    "\"file\"||","\"filename\"||","\"extension\"||","\"size\"||",
    "\"format\"||","\"encoding\"||","\"charset\"||","\"mime\"||",
    "\"content\"||","\"text\"||","\"html\"||","\"json\"||","\"xml\"||",
    "\"binary\"||","\"base64\"||","\"hash\"||","\"checksum\"||",
    
    /* Location/Geo Properties */
    "\"location\"||","\"address\"||","\"city\"||","\"country\"||",
    "\"latitude\"||","\"longitude\"||","\"coordinates\"||","\"region\"||",
    "\"timezone\"||","\"locale\"||","\"language\"||","\"currency\"||",
    
    /* UI/Display Properties */
    "\"label\"||","\"caption\"||","\"placeholder\"||","\"tooltip\"||",
    "\"icon\"||","\"image\"||","\"src\"||","\"alt\"||","\"width\"||",
    "\"height\"||","\"color\"||","\"background\"||","\"style\"||",
    "\"class\"||","\"className\"||","\"disabled\"||","\"hidden\"||",
    "\"visible\"||","\"active\"||","\"selected\"||","\"checked\"||",
    
    /* Metadata Properties */
    "\"meta\"||","\"metadata\"||","\"info\"||","\"details\"||",
    "\"tags\"||","\"categories\"||","\"classification\"||","\"priority\"||",
    "\"weight\"||","\"order\"||","\"position\"||","\"level\"||",
    "\"parent\"||","\"children\"||","\"siblings\"||","\"hierarchy\"||",
    
    /* Event/Action Properties */
    "\"event\"||","\"action\"||","\"trigger\"||","\"handler\"||",
    "\"callback\"||","\"listener\"||","\"observer\"||","\"subscriber\"||",
    "\"publisher\"||","\"channel\"||","\"topic\"||","\"queue\"||",
    
    /* Validation Properties */
    "\"required\"||","\"optional\"||","\"default\"||","\"min\"||","\"max\"||",
    "\"minLength\"||","\"maxLength\"||","\"pattern\"||","\"regex\"||",
    "\"format\"||","\"enum\"||","\"const\"||","\"examples\"||",
    
    /* Schema Properties (JSON Schema) */
    "\"$schema\"||","\"$id\"||","\"$ref\"||","\"$defs\"||",
    "\"definitions\"||","\"properties\"||","\"items\"||","\"additionalProperties\"||",
    "\"patternProperties\"||","\"allOf\"||","\"anyOf\"||","\"oneOf\"||",
    "\"not\"||","\"if\"||","\"then\"||","\"else\"||","\"contains\"||",
    "\"minItems\"||","\"maxItems\"||","\"uniqueItems\"||","\"minProperties\"||",
    "\"maxProperties\"||","\"multipleOf\"||","\"minimum\"||","\"maximum\"||",
    "\"exclusiveMinimum\"||","\"exclusiveMaximum\"||","\"contentMediaType\"||",
    "\"contentEncoding\"||","\"readOnly\"||","\"writeOnly\"||","\"deprecated\"||",
    
    /* Package.json Specific */
    "\"publishConfig\"||","\"bundledDependencies\"||","\"preferGlobal\"||",
    "\"man\"||","\"directories\"||","\"contributors\"||","\"funding\"||",
    "\"browserslist\"||","\"sideEffects\"||","\"types\"||","\"typings\"||",
    "\"module\"||","\"browser\"||","\"esnext\"||","\"esm\"||",
    
    /* Common Status Values */
    "\"ok\"||","\"error\"||","\"pending\"||","\"loading\"||","\"complete\"||",
    "\"success\"||","\"failure\"||","\"warning\"||","\"info\"||",
    "\"active\"||","\"inactive\"||","\"enabled\"||","\"disabled\"||",
    "\"public\"||","\"private\"||","\"draft\"||","\"published\"||",
    
    /* HTTP/REST Properties */
    "\"GET\"||","\"POST\"||","\"PUT\"||","\"DELETE\"||","\"PATCH\"||",
    "\"HEAD\"||","\"OPTIONS\"||","\"CONNECT\"||","\"TRACE\"||",
    "\"statusCode\"||","\"statusText\"||","\"contentType\"||",
    "\"userAgent\"||","\"origin\"||","\"referer\"||","\"accept\"||",
    
    /* Common Numeric/Boolean Values */
    "\"0\"||","\"1\"||","\"-1\"||","\"true\"||","\"false\"||",
    "\"yes\"||","\"no\"||","\"on\"||","\"off\"||","\"null\"||",
    
    NULL
};

/* CSV */
char *CSV_HL_extensions[] = {".csv",".tsv",".dsv",".psv",NULL};
char *CSV_HL_keywords[] = {
    /* CSV Common Headers (HL_KEYWORD1) */
    "id","name","title","description","type","value","data","date",
    "time","timestamp","created","updated","modified","status","state",
    "category","tag","label","group","class","priority","level",
    "order","index","position","rank","score","rating","count",
    "total","sum","average","min","max","median","std","variance",
    "amount","quantity","price","cost","revenue","profit","loss",
    "weight","height","width","length","size","volume","area",
    "first_name","last_name","full_name","email","phone","address",
    "city","country","state","region","zip","postal_code","coordinates",
    "latitude","longitude","location","street","building","floor",
    "department","company","organization","team","role","position",
    "age","gender","birth_date","nationality","language","currency",
    "product","service","item","sku","barcode","model","brand",
    "version","revision","edition","series","batch","lot","serial",
    "user","customer","client","vendor","supplier","partner",
    "reference","code","key","token","session","auth","permission",
    "start_date","end_date","due_date","expiry","duration","period",
    "frequency","interval","schedule","timezone","locale","format",
    "source","target","destination","origin","path","url","link",
    "file","filename","extension","size","format","encoding","checksum",
    "error","warning","message","note","comment","remark","feedback",
    "public","private","internal","external","visible","hidden",
    "required","optional","mandatory","default","custom","standard",
    "approved","rejected","pending","completed","cancelled","archived",
    "parent","child","sibling","ancestor","descendant","relation",
    "previous","next","current","latest","oldest","newest","recent",
    "primary","secondary","tertiary","main","sub","auxiliary","backup",
    "header","footer","body","content","text","html","json","xml",
    "config","settings","options","preferences","parameters","attributes",
    "meta","metadata","info","details","summary","abstract","keywords",
    "tags","categories","classification","taxonomy","hierarchy","tree",
    "event","action","trigger","handler","callback","response","request",
    "input","output","result","outcome","feedback","review","audit",
    "log","trace","debug","info","warn","error","fatal","critical",
    "performance","benchmark","metric","kpi","indicator","measure",
    "feature","functionality","capability","specification","requirement",
    "test","validation","verification","quality","compliance","standard",
    "invoice","receipt","payment","transaction","transfer","refund",
    "discount","tax","fee","charge","commission","bonus","penalty",
    "stock","inventory","supply","demand","capacity","availability",
    "sales","marketing","advertising","campaign","promotion","offer",
    "contract","agreement","terms","conditions","policy","procedure",
    "asset","liability","equity","capital","investment","portfolio",
    "budget","forecast","projection","estimate","actual","variance",
    "milestone","deliverable","task","subtask","phase","stage","step",
    "workflow","process","procedure","method","technique","approach",
    "issue","bug","defect","incident","problem","solution","fix",
    "change","update","upgrade","patch","hotfix","release","deployment",
    "backup","restore","archive","compress","encrypt","decrypt","secure",
    "license","copyright","trademark","patent","intellectual_property",
    "training","education","certification","qualification","skill","expertise",
    "report","dashboard","chart","graph","visualization","analytics",
    "api","endpoint","service","microservice","component","module",
    "database","table","column","row","record","field","schema",
    "query","filter","sort","search","index","cache","session",
    "network","server","client","host","port","protocol","connection",
    "security","authentication","authorization","access","privilege","role",
    "monitor","alert","notification","warning","alarm","threshold",
    "health","status","uptime","downtime","availability","reliability",
    "sync","async","real_time","batch","streaming","queue","buffer",
    "import","export","migrate","transform","convert","parse","validate",
    "template","pattern","regex","format","syntax","grammar","rule",
    "theme","style","layout","design","ui","ux","interface","display",
    "mobile","desktop","tablet","browser","device","platform","os",
    "social","profile","avatar","bio","about","contact","follow",
    "like","share","comment","rate","review","subscribe","unsubscribe",
    "shopping","cart","checkout","order","shipping","delivery","tracking",
    "medical","patient","doctor","diagnosis","treatment","prescription",
    "education","student","teacher","course","grade","exam","assignment",
    "legal","law","regulation","compliance","court","judge","lawyer",
    
    /* CSV Data Types (HL_KEYWORD2) */
    "string|","text|","varchar|","char|","nvarchar|","nchar|","clob|",
    "number|","numeric|","decimal|","float|","double|","real|","money|",
    "integer|","int|","smallint|","bigint|","tinyint|","mediumint|",
    "boolean|","bool|","bit|","binary|","varbinary|","blob|","longblob|",
    "date|","datetime|","timestamp|","time|","year|","interval|",
    "json|","jsonb|","xml|","array|","object|","record|","row|",
    "uuid|","guid|","enum|","set|","list|","map|","dictionary|",
    "currency|","percentage|","ratio|","fraction|","coordinate|",
    "url|","uri|","email|","phone|","fax|","mobile|","landline|",
    "ip|","ipv4|","ipv6|","mac|","hostname|","domain|","subdomain|",
    "address|","street|","city|","state|","country|","zip|","postal|",
    "latitude|","longitude|","geolocation|","geopoint|","polygon|",
    "html|","markdown|","rtf|","pdf|","doc|","docx|","xls|","xlsx|",
    "image|","video|","audio|","file|","document|","archive|","compressed|",
    "base64|","hex|","binary|","encoded|","encrypted|","hashed|","signed|",
    "reference|","foreign_key|","primary_key|","unique|","index|","constraint|",
    "nullable|","required|","optional|","default|","auto_increment|","serial|",
    "version|","revision|","etag|","checksum|","crc|","md5|","sha1|","sha256|",
    "mimetype|","contenttype|","encoding|","charset|","format|","extension|",
    "locale|","language|","timezone|","currency_code|","country_code|",
    "status_code|","error_code|","response_code|","http_status|","exit_code|",
    "priority|","severity|","level|","rank|","score|","rating|","weight|",
    "count|","quantity|","amount|","total|","sum|","average|","median|",
    "minimum|","maximum|","range|","variance|","deviation|","percentile|",
    "rate|","frequency|","interval|","duration|","timeout|","delay|",
    "capacity|","limit|","threshold|","quota|","budget|","allocation|",
    "percentage|","ratio|","proportion|","factor|","multiplier|","coefficient|",
    "coordinate|","vector|","matrix|","point|","line|","polygon|","circle|",
    "color|","rgb|","rgba|","hex|","hsl|","hsla|","cmyk|","pantone|",
    "font|","size|","weight|","style|","family|","variant|","decoration|",
    "width|","height|","depth|","length|","area|","volume|","mass|",
    "temperature|","pressure|","humidity|","speed|","acceleration|","force|",
    "energy|","power|","voltage|","current|","resistance|","frequency|",
    "ph|","concentration|","density|","viscosity|","conductivity|","salinity|",
    "social_security|","tax_id|","passport|","license|","permit|","certificate|",
    "credit_card|","bank_account|","routing|","iban|","swift|","bic|",
    "barcode|","qr_code|","ean|","upc|","isbn|","issn|","doi|",
    "mac_address|","serial_number|","model_number|","part_number|","lot_number|",
    "user_agent|","session_id|","transaction_id|","order_id|","invoice_id|",
    "thread_id|","process_id|","job_id|","task_id|","request_id|","trace_id|",
    
    /* CSV Common Values (HL_KEYWORD3) */
    "true||","false||","yes||","no||","on||","off||","1||","0||",
    "enabled||","disabled||","active||","inactive||","valid||","invalid||",
    "open||","closed||","locked||","unlocked||","public||","private||",
    "visible||","hidden||","shown||","collapsed||","expanded||","minimized||",
    "available||","unavailable||","online||","offline||","connected||","disconnected||",
    "running||","stopped||","paused||","waiting||","ready||","busy||",
    "success||","failure||","error||","warning||","info||","debug||",
    "ok||","fail||","pass||","skip||","ignore||","cancel||","abort||",
    "approved||","rejected||","pending||","reviewing||","draft||","published||",
    "new||","old||","current||","previous||","next||","latest||","first||","last||",
    "high||","medium||","low||","urgent||","normal||","critical||","minor||",
    "ascending||","descending||","asc||","desc||","up||","down||","forward||","backward||",
    "left||","right||","center||","top||","bottom||","middle||","start||","end||",
    "null||","empty||","undefined||","missing||","unknown||","tbd||","tba||",
    "n/a||","N/A||","na||","NA||","none||","None||","NONE||","nil||","NIL||",
    "null||","NULL||","void||","VOID||","blank||","BLANK||","default||","DEFAULT||",
    "auto||","manual||","automatic||","custom||","standard||","default||","preset||",
    "internal||","external||","public||","private||","protected||","restricted||",
    "local||","remote||","global||","regional||","national||","international||",
    "temporary||","permanent||","volatile||","persistent||","cached||","stored||",
    "compressed||","uncompressed||","encrypted||","decrypted||","encoded||","decoded||",
    "signed||","unsigned||","verified||","unverified||","authenticated||","anonymous||",
    "male||","female||","other||","unknown||","prefer_not_to_say||",
    "single||","married||","divorced||","widowed||","separated||","engaged||",
    "student||","employed||","unemployed||","retired||","self_employed||",
    "beginner||","intermediate||","advanced||","expert||","master||","novice||",
    "free||","paid||","premium||","basic||","standard||","professional||","enterprise||",
    "trial||","subscription||","one_time||","recurring||","lifetime||","limited||",
    "bronze||","silver||","gold||","platinum||","diamond||","vip||","premium||",
    "small||","medium||","large||","extra_large||","xs||","s||","m||","l||","xl||","xxl||",
    "monday||","tuesday||","wednesday||","thursday||","friday||","saturday||","sunday||",
    "january||","february||","march||","april||","may||","june||",
    "july||","august||","september||","october||","november||","december||",
    "am||","pm||","utc||","gmt||","est||","pst||","cst||","mst||",
    "get||","post||","put||","delete||","patch||","head||","options||","trace||",
    "http||","https||","ftp||","ftps||","sftp||","ssh||","telnet||","smtp||",
    "tcp||","udp||","icmp||","dns||","dhcp||","ntp||","snmp||","ldap||",
    "json||","xml||","yaml||","csv||","tsv||","html||","text||","binary||",
    "utf8||","utf16||","ascii||","latin1||","iso||","cp1252||","base64||",
    "gzip||","zip||","tar||","rar||","7z||","bz2||","xz||","lz4||",
    "jpg||","jpeg||","png||","gif||","bmp||","tiff||","svg||","webp||",
    "mp3||","wav||","flac||","ogg||","m4a||","wma||","aac||","opus||",
    "mp4||","avi||","mkv||","mov||","wmv||","flv||","webm||","m4v||",
    "pdf||","doc||","docx||","xls||","xlsx||","ppt||","pptx||","txt||",
    "red||","green||","blue||","yellow||","orange||","purple||","pink||",
    "black||","white||","gray||","grey||","brown||","cyan||","magenta||",
    "transparent||","opaque||","solid||","dashed||","dotted||","double||",
    "bold||","italic||","underline||","strikethrough||","normal||","lighter||",
    "left||","right||","center||","justify||","top||","bottom||","middle||",
    "portrait||","landscape||","square||","circle||","rectangle||","triangle||",
    "solid||","liquid||","gas||","plasma||","frozen||","melted||","boiled||",
    "celsius||","fahrenheit||","kelvin||","meter||","kilometer||","mile||",
    "gram||","kilogram||","pound||","ounce||","liter||","gallon||","quart||",
    "second||","minute||","hour||","day||","week||","month||","year||",
    "usd||","eur||","gbp||","jpy||","cad||","aud||","chf||","cny||",
    "bitcoin||","ethereum||","litecoin||","dogecoin||","ada||","xrp||",
    "mastercard||","visa||","amex||","discover||","paypal||","stripe||",
    "windows||","mac||","linux||","ios||","android||","ubuntu||","debian||",
    "chrome||","firefox||","safari||","edge||","opera||","ie||","brave||",
    "mobile||","desktop||","tablet||","laptop||","server||","embedded||",
    "development||","staging||","production||","testing||","beta||","alpha||",
    "frontend||","backend||","fullstack||","api||","database||","middleware||",
    "agile||","waterfall||","scrum||","kanban||","devops||","cicd||","git||",
    "created||","updated||","deleted||","archived||","restored||","migrated||",
    "imported||","exported||","synced||","backed_up||","deployed||","rolled_back||",
    "scheduled||","queued||","processing||","completed||","failed||","timeout||",
    "urgent||","high||","medium||","low||","critical||","major||","minor||",
    "blocker||","feature||","bug||","enhancement||","task||","story||","epic||",
    "todo||","in_progress||","testing||","review||","done||","closed||","reopened||",
    "planned||","unplanned||","expected||","unexpected||","known||","unknown||",
    "internal||","external||","customer||","vendor||","partner||","competitor||",
    "profit||","loss||","revenue||","expense||","investment||","dividend||",
    "asset||","liability||","equity||","debt||","credit||","debit||","balance||",
    "buy||","sell||","hold||","trade||","invest||","withdraw||","deposit||",
    "order||","cancel||","refund||","return||","exchange||","warranty||",
    "shipping||","delivery||","pickup||","express||","standard||","economy||",
    "in_stock||","out_of_stock||","backordered||","discontinued||","clearance||",
    "new||","used||","refurbished||","damaged||","returned||","defective||",
    "excellent||","good||","fair||","poor||","damaged||","broken||","working||",
    "guaranteed||","warranty||","as_is||","final_sale||","returnable||",
    "wholesale||","retail||","bulk||","individual||","sample||","demo||",
    "medical||","dental||","vision||","mental||","physical||","therapy||",
    "prescription||","over_counter||","generic||","brand||","dosage||","frequency||",
    "allergic||","intolerant||","sensitive||","immune||","resistant||","susceptible||",
    "positive||","negative||","normal||","abnormal||","healthy||","unhealthy||",
    "acute||","chronic||","mild||","moderate||","severe||","critical||","stable||",
    "diagnosed||","undiagnosed||","treated||","untreated||","cured||","terminal||",
    "inpatient||","outpatient||","emergency||","elective||","routine||","urgent||",
    
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
        "#","\"\"\"","\"\"\"",
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
    },
    {
        /* Markdown */
        Markdown_HL_extensions,
        Markdown_HL_keywords,
        "","<!--","-->",
        HL_HIGHLIGHT_STRINGS
    },
    {
        /* LaTeX */
        LaTeX_HL_extensions,
        LaTeX_HL_keywords,
        "%","","",
        HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
    },
    {
        /* JSON */
        JSON_HL_extensions,
        JSON_HL_keywords,
        "","","",
        HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
    },
    {
        /* CSV */
        CSV_HL_extensions,
        CSV_HL_keywords,
        "#","","",
        HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
    }
};

const int HLDB_ENTRIES = sizeof(HLDB) / sizeof(HLDB[0]);