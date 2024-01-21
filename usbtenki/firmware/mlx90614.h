#ifndef _mlx90614_h__
#define _mlx90614_h__

/* 000x xxxx */
#define MLX90614_CMD_RAM(addr)		((addr)&0x1F)

/* 001x xxxx */
#define MLX90614_CMD_EEPROM(addr)	(0x20 | ((addr) & 0x1F))

/* 1111 0000 */
#define MLX90614_CMD_READ_FLAGS		0xF0

/* 1111 1111 */
#define MLX90614_CMD_SLEEP			0xFF

#define MLX90614_RAM_AMBIANT_SENSOR_DATA	0x03
#define MLX90614_RAM_IR_SENS_DATA1			0x04
#define MLX90614_RAM_IR_SENS_DATA2			0x05
#define MLX90614_RAM_TA						0x06
#define MLX90614_RAM_TOBJ1					0x07
#define MLX90614_RAM_TOBJ2					0x08

int mlx90614_readRam(char i2c_addr, unsigned char addr, unsigned char *dst, unsigned char len);


#endif
