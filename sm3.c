#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "sm3.h"

static const int endianTest = 0x12345678;
#define isLittleEndian ((* (char *)&endianTest) ==  0x78)

#define ls_w(a, n) (a >> (32 - n) | a << n)
#define rs_w(a, n) (a << (32 - n) | a >> n)


byte *
sm3(FILE *fp) {
	long sz, nread=0, nremain;
	byte buf[64];
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	rewind(fp);
	nremain = sz;

	// size_t len;
	// byte *padding_s = padding((byte *)s, strlen(s), &len);
	word *W_p, *W;
	word *iv = malloc(sizeof(word) * 8);
	iv_init(iv);

	int n=0;
	while(nremain>=64) {
		n = fread(buf, 1, 64, fp);
		nremain-=n;
		nread+=n;
		W = expand_blk(buf, &W_p);
		CF(iv, buf, W, W_p);
	}

	// if(nremain >0) {
		n = fread(buf, 1, 64, fp);
		nremain -= n;
		nread += n;
		
	// }
	size_t len;
	byte *padding_s = padding(buf, n, nread, &len);
	W = expand_blk(padding_s, &W_p);
	CF(iv, padding_s, W, W_p);

	// for (int i=0;i<len>>6;i++) {
	// 	W = expand_blk(&padding_s[i*64], &W_p);
	// 	CF(iv, &padding_s[i*64], W, W_p);
	// }

	// time to free some memory
	free(padding_s);
	free(W);
	free(W_p);
	word *final_result = malloc(sizeof(word) * 8);
	for (int i=0;i<8;i++) {
		memcpy(&final_result[i], &iv[i], sizeof(word));
		if(isLittleEndian) {
			reverse_by_byte((byte *)&final_result[i], 4);
		}
	}
	free(iv);
	return (char *)final_result;
}


void iv_init(word * const iv) {
	iv[0] = 0x7380166f;
	iv[1] = 0x4914b2b9; 
	iv[2] = 0x172442d7;
	iv[3] = 0xda8a0600;
	iv[4] = 0xa96f30bc;
	iv[5] = 0x163138aa;
	iv[6] = 0xe38dee4d;
	iv[7] = 0xb0fb0e4e;
}



/**
 * reverse_by_byte convert a little endian  array of byte into bigendian 
 * convert happens in place
 * parameter le is the byte array, len is the size of byte array
 */
void reverse_by_byte(byte * const le, size_t len) {
	for (size_t i=0;i<len>>1;i++) {
		byte tmp = le[i];
		le[i] = le[len-1-i];
		le[len-1-i]=tmp;
	}
}

/**
 * padding pading the given chunk of byte into a valid sm3 block
 * parameter src is byte array, src_len is the len of byte array
 * the output padded byte array is returned and its len set in out_len if it is not NULL
 *
 */
byte *
padding(const byte *src, size_t src_len, size_t content_len, size_t *out_len) {
	size_t src_bit_len = sizeof(byte) * src_len * 8;
	size_t content_bit_len = sizeof(byte) * content_len * 8;
	size_t k_bit_len = (448 - ((src_bit_len % 512) + 1)  + 512) % 512 ;
	int output_len = (src_bit_len + 1 + 64 + k_bit_len) >> 3;
	byte *result = malloc(sizeof(byte) * output_len);
	// set all zero
	memset(result, 0, sizeof(result));

	memcpy(result, src, sizeof(byte) * src_len);
	// set a bit to 1
	byte one = 0x80;
	result[src_len] |= one;

	// set src_len bigendian to last 8 byte
	byte *big_content_bit_len = malloc(sizeof(byte) * 8);
	memcpy(big_content_bit_len, &content_bit_len, 8);
	reverse_by_byte(big_content_bit_len, 8);
	memcpy(&result[output_len-8], big_content_bit_len, 8);
	free(big_content_bit_len);
	if(out_len != NULL) *out_len = output_len;
	return result;
}


static inline word P0(word w) {
	w = (w) ^ ls_w(w, 9) ^ ls_w(w, 17);
	return w;
}

static inline word P1(word w) {
	w = (w) ^ ls_w(w, 15) ^ ls_w(w, 23);
	return w;
}


// return big endian W and W'
word *expand_blk(const char *blks, word **W_p) {
	word *W_result = malloc(68 * sizeof(word));
	word *W_p_result = malloc(64 * sizeof(word));
	memcpy(W_result, blks, sizeof(byte) * 64); // copy blks to W
	
	if(isLittleEndian) {
		for (int i=0;i<68;i++) {
			reverse_by_byte((byte *)&W_result[i], 4);
		}
	}

	for (int i=16;i<=67;i++) {
		word tmp = P1( W_result[i-16] ^ W_result[i-9] ^ ls_w(W_result[i-3], 15) ) ^ ls_w(W_result[i-13], 7) ^ W_result[i-6]; 
		W_result[i] = tmp;
	}
	for (int i=0;i<=63;i++) {
		W_p_result[i] = W_result[i] ^ W_result[i+4];
	}

	if(W_p != NULL) *W_p = W_p_result;
	return  W_result;

}

static inline word Tj(int j) {
	word w;
	if(j>=0 && j<=15) w = 0x79cc4519;
	else if(j>=16 && j<=63) w = 0x7a879d8a ;
	return  w;
}

static inline word FF(int j, word x, word y, word z) {
	word w;
	if(j>=0 && j<=15) w = x ^ y ^ z;
	else if(j>=16 && j<= 63) w = (x & y) | (x & z) | (y & z);
	return w;
}

static inline word GG(int j, word x, word y, word z) {
	word w;
	if(j>=0 && j<=15) w = x ^ y ^ z;
	else if(j>=16 && j<=63) w = (x & y) | ( (~x) & z);
	return  w;
}

void CF(word * const iv, byte* blk, const word * const W, const word * const W_p) {
	word A = iv[0];
	word B = iv[1];
	word C = iv[2];
	word D = iv[3];
	word E = iv[4];
	word F = iv[5];
	word G = iv[6];
	word H = iv[7];

	word SS1, SS2, TT1, TT2;
	
	for (int i=0;i<64;i++) {
		SS1 =  ls_w(ls_w(A, 12) + E + ls_w(Tj(i), i), 7);
		SS2 = SS1 ^ ls_w(A, 12);
		TT1 = FF(i, A, B, C) + D + SS2 + W_p[i];
		TT2 = GG(i, E, F, G) + H + SS1 + W[i];
		D = C;
		C = ls_w(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = ls_w(F, 19);
		F = E;
		E = P0(TT2);
		
	}
	iv[0] = A ^ iv[0];
	iv[1] = B ^ iv[1];
	iv[2] = C ^ iv[2];
	iv[3] = D ^ iv[3];
	iv[4] = E ^ iv[4];
	iv[5] = F ^ iv[5];
	iv[6] = G ^ iv[6];
	iv[7] = H ^ iv[7];
}

