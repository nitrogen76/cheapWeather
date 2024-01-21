#ifndef _mcp_3423_h__
#define _mcp_3423_h__

#define MCP3423_BASE_ADDR 			0x68

#define MCP3423_CFGREG_RDY			0x80

#define MCP3423_CFGREG_C1			0x40
#define MCP3423_CFGREG_C0			0x20
#define MCP3423_CHANNEL(c)			( ((c)&0x03)  << 5)

#define MCP3423_CFGREG_S1			0x08
#define MCP3423_CFGREG_S2			0x04
#define MCP3423_CFGREG_18BITS		(MCP3423_CFGREG_S2|MCP3423_CFGREG_S1)


#define MCP3423_CFGREG_G1			0x02
#define MCP3423_CFGREG_G0			0x01

#define MCP3423_GAIN1X				0
#define MCP3423_GAIN2X				MCP3423_CFGREG_G0
#define MCP3423_GAIN4X				MCP3423_CFGREG_G1
#define MCP3423_GAIN8X				(MCP3423_CFGREG_G0 | MCP3423_CFGREG_G1)

char mcp3423_readChannel(unsigned char mcp_addr, unsigned char chn, unsigned char gain, unsigned char *dst);


#endif // _mcp_3423_h__

