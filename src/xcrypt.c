#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xcrypt.h"
#include "xsh.h"

// Platform-specific includes for get_password
#ifdef _WIN32
#include <windows.h>
#else // POSIX
#include <unistd.h>
#include <termios.h>
#endif

#define XOR_BUFFER_SIZE 4096 // Buffer size for file processing

// Forward Declarations
static void get_password(const char *prompt, char *password, size_t size);
static int process_file(const char *in_path, const char *out_path, const unsigned char *password);
static void print_xcrypt_usage(void);

/**
 * @brief Main function for the 'xcrypt' built-in command.
 */
int xsh_xcrypt(char **args) {
    if (args[1] == NULL || args[2] == NULL || args[3] == NULL || (strcmp(args[1], "encrypt") != 0 && strcmp(args[1], "decrypt") != 0)) {
        print_xcrypt_usage();
        return 1;
    }

    const char *mode = args[1];
    const char *input_file = args[2];
    const char *output_file = args[3];
    
    char password[XSH_MAXLINE];
    char password_confirm[XSH_MAXLINE];

    get_password("Enter password: ", password, sizeof(password));

    if (strlen(password) == 0) {
        fprintf(stderr, "xsh: xcrypt: password cannot be empty.\n");
        return 1;
    }

    get_password("Confirm password: ", password_confirm, sizeof(password_confirm));
    if (strcmp(password, password_confirm) != 0) {
        fprintf(stderr, "xsh: xcrypt: passwords do not match.\n");
        memset(password, 0, sizeof(password)); // Clear password for security
        memset(password_confirm, 0, sizeof(password_confirm)); // Clear confirm password
        return 1;
    }

    memset(password_confirm, 0, sizeof(password_confirm)); // Clear confirm password

    if (process_file(input_file, output_file, (unsigned char*)password) == 0) {
        printf("Successfully %sed '%s' to '%s'.\n", mode, input_file, output_file);
    }

    memset(password, 0, sizeof(password)); // Clear password for security

    return 1;
}

static void print_xcrypt_usage(void) {
    fprintf(stderr, "xcrypt: Simple file encryption/decryption tool.\n");
    fprintf(stderr, "Usage: xcrypt <encrypt|decrypt> <input_file> <output_file>\n");
    fprintf(stderr, "Note: 'encrypt' and 'decrypt' use the same symmetric XOR operation.\n");
}

/**
 * @brief Processes a file using a simple password-based XOR cipher.
 * @return 0 on success, -1 on failure.
 */
static int process_file(const char *in_path, const char *out_path, const unsigned char *password) {
    FILE *in_file = fopen(in_path, "rb");
    if (!in_file) {
        fprintf(stderr, "xsh: xcrypt: cannot open input file '%s': ", in_path);
        perror("");
        return -1;
    }

    FILE *out_file = fopen(out_path, "wb");
    if (!out_file) {
        fprintf(stderr, "xsh: xcrypt: cannot open output file '%s': ", out_path);
        perror("");
        fclose(in_file);
        return -1;
    }

    size_t key_len = strlen((const char *)password);
    if (key_len == 0) {
        fprintf(stderr, "xsh: xcrypt: password cannot be empty.\n");
        fclose(in_file);
        fclose(out_file);
        return -1;
    }

    unsigned char buffer[XOR_BUFFER_SIZE];
    size_t bytes_read;
    size_t key_index = 0;

    while ((bytes_read = fread(buffer, 1, XOR_BUFFER_SIZE, in_file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            buffer[i] ^= password[key_index % key_len];
            key_index++;
        }
        if (fwrite(buffer, 1, bytes_read, out_file) != bytes_read) {
            fprintf(stderr, "xsh: xcrypt: error writing to output file '%s': ", out_path);
            perror("");
            fclose(in_file);
            fclose(out_file);
            return -1;
        }
    }

    if (ferror(in_file)) {
        fprintf(stderr, "xsh: xcrypt: error reading input file '%s': ", in_path);
        perror("");
        fclose(in_file);
        fclose(out_file);
        return -1;
    }

    fclose(in_file);
    fclose(out_file);
    return 0;
}

/**
 * @brief Securely reads a password from the console without echoing.
 */
static void get_password(const char *prompt, char *password, size_t size) {
    printf("%s", prompt);
    fflush(stdout);
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif

    if (fgets(password, size, stdin) != NULL) {
        password[strcspn(password, "\r\n")] = 0; // Remove trailing newline
    } else {
        password[0] = '\0';
    }

#ifdef _WIN32
    SetConsoleMode(hStdin, mode);
#else
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    printf("\n");
}