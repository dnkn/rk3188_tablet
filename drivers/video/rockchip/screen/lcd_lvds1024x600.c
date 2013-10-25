/*
 * This Lcd Driver is for BYD 5' LCD BM800480-8545FTGE.
 * written by Michael Lin, 2010-06-18
 */

#ifndef _LCD_SKLVDS1024x600_
#define _LCD_SKLVDS1024x600_

/* Base */
//#define OUT_TYPE		SCREEN_RGB
//#define OUT_FACE		OUT_P888

#if  1//def CONFIG_RK610_LVDS
#define SCREEN_TYPE	    	SCREEN_LVDS
#define LVDS_FORMAT      	LVDS_8BIT_1
#else
#define SCREEN_TYPE	    SCREEN_RGB
#endif

#define OUT_FACE	    OUT_P888

//tcl miaozh modify
//#define OUT_CLK			50000000
#define DCLK			50000000   //64000000
//#define OUT_CLK			42000000
#define LCDC_ACLK      300000000 //312000000// 150000000     //29 lcdc axi DMA ÆµÂÊ

/* Timing */



#define H_PW			10
#define H_BP			100
#define H_VD			1024
#define H_FP			18

#define V_PW			2
#define V_BP			8
#define V_VD			600
#define V_FP			6



/* Other */
#define DCLK_POL                0
#define DEN_POL			0
#define VSYNC_POL		0
#define HSYNC_POL		0

#define SWAP_RB			0
#define SWAP_RG			0
#define SWAP_GB			0 



#define LCD_WIDTH       	153    //need modify
#define LCD_HEIGHT      	90

#endif
