#include <string.h>
#include "i2c.h"
#include "mlx90614.h"

int mlx90614_readRam(char i2c_addr, unsigned char addr, unsigned char *dst, unsigned char len)
{
	unsigned char tmp;

	tmp = MLX90614_CMD_RAM(addr);
	return i2c_transaction(i2c_addr, 1, &tmp, len, dst);
}


