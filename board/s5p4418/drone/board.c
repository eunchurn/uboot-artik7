/*
 * (C) Copyright 2016 Nexell
 * Hyunseok, Jung <hsjung@nexell.co.kr>
 *
 * SPDX-License-Identifier:      GPL-2.0+
 */

#include <config.h>
#include <common.h>
#ifdef CONFIG_PWM_NX
#include <pwm.h>
#endif
#include <asm/io.h>

#include <asm/arch/nexell.h>
#include <asm/arch/clk.h>
#include <asm/arch/reset.h>
#include <asm/arch/nx_gpio.h>

#ifdef CONFIG_USB_GADGET
#include <usb.h>
#include <usb/dwc2_udc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/*------------------------------------------------------------------------------
 * intialize nexell soc and board status.
 */

void serial_clock_init(void)
{
	char dev[10];
	int id;

	sprintf(dev, "nx-uart.%d", CONFIG_CONS_INDEX);
	id = RESET_ID_UART0 + CONFIG_CONS_INDEX;

	struct clk *clk = clk_get((const char *)dev);

	/* reset control: Low active ___|---   */
	nx_rstcon_setrst(id, RSTCON_ASSERT);
	udelay(10);
	nx_rstcon_setrst(id, RSTCON_NEGATE);
	udelay(10);

	/* set clock   */
	clk_disable(clk);
	clk_set_rate(clk, CONFIG_PL011_CLOCK);
	clk_enable(clk);
}

/* call from u-boot */
int board_early_init_f(void)
{
	serial_clock_init();
	return 0;
}

void board_gpio_init(void)
{
	nx_gpio_initialize();
	nx_gpio_set_base_address(0, (void *)PHY_BASEADDR_GPIOA);
	nx_gpio_set_base_address(1, (void *)PHY_BASEADDR_GPIOB);
	nx_gpio_set_base_address(2, (void *)PHY_BASEADDR_GPIOC);
	nx_gpio_set_base_address(3, (void *)PHY_BASEADDR_GPIOD);
	nx_gpio_set_base_address(4, (void *)PHY_BASEADDR_GPIOE);
}

#ifdef CONFIG_PWM_NX
void board_backlight_init(void)
{
	pwm_init(CONFIG_BACKLIGHT_CH, CONFIG_BACKLIGHT_DIV,
		 CONFIG_BACKLIGHT_INV);
	pwm_config(CONFIG_BACKLIGHT_CH, TO_DUTY_NS(CONFIG_BACKLIGHT_DUTY,
						   CONFIG_BACKLIGHT_HZ),
		   TO_PERIOD_NS(CONFIG_BACKLIGHT_HZ));
}
#endif

int mmc_get_env_dev(void)
{
	return 0;
}

int board_init(void)
{
	board_gpio_init();
#ifdef CONFIG_PWM_NX
	board_backlight_init();
#endif
	return 0;
}

/* u-boot dram initialize  */
int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

/* u-boot dram board specific */
void dram_init_banksize(void)
{
	/* set global data memory */
	gd->bd->bi_arch_number = machine_arch_type;
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x00000100;

	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size  = CONFIG_SYS_SDRAM_SIZE;
}

int board_late_init(void)
{
	return 0;
}

#ifdef CONFIG_USB_GADGET
struct dwc2_plat_otg_data s5p4418_otg_data = {
	.phy_control	= NULL,
	.regs_phy	= PHY_BASEADDR_TIEOFF,
	.regs_otg	= PHY_BASEADDR_HSOTG,
	.usb_phy_ctrl	= NULL,
	.usb_flags	= NULL,
};

int board_usb_init(int index, enum usb_init_type init)
{
	debug("USB_udc_probe\n");
	return dwc2_udc_probe(&s5p4418_otg_data);
}
#endif
