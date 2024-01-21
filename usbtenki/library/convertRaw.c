#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "usbtenki.h"
#include "usbtenki_cmds.h"
#include "usbtenki_units.h"
#include "convert_type_k.h"

/* Calculate the theoric resistance of an RTD for a given temperature. */
static double temp_to_pt100_r(double temp)
{
	// DIN 43760
	double r0 = 100;
	double a = 3.9080 * pow(10, -3);
	double b = -5.8019 * pow(10, -7);
	double c = -4.2735 * pow(10, -12);

//	printf("Considering %.4f\n", temp);

	if (temp > 0) {
		return r0 * (1 + a*temp + b*pow(temp,2));
	} else {
		return r0 * (1 + a*temp + b*pow(temp,2) + c*pow(temp,3));
	}
}

/* Recusively zoom-in a temperature to resistance match. */
static double _searchTempFromR(double r, double t_start, double step)
{
	double t;
	double sr;

	sr = temp_to_pt100_r(t_start);
	if (sr > r) {
		return -999;
	}

	// looks like we are close enough.
	if (step < 0.00001)
		return t_start;

	for (t=t_start; t<1000; t+=step)
	{
		sr = temp_to_pt100_r(t);

		if (sr > r) {
			return _searchTempFromR(r, t-step, step/10.0);
		}
	}

	return -999;
}

double zk(double k, double p, double q)
{
	return 2 * sqrt(-p/3.0) * cos(1/3.0 * acos((-q/2.0)*sqrt(27/-(p*p*p))) + 2*k*M_PI/3.0);
//	return 2 * sqrt(-p/3.0) * cos(1/3.0 * acos((3*q/2*p)*sqrt(-3 / p)) - k*((2*M_PI)/3.0));
}

/* Using a recursive algorithm, find the RTD temperature from its resistance. */
double searchTempFromR(double r)
{
	double a = 3.9080 * pow(10, -3);
	double b = -5.8019 * pow(10, -7);
//	double c = -4.2735 * pow(10, -12);


	if (r > 100) {
//		printf("Using formula\n");
		return (-a + sqrt(pow(a,2)-4*b*(1-(r/100.0))) ) / (2*b);
	}
#if 0
	else {
		double p = (-(b*b / (3*c*c)) + (a/c) );
		double q = ( (b/(27*c)) * (  ( (2*(b*b)) / (c*c) )  - (9*a/c)) ) + (1-(r/100.0))/c;
		double delta = q*q + 4.0 / 27.0 * p*p*p;
		double z0, z1, z2;

		z0 = zk(0, p, q);
		z1 = zk(1, p, q);
		z2 = zk(2, p, q);

		printf("%f %f\n", p, q);
		printf("%f %f %f\n", z0,z1,z2);

		return pow((-q * sqrt(delta) / 2.0)  , (1/3.0) ) + pow ((-q - sqrt(delta)) / 2.0 , (1/3.0));
	}
#endif

	// completes after approx. 40 calls to temp_to_pt100_r
	return _searchTempFromR(r, -274, 100);
}

static double getColdTemp(struct USBTenki_channel *chn)
{
	uint32_t output_code;
	uint16_t ref_res_base;
	int16_t ref_trim;
	double rtd_res, gain, ref_res, temp_raw;
	unsigned char *raw_data = chn->raw_data;
	double temperature;

	output_code = (raw_data[1] << 16) | (raw_data[2] << 8) | (raw_data[3]);
	ref_res_base = raw_data[4] << 8;
	ref_res_base |= raw_data[5];
	gain = pow(2, (raw_data[6]>>4));
	ref_trim = raw_data[7] << 8;
	ref_trim |= raw_data[8];
	ref_res = ref_res_base + ref_trim / 10000;

	//printf("Gain: %f, ref_res: %.5f\n", gain, ref_res);
	//printf("Output code: %d (%06x)\n", output_code, output_code);
	//printf("amplitude: %.10f\n", output_code / pow(2,23));

	rtd_res = ref_res * output_code / (pow(2, 23) * gain);
	//printf("RTD resistance: %.6f\n", rtd_res);

	temp_raw = searchTempFromR(rtd_res);
	temperature = temp_raw;
//				temperature = round(temp_raw * 100) / 100;
//				printf("Temperature: %.06f : %.06f\n", temperature);


	return temperature;
}

static double getColdTempAlt(struct USBTenki_channel *chn)
{
	uint32_t output_code;
	double rtd_res, gain, temp_raw, ref = 2.5;
	unsigned char *raw_data = chn->raw_data;
	double temperature;
	double irefs[10] = { 0,10,50,100,250,500,750,1000,1500,2000 };
	int16_t ref_trim;

	double Iref;

	output_code = (raw_data[1] << 16) | (raw_data[2] << 8) | (raw_data[3]);
	gain = pow(2, (raw_data[6]>>4));
	Iref = irefs[raw_data[6] & 0xf] / 1000000;
	ref_trim = raw_data[7] << 8;
	ref_trim |= raw_data[8];

	rtd_res = (output_code / pow(2,23) / gain * ref) / Iref;
//	printf("RTD res: %.4f\n", rtd_res);
//	printf("ref trim: %d\n", ref_trim);
//  printf("%02x %02x\n", raw_data[7], raw_data[8]);
	rtd_res += ref_trim / 1000.0;
//	printf("RTD res: %.4f\n", rtd_res);

	temp_raw = searchTempFromR(rtd_res);
	temperature = temp_raw;

	return temperature;
}


int usbtenki_convertRaw(struct USBTenki_channel *chn, unsigned long flags, unsigned char *caldata, int caldata_len)
{
	float temperature;
	int chip_fmt = TENKI_UNIT_KELVIN;
	unsigned char *raw_data;

	/*
	int i;
	printf("Raw data: ");
	for (i=0; i<chn->raw_length; i++) {
		int b;
		for (b=0x80; b; b>>=1) {
			printf("%c", raw_data[i] & b ? '1' : '0');
		}
		printf(" ");
	}
	printf("\n");*/

	raw_data = chn->raw_data;

	switch (chn->chip_id)
	{
		case USBTENKI_CHIP_MCP9800:
			{
				signed short t;

				if (chn->raw_length!=2)
					goto wrongData;

				/* The sensor will be initailized in 12 bits mode  */
				t = ((raw_data[0] << 4) | (raw_data[1]>>4))<<4;
				t >>= 4;
				temperature = ((float)t) * pow(2.0,-4.0);
				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_ADT7410:
			{
				signed short t;

				if (chn->raw_length!=2)
					goto wrongData;

				/* This sensor offers 16 bits of resolution.
				 *
				 * [0]                                 [1]
				 *   7   6   5   4   3   2   1   0      7   6    5   4   3   2   1   0
				 * D12 D11 D10  D9  D8  D7  D6  D5  |  D4  D3  D2   D1  D0   X   X   X
				 */

				t = raw_data[0] << 8 | raw_data[1];
				temperature = ((float)t) * pow(2.0,-7.0);
				chip_fmt = TENKI_UNIT_CELCIUS;

			}
			break;

		case USBTENKI_CHIP_SE95:
			{
				signed short t;

				if (chn->raw_length!=2)
					goto wrongData;

				/* This sensor offers 13 bits of resolution as follows:
				 *
				 * [0]                                 [1]
				 *   7   6   5   4   3   2   1   0      7   6    5   4   3   2   1   0
				 * D12 D11 D10  D9  D8  D7  D6  D5  |  D4  D3  D2   D1  D0   X   X   X
				 */

				t = ((raw_data[0] << 5) | (raw_data[1]>>3))<<3;
				t >>= 3;
				temperature = ((float)t) * pow(2.0,-5.0);
				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_LM75:
			{
				signed short t;

				if (chn->raw_length!=2)
					goto wrongData;

				/* The sensor only supports 9 bits */
				t = (raw_data[0] << 1) | (raw_data[1]>>7);
				temperature = ((float)t) * pow(2.0,-1.0);
				chip_fmt = TENKI_UNIT_CELCIUS;

			}
			break;

		case USBTENKI_CHIP_SHT_TEMP:
			{
				unsigned  short t;

				if (chn->raw_length!=2)
					goto wrongData;

				t = (raw_data[0]<<8) | raw_data[1];

				temperature = -40.0 + 0.01  * (float)t;

				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_SHT_RH:
			{
				// The following coefficients are from the 2011 Datasheet
				float c1 = -2.0468;
				float c2 = 0.0367;
				float c3 = -1.5955 * powf(10.0, -6.0);
				float sorh;

				if (flags & USBTENKI_FLAG_USE_OLD_SHT75_COMPENSATION) {
					// The following coefficients are from the 2007 Datasheet
					c1 = -4.0;
					c2 = 0.0405;
					c3 = -2.8 * powf(10.0, -6.0);
				}

				if (chn->raw_length!=2)
					goto wrongData;

				sorh = (float)( (unsigned short)((raw_data[0]<<8) | raw_data[1]) );

				// Keep the raw value around for the temperature compensated
				// humidity reading
				chn->raw_value = sorh;

				temperature = c1 + c2*sorh + c3 * powf(sorh, 2.0);

				if (temperature < 0)
					temperature = 0;
				if (temperature > 100)
					temperature = 100;

				chip_fmt = TENKI_UNIT_RH;
			}
			break;


		case USBTENKI_CHIP_BS02_TEMP:
			{
				unsigned  short t;

				if (chn->raw_length!=2)
					goto wrongData;

				t = (raw_data[0]<<8) | raw_data[1];

				temperature = -40.0 + 0.04  * (float)t;

				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_BS02_RH:
			{
				float c1 = -4.0;
				float c2 = 0.648;
				float c3 = -7.2 * powf(10.0, -4.0);
				float sorh;

				if (chn->raw_length!=2)
					goto wrongData;

				sorh = (float)( (unsigned short)((raw_data[0]<<8) | raw_data[1]) );

				temperature = c1 + c2*sorh + c3 * powf(sorh, 2.0);
				if (temperature < 0)
					temperature = 0;
				if (temperature > 100)
					temperature = 100;

				chip_fmt = TENKI_UNIT_RH;
			}
			break;

		case USBTENKI_CHIP_MPXV7002:
			{
				float vout, vs, vorigin;
				unsigned int adc_out;
				unsigned int adc_origin;

				/* "Datasheet figure 4, Output versus pressure differential"
				 *
				 * Transfer function:
				 *
				 * Vout = Vs * (0.2 * P(kPa) + 0.5) +- 6.25% Vfss
				 *
				 * Vs: Supply voltage (5 volt)
				 * Vfss = Full Scale Span (max 4.5v)
				 *
				 * At nominal accuracy (0% Vfss):
				 *
				 * Vout = Vs * (0.2 * P + 0.5)
				 *
				 * Hence
				 *
				 * Vout / VS - 0.5
				 * ---------------   = P
				 *         0.2
				 *
				 *
				 *
				 */

				vs = 5.0;
				adc_out = raw_data[0] << 8 | raw_data[1];
				adc_origin = raw_data[2] << 8 | raw_data[3];

//				printf("ADC: 0x%04x, ADC_ORIG: 0x%04x\n", adc_out, adc_origin);

				vout = (adc_out * vs) / (float)0xffff;
				vorigin = (adc_origin * vs) / (float)0xffff;

//				printf("Vout: %.3f, Vorigin: %.3f\n", vout, vorigin);

				vout -= vorigin - 2.5;

//				printf("Final: %.3f\n", vout);

				temperature = (vout / vs - 0.5) / 0.2;
				chip_fmt = TENKI_UNIT_KPA;
			}
			break;

		case USBTENKI_CHIP_MP3H6115A:
		case USBTENKI_CHIP_MPX4115:
			{
				float vout, vs, p;
				unsigned short adc_out;

				/* -- Sensor formulas:
				 *   "Vout = Vs * (.009 * P -0.095)"
				 *   where Vs is 5.1 Vdc. Output is ratiometric
				 *   to Vs between 4.85 and 5.35 volts.
				 *
				 * -- Atmel adc:
				 *   In 10 bit mode, 0x000 represents ground and 0x3ff represents
				 *   the selected reference voltage (VCC in our case) minus one
				 *   LSB.
				 *
				 * The code in the Atmel averages multiple samples and
				 * outputs a 16 bit value.
				 *
				 * The ADC reference voltage is the same as the sensor's Vs,
				 * So Vs does not really matter here.
				 */
				vs = 5.0;
				adc_out = raw_data[0] << 8 | raw_data[1];
				//vout = (adc_out * vs) / 1024.0;
				vout = (adc_out * vs) / (float)0xffff;
				p = ((vout/vs)+0.095)/.009;

				temperature = p;
				chip_fmt = TENKI_UNIT_KPA;
			}
			break;

		case USBTENKI_CHIP_MLH_A:
			{
				unsigned short adc_out;
				float vs, v;

				/*the code in the atmel averages multiple samples and
				 * outputs a 16 bit value.
				 *
				 * the adc reference voltage is the same as the sensor's vs,
				 * so this is ok for ratiometric measurements.
				 */
				vs = 5.0;
				adc_out = raw_data[0] << 8 | raw_data[1];
				v = (adc_out * vs) / (float)0xffff;

				/* Output range: 0.5v to 4.5v (0 PSI to 150 PSI) */
				v -= 0.5;
				if (v<0)
					v = 0;

				temperature = v * 150 / 4;
				chip_fmt = TENKI_UNIT_PSI;
			}
			break;

		case USBTENKI_CHIP_TSL2561_IR_VISIBLE_16X:
			temperature = raw_data[1] << 8 | raw_data[0];
			chip_fmt = TENKI_UNIT_RAW;
			break;

		case USBTENKI_CHIP_TSL2561_IR_16X:
			temperature = raw_data[1] << 8 | raw_data[0];
			chip_fmt = TENKI_UNIT_RAW;
			break;

		case USBTENKI_CHIP_TSL2568_IR_VISIBLE_16X:
			temperature = raw_data[1] << 8 | raw_data[0];
			chip_fmt = TENKI_UNIT_RAW;
			break;

		case USBTENKI_CHIP_TSL2568_IR_16X:
			temperature = raw_data[1] << 8 | raw_data[0];
			chip_fmt = TENKI_UNIT_RAW;
			break;

		case USBTENKI_CHIP_TSL2561_IR_VISIBLE:
			temperature = raw_data[1] << 8 | raw_data[0];
			chip_fmt = TENKI_UNIT_RAW;
			break;

		case USBTENKI_CHIP_TSL2561_IR:
			temperature = raw_data[1] << 8 | raw_data[0];
			chip_fmt = TENKI_UNIT_RAW;
			break;

		case USBTENKI_CHIP_TSL2568_IR_VISIBLE:
			temperature = raw_data[1] << 8 | raw_data[0];
			chip_fmt = TENKI_UNIT_RAW;
			break;

		case USBTENKI_CHIP_TSL2568_IR:
			temperature = raw_data[1] << 8 | raw_data[0];
			chip_fmt = TENKI_UNIT_RAW;
			break;

		case USBTENKI_CHIP_D6F_V03A1:
			{
				unsigned short adc_out;
				float vs;
				float voltage;
				float flow;

				/*the code in the atmel averages multiple samples and
				 * outputs a 16 bit value.
				 *
				 * the adc reference voltage is the same as the sensor's vs,
				 * so this is ok for ratiometric measurements.
				 */
				vs = 5.0;
				adc_out = raw_data[0] << 8 | raw_data[1];

				voltage = (adc_out * vs) / (float)0xffff;

				// The datasheet contains a curve and a table with 4 data points
				// Flow m/sec :  0      0.75    1.50    2.25    3.00
				// Voltage    :  0.50   0.70    1.11    1.58    2.00
				//
				// Note: All voltage +/- 0.15

				if (voltage < 0.70) {
					flow = 0 + (voltage - 0.5) / (0.70 - 0.50) * 0.75;
				} else if (voltage < 1.11) {
					flow = 0.75 + (voltage - 0.70) / (1.11 - 0.70) * 0.75;
				} else if (voltage < 1.58) {
					flow = 1.50 + (voltage - 1.11) / (1.58 - 1.11) * 0.75;
				} else { // voltage < 3 ?
					flow = 2.25 + (voltage - 1.58) / (2.00 - 1.58) * 0.75;
				}

				if (flow<0)
					flow = 0;
				if (flow>3.0)
					flow = 3.0;

				temperature = flow;
				chip_fmt = TENKI_UNIT_METER_SEC;
			}
			break;

		case USBTENKI_CHIP_VOLTS_REVERSE:
		case USBTENKI_CHIP_VOLTS:
			{
				unsigned short adc_out;
				float vs;

				/*the code in the atmel averages multiple samples and
				 * outputs a 16 bit value.
				 *
				 * the adc reference voltage is the same as the sensor's vs,
				 * so this is ok for ratiometric measurements.
				 */
				vs = 5.0;
				adc_out = raw_data[0] << 8 | raw_data[1];

				if (chn->chip_id==USBTENKI_CHIP_VOLTS_REVERSE)
					adc_out ^= 0xffff;

				temperature = (adc_out * vs) / (float)0xffff;
				chip_fmt = TENKI_UNIT_VOLTS;
			}
			break;

		case USBTENKI_MCU_ADC0:
		case USBTENKI_MCU_ADC1:
		case USBTENKI_MCU_ADC2:
		case USBTENKI_MCU_ADC3:
		case USBTENKI_MCU_ADC4:
		case USBTENKI_MCU_ADC5:
			temperature = raw_data[1] << 8 | raw_data[0];
			chip_fmt = TENKI_UNIT_RAW;
			break;

		case USBTENKI_CHIP_TACHOMETER:
			{
				float clk = 12000000 / 1024;
				unsigned short count = raw_data[0]<<8 | raw_data[1];

				chip_fmt = TENKI_UNIT_HZ;

				if (count == 0) {
					// Stopped
					temperature = 0;
					break;
				}

				temperature = 1 / ((1/clk) * count);

				if (0)
				{
					int i;
					for (i=0; i<chn->raw_length; i++) {
						printf("%02X ", raw_data[i]);
					}
					printf("\n");
				}
			}
			break;

		case USBTENKI_CHIP_PT100_RTD:
			{
				int raw_ch0, raw_ch1;
				double lsb = 15.625 * pow(10, -6); // 15.625 uV
				double volts_ch0, volts_ch1;
				double i_src = 0.001; // 1mA
				double r_wire;
				double rt;
				double r_pt100;
				double chn0_gain = 2;
				double chn1_gain = 8;
				double ferrite_r = 0;

				if (chn->raw_length != 6)
					return -1;

				// Assign with most significant bits aligned (signed).
				// Even though the negative inputs of the ADC are at GND,
				// the conversion can still result venture below 0 (values of -1 or -2). This
				// happens when using 2 wire RTDs because we short CH1 to GND.
				raw_ch0 = ( ((raw_data[0] & 0x03) << 30) | (raw_data[1] << 22) | (raw_data[2] << 14) );
				raw_ch0 >>= 14;

				raw_ch1 = ( ((raw_data[3] & 0x03) << 30) | (raw_data[4] << 22) | (raw_data[5] << 14) );
				raw_ch1 >>= 14;

//				printf("ch0: %02X %02X %02X %08x\n", raw_data[0], raw_data[1], raw_data[2], raw_ch0);
//				printf("ch1: %02X %02X %02X %08x\n", raw_data[3], raw_data[4], raw_data[5], raw_ch1);

				// CALIBRATION
				//
				// The most significant factor seems to be the current source precision
				// of 1%. The ADC gain error and offset has almost no influence.
				//
				if (caldata_len > 0) {
					double current_error = 0;
					signed short cal_value = caldata[0] | (caldata[1]<<8);
					current_error = (double)(cal_value) / 10000000.0;
//					printf("Caldata: %02X %02X\n", caldata[0], caldata[1]);
//					printf("Current source error: %.8f\n", current_error);
					i_src += current_error;
				}

				volts_ch0 = raw_ch0 * lsb / chn0_gain;
				volts_ch1 = raw_ch1 * lsb / chn1_gain;


//				printf("ch0: %.8f volts\n", volts_ch0);
//				printf("ch1: %.8f volts\n", volts_ch1);
//
				r_wire = volts_ch1 / i_src;

//				printf("Lead resistance: %.8f ohm\n", r_wire);

				rt = volts_ch0 / i_src;
//				printf("Total resistance: %.8f ohm\n", rt);

				r_pt100 = rt - r_wire * 2  - ferrite_r;
				//printf("PT100 resistance: %.8f ohm\n", r_pt100);

				chip_fmt = TENKI_UNIT_CELCIUS;

				//r_pt100 = 90; // fake ~-20
				temperature = searchTempFromR(r_pt100);
			}
			break;

		case USBTENKI_CHIP_DRACAL_EM1_BUS_VOLTAGE:
			chip_fmt = TENKI_UNIT_VOLTS;
			//printf("Bus voltage: %02x %02x \n", raw_data[0], raw_data[1]);
			// LSB = 1.25mV
			temperature = ((short)(raw_data[0] << 8 | raw_data[1])) * 0.00125;
			break;

		case USBTENKI_CHIP_DRACAL_EM1_SHUNT_VOLTAGE:
			// LSB = 2.5uV
//			printf("Shunt voltage: %02x %02x \n", raw_data[0], raw_data[1]);
			chip_fmt = TENKI_UNIT_VOLTS;
			temperature = ((short)(raw_data[0] << 8 | raw_data[1])) * 0.0000025;
			break;

		case USBTENKI_CHIP_DRACAL_EM1_POWER:
			{
				double current_lsb = (raw_data[4] / pow(2,15));
//				printf("power reg: %02x %02x \n", raw_data[0], raw_data[1]);
				temperature = ((short)(raw_data[0] << 8 | raw_data[1])) * current_lsb * 25;
				chip_fmt = TENKI_UNIT_WATTS;
			}
			break;

		case USBTENKI_CHIP_DRACAL_EM1_CURRENT:
			{
				double current_lsb = (raw_data[4] / pow(2,15));
//				printf("current reg: %02x %02x \n", raw_data[0], raw_data[1]);
				temperature = ((short)(raw_data[0] << 8 | raw_data[1])) * current_lsb;
				chip_fmt = TENKI_UNIT_AMPS;
			}
			break;

		case USBTENKI_CHIP_RTD300_PT1000_3W:
			{
				uint32_t output_code;
				uint16_t ref_res_base;
				int16_t ref_trim;
				double rtd_res, gain, ref_res;

				if (chn->raw_length < 3) {
					goto wrongData;
				}

				output_code = (raw_data[0] << 16) | (raw_data[1] << 8) | (raw_data[2]);
				ref_res_base = raw_data[3] << 8;
				ref_res_base |= raw_data[4];
				gain = pow(2, (raw_data[5]>>4));
				ref_trim = raw_data[6] << 8;
				ref_trim |= raw_data[7];
				ref_res = ref_res_base + ref_trim / 8192.0;

				//printf("Gain: %f, ref_res: %.5f\n", gain, ref_res);
				//printf("Output code: %d (%06x)\n", output_code, output_code);
				//printf("amplitude: %.10f\n", output_code / pow(2,23));

				rtd_res = ref_res * output_code / (pow(2, 23) * gain);
				//printf("RTD resistance: %.6f\n", rtd_res);

				temperature = rtd_res; //  searchTempFromR(rtd_res);
				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_RTD300_PT100_3W:
			{
				uint32_t output_code;
				uint16_t ref_res_base;
				int16_t ref_trim;
				double rtd_res, gain, ref_res, temp_raw;

				if (chn->raw_length < 3) {
					goto wrongData;
				}

				output_code = (raw_data[0] << 16) | (raw_data[1] << 8) | (raw_data[2]);
				ref_res_base = raw_data[3] << 8;
				ref_res_base |= raw_data[4];
				gain = pow(2, (raw_data[5]>>4));
				ref_trim = raw_data[6] << 8;
				ref_trim |= raw_data[7];
				ref_res = ref_res_base + ref_trim / 8192.0;

				//printf("Gain: %f, ref_res: %.5f\n", gain, ref_res);
				//printf("Output code: %d (%06x)\n", output_code, output_code);
				//printf("amplitude: %.10f\n", output_code / pow(2,23));

				rtd_res = ref_res * output_code / (pow(2, 23) * gain);
				//printf("RTD resistance: %.6f\n", rtd_res);

				temp_raw = searchTempFromR(rtd_res);
				temperature = temp_raw;
//				temperature = round(temp_raw * 100) / 100;
//				printf("Temperature: %.06f : %.06f\n", temperature);

				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_RTD300_PT100_2W:
			{
				uint32_t output_code;
				double rtd_res, gain = 4.0, ref = 2.5, Iref = 0.000500;
				double irefs[10] = { 0,10,50,100,250,500,750,1000,1500,2000 };

				if (chn->raw_length < 3) {
					goto wrongData;
				}

				output_code = (raw_data[0] << 16) | (raw_data[1] << 8) | (raw_data[2]);

				gain = pow(2, (raw_data[5]>>4));

				if ((raw_data[5] & 0xf) >= 10) {
					goto wrongData;
				}

				Iref = irefs[raw_data[5] & 0xf] / 1000000;

				//printf("Gain: %f, Iref: %f\n", gain, Iref);
				//printf("Output code: %d (%06x)\n", output_code, output_code);
				//printf("DeltaV: %.10f\n", output_code / pow(2,23));
				rtd_res = (output_code / pow(2,23) / gain * ref) / Iref;
				//printf("RTD resistance: %.6f\n", rtd_res);

				temperature = searchTempFromR(rtd_res);
				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_TMC200_COLD:
			{
				if (chn->raw_length != 9) {
					goto wrongData;
				}

				//temperature = getColdTemp(chn);
				temperature = getColdTempAlt(chn);
				chip_fmt = TENKI_UNIT_CELCIUS;

				// This is an on-board cold junction sensor. Reasonable limits
				// are expected. Anything else strongly suggest defective components
				// rather than a true temperature.
				if ((temperature < -40) || (temperature > 90)) {
					goto outOfRange;
				}
			}
			break;

		case USBTENKI_CHIP_TMC200_TYPE_K:
			{
				uint8_t status;
				double cold_temp, cold_mv, comp_mv, t_v;
				int32_t output_code;
				double gain, vref = 2.5;
				gain = pow(2, (raw_data[5]>>4));

				if (chn->raw_length < 15) {
					goto wrongData;
				}

				cold_temp = getColdTempAlt(chn);
				cold_mv = typeK_temp_to_mv(cold_temp);

				status = raw_data[14];
				gain = pow(2, (raw_data[13]>>4));
				output_code = (raw_data[10] << 24) | (raw_data[11] << 16) | (raw_data[12]<<8);
				output_code >>= 8;
				t_v = output_code * (2.0 * vref / gain) / pow(2,24); // ( vref * output_code ) / ( pow(2, 15) * gain );

				comp_mv = cold_mv + t_v * 1000;
				temperature = typeK_mv_to_temp(comp_mv);

//				printf("TMC cold junction temperature: %.4f C\n", cold_temp);
//				printf("TMC cold junction voltage: %.4f mV\n", cold_mv);
//				printf("TMC Gain: %f, vref: %f\n", gain, vref);
//				printf("TMC output code: %06x\n", output_code);
//				printf("TMC voltage: %.8f V\n", t_v);
//				printf("Cold-junction compensated voltage: %.4f mV\n", comp_mv );
//				printf("Burn-out detection status: 0x%02x\n", status);

				if ((cold_temp < -40) || (cold_temp > 90)) {
					goto sensorError;
				}

				if (status & 0x3C) {
					goto probeDisconnected;
				}

				// Limit output temperature to what is *possible* for a working Type-K thermocouple +/- 5C
				if ((temperature < (-270-5)) || (temperature > (1372+5))) {
					goto outOfRange;
				}


				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_CO2_DXC200_FILTERED:
		case USBTENKI_CHIP_CO2_DXC200_INSTANT:
			{
				uint8_t status;
				uint16_t value, multiplier;

				/*
				int i;
				printf("Data: ");
				for (i=0; i<chn->raw_length; i++) {
					printf("%02x ", raw_data[i]);
				}
				printf("\n");
				*/

				if (chn->raw_length != 5) {
					goto wrongData;
				}

				status = raw_data[0];
				value = (raw_data[1]<<8) | raw_data[2];
				multiplier = (raw_data[3]<<8) | raw_data[4];

				//printf("Status: %d, value: %d, multiplier: %d\n", status, value, multiplier);

				if (status != 0x01) {
					goto sensorError;
				}
				if (multiplier == 0) {
					goto sensorError;
				}

				temperature = (value * multiplier) / 10000.0;
				chip_fmt = TENKI_UNIT_PERCENT;
			}
			break;

		case USBTENKI_CHIP_MLX90614_TA:
		case USBTENKI_CHIP_MLX90614_TOBJ:
			{
				unsigned short val;
				val = raw_data[0] | raw_data[1] << 8;

//				printf("len: %d\n", chn->raw_length);
//				printf("Raw[] = %04x\n", val);

				/* RAM address will sweep between 0x27AD to 0x7FFF as the
				 * object temperature rises from -70.01 to +382.19 °C */
				if (val < 0x27AD || val > 0x7FFF) {
					goto wrongData; // MSB is error flag
				}
				temperature = -70.01 + (val - 0x27AD)* ((382.19+70.01)/(0x7FFF-0x27AD));
				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_MS5611_P:
			{
				uint16_t c1,c2,c3,c4,c5,c6;
				int32_t dT;
				int32_t TEMP, T2;
				uint32_t D1,D2;
				int64_t OFF, SENS, OFF2, SENS2;
				int32_t P;

				D1 = raw_data[0] | raw_data[1]<<8 | raw_data[2]<<16;
				D2 = raw_data[3] | raw_data[4]<<8 | raw_data[5]<<16;

				c1 = caldata[0] | caldata[1]<<8;
				c2 = caldata[2] | caldata[3]<<8;
				c3 = caldata[4] | caldata[5]<<8;
				c4 = caldata[6] | caldata[7]<<8;
				c5 = caldata[8] | caldata[9]<<8;
				c6 = caldata[10] | caldata[11]<<8;

#if 0
				printf("C1 0x%04x (%d)\n", c1, c1);
				printf("C2 0x%04x (%d)\n", c2, c2);
				printf("C3 0x%04x (%d)\n", c3, c3);
				printf("C4 0x%04x (%d)\n", c4, c4);
				printf("C5 0x%04x (%d)\n", c5, c5);
				printf("C6 0x%04x (%d)\n", c6, c6);
				printf("D1 0x%08x (%d)\n", D1, D1);
				printf("D2 0x%08x (%d)\n", D2, D2);
#endif

				dT = D2 - c5 * 256;
				TEMP = 2000ll + (int64_t)dT * ((uint64_t)c6) / 8388608ll;
				OFF = (int64_t)c2 * 65536ll + ((int64_t)c4 * (int64_t)dT) / 128ll;
				SENS = (int64_t)c1 * 32768ll + ((int64_t)c3 * (int64_t)dT) / 256ll;
				P = ((int64_t)D1 * SENS / 2097152ll - OFF) / 32768;

	//			printf("Pressure pre-compensation: %.4f Bar\n", P / 100000.0);

				if (TEMP < 2000) {
					// Low temperature
					T2 = (dT * dT) / 0x80000000;						// DT^2 / 2^31
					OFF2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 2;		// OFF2 = 5 * (TEMP - 2000)^2 / 2^1
					SENS2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 4;	// SENS2 = 5 * (TEMP -2000)~2 / 2^2
//					printf("Low temp.\n");

					// Very low temperature
					if (TEMP < -1500) {
						OFF2 = OFF2 + 7 * (TEMP + 1500) * (TEMP + 1500);
						SENS2 = SENS2 + 11 * ((TEMP + 1500) * (TEMP + 1500)) / 2;
//						printf("Very low temp.\n");
					}
				} else {
					T2 = 0;
					OFF2 = 0;
					SENS2 = 0;
				}

	//			printf("Pre-comp: %d %ld %ld\n", TEMP, OFF, SENS);

				TEMP = TEMP - T2;
				OFF = OFF - OFF2;
				SENS = SENS - SENS2;

	//			printf("Post-comp: %d %ld %ld\n", TEMP, OFF, SENS);


				P = ((int64_t)D1 * SENS / 2097152ll - OFF) / 32768;

	//			printf("Pressure post-compensation: %.4f Bar\n", P / 100000.0);

				temperature = P;
				temperature /= 100000.0;
				chip_fmt = TENKI_UNIT_BAR;
			}
			break;

		case USBTENKI_CHIP_MS5611_T:
			{
				int32_t TEMP;
				int32_t dT;
				uint32_t D2;
				uint16_t c5,c6;

				D2 = raw_data[0] | raw_data[1]<<8 | raw_data[2] << 16;
				c5 = caldata[8] | caldata[9]<<8;
				c6 = caldata[10] | caldata[11]<<8;
				dT = D2 - c5 * 256;
				TEMP = 2000ll + (int64_t)dT * ((uint64_t)c6) / 8388608ll;
/*
				printf("C5 0x%04x (%d)\n", c5, c5);
				printf("C6 0x%04x (%d)\n", c6, c6);
*/
				temperature = TEMP;
				temperature /= 100.0;
				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_CC2_RH:
			{
				unsigned short rh_reg;

				if (chn->raw_length != 2)
					goto wrongData;

				// ChipCap2 Application guide
				rh_reg = ((raw_data[0] & 0x3f)) << 8;
				rh_reg |= raw_data[1];
				temperature = rh_reg / pow(2,14) * 100;
				chip_fmt = TENKI_UNIT_RH;
			}
			break;

		case USBTENKI_CHIP_CC2_T:
			{
				unsigned short t_reg;

				if (chn->raw_length != 2)
					goto wrongData;

				// ChipCap2 Application guide
				t_reg = raw_data[0] << 6;
				t_reg |= raw_data[1] >> 2;
				temperature = t_reg / pow(2,14) * 165 - 40;
				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_SHT31_RH_INTERNAL:
		case USBTENKI_CHIP_SHT31_RH_EXTERNAL:
		case USBTENKI_CHIP_SHT35_RH:
		case USBTENKI_CHIP_SHT31_RH:
			{
				unsigned short rh_reg;

				rh_reg = raw_data[0] << 8 | raw_data[1];

				temperature = 100 * rh_reg / 65535.0;
				chip_fmt = TENKI_UNIT_RH;
			}
			break;

		case USBTENKI_CHIP_SHT31_T_INTERNAL:
		case USBTENKI_CHIP_SHT31_T_EXTERNAL:
		case USBTENKI_CHIP_SHT35_T:
		case USBTENKI_CHIP_SHT31_T:
			{
				unsigned short t_reg;

				t_reg = raw_data[0]<<8 | raw_data[1];
				temperature = -45 + 175 * (t_reg / 65535.0);
				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_CCS811_TVOC:
			{
				uint16_t TVOC;
				uint8_t data_valid;

				if (chn->raw_length != 4)
					goto wrongData;

				data_valid = raw_data[0];
				TVOC = raw_data[1]<<8 | raw_data[2];

				if (!data_valid) {
					goto wrongData;
				}
				//printf("Status: 0x%02x\n", raw_data[3]);

				temperature = TVOC;
				chip_fmt = TENKI_UNIT_PPB;

			}
			break;

		case USBTENKI_CHIP_CCS811_eCO2:
			{
				uint16_t eCO2;
				uint8_t data_valid;

				if (chn->raw_length != 4)
					goto wrongData;

				data_valid = raw_data[0];
				eCO2 = raw_data[1]<<8 | raw_data[2];

				if (!data_valid) {
					goto wrongData;
				}

				temperature = eCO2;
				chip_fmt = TENKI_UNIT_PPM;
			}
			break;

		case USBTENKI_CHIP_CO2_PPM:
			{
				unsigned short ppm_reg;
				unsigned short status_reg;
				unsigned char autocal_status;

				status_reg = raw_data[0]<<8 | raw_data[1];
				ppm_reg = raw_data[2]<<8 | raw_data[3];
				autocal_status = raw_data[4];
				(void)autocal_status;

				if (status_reg & 0x7) {
					fprintf(stderr, "Sensor error 0x%04x\n", status_reg);
					goto sensorError;
				}
//				printf("Status reg: 0x%04x\n", status_reg);

				temperature = ppm_reg;
				chip_fmt = TENKI_UNIT_PPM;
			}
			break;

		case USBTENKI_CHIP_SCD30_T:
			{
				uint32_t tmp;

				if (chn->raw_length != 4)
					goto wrongData;

				tmp = (raw_data[0]<<24) | (raw_data[1]<<16) | (raw_data[2]<<8) | raw_data[3];

				temperature = *(float*)&tmp;
				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_SCD30_RH:
			{
				uint32_t tmp;

				if (chn->raw_length != 4)
					goto wrongData;

				tmp = (raw_data[0]<<24) | (raw_data[1]<<16) | (raw_data[2]<<8) | raw_data[3];
				temperature = *(float*)&tmp;
				chip_fmt = TENKI_UNIT_RH;
			}
			break;

		case USBTENKI_CHIP_SCD30_CO2:
			{
				uint32_t tmp;

				if (chn->raw_length != 4)
					goto wrongData;

				tmp = (raw_data[0]<<24) | (raw_data[1]<<16) | (raw_data[2]<<8) | raw_data[3];
				temperature = *(float*)&tmp;
				chip_fmt = TENKI_UNIT_PPM;
			}
			break;

		case USBTENKI_CHIP_SPS30_MC_PM1_0:
		case USBTENKI_CHIP_SPS30_MC_PM2_5:
		case USBTENKI_CHIP_SPS30_MC_PM4_0:
		case USBTENKI_CHIP_SPS30_MC_PM10:
			{
				uint32_t tmp;

				if (chn->raw_length != 4)
					goto wrongData;

				tmp = (raw_data[0]<<24) | (raw_data[1]<<16) | (raw_data[2]<<8) | raw_data[3];
				temperature = *(float*)&tmp;
				chip_fmt = TENKI_UNIT_uG_PER_M3;
			}
			break;
		case USBTENKI_CHIP_SPS30_NC_PM0_5:
		case USBTENKI_CHIP_SPS30_NC_PM1_0:
		case USBTENKI_CHIP_SPS30_NC_PM2_5:
		case USBTENKI_CHIP_SPS30_NC_PM4_0:
		case USBTENKI_CHIP_SPS30_NC_PM10:
			{
				uint32_t tmp;

				if (chn->raw_length != 4)
					goto wrongData;

				tmp = (raw_data[0]<<24) | (raw_data[1]<<16) | (raw_data[2]<<8) | raw_data[3];
				temperature = *(float*)&tmp;
				chip_fmt = TENKI_UNIT_COUNT_PER_CM3;
			}
			break;

		case USBTENKI_CHIP_SPS30_TYP_PART_SIZE:
			{
				uint32_t tmp;

				if (chn->raw_length != 4)
					goto wrongData;

				tmp = (raw_data[0]<<24) | (raw_data[1]<<16) | (raw_data[2]<<8) | raw_data[3];
				temperature = *(float*)&tmp;
				chip_fmt = TENKI_UNIT_MICROMETERS;
			}
			break;

		case USBTENKI_CHIP_THC_TYPE_K:
		case USBTENKI_CHIP_THC_TYPE_J:
		case USBTENKI_CHIP_THC_TYPE_T:
		case USBTENKI_CHIP_THC_TYPE_N:
		case USBTENKI_CHIP_THC_TYPE_S:
		case USBTENKI_CHIP_THC_TYPE_E:
		case USBTENKI_CHIP_THC_TYPE_B:
		case USBTENKI_CHIP_THC_TYPE_R:
		case USBTENKI_CHIP_THC_HOT:
		case USBTENKI_CHIP_THC_COLD:
			{
				int16_t t_reg;

				t_reg = raw_data[0] << 8 | raw_data[1];
				temperature = t_reg * 0.0625;
				chip_fmt = TENKI_UNIT_CELCIUS;
			}
			break;

		case USBTENKI_CHIP_RED:
		case USBTENKI_CHIP_GREEN:
		case USBTENKI_CHIP_BLUE:
		case USBTENKI_CHIP_IR:
			{
				uint32_t value;

				value = raw_data[0] | raw_data[1] << 8 | raw_data[2] << 16;
				temperature = value;
				chip_fmt = TENKI_UNIT_ARBITRARY;
			}
			break;

		case USBTENKI_CHIP_VEML6030_ALS:
			{
				uint16_t value;
				double minstep = 0.0036;
				double gain; // gain
				double it;

				if (chn->raw_length != 4) {
					goto wrongData;
				}

				value = raw_data[0] | raw_data[1] << 8;

				/*
				value = 1480;
				raw_data[2] = 3;
				raw_data[3] = 0;
				*/

				// Appnote: The resolution is most sensitive with gain = 2 and an integration
				// time of 800ms, specified to 0.0036 lx/step.


				// For each shorter integration time by half, the resolution value is doubled.
				//
				switch(raw_data[3])
				{
					case 3: it = 1.0; break; // 800 MS
					case 2: it = 2.0; break; // 400 MS
					case 1: it = 4.0; break; // 200 MS
					case 0: it = 8.0; break; // 100 MS
					case 8: it = 16.0; break; // 50 MS
					case 0xc: it = 32.0; break; // 25 MS
					default:
						goto wrongData;
				}

				// .. The same principle is valid for the gain. For gain = 1 it is
				// again doubled, and for gain = 1/4 it is four times higher, and
				// for gain = 1/8 it is again doubled.
				switch(raw_data[2])
				{
					case 0: gain = 2.0; break; // Gain 1
					case 1: gain = 1.0; break; // Gain 2
					case 3: gain = 4.0; break; // Gain 1/4
					case 2: gain = 8.0; break; // Gain 1/8
					default:
						goto wrongData;
				}

				//printf("Raw: %d, gain: %.3f, it: %.3f\n", value, gain, it);

				temperature = value  * (minstep * gain * it);
				chip_fmt = TENKI_UNIT_LUX;
			}
			break;

		case USBTENKI_CHIP_VEML6030_WHITE:
			{
				uint16_t value;

				if (chn->raw_length < 2) {
					goto wrongData;
				}

				value = raw_data[0] | raw_data[1] << 8;
				temperature = value;
				chip_fmt = TENKI_UNIT_ARBITRARY;
			}
			break;

		case USBTENKI_CHIP_VEML6075_UVA:
			{
				uint16_t value;

				if (chn->raw_length!=2) {
					goto wrongData;
				}

				value = raw_data[0] | raw_data[1] << 8;
				temperature = value * 0.93;
				chip_fmt = TENKI_UNIT_uW_PER_CM2;
			}
			break;


		case USBTENKI_CHIP_VEML6075_UVB:
			{
				uint16_t value;

				if (chn->raw_length!=2) {
					goto wrongData;
				}

				value = raw_data[0] | raw_data[1] << 8;
				temperature = value * 2.1;
				chip_fmt = TENKI_UNIT_uW_PER_CM2;
			}
			break;

		case USBTENKI_CHIP_VEML6075_UVCOMP1:
		case USBTENKI_CHIP_VEML6075_UVCOMP2:
			{
				uint16_t value;

				if (chn->raw_length!=2) {
					goto wrongData;
				}

				value = raw_data[0] | raw_data[1] << 8;
				temperature = value;
				chip_fmt = TENKI_UNIT_ARBITRARY;
			}
			break;

		default:
			{
				int i;
				temperature = raw_data[1] << 8 | raw_data[0];
				chip_fmt = TENKI_UNIT_RAW;

				printf("Unknown chip id 0x%02x\n", chn->chip_id);
				printf("HEX(%d) : ", chn->raw_length);
				for (i=0; i<chn->raw_length; i++) {
					printf("%02X ", raw_data[i]);
				}
				printf("\n");
			}

			break;
	}

	chn->converted_data = temperature;
	chn->converted_unit = chip_fmt;
	chn->status = USBTENKI_CHN_STATUS_VALID;

	return 0;

outOfRange:
	chn->status = USBTENKI_CHN_STATUS_OUT_OF_RANGE;
	return -1;

probeDisconnected:
	chn->status = USBTENKI_CHN_STATUS_PROBE_DISCONNECTED;
	return -1;

sensorError:
	chn->status = USBTENKI_CHN_STATUS_SENSOR_ERROR;
	return -1;

wrongData:
	chn->status = USBTENKI_CHN_STATUS_INVALID_DATA;
//	fprintf(stderr, "Wrong data received\n");
	return -1;
}


