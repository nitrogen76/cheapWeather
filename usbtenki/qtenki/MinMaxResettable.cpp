#include <QDebug>
#include "MinMaxResettable.h"
#include "globals.h"

MinMaxResettable::MinMaxResettable(bool mode)
{
	minimum_mode = mode;
	empty = true;
}

MinMaxResettable::~MinMaxResettable()
{
}

void MinMaxResettable::reset(void)
{
	empty = true;
	setText("");
}

void MinMaxResettable::submitValue(float value)
{
	QString d;

	// Ignore NaN values
	if (value != value)
		return;

	if (empty) {
		cur_min = cur_max = value;
		empty = false;

		g_tenkisources->formatValue(&d, value);
		setText(d);

	} else {
		if (minimum_mode) {
			if (value < cur_min) {
				cur_min = value;

				g_tenkisources->formatValue(&d, cur_min);
				setText(d);
			}
		} else {
			if (value > cur_max) {
				cur_max = value;

				g_tenkisources->formatValue(&d, cur_max);
				setText(d);
			}
		}
	}
}

