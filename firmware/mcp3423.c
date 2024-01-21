#include <string.h>
#include <util/delay.h>
#include "i2c.h"
#include "mcp3423.h"


char mcp3423_readChannel(unsigned char mcp_addr, unsigned char chn, unsigned char gain, unsigned char *dst)
{
	unsigned char cfg_reg;
	unsigned char rd_data[4]; // 3 data byts + cfg_byte
	int res;

	cfg_reg = MCP3423_CFGREG_RDY | MCP3423_CFGREG_18BITS | gain | MCP3423_CHANNEL(chn);

	_delay_us(10);
	// Start one-shot conversion	
	res = i2c_transaction(mcp_addr, 1, &cfg_reg, 0, NULL);
	if (res<0)
		return res;

	// When the device is configured for 18-bit conversion mode, it outpus
	// three data bytes followed by a configuration byte.	
	do {
		_delay_us(10);
		res = i2c_transaction(mcp_addr, 0, NULL, 4, rd_data);
		if (res<0)
			return res;
	} while(rd_data[3] & MCP3423_CFGREG_RDY);

	memcpy(dst, rd_data, 3);

	return 0;
}




