/* This code generates the DRV value with the hamming code embedded in it.
 * The Hamming algorithm as per the reference manual has been implemented.
 */

/* Copyright (c) 2012, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Freescale Semiconductor nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY Freescale Semiconductor ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Freescale Semiconductor BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define DRV_SIZE_BITS 64
#define DRV_SIZE_BYTES (DRV_SIZE_BITS/8)
#define DRV_REG_NO (DRV_SIZE_BITS/32)
#define DRV_REG_BITS 32

typedef unsigned char u8;

u8 drv_hex[DRV_SIZE_BYTES];

/* Generate the Hamming code bits for the 256 bits stored in number.
 * The values at the locations of the code bits are ignored and is
 * overwritten with the generated values.
 */
void print_drv()
{
	int i;
	printf("\n NAME    |     BITS     |    VALUE  ");
	printf("\n_________|______________|____________");

	for (i = DRV_SIZE_BYTES - 4; i >= 0; i = i - 4) {
		printf("\nDRV %d    | %3d-%3d\t|   %.2x%.2x%.2x%.2x ",
		       DRV_REG_NO - 1 - i/4,
		       (DRV_REG_BITS * (DRV_REG_NO - i/4)) - 1,
		       (DRV_REG_BITS * (DRV_REG_NO - i/4 - 1)),
		       drv_hex[i], drv_hex[i + 1], drv_hex[i + 2],
		       drv_hex[i + 3]);
	}

}

void generate_code_bits(bool number[])
{
	int i, j;
	char k[9];
/* Calculate each code bit in turn */
	for (i = 1; i <= DRV_SIZE_BITS / 2; i = (i << 1)) {
/* Examine each data bit
 * Only bits greater than i need to be checked as no
 * bit less than i will ever be XORed into i
 * J starts at i so that number[i] is initialized to 0
 */
		for (j = i; j <= DRV_SIZE_BITS - 1; j = j + 1) {
			if ((i & j) != 0)
				number[i] = number[i] ^ number[j];
		}
	}
/* Calculate the overall parity
 * J starts at 0 so that number[0] is initialized to 0
 * number[0] contains the even parity of all of the bits
 */
	for (j = 0; j <= DRV_SIZE_BITS - 1; j = j + 1)
		number[0] = number[0] ^ number[j];
#ifdef DEBUG
	printf("\nHamming code -\n");
	for (i = 0; i < DRV_SIZE_BITS; i++)
		printf("%d", number[i]);
#endif
	for (i = 0; i < DRV_SIZE_BYTES; i++) {
		for (j = 0; j < 8; j++) {
			k[j] =
			    (number[(DRV_SIZE_BYTES - i) * 8 - (j + 1)]) + 48;
		}
		k[8] = '\0';
		drv_hex[i] = strtoul(k, NULL, 2);
	}
	print_drv();
}

void gen_rand_string()
{
	unsigned int iseed = (unsigned int)time(NULL);
	int i, l, index1, index2;
	char hex_digits[] = "0123456789abcdef";
	char rand_string[2];

	/*providing seed for random number generation*/
	srand(iseed);

	for (i = 0; i < 2 * DRV_SIZE_BYTES; i = i + 2) {
		/*generate first random nibble of the byte*/
		index1 = rand() % strlen(hex_digits);
		rand_string[0] = hex_digits[index1];

		/*generate second random nibble of the byte*/
		index2 = rand() % strlen(hex_digits);
		rand_string[1] = hex_digits[index2];

		l = i / 2;
		drv_hex[l] = strtoul(rand_string, NULL, 16);
	}

}

int check_string(char *str)
{
	while (*str) {
		if ((*(str) >= 48 && *(str) <= 57)
		    || (*(str) >= 65 && *(str) <= 70) || (*(str) >= 97
							  && *(str) <= 102)) {

		} else {
			return -1;
		}

		str++;
	}

	return 0;

}

void usage()
{
	printf("\nUsage: ./gen_drv [string]\n");
	printf("string : 8 byte string\n");
	printf("e.g. gen_drv 1111111122222222\n");
}

int main(int argc, char *argv[])
{
	bool num[DRV_SIZE_BITS];
	char drv_in[2];
	int i, j, l, ret;
	if (argc == 2) {
		if ((strcmp(argv[1], "--help") == 0)
			|| (strcmp(argv[1], "-h") == 0)) {
			usage();
			exit(0);
		}
		/*check length of hexadecimal string*/
		if (strlen(argv[1]) == 2 * DRV_SIZE_BYTES) {
			/*check if string is valid hexadecimal string*/
			ret = check_string(argv[1]);
			if (ret == -1) {
				printf
				    ("\nError: Input string is not having "
					"valid hexadecimal character\n");
				return -1;
			}
			for (i = 0; i < 2 * DRV_SIZE_BYTES; i = i + 2) {
				drv_in[0] = argv[1][i + 0];
				drv_in[1] = argv[1][i + 1];
				l = i / 2;
				drv_hex[l] = strtoul(drv_in, NULL, 16);
			}
		} else {
			printf("\nError: Invalid Input string Length\n");
			usage();
			exit(1);

		}
	} else if (argc == 1) {
		printf("\nGenerating random string as input "
				"string not provided\n");
		gen_rand_string();
	} else {
		printf("\nError: Wrong Usage\n");
		usage();
		exit(1);
	}

	/*Create array of bits to be used as an input*/
	/*Also reversing the bytes provided as input*/
	for (i = 0; i < DRV_SIZE_BYTES; i++) {

		l = 0x80;
		for (j = 0; j < 8; j++) {
			num[DRV_SIZE_BYTES * 8 - i * 8 - 1 - j] =
			    (drv_hex[i]) & (l);
			l = l >> 1;
		}
	}

#ifdef DEBUG
	for (i = 0; i < DRV_SIZE_BITS; i++)
		printf("%d", num[i]);
#endif
	/*generate Hamming code and replace bits in DRV key*/
	generate_code_bits(num);

	printf("\n");
	return 0;
}
