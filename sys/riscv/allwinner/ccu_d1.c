/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018 Kyle Evans <kevans@FreeBSD.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/rman.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <machine/bus.h>

#include <dev/fdt/simplebus.h>

#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#include <dev/extres/clk/clk_div.h>
#include <dev/extres/clk/clk_fixed.h>
#include <dev/extres/clk/clk_mux.h>

#include <arm/allwinner/clkng/aw_ccung.h>

#include <dt-bindings/clock/sun20i-d1-ccu.h>
#include <dt-bindings/reset/sun20i-d1-ccu.h>

//~ #define HOSC "hosc"
#define HOSC "dcxo"
#define LOSC "losc"
#define IOSC "iosc"

static struct aw_ccung_reset d1_ccu_resets[] = {
	/*		  id					offset		shift */
	CCU_RESET(RST_MBUS,				0x0540,		30)
	CCU_RESET(RST_BUS_DE,			0x060c,		16)
	CCU_RESET(RST_BUS_DI,			0x062c,		16)
	CCU_RESET(RST_BUS_G2D,			0x063c,		16)
	CCU_RESET(RST_BUS_CE,			0x068c,		16)
	CCU_RESET(RST_BUS_VE,			0x069c,		16)
	CCU_RESET(RST_BUS_DMA,			0x070c,		16)
	CCU_RESET(RST_BUS_MSGBOX0,		0x071c,		16)
	CCU_RESET(RST_BUS_MSGBOX1,		0x071c,		17)
	CCU_RESET(RST_BUS_MSGBOX2,		0x071c,		18)
	CCU_RESET(RST_BUS_SPINLOCK,		0x072c,		16)
	CCU_RESET(RST_BUS_HSTIMER,		0x073c,		16)
	CCU_RESET(RST_BUS_DBG,			0x078c,		16)
	CCU_RESET(RST_BUS_PWM,			0x07ac,		16)
	CCU_RESET(RST_BUS_DRAM,			0x080c,		16)
	CCU_RESET(RST_BUS_MMC0,			0x084c,		16)
	CCU_RESET(RST_BUS_MMC1,			0x084c,		17)
	CCU_RESET(RST_BUS_MMC2,			0x084c,		18)
	CCU_RESET(RST_BUS_UART0,		0x090c,		16)
	CCU_RESET(RST_BUS_UART1,		0x090c,		17)
	CCU_RESET(RST_BUS_UART2,		0x090c,		18)
	CCU_RESET(RST_BUS_UART3,		0x090c,		19)
	CCU_RESET(RST_BUS_UART4,		0x090c,		20)
	CCU_RESET(RST_BUS_UART5,		0x090c,		21)
	CCU_RESET(RST_BUS_I2C0,			0x091c,		16)
	CCU_RESET(RST_BUS_I2C1,			0x091c,		17)
	CCU_RESET(RST_BUS_I2C2,			0x091c,		18)
	CCU_RESET(RST_BUS_I2C3,			0x091c,		19)
	CCU_RESET(RST_BUS_CAN0,			0x092c,		16)
	CCU_RESET(RST_BUS_CAN1,			0x092c,		17)
	CCU_RESET(RST_BUS_SPI0,			0x096c,		16)
	CCU_RESET(RST_BUS_SPI1,			0x096c,		17)
	CCU_RESET(RST_BUS_EMAC,			0x097c,		16)
	CCU_RESET(RST_BUS_IR_TX,		0x09cc,		16)
	CCU_RESET(RST_BUS_GPADC,		0x09ec,		16)
	CCU_RESET(RST_BUS_THS,			0x09fc,		16)
	CCU_RESET(RST_BUS_I2S0,			0x0a20,		16)
	CCU_RESET(RST_BUS_I2S1,			0x0a20,		17)
	CCU_RESET(RST_BUS_I2S2,			0x0a20,		18)
	CCU_RESET(RST_BUS_SPDIF,		0x0a2c,		16)
	CCU_RESET(RST_BUS_DMIC,			0x0a4c,		16)
	CCU_RESET(RST_BUS_AUDIO,		0x0a5c,		16)
	CCU_RESET(RST_USB_PHY0,			0x0a70,		30)
	CCU_RESET(RST_USB_PHY1,			0x0a74,		30)
	CCU_RESET(RST_BUS_OHCI0,		0x0a8c,		16)
	CCU_RESET(RST_BUS_OHCI1,		0x0a8c,		17)
	CCU_RESET(RST_BUS_EHCI0,		0x0a8c,		20)
	CCU_RESET(RST_BUS_EHCI1,		0x0a8c,		21)
	CCU_RESET(RST_BUS_OTG,			0x0a8c,		24)
	CCU_RESET(RST_BUS_LRADC,		0x0a9c,		16)
	CCU_RESET(RST_BUS_DPSS_TOP,		0x0abc,		16)
	CCU_RESET(RST_BUS_HDMI_MAIN,	0x0b1c,		16)
	CCU_RESET(RST_BUS_HDMI_SUB,		0x0b1c,		17)
	CCU_RESET(RST_BUS_MIPI_DSI,		0x0b4c,		16)
	CCU_RESET(RST_BUS_TCON_LCD0,	0x0b7c,		16)
	CCU_RESET(RST_BUS_TCON_TV,		0x0b9c,		16)
	CCU_RESET(RST_BUS_LVDS0,		0x0bac,		16)
	CCU_RESET(RST_BUS_TVE_TOP,		0x0bbc,		16)
	CCU_RESET(RST_BUS_TVE,			0x0bbc,		17)
	CCU_RESET(RST_BUS_TVD_TOP,		0x0bdc,		16)
	CCU_RESET(RST_BUS_TVD,			0x0bdc,		17)
	CCU_RESET(RST_BUS_LEDC,			0x0bfc,		16)
	CCU_RESET(RST_BUS_CSI,			0x0c1c,		16)
	CCU_RESET(RST_BUS_TPADC,		0x0c5c,		16)
	CCU_RESET(RST_DSP,				0x0c7c,		16)
	CCU_RESET(RST_BUS_DSP_CFG,		0x0c7c,		17)
	CCU_RESET(RST_BUS_DSP_DBG,		0x0c7c,		18)
	CCU_RESET(RST_BUS_RISCV_CFG,	0x0d0c,		16)
};

static struct aw_ccung_gate d1_ccu_gates[] = {
	/*		 id					name				parent		offset	shift */
	CCU_GATE(CLK_BUS_DE,		"bus-de",			"psi-ahb",	0x060c,	0)
	CCU_GATE(CLK_BUS_DI,		"bus-di",			"psi-ahb",	0x062c,	0)
	CCU_GATE(CLK_BUS_G2D,		"bus-g2d",			"psi-ahb",	0x063c,	0)
	CCU_GATE(CLK_BUS_CE,		"bus-ce",			"psi-ahb",	0x068c,	0)
	CCU_GATE(CLK_BUS_VE,		"bus-ve",			"psi-ahb",	0x069c,	0)
	CCU_GATE(CLK_BUS_DMA,		"bus-dma",			"psi-ahb",	0x070c,	0)
	CCU_GATE(CLK_BUS_MSGBOX0,	"bus-msgbox0",		"psi-ahb",	0x071c,	0)
	CCU_GATE(CLK_BUS_MSGBOX1,	"bus-msgbox1",		"psi-ahb",	0x071c,	1)
	CCU_GATE(CLK_BUS_MSGBOX2,	"bus-msgbox2",		"psi-ahb",	0x071c,	2)
	CCU_GATE(CLK_BUS_SPINLOCK,	"bus-spinlock",		"psi-ahb",	0x072c,	0)
	CCU_GATE(CLK_BUS_HSTIMER,	"bus-hstimer",		"psi-ahb",	0x073c,	0)
	CCU_GATE(CLK_AVS,			"avs",				"hosc",		0x0740,	31)
	CCU_GATE(CLK_BUS_DBG,		"bus-dbg",			"psi-ahb",	0x078c,	0)
	CCU_GATE(CLK_BUS_PWM,		"bus-pwm",			"apb0",		0x07ac,	0)
	CCU_GATE(CLK_BUS_IOMMU,		"bus-iommu",		"apb0",		0x07bc,	0)
	CCU_GATE(CLK_MBUS_DMA,		"mbus-dma",			"mbus",		0x0804,	0)
	CCU_GATE(CLK_MBUS_VE,		"mbus-ve",			"mbus",		0x0804,	1)
	CCU_GATE(CLK_MBUS_CE,		"mbus-ce",			"mbus",		0x0804,	2)
	CCU_GATE(CLK_MBUS_TVIN,		"mbus-tvin",		"mbus",		0x0804,	7)
	CCU_GATE(CLK_MBUS_CSI,		"mbus-csi",			"mbus",		0x0804,	8)
	CCU_GATE(CLK_MBUS_G2D,		"mbus-g2d",			"mbus",		0x0804,	10)
	CCU_GATE(CLK_MBUS_RISCV,	"mbus-riscv",		"mbus",		0x0804,	11)
	CCU_GATE(CLK_BUS_DRAM,		"bus-dram",			"psi-ahb",	0x080c,	0)
	CCU_GATE(CLK_BUS_MMC0,		"bus-mmc0",			"psi-ahb",	0x084c,	0)
	CCU_GATE(CLK_BUS_MMC1,		"bus-mmc1",			"psi-ahb",	0x084c,	1)
	CCU_GATE(CLK_BUS_MMC2,		"bus-mmc2",			"psi-ahb",	0x084c,	2)
	CCU_GATE(CLK_BUS_UART0,		"bus-uart0",		"apb1",		0x090c, 0)
	CCU_GATE(CLK_BUS_UART1,		"bus-uart1",		"apb1",		0x090c, 1)
	CCU_GATE(CLK_BUS_UART2,		"bus-uart2",		"apb1",		0x090c, 2)
	CCU_GATE(CLK_BUS_UART3,		"bus-uart3",		"apb1",		0x090c, 3)
	CCU_GATE(CLK_BUS_UART4,		"bus-uart4",		"apb1",		0x090c, 4)
	CCU_GATE(CLK_BUS_UART5,		"bus-uart5",		"apb1",		0x090c, 5)
	CCU_GATE(CLK_BUS_I2C0,		"bus-i2c0",			"apb1",		0x091c,	0)
	CCU_GATE(CLK_BUS_I2C1,		"bus-i2c1",			"apb1",		0x091c,	1)
	CCU_GATE(CLK_BUS_I2C2,		"bus-i2c2",			"apb1",		0x091c,	2)
	CCU_GATE(CLK_BUS_I2C3,		"bus-i2c3",			"apb1",		0x091c,	3)
	CCU_GATE(CLK_BUS_CAN0,		"bus-can0",			"apb1",		0x092c, 0)
	CCU_GATE(CLK_BUS_CAN1,		"bus-can1",			"apb1",		0x092c, 1)
	CCU_GATE(CLK_BUS_SPI0,		"bus-spi0",			"psi-ahb",	0x096c,	0)
	CCU_GATE(CLK_BUS_SPI1,		"bus-spi1",			"psi-ahb",	0x096c,	1)
	CCU_GATE(CLK_BUS_EMAC,		"bus-emac",			"psi-ahb",	0x097c,	0)
	CCU_GATE(CLK_BUS_IR_TX,		"bus-ir-tx",		"apb0",		0x09cc,	0)
	CCU_GATE(CLK_BUS_GPADC,		"bus-gpadc",		"apb0",		0x09ec,	0)
	CCU_GATE(CLK_BUS_THS,		"bus-ths",			"apb0",		0x09fc,	0)
	CCU_GATE(CLK_BUS_I2S0,		"bus-i2s0",			"apb0",		0x0a20,	0)
	CCU_GATE(CLK_BUS_I2S1,		"bus-i2s1",			"apb0",		0x0a20,	1)
	CCU_GATE(CLK_BUS_I2S2,		"bus-i2s2",			"apb0",		0x0a20,	2)
	CCU_GATE(CLK_BUS_SPDIF,		"bus-spdif",		"apb0",		0x0a28, 0)
	CCU_GATE(CLK_BUS_DMIC,		"bus-dmic",			"apb0",		0x0a4c,	0)
	CCU_GATE(CLK_BUS_AUDIO,		"bus-audio",		"apb0",		0x0a5c,	0)
	//~ CCU_GATE(CLK_BUS_OHCI0,		"bus-ohci0",		"psi-ahb",	0x0a8c,	0)
	//~ CCU_GATE(CLK_BUS_OHCI1,		"bus-ohci1",		"psi-ahb",	0x0a8c,	1)
	//~ CCU_GATE(CLK_BUS_EHCI0,		"bus-ehci0",		"psi-ahb",	0x0a8c,	4)
	//~ CCU_GATE(CLK_BUS_EHCI1,		"bus-ehci1",		"psi-ahb",	0x0a8c,	5)
	//~ CCU_GATE(CLK_BUS_OTG,		"bus-otg",			"psi-ahb",	0x0a8c,	8)
	CCU_GATE(CLK_BUS_LRADC,		"bus-lradc",		"apb0",		0x0a9c,	0)
	CCU_GATE(CLK_BUS_DPSS_TOP,	"bus-dpss-top",		"psi-ahb",	0x0abc,	0)
	/* TODO: HDMI gates */
	CCU_GATE(CLK_BUS_MIPI_DSI,	"bus-mipi-dsi",		"psi-ahb",	0x0b4c,	0)
	CCU_GATE(CLK_BUS_TCON_LCD0,	"bus-tcon-lcd0",	"psi-ahb",	0x0b7c,	0)
	CCU_GATE(CLK_BUS_TCON_TV,	"bus-tcon-tv",		"psi-ahb",	0x0b9c,	0)
	CCU_GATE(CLK_BUS_TVE_TOP,	"bus-tve-top",		"psi-ahb",	0x0bbc,	0)
	CCU_GATE(CLK_BUS_TVE,		"bus-tve",			"psi-ahb",	0x0bbc,	1)
	CCU_GATE(CLK_BUS_TVD_TOP,	"bus-tvd-top",		"psi-ahb",	0x0bdc,	0)
	CCU_GATE(CLK_BUS_TVD,		"bus-tvd",			"psi-ahb",	0x0bdc,	1)
	CCU_GATE(CLK_BUS_LEDC,		"bus-ledc",			"psi-ahb",	0x0bfc,	0)
	CCU_GATE(CLK_BUS_CSI,		"bus-csi",			"psi-ahb",	0x0c1c,	0)
	CCU_GATE(CLK_BUS_TPADC,		"bus-tpadc",		"apb0",		0x0c5c,	0)
	CCU_GATE(CLK_BUS_TZMA,		"bus-tzma",			"apb0",		0x0c6c,	0)
	CCU_GATE(CLK_BUS_DSP_CFG,	"bus-dsp-cfg",		"psi-ahb",	0x0c7c,	1)
	/* TODO: risc-v gates */
};

/* implement clocks here */
static const char *pll_cpux_parents[] = { "dcxo" };
NP_CLK(pll_cpux_clk,
	CLK_PLL_CPUX,							/* id */
	"pll_cpux",								/* name */
	pll_cpux_parents,						/* parents */
	0x0000,									/* offset */
	8, 8, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* n factor */
	0, 0, 1, AW_CLK_FACTOR_FIXED,			/* m factor (fake) */
	27,										/* gate */
	28, 1000,								/* lock, retry */
	AW_CLK_HAS_GATE | AW_CLK_HAS_LOCK);

static const char *pll_ddr0_parents[] = { "hosc" };
NKMP_CLK(pll_ddr0_clk,
	CLK_PLL_DDR0,							/* id */
	"pll_ddr0",								/* name */
	pll_ddr0_parents,						/* parents */
	0x0010,									/* offset */
	8, 8, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* n factor */
	0, 0, 1, AW_CLK_FACTOR_FIXED,			/* k factor (fake) */
	1, 1, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	0, 1, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* p factor */
	27,										/* gate */
	28, 1000,								/* lock, retry */
	AW_CLK_HAS_GATE | AW_CLK_HAS_LOCK);

static const char *pll_periph0_4x_parents[] = { "hosc" };
NKMP_CLK(pll_periph0_4x_clk,
	CLK_PLL_PERIPH0_4X,						/* id */
	"pll_periph0-4x",						/* name */
	pll_periph0_4x_parents,					/* parents */
	0x0020,									/* offset */
	8, 8, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* n factor */
	0, 0, 1, AW_CLK_FACTOR_FIXED,			/* k factor (fake) */
	1, 1, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	0, 0, 1, AW_CLK_FACTOR_FIXED,			/* p factor (fake) */
	27,										/* gate */
	28, 1000,								/* lock, retry */
	AW_CLK_HAS_GATE | AW_CLK_HAS_LOCK);

static const char *pll_periph0_2x_parents[] = { "pll_periph0-4x" };
DIV_CLK(pll_periph0_2x_clk,
	CLK_PLL_PERIPH0_2X,						/* id */
	"pll_periph0-2x",						/* name */
	pll_periph0_2x_parents,					/* parents */
	0x0020,									/* offset */
	16, 3,									/* divisor */
	0, NULL);

static const char *pll_periph0_parents[] = { "pll_periph0-4x" };
DIV_CLK(pll_periph0_clk,
	CLK_PLL_PERIPH0,						/* id */
	"pll_periph0",							/* name */
	pll_periph0_parents,					/* parents */
	0x0020,									/* offset */
	2, 1,									/* divisor */
	0, NULL);

static const char *pll_periph0_800m_parents[] = { "pll_periph0-4x" };
DIV_CLK(pll_periph0_800m_clk,
	CLK_PLL_PERIPH0_800M,					/* id */
	"pll_periph0-800m",						/* name */
	pll_periph0_800m_parents,				/* parents */
	0x0020,									/* offset */
	20, 3,									/* divisor */
	0, NULL);

static const char *pll_periph0_div3_parents[] = { "pll_periph0-4x" };
DIV_CLK(pll_periph0_div3_clk,
	CLK_PLL_PERIPH0_DIV3,					/* id */
	"pll_periph0-div3",						/* name */
	pll_periph0_div3_parents,				/* parents */
	0x0020,									/* offset */
	6, 1,									/* divisor */
	0, NULL);

static const char *pll_video0_4x_parents[] = { "hosc" };
NKMP_CLK(pll_video0_4x_clk,
	CLK_PLL_VIDEO0_4X,						/* id */
	"pll_video0-4x",						/* name */
	pll_video0_4x_parents,					/* parents */
	0x0040,									/* offset */
	8, 8, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* n factor */
	0, 0, 1, AW_CLK_FACTOR_FIXED,			/* k factor (fake) */
	1, 1, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	0, 0, 1, AW_CLK_FACTOR_FIXED,			/* p factor (fake) */
	27,										/* gate */
	28, 1000,								/* lock */
	AW_CLK_HAS_GATE | AW_CLK_HAS_LOCK);

static const char *pll_video0_2x_parents[] = { "pll_video0-4x" };
FIXED_CLK(pll_video0_2x_clk,
	CLK_PLL_VIDEO0_2X,						/* id */
	"pll_video0-2x",						/* name */
	pll_video0_2x_parents,					/* parents */
	0,										/* frequency */
	1,										/* multiplier */
	2,										/* divisor */
	0);

static const char *pll_video0_parents[] = { "pll_video0-4x" };
FIXED_CLK(pll_video0_clk,
	CLK_PLL_VIDEO0,							/* id */
	"pll_video0",							/* name */
	pll_video0_parents,						/* parents */
	0,										/* frequency */
	1,										/* multiplier */
	4,										/* divisor */
	0);

static const char *pll_video1_4x_parents[] = { "hosc" };
NKMP_CLK(pll_video1_4x_clk,
	CLK_PLL_VIDEO1_4X,						/* id */
	"pll_video1-4x",						/* name */
	pll_video1_4x_parents,					/* parents */
	0x0048,									/* offset */
	8, 8, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* n factor */
	0, 0, 1, AW_CLK_FACTOR_FIXED,			/* k factor (fake) */
	1, 1, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	0, 0, 1, AW_CLK_FACTOR_FIXED,			/* p factor (fake) */
	27,										/* gate */
	28, 1000,								/* lock */
	AW_CLK_HAS_GATE | AW_CLK_HAS_LOCK);

static const char *pll_video1_2x_parents[] = { "pll_video1-4x" };
FIXED_CLK(pll_video1_2x_clk,
	CLK_PLL_VIDEO1_2X,						/* id */
	"pll_video1-2x",						/* name */
	pll_video1_2x_parents,					/* parents */
	0,										/* frequency */
	1,										/* multiplier */
	2,										/* divisor */
	0);

static const char *pll_video1_parents[] = { "pll_video1-4x" };
FIXED_CLK(pll_video1_clk,
	CLK_PLL_VIDEO1,							/* id */
	"pll_video1",							/* name */
	pll_video1_parents,						/* parents */
	0,										/* frequency */
	1,										/* multiplier */
	4,										/* divisor */
	0);

static const char *pll_ve_parents[] = { "hosc" };
NKMP_CLK(pll_ve_clk,
	CLK_PLL_VE,								/* id */
	"pll_ve",								/* name */
	pll_ve_parents,							/* parents */
	0x0058,									/* offset */
	8, 8, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* n factor */
	0, 0, 1, AW_CLK_FACTOR_FIXED,			/* k factor (fake) */
	1, 1, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	0, 1, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* p factor */
	27,										/* gate */
	28, 1000,								/* lock, retry */
	AW_CLK_HAS_GATE | AW_CLK_HAS_LOCK);

static const char *pll_audio0_4x_parents[] = { "hosc" };
NMM_CLK(pll_audio0_4x_clk,
	CLK_PLL_AUDIO0_4X,						/* id */
	"pll_audio0-4x",						/* name */
	pll_audio0_4x_parents,					/* parents */
	0x0078,									/* offset */
	8, 8, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* n factor */
	0, 1, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m0 factor */
	1, 1, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m1 factor */
	27,										/* gate */
	28, 1000,								/* lock, retry */
	AW_CLK_HAS_GATE | AW_CLK_HAS_LOCK);

static const char *pll_audio0_2x_parents[] = { "pll_audio0-4x" };
FIXED_CLK(pll_audio0_2x_clk,
	CLK_PLL_AUDIO0_2X,						/* id */
	"pll_audio0-2x",						/* name */
	pll_audio0_2x_parents,					/* parents */
	0,										/* frequency */
	1,										/* multiplier */
	2,										/* divisor */
	0);

static const char *pll_audio0_parents[] = { "pll_audio0-4x" };
FIXED_CLK(pll_audio0_clk,
	CLK_PLL_AUDIO0,							/* id */
	"pll_audio0",							/* name */
	pll_audio0_parents,						/* parents */
	0,										/* frequency */
	1,										/* multiplier */
	2,										/* divisor */
	0);

static const char *pll_audio1_parents[] = { "hosc" };
NKMP_CLK(pll_audio1_clk,
	CLK_PLL_AUDIO1,							/* id */
	"pll_audio1",							/* name */
	pll_audio1_parents,						/* parents */
	0x0080,									/* offset */
	8, 8, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* n factor */
	0, 0, 1, AW_CLK_FACTOR_FIXED,			/* k factor (fake) */
	1, 1, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	0, 0, 1, AW_CLK_FACTOR_FIXED,			/* p factor (fake) */
	27,										/* gate */
	28, 1000,								/* lock, retry */
	AW_CLK_HAS_GATE | AW_CLK_HAS_LOCK);

static const char *pll_audio1_div2_parents[] = { "pll_audio1" };
DIV_CLK(pll_audio1_div2_clk,
	CLK_PLL_AUDIO1_DIV2,					/* id */
	"pll_audio1-div2",						/* name */
	pll_audio1_div2_parents,				/* parents */
	0x0080,									/* offset */
	16, 3,									/* p0 factor */
	0, NULL);

static const char *pll_audio1_div5_parents[] = { "pll_audio1" };
DIV_CLK(pll_audio1_div5_clk,
	CLK_PLL_AUDIO1_DIV5,					/* id */
	"pll_audio1-div5",						/* name */
	pll_audio1_div5_parents,				/* parents */
	0x0080,									/* offset */
	20, 3,									/* p1 factor */
	0, NULL);

static const char *cpux_parents[] = { "hosc", "losc", "iosc", "pll_cpux",
	"pll_periph0", "pll_periph0-2x", "pll_periph0-800m" };
MUX_CLK(cpux_clk,
	CLK_CPUX,								/* id */
	"cpux",									/* name */
	cpux_parents,							/* parents */
	0x0500,									/* offset */
	24, 3);									/* selector */

static const char *cpux_axi_parents[] = { "cpux" };
DIV_CLK(cpux_axi_clk,
	CLK_CPUX_AXI,							/* id */
	"cpux-axi",								/* name */
	cpux_axi_parents,						/* parents */
	0x0500,									/* offset */
	0, 2,									/* m factor */
	0, NULL);

static const char *cpux_apb_parents[] = { "cpux" };
DIV_CLK(cpux_apb_clk,
	CLK_CPUX_APB,							/* id */
	"cpux-apb",								/* name */
	cpux_apb_parents,						/* parents */
	0x0500,									/* offset */
	8, 2,									/* n factor */
	0, NULL);

static const char *psi_ahb_parents[] = { "hosc", "losc", "iosc",
	"pll_periph0" };
NM_CLK(psi_ahb_clk,
	CLK_PSI_AHB,							/* id */
	"psi-ahb",								/* name */
	psi_ahb_parents,						/* parents */
	0x0510,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 2, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 2,									/* mux */
	0,										/* gate */
	AW_CLK_HAS_MUX);

static const char *apb0_parents[] = { "hosc", "losc", "psi-ahb",
	"pll_periph0" };
NM_CLK(apb0_clk,
	CLK_APB0,								/* id */
	"apb0",									/* name */
	apb0_parents,							/* parents */
	0x0520,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 2,									/* mux */
	0,										/* gate */
	AW_CLK_HAS_MUX);

static const char *apb1_parents[] = { "hosc", "losc", "psi-ahb",
	"pll_periph0" };
NM_CLK(apb1_clk,
	CLK_APB1,								/* id */
	"apb1",									/* name */
	apb1_parents,							/* parents */
	0x0524,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 2,									/* mux */
	0,										/* gate */
	AW_CLK_HAS_MUX);

static const char *de_parents[] = { "pll_periph0-2x", "pll_video0-4x",
	"pll_video1-4x", "pll_audio1-div2" };
M_CLK(de_clk,
	CLK_DE,									/* id */
	"de",									/* name */
	de_parents,								/* parents */
	0x0600,									/* offset */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *di_parents[] = { "pll_periph0-2x", "pll_video0-4x",
	"pll_video1-4x", "pll_audio1-div2" };
M_CLK(di_clk,
	CLK_DI,									/* id */
	"di",									/* name */
	di_parents,								/* parents */
	0x0620,									/* offset */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *g2d_parents[] = { "pll_periph0-2x", "pll_video0-4x",
	"pll_video1-4x", "pll_audio1-div2" };
M_CLK(g2d_clk,
	CLK_G2D,								/* id */
	"g2d",									/* name */
	g2d_parents,							/* parents */
	0x0630,									/* offset */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *ce_parents[] = { "hosc", "pll_periph0-2x", "pll_periph0" };
NM_CLK(ce_clk,
	CLK_CE,									/* id */
	"ce",									/* name */
	ce_parents,								/* parents */
	0x0680,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 2,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *ve_parents[] = { "pll_ve", "pll_periph0-2x" };
M_CLK(ve_clk,
	CLK_VE,									/* id */
	"ve",									/* name */
	ve_parents,								/* parents */
	0x0690,									/* offset */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 1,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *dram_parents[] = { "pll_ddr0", "pll_audio1-div2",
	"pll_periph0-2x", "pll_periph0-800m" };
NM_CLK(dram_clk,
	CLK_DRAM,								/* id */
	"dram",									/* name */
	dram_parents,							/* parents */
	0x0800,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 2, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	0,										/* gate */
	AW_CLK_HAS_MUX);

static const char *mbus_parents[] = { "dram" };
FIXED_CLK(mbus_clk,
	CLK_MBUS,								/* id */
	"mbus",									/* name */
	mbus_parents,							/* parents */
	0,										/* frequency */
	1,										/* multiplier */
	4,										/* divisor */
	0);

static const char *mmc0_parents[] = { "hosc", "pll_periph0", "pll_periph0-2x",
	"pll_audio1-div2" };
NM_CLK(mmc0_clk,
	CLK_MMC0,								/* id */
	"mmc0",									/* name */
	mmc0_parents,							/* parents */
	0x0830,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *mmc1_parents[] = { "hosc", "pll_periph0", "pll_periph0-2x",
	"pll_audio1-div2" };
NM_CLK(mmc1_clk,
	CLK_MMC1,								/* id */
	"mmc1",									/* name */
	mmc1_parents,							/* parents */
	0x0834,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *mmc2_parents[] = { "hosc", "pll_periph0", "pll_periph0-2x",
	"pll_periph0-800m", "pll_audio1-div2" };
NM_CLK(mmc2_clk,
	CLK_MMC2,								/* id */
	"mmc2",									/* name */
	mmc2_parents,							/* parents */
	0x0838,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *spi0_parents[] = { "hosc", "pll_periph0", "pll_periph0-2x",
	"pll_audio1-div2", "pll_audio1-div5" };
NM_CLK(spi0_clk,
	CLK_SPI0,								/* id */
	"spi0",									/* name */
	spi0_parents,							/* parents */
	0x0940,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *spi1_parents[] = { "hosc", "pll_periph0", "pll_periph0-2x",
	"pll_audio1-div2", "pll_audio1-div5" };
NM_CLK(spi1_clk,
	CLK_SPI1,								/* id */
	"spi1",									/* name */
	spi1_parents,							/* parents */
	0x0944,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *emac_25m_parents[] = { "pll_periph0" };
M_CLK(emac_25m_clk,
	CLK_EMAC_25M,							/* id */
	"emac-25m",								/* name */
	emac_25m_parents,						/* parents */
	0x0970,									/* offset */
	0, 0, 24, AW_CLK_FACTOR_FIXED,			/* m factor (fake) */
	0, 0,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_REPARENT);

static const char *ir_tx_parents[] = { "hosc", "pll_periph0" };
NM_CLK(ir_tx_clk,
	CLK_IR_TX,								/* id */
	"ir-tx",								/* name */
	ir_tx_parents,							/* parents */
	0x09c0,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 2,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *i2s0_parents[] = { "pll_audio0", "pll_audio0-4x",
	"pll_audio1-div2", "pll_audio1-div5" };
NM_CLK(i2s0_clk,
	CLK_I2S0,								/* id */
	"i2s0",									/* name */
	i2s0_parents,							/* parents */
	0x0a10,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *i2s1_parents[] = { "pll_audio0", "pll_audio0-4x",
	"pll_audio1-div2", "pll_audio1-div5" };
NM_CLK(i2s1_clk,
	CLK_I2S1,                               /* id */
	"i2s1",									/* name */
	i2s1_parents,							/* parents */
	0x014,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *i2s2_parents[] = { "pll_audio0", "pll_audio0-4x",
	"pll_audio1-div2", "pll_audio1-div5" };
NM_CLK(i2s2_clk,
	CLK_I2S2,								/* id */
	"i2s2",									/* name */
	i2s2_parents,							/* parents */
	0x0a18,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *i2s2_asrc_parents[] = { "pll_audio0-4x", "pll_periph0",
	"pll_audio1-div2", "pll_audio1-div5" };
NM_CLK(i2s2_asrc_clk,
	CLK_I2S2_ASRC,							/* id */
	"i2s2_asrc",							/* name */
	i2s2_asrc_parents,						/* parents */
	0x0a1c,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *spdif_tx_parents[] = { "pll_audio0-4x", "pll_periph0",
	"pll_audio1-div2", "pll_audio1-div5" };
NM_CLK(spdif_tx_clk,
	CLK_SPDIF_TX,							/* id */
	"spdif-tx",								/* name */
	spdif_tx_parents,						/* parents */
	0x0a24,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *spdif_rx_parents[] = { "pll_periph0", "pll_audio0-div2",
	"pll_audio0-div5" };
NM_CLK(spdif_rx_clk,
	CLK_SPDIF_RX,							/* id */
	"spdif-rx",								/* name */
	spdif_rx_parents,						/* parents */
	0x0a28,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *dmic_parents[] = { "pll_audio0", "pll_audio1-div2",
	"pll_audio1-div5" };
NM_CLK(dmic_clk,
	CLK_DMIC,								/* id */
	"dmic",									/* name */
	dmic_parents,							/* parents */
	0x0a40,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *audio_dac_parents[] = { "pll_audio0", "pll_audio1-div2",
	"pll_audio1-div5" };
NM_CLK(audio_dac_clk,
	CLK_AUDIO_DAC,							/* id */
	"audio-dac",							/* name */
	audio_dac_parents,						/* parents */
	0x0a50,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *audio_adc_parents[] = { "pll_audio0", "pll_audio1-div2",
	"pll_audio1-div5" };
NM_CLK(audio_adc_clk,
	CLK_AUDIO_ADC,							/* id */
	"audio-dac",							/* name */
	audio_adc_parents,						/* parents */
	0x0a54,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

//static const char *usb_ohci_parents[] = { "pll_periph0", "hosc", "losc" };
/* TODO: complete usb0 and usb1 clocks */
/* possibly need FIXED_CLK for clock sources */
/* register 0x0a74 */

/* TODO: HDMI clocks */

static const char *mipi_dsi_parents[] = { "hosc", "pll_periph0", "pll_video0-2x",
	"pll_video1-2x", "pll_audio1-div2" };
M_CLK(mipi_dsi_clk,
	CLK_MIPI_DSI,							/* id */
	"mipi-dsi",								/* name */
	mipi_dsi_parents,						/* parents */
	0x0b24,									/* offset */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *tcon_tve_parents[] = { "pll_video0", "pll_video0-4x",
	"pll_video1", "pll_video1-4x", "pll_periph0-2x", "pll_audio1-div2" };
NM_CLK(tcon_lcd0_clk,
	CLK_TCON_LCD0,							/* id */
	"tcon-lcd0",							/* name */
	tcon_tve_parents,						/* parents */
	0x0b60,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 2,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);
NM_CLK(tcon_tv_clk,
	CLK_TCON_TV,							/* id */
	"tcon-tv",								/* name */
	tcon_tve_parents,						/* parents */
	0x0b80,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 2,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);
NM_CLK(tve_clk,
	CLK_TVE,								/* id */
	"tve",									/* name */
	tcon_tve_parents,						/* parents */
	0x0bb0,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 2,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *tvd_parents[] = { "hosc", "pll_video0", "pll_video1",
	"pll_periph0" };
M_CLK(tvd_clk,
	CLK_TVD,								/* id */
	"tvd",									/* name */
	tvd_parents,							/* parents */
	0x0bc0,									/* offset */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *ledc_parents[] = { "hosc", "pll_periph0" };
NM_CLK(ledc_clk,
	CLK_LEDC,								/* id */
	"ledc",									/* name */
	ledc_parents,							/* parents */
	0x0bf0,									/* offset */
	8, 2, 0, AW_CLK_FACTOR_POWER_OF_TWO,	/* n factor */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 1,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *csi_top_parents[] = { "pll_periph0-2x", "pll_video0-2x",
	"pll_video1-2x" };
M_CLK(csi_top_clk,
	CLK_CSI_TOP,							/* id */
	"csi-top",								/* name */
	csi_top_parents,						/* parents */
	0x0c04,									/* offset */
	0, 4, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *csi_mclk_parents[] = { "hosc", "pll_periph0", "pll_video0",
	"pll_video1", "pll_audio1-div2", "pll_audio1-div5" };
M_CLK(csi_mclk_clk,
	CLK_CSI_MCLK,							/* id */
	"csi-mclk",								/* name */
	csi_mclk_parents,						/* parents */
	0x0c08,									/* offset */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *tpadc_parents[] = { "hosc", "pll_audio0" };
M_CLK(tpadc_clk,
	CLK_TPADC,								/* id */
	"tpadc",								/* name */
	tpadc_parents,							/* parents */
	0x0c50,									/* offset */
	0, 0, 1, AW_CLK_FACTOR_FIXED,			/* m factor (fake) */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

static const char *dsp_parents[] = { "hosc", "losc", "iosc", "pll_periph0-2x",
	"pll_audio1-div2" };
M_CLK(dsp_clk,
	CLK_DSP,								/* id */
	"dsp",									/* name */
	dsp_parents,							/* parents */
	0x0c70,									/* offset */
	0, 5, 0, AW_CLK_FACTOR_ZERO_IS_ONE,		/* m factor */
	24, 3,									/* mux */
	31,										/* gate */
	AW_CLK_HAS_GATE | AW_CLK_HAS_MUX);

/* TODO: risc-v clocks */

static struct aw_ccung_clk d1_ccu_clks[] = {
	{ .type = AW_CLK_NP,	.clk.np = &pll_cpux_clk },
	{ .type = AW_CLK_NKMP,	.clk.nkmp = &pll_ddr0_clk },
	{ .type = AW_CLK_NKMP,	.clk.nkmp = &pll_periph0_4x_clk },
	{ .type = AW_CLK_DIV,	.clk.div = &pll_periph0_2x_clk },
	{ .type = AW_CLK_DIV,	.clk.div = &pll_periph0_clk },
	{ .type = AW_CLK_DIV,	.clk.div = &pll_periph0_800m_clk },
	{ .type = AW_CLK_DIV,	.clk.div = &pll_periph0_div3_clk },
	{ .type = AW_CLK_NKMP,	.clk.nkmp = &pll_video0_4x_clk },
	{ .type = AW_CLK_FIXED,	.clk.fixed = &pll_video0_2x_clk },
	{ .type = AW_CLK_FIXED,	.clk.fixed = &pll_video0_clk },
	{ .type = AW_CLK_NKMP,	.clk.nkmp = &pll_video1_4x_clk },
	{ .type = AW_CLK_FIXED,	.clk.fixed = &pll_video1_2x_clk },
	{ .type = AW_CLK_FIXED,	.clk.fixed = &pll_video1_clk },
	{ .type = AW_CLK_NKMP,	.clk.nkmp = &pll_ve_clk },
	{ .type = AW_CLK_NMM,	.clk.nmm = &pll_audio0_4x_clk },
	{ .type = AW_CLK_FIXED,	.clk.fixed = &pll_audio0_2x_clk },
	{ .type = AW_CLK_FIXED,	.clk.fixed = &pll_audio0_clk },
	{ .type = AW_CLK_NKMP,	.clk.nkmp = &pll_audio1_clk },
	{ .type = AW_CLK_DIV,	.clk.div = &pll_audio1_div2_clk },
	{ .type = AW_CLK_DIV,	.clk.div = &pll_audio1_div5_clk },
	{ .type = AW_CLK_MUX,	.clk.mux = &cpux_clk },
	{ .type = AW_CLK_DIV,	.clk.div = &cpux_axi_clk },
	{ .type = AW_CLK_DIV,	.clk.div = &cpux_apb_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &psi_ahb_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &apb0_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &apb1_clk },
	{ .type = AW_CLK_M,		.clk.m = &de_clk },
	{ .type = AW_CLK_M,		.clk.m = &di_clk },
	{ .type = AW_CLK_M,		.clk.m = &g2d_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &ce_clk },
	{ .type = AW_CLK_M,		.clk.m = &ve_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &dram_clk },
	{ .type = AW_CLK_FIXED,	.clk.fixed = &mbus_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &mmc0_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &mmc1_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &mmc2_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &spi0_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &spi1_clk },
	{ .type = AW_CLK_M,		.clk.m = &emac_25m_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &ir_tx_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &i2s0_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &i2s1_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &i2s2_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &i2s2_asrc_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &spdif_tx_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &spdif_rx_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &dmic_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &audio_dac_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &audio_adc_clk },
	{ .type = AW_CLK_M,		.clk.m = &mipi_dsi_clk },
	/* TODO: HDMI clocks */
	{ .type = AW_CLK_NM,	.clk.nm = &tcon_lcd0_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &tcon_tv_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &tve_clk },
	{ .type = AW_CLK_M,		.clk.m = &tvd_clk },
	{ .type = AW_CLK_NM,	.clk.nm = &ledc_clk },
	{ .type = AW_CLK_M,		.clk.m = &csi_top_clk },
	{ .type = AW_CLK_M,		.clk.m = &csi_mclk_clk },
	{ .type = AW_CLK_M,		.clk.m = &tpadc_clk },
	{ .type = AW_CLK_M,		.clk.m = &dsp_clk }
	/* TODO: risc-v clocks */
};

static struct aw_clk_init d1_init_clks[] = { };

static struct ofw_compat_data compat_data[] = {
	{ "allwinner,sun20i-d1-ccu", 1 },
	{ NULL, 0 }
};

static int
ccu_d1_probe(device_t dev)
{
	if (!ofw_bus_status_okay(dev))
		return ENXIO;

	if (ofw_bus_search_compatible(dev, compat_data)->ocd_data == 0)
		return ENXIO;

	device_set_desc(dev, "Allwinner D1/R528/T113 Clock Control Unit NG");
	return BUS_PROBE_DEFAULT;
}

static int
ccu_d1_attach(device_t dev)
{
	struct aw_ccung_softc *sc;

	sc = device_get_softc(dev);

	sc->resets = d1_ccu_resets;
	sc->nresets = nitems(d1_ccu_resets);
	sc->gates = d1_ccu_gates;
	sc->ngates = nitems(d1_ccu_gates);
	sc->clks = d1_ccu_clks;
	sc->nclks = nitems(d1_ccu_clks);
	sc->clk_init = d1_init_clks;
	sc->n_clk_init = nitems(d1_init_clks);

	return aw_ccung_attach(dev);
}

static device_method_t ccu_d1_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		ccu_d1_probe),
	DEVMETHOD(device_attach,	ccu_d1_attach),

	DEVMETHOD_END
};

DEFINE_CLASS_1(ccu_d1, ccu_d1_driver, ccu_d1_methods,
	sizeof(struct aw_ccung_softc), aw_ccung_driver);

EARLY_DRIVER_MODULE(ccu_d1, simplebus, ccu_d1_driver, 0, 0,
	BUS_PASS_RESOURCE + BUS_PASS_ORDER_MIDDLE);
