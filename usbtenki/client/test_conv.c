#include <stdio.h>
#include <math.h>

float raw12bit_to_temp(unsigned char dat[2])
{
	signed short t;

	t = ((dat[0] << 4) | (dat[1]>>4))<<4;
	t >>= 4;

	return ((float)t) * pow(2.0,-4.0);
}

/* Test data conversion using samples from MCP9800
 * datasheet in 12 bit mode. */
int main(void)
{
	unsigned char sample1[2] = { 0x7D, 0x00 };
	unsigned char sample2[2] = { 0x19, 0x70 };
	unsigned char sample3[2] = { 0x00, 0x10 };
	unsigned char sample4[2] = { 0xff, 0xf0 };
	unsigned char sample5[2] = { 0xe6, 0x90 };
	unsigned char sample6[2] = { 0xc9, 0x00 };

	printf("Temp1: %f\n", raw12bit_to_temp(sample1));
	printf("Temp2: %f\n", raw12bit_to_temp(sample2));
	printf("Temp3: %f\n", raw12bit_to_temp(sample3));
	printf("Temp4: %f\n", raw12bit_to_temp(sample4));
	printf("Temp5: %f\n", raw12bit_to_temp(sample5));
	printf("Temp6: %f\n", raw12bit_to_temp(sample6));

	return 0;
}
