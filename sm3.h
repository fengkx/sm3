#ifndef _sm3_H_
#define _sm3_H_

#include <stddef.h>
#include <stdio.h>

typedef unsigned char byte;
typedef unsigned int word;

byte *padding(const byte *src, size_t src_len,size_t content_len, size_t *out_len);

static word *expand_blk(const char *block, word **W_p);

void reverse_by_byte(byte * const le, size_t len);

void iv_init(word * const ivs);

static inline void CF(word * const iv, byte* blk, const word * const W, const word * const W_p);

byte *sm3(FILE *fp);

#endif
