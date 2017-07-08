/* linux/arch/arm/mach-s3c2440/mach-smdk2440.c
 *
 * Copyright (c) 2004-2005 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * http://www.fluff.org/ben/smdk2440/
 *
 * Thanks to Dimity Andric and TomTom for the loan of an SMDK2440.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <mach/regs-gpio.h>
#include <mach/regs-lcd.h>

#include <mach/idle.h>
#include <mach/fb.h>
#include <plat/iic.h>

#include <plat/s3c2410.h>
#include <plat/s3c244x.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>

#include <plat/common-smdk.h>

#include "common.h"
#include <asm/io.h>

#include <sound/s3c24xx_wm8976.h>

static struct map_desc smdk2440_iodesc[] __initdata = {
	/* ISA IO Space map (memory space selected by A24) */

	{
		.virtual	= (u32)S3C24XX_VA_ISA_WORD,
		.pfn		= __phys_to_pfn(S3C2410_CS2),
		.length		= 0x10000,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (u32)S3C24XX_VA_ISA_WORD + 0x10000,
		.pfn		= __phys_to_pfn(S3C2410_CS2 + (1<<24)),
		.length		= SZ_4M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (u32)S3C24XX_VA_ISA_BYTE,
		.pfn		= __phys_to_pfn(S3C2410_CS2),
		.length		= 0x10000,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (u32)S3C24XX_VA_ISA_BYTE + 0x10000,
		.pfn		= __phys_to_pfn(S3C2410_CS2 + (1<<24)),
		.length		= SZ_4M,
		.type		= MT_DEVICE,
	}
};

#define UCON S3C2410_UCON_DEFAULT | S3C2410_UCON_UCLK
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

static struct s3c2410_uartcfg smdk2440_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	/* IR port */
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x43,
		.ufcon	     = 0x51,
	}
};

/* LCD driver info */

static struct s3c2410fb_display smdk2440_lcd_cfg_480_272 __initdata = {

	.lcdcon5	= S3C2410_LCDCON5_FRM565 |
			  S3C2410_LCDCON5_INVVLINE |
			  S3C2410_LCDCON5_INVVFRAME |
			  S3C2410_LCDCON5_PWREN |
			  S3C2410_LCDCON5_HWSWP,
			  

    .type     = (S3C2410_LCDCON1_TFT16BPP |\
                       S3C2410_LCDCON1_TFT),


	.width		= 480,
	.height		= 272,

	.pixclock	= 111111, /* HCLK 100 MHz, divisor 9 */
	.xres		= 480,
	.yres		= 272,
	.bpp		= 16,
	/*left_margin: HFPD, 发出最后一行里最后一个象素数据之后，再过多长时间才发出HSYNC*/
	.left_margin	= 2,
	
	/*right_margin: HBPD, VSYNC之后再过多长时间才能发出第1行数据*/
	.right_margin	= 2,
	
	/* hsync_len:HSPW, HSYNC信号的脉冲宽度,*/	
	.hsync_len	= 41,

	/*upper_margin: VBPD, VSYNC之后再过多长时间才能发出第1行数据*/
	.upper_margin	= 2,

	/*lower_margin: VFPD, 发出最后一行数据之后，再过多长时间才发出VSYNC*/
	.lower_margin	= 2,

	/*vsync_len: VSPW, VSYNC信号的脉冲宽度,*/
	.vsync_len	= 10,

};

static struct s3c2410fb_display smdk2440_lcd_cfg_320_240 __initdata = {

	.lcdcon5	= S3C2410_LCDCON5_FRM565 |
			  S3C2410_LCDCON5_INVVLINE |
			  S3C2410_LCDCON5_INVVFRAME |
			  S3C2410_LCDCON5_PWREN |
			  S3C2410_LCDCON5_HWSWP,
			  

    .type     = (S3C2410_LCDCON1_TFT16BPP |\
                       S3C2410_LCDCON1_TFT),

	.width		= 240,
	.height		= 320,


	/*DCF是Dot Clk Frequency，单位是MHz，是扫描像素点的频率。计算出来的pixclock单位是ps。*/
	.pixclock	= 111111, /* HCLK 100 MHz, divisor 9 */
	.xres		= 240,
	.yres		= 320,
	.bpp		= 16,
	/*left_margin: HFPD, 发出最后一行里最后一个象素数据之后，再过多长时间才发出HSYNC*/
	.left_margin	= 2,
	
	/*right_margin: HBPD, VSYNC之后再过多长时间才能发出第1行数据*/
	.right_margin	= 2,
	
	/* hsync_len:HSPW, HSYNC信号的脉冲宽度,*/	
	.hsync_len	= 41,

	/*upper_margin: VBPD, VSYNC之后再过多长时间才能发出第1行数据*/
	.upper_margin	= 2,

	/*lower_margin: VFPD, 发出最后一行数据之后，再过多长时间才发出VSYNC*/
	.lower_margin	= 2,

	/*vsync_len: VSPW, VSYNC信号的脉冲宽度,*/
	.vsync_len	= 10,




};

static struct s3c2410fb_mach_info smdk2440_fb_info_480_272 __initdata = {
	.displays	= &smdk2440_lcd_cfg_480_272,
	.num_displays	= 1,
	.default_display = 0,

#if 0
	/* currently setup by downloader */
	.gpccon		= 0xaa940659,
	.gpccon_mask	= 0xffffffff,
	.gpcup		= 0x0000ffff,
	.gpcup_mask	= 0xffffffff,
	.gpdcon		= 0xaa84aaa0,
	.gpdcon_mask	= 0xffffffff,
	.gpdup		= 0x0000faff,
	.gpdup_mask	= 0xffffffff,
#endif

        //.lpcsel                = ((0xCE6) & ~7) | 1<<4,
        .lpcsel = 0xf82,
};

static struct s3c2410fb_mach_info smdk2440_fb_info_320_240 __initdata = {
	.displays	= &smdk2440_lcd_cfg_320_240,
	.num_displays	= 1,
	.default_display = 0,

#if 0
	/* currently setup by downloader */
	.gpccon		= 0xaa940659,
	.gpccon_mask	= 0xffffffff,
	.gpcup		= 0x0000ffff,
	.gpcup_mask	= 0xffffffff,
	.gpdcon		= 0xaa84aaa0,
	.gpdcon_mask	= 0xffffffff,
	.gpdup		= 0x0000faff,
	.gpdup_mask	= 0xffffffff,
#endif

        //.lpcsel                = ((0xCE6) & ~7) | 1<<4,
        .lpcsel = 0xf82,
};


/* AUDIO */

static struct s3c24xx_wm8976_platform_data smdk2440_audio_pins = {
	.l3_clk  = S3C2410_GPB(4),
	.l3_mode = S3C2410_GPB(2),
	.l3_data = S3C2410_GPB(3),
};

static struct platform_device smdk2440_audio = {
	.name		= "s3c24xx_wm8976",
	.id		= 0,
	.dev		= {
		.platform_data	= &smdk2440_audio_pins,
	},
};



static struct platform_device *smdk2440_devices[] __initdata = {
	&s3c_device_ohci,
	&s3c_device_lcd,
	&s3c_device_wdt,
	&s3c_device_i2c0,
	&s3c_device_iis,
	&smdk2440_audio,
	&samsung_asoc_dma,
};

static void __init smdk2440_map_io(void)
{
	s3c24xx_init_io(smdk2440_iodesc, ARRAY_SIZE(smdk2440_iodesc));
	s3c24xx_init_clocks(12000000);
	s3c24xx_init_uarts(smdk2440_uartcfgs, ARRAY_SIZE(smdk2440_uartcfgs));
}

static void __init smdk2440_machine_init(void)
{
	
#if defined(CONFIG_FB_JZ2440_3_5)

	s3c24xx_fb_set_platdata(&smdk2440_fb_info_320_240);
	
#endif

#if defined(CONFIG_FB_JZ2440_4_3)

	s3c24xx_fb_set_platdata(&smdk2440_fb_info_480_272);
	
#endif
	
	s3c_i2c0_set_platdata(NULL);
	

	platform_add_devices(smdk2440_devices, ARRAY_SIZE(smdk2440_devices));
	smdk_machine_init();

	/*支持启动背光灯和LCD_PWREN*/
	writel((readl(S3C2410_GPBCON) & ~(3)) | 1, S3C2410_GPBCON);
	writel((readl(S3C2410_GPBDAT) | 1), S3C2410_GPBDAT);
	writel((readl(S3C2410_GPGCON) | (3<<8)), S3C2410_GPGCON);
	
}

MACHINE_START(S3C2440, "SMDK2440")
	/* Maintainer: Ben Dooks <ben-linux@fluff.org> */
	.atag_offset	= 0x100,

	.init_irq	= s3c24xx_init_irq,
	.map_io		= smdk2440_map_io,
	.init_machine	= smdk2440_machine_init,
	.timer		= &s3c24xx_timer,
	.restart	= s3c244x_restart,
MACHINE_END
