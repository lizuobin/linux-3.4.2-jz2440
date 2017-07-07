/*
 * uda134x.h  --  UDA134x ALSA SoC Codec driver
 *
 * Copyright 2007 Dension Audio Systems Ltd.
 * Author: Zoltan Devai
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _WM8976_H
#define _WM8976_H

#include <sound/l3.h>

struct wm8976_platform_data {
	struct l3_pins l3;
	void (*power) (int);
	int model;
	/*
	  ALSA SOC usually puts the device in standby mode when it's not used
	  for sometime. If you unset is_powered_on_standby the driver will
	  turn off the ADC/DAC when this callback is invoked and turn it back
	  on when needed. Unfortunately this will result in a very light bump
	  (it can be audible only with good earphones). If this bothers you
	  set is_powered_on_standby, you will have slightly higher power
	  consumption. Please note that sending the L3 command for ADC is
	  enough to make the bump, so it doesn't make difference if you
	  completely take off power from the codec.
	*/
	int is_powered_on_standby;
};

#endif /* _UDA134X_H */
