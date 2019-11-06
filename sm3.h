#ifndef _sm3_H_
#define _sm3_H_

#include <stddef.h>

typedef unsigned char byte;
typedef unsigned int word;

byte *padding(const byte *src, size_t src_len, size_t *out_len);

word *expand_blk(const char *block, word **W_p);

void reverse_by_byte(byte * const le, size_t len);

void iv_init(word * const ivs);

void CF(word * const iv, byte* blk, const word * const W, const word * const W_p);

#endif