#include <stdio.h>

#define POLYNOMIAL			0x1021
#define INITIAL_REMAINDER	0x1D0F
#define FINAL_XOR_VALUE		0x0000

typedef unsigned short width;

#define WIDTH 	(sizeof(width)*8)
#define TOPBIT	(1 << (WIDTH - 1))

width crcTable[256];

void crcInit(){
	width remainder;
	width dividend;
	int bit;
	
	for(dividend = 0; dividend < 256; dividend++){
		remainder = dividend << (WIDTH - 8);
		
		for(bit = 0; bit < 8; bit++){
			if (remainder & TOPBIT){
				remainder = (remainder << 1) ^ POLYNOMIAL;
			}else{
				remainder = remainder << 1;
			}
		}
		crcTable[dividend] = remainder;
	}
}

width crcCompute(unsigned char *message, unsigned int nBytes){
	unsigned int offset;
	unsigned char byte;
	width remainder = INITIAL_REMAINDER;
	
	for(offset = 0; offset < nBytes; offset++){
		byte = (remainder >> (WIDTH - 8)) ^ message[offset];
		remainder = crcTable[byte] ^ (remainder << 8);
	}
	return (remainder ^ FINAL_XOR_VALUE);
}

int main(){
	crcInit();
	width value = crcCompute("65 0", 4);
	printf("%d, %d", value, sizeof(value));
	
	return 0;
}