#ifndef XCRYPT_H
#define XCRYPT_H

#include "xsh.h"

/**
 * @brief Encrypts or decrypts a file using a simple password-based XOR cipher.
 * 
 * @param args Null-terminated array of strings. 
 *             Expected format: xcrypt <encrypt|decrypt> <input_file> <output_file>
 * @return int Returns 1 to continue shell execution.
 */
int xsh_xcrypt(char **args);

#endif // XCRYPT_H