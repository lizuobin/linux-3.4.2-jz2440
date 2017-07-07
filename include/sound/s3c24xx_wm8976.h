#ifndef _S3C24XX_WM8976_H_
#define _S3C24XX_WM8976_H_ 1

#include <sound/wm8976.h>

struct s3c24xx_wm8976_platform_data {
	int l3_clk;
	int l3_mode;
	int l3_data;
	void (*power) (int);
	int model;
};

#endif
