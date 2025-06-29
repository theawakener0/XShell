#ifndef XCODEX_H
#define XCODEX_H

/* 
 * XCodex - Simple text editor for XShell
 * POSIX systems only
 */

#if !defined(_WIN32) && !defined(_WIN64)
#define XCODEX_ENABLED 1
#endif

#ifdef XCODEX_ENABLED
/* Full editor functionality */
int xcodex_main(int argc, char **argv);
#else
/* Stub for Windows */
int XcodexMain(int argc, char **argv);
#endif

#endif /* XCODEX_H */