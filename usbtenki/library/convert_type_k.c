#include <math.h>
#include "usbtenki.h"
#include "convert_type_k.h"

double typeK_temp_to_mv(double t90)
{
	unsigned int i;
	double mv = 0.0;
	double a0 = 0.118597600000E+00;
	double a1 = -0.118343200000E-03;
	double a2 = 0.126968600000E+03;
	const double coefficients_above_zero[] = {
		-0.176004136860E-01,
		0.389212049750E-01,
		0.185587700320E-04,
		-0.994575928740E-07,
		0.318409457190E-09,
		-0.560728448890E-12,
		0.560750590590E-15,
		-0.320207200030E-18,
		0.971511471520E-22,
		-0.121047212750E-25
	};
	const double coefficients_below_zero[] = {
		0.000000000000E+00,
		0.394501280250E-01,
		0.236223735980E-04,
		-0.328589067840E-06,
		-0.499048287770E-08,
		-0.675090591730E-10,
		-0.574103274280E-12,
		-0.310888728940E-14,
		-0.104516093650E-16,
		-0.198892668780E-19,
		-0.163226974860E-22
	};

	if (t90 > 0) {
		for (i=0; i<ARRAY_SIZE(coefficients_above_zero); i++) {
			mv += coefficients_above_zero[i] * powf(t90, i);
		}
		mv += a0 * expf(a1 * powf(t90 - a2, 2));
	} else {
		for (i=0; i<ARRAY_SIZE(coefficients_below_zero); i++) {
			mv += coefficients_below_zero[i] * powf(t90, i);
		}
	}

	return mv;
}

double typeK_mv_to_temp(double mv)
{
	double t90 = 0;
	unsigned int i;

	// -5.891 to 0
	const double range1_d[] = {
		0.0000000E+00,
		2.5173462E+01,
		-1.1662878E+00,
		-1.0833638E+00,
		-8.9773540E-01,
		-3.7342377E-01,
		-8.6632643E-02,
		-1.0450598E-02,
		-5.1920577E-04,
		0.0000000E+00,
	};
	const double range2_d[] = {
		0.000000E+00,
		2.508355E+01,
		7.860106E-02,
		-2.503131E-01,
		8.315270E-02,
		-1.228034E-02,
		9.804036E-04,
		-4.413030E-05,
		1.057734E-06,
		-1.052755E-08
	};
	const double range3_d[] = {
		-1.318058E+02,
		4.830222E+01,
		-1.646031E+00,
		5.464731E-02,
		-9.650715E-04,
		8.802193E-06,
		-3.110810E-08,
		0.000000E+00,
		0.000000E+00,
		0.000000E+00
	};

	if (mv < 0) {
		for (i=0; i<ARRAY_SIZE(range1_d); i++) {
			t90 += range1_d[i] * powf(mv, i);
		}
	} else if (mv < 20.644) {
		for (i=0; i<ARRAY_SIZE(range2_d); i++) {
			t90 += range2_d[i] * powf(mv, i);
		}
	} else {
		for (i=0; i<ARRAY_SIZE(range3_d); i++) {
			t90 += range3_d[i] * powf(mv, i);
		}
	}

	return t90;
}


