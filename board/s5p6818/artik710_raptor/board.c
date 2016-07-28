/*
 * (C) Copyright 2016 Nexell
 * Hyunseok, Jung <hsjung@nexell.co.kr>
 *
 * SPDX-License-Identifier:      GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>

#include <asm/arch/nexell.h>
#include <asm/arch/nx_gpio.h>

#ifdef CONFIG_DM_PMIC_NXE2000
#include <dm.h>
#include <dm/uclass-internal.h>
#include <power/pmic.h>
#include <power/nxe2000.h>
#endif

#ifdef CONFIG_USB_GADGET
#include <usb.h>
#include <usb/dwc2_udc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_REVISION_TAG
u32 board_rev;

u32 get_board_rev(void)
{
	return board_rev;
}

static void check_hw_revision(void)
{
	u32 val = 0;

	val |= nx_gpio_get_input_value(4, 6);
	val <<= 1;

	val |= nx_gpio_get_input_value(4, 5);
	val <<= 1;

	val |= nx_gpio_get_input_value(4, 4);

	board_rev = val;
}

static void set_board_rev(u32 revision)
{
	char info[64] = {0, };

	snprintf(info, ARRAY_SIZE(info), "%d", revision);
	setenv("board_rev", info);
}
#endif

/*------------------------------------------------------------------------------
 * intialize nexell soc and board status.
 */

static void nx_phy_init(void)
{
	/* Set interrupt config */
	nx_gpio_set_pad_function(0, 11, 0);
	nx_gpio_set_output_enable(0, 11, 1);
	nx_gpio_set_pad_function(0, 12, 0);	/* GPIO */

	/* PHY reset */
	nx_gpio_set_output_value(0, 12, 1);
	nx_gpio_set_output_enable(0, 12, 1);

	/* ALT FUNCTION - GMAC */
	nx_gpio_set_pad_function(4, 7, 1);
	nx_gpio_set_pad_function(4, 8, 1);
	nx_gpio_set_pad_function(4, 9, 1);
	nx_gpio_set_pad_function(4, 10, 1);
	nx_gpio_set_pad_function(4, 11, 1);
	nx_gpio_set_pad_function(4, 14, 1);
	nx_gpio_set_pad_function(4, 15, 1);
	nx_gpio_set_pad_function(4, 16, 1);
	nx_gpio_set_pad_function(4, 17, 1);
	nx_gpio_set_pad_function(4, 18, 1);
	nx_gpio_set_pad_function(4, 19, 1);
	nx_gpio_set_pad_function(4, 20, 1);
	nx_gpio_set_pad_function(4, 21, 1);
	nx_gpio_set_pad_function(4, 24, 1);

	/* PAD STRENGTH */
	nx_gpio_set_drive_strength(4, 7, 3);
	nx_gpio_set_drive_strength(4, 8, 3);
	nx_gpio_set_drive_strength(4, 9, 3);
	nx_gpio_set_drive_strength(4, 10, 3);
	nx_gpio_set_drive_strength(4, 11, 3);
	nx_gpio_set_drive_strength(4, 14, 3);
	nx_gpio_set_drive_strength(4, 15, 3);
	nx_gpio_set_drive_strength(4, 16, 3);
	nx_gpio_set_drive_strength(4, 17, 3);
	nx_gpio_set_drive_strength(4, 18, 0);	/* RX clk */
	nx_gpio_set_drive_strength(4, 19, 3);
	nx_gpio_set_drive_strength(4, 20, 3);
	nx_gpio_set_drive_strength(4, 21, 3);
	nx_gpio_set_drive_strength(4, 24, 3);	/* TX clk */
}

#ifdef CONFIG_USB_EHCI_EXYNOS
void board_ehci_power_en(void)
{
	/* ehci host power */
	nx_gpio_set_pad_function(0, 16, 0);     /* GPIO */
	nx_gpio_set_output_value(0, 16, 1);
	nx_gpio_set_output_enable(0, 16, 1);
}
#endif

/* call from u-boot */
int board_early_init_f(void)
{
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

#ifdef CONFIG_VIDEO_NX_LVDS
void board_display_reset(void)
{
	nx_gpio_set_pad_function(4, 3, 0);	/* E_3 : LVDS RESET */
	nx_gpio_set_output_enable(4, 3, 0);
	mdelay(1);
	nx_gpio_set_output_enable(4, 3, 1);
}
#endif

int mmc_get_env_dev(void)
{
	int port_num;
	int boot_mode = readl(PHY_BASEADDR_CLKPWR + SYSRSTCONFIG);

	if ((boot_mode & BOOTMODE_MASK) == BOOTMODE_SDMMC) {
		port_num = readl(SCR_ARM_SECOND_BOOT_REG1);

		if (port_num == EMMC_PORT_NUM)
			return 0;
		else if (port_num == SD_PORT_NUM)
			return 1;
	} else if ((boot_mode & BOOTMODE_MASK) == BOOTMODE_USB) {
		return 0;
	}

	return -1;
}

int board_init(void)
{
	board_gpio_init();

#ifdef CONFIG_REVISION_TAG
	check_hw_revision();
	printf("HW Revision:\t%d\n", board_rev);

#endif

	nx_phy_init();
#ifdef CONFIG_USB_EHCI_EXYNOS
	board_ehci_power_en();
#endif

#ifdef CONFIG_VIDEO_NX_LVDS
	board_display_reset();
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

#ifdef CONFIG_DM_PMIC_NXE2000
void pmic_init(void)
{
	static struct udevice *dev;
	int ret = -ENODEV;
	uint8_t bit_mask = 0;

#ifdef CONFIG_REVISION_TAG
	if (get_board_rev() >= 3) {
		ret = pmic_get("nxe2000_gpio@32", &dev);
		if (ret)
			printf("Can't get PMIC: %s!\n", "nxe2000_gpio@32");
	} else {
#endif
		ret = pmic_get("nxe2000@32", &dev);
		if (ret)
			printf("Can't get PMIC: %s!\n", "nxe2000@32");
#ifdef CONFIG_REVISION_TAG
	}
#endif

	bit_mask = 0x00;
	ret = pmic_reg_write(dev, (u32)NXE2000_REG_BANKSEL, (u32)bit_mask);
	if (ret)
		printf("Can't write PMIC register: %d!\n", NXE2000_REG_BANKSEL);

	bit_mask = ((0 << NXE2000_POS_DCxCTL2_DCxOSC) |
			(0 << NXE2000_POS_DCxCTL2_DCxSR) |
			(3 << NXE2000_POS_DCxCTL2_DCxLIM) |
			(0 << NXE2000_POS_DCxCTL2_DCxLIMSDEN));
	ret = pmic_write(dev, NXE2000_REG_DC1CTL2, &bit_mask, 1);
	if (ret)
		printf("Can't write PMIC register: %d!\n", NXE2000_REG_DC1CTL2);

	bit_mask = ((0 << NXE2000_POS_DCxCTL2_DCxOSC) |
			(0 << NXE2000_POS_DCxCTL2_DCxSR) |
			(3 << NXE2000_POS_DCxCTL2_DCxLIM) |
			(0 << NXE2000_POS_DCxCTL2_DCxLIMSDEN));
	ret = pmic_write(dev, NXE2000_REG_DC2CTL2, &bit_mask, 1);
	if (ret)
		printf("Can't write PMIC register: %d!\n", NXE2000_REG_DC2CTL2);

	bit_mask = ((0 << NXE2000_POS_DCxCTL2_DCxOSC) |
			(0 << NXE2000_POS_DCxCTL2_DCxSR) |
			(1 << NXE2000_POS_DCxCTL2_DCxLIM) |
			(1 << NXE2000_POS_DCxCTL2_DCxLIMSDEN));
	ret = pmic_write(dev, NXE2000_REG_DC3CTL2, &bit_mask, 1);
	if (ret)
		printf("Can't write PMIC register: %d!\n", NXE2000_REG_DC3CTL2);

	bit_mask = ((0 << NXE2000_POS_DCxCTL2_DCxOSC) |
			(0 << NXE2000_POS_DCxCTL2_DCxSR) |
			(1 << NXE2000_POS_DCxCTL2_DCxLIM) |
			(1 << NXE2000_POS_DCxCTL2_DCxLIMSDEN));
	ret = pmic_write(dev, NXE2000_REG_DC4CTL2, &bit_mask, 1);
	if (ret)
		printf("Can't write PMIC register: %d!\n", NXE2000_REG_DC4CTL2);

	bit_mask = ((0 << NXE2000_POS_DCxCTL2_DCxOSC) |
			(0 << NXE2000_POS_DCxCTL2_DCxSR) |
			(1 << NXE2000_POS_DCxCTL2_DCxLIM) |
			(1 << NXE2000_POS_DCxCTL2_DCxLIMSDEN));
	ret = pmic_write(dev, NXE2000_REG_DC5CTL2, &bit_mask, 1);
	if (ret)
		printf("Can't write PMIC register: %d!\n", NXE2000_REG_DC5CTL2);

	bit_mask = (1 << NXE2000_POS_CHGCTL1_SUSPEND);
	ret = pmic_write(dev, NXE2000_REG_CHGCTL1, &bit_mask, 1);
	if (ret)
		printf("Can't write PMIC register: %d!\n", NXE2000_REG_CHGCTL1);
}
#endif

int board_late_init(void)
{
#ifdef CONFIG_DM_PMIC_NXE2000
	pmic_init();
#endif
#ifdef CONFIG_REVISION_TAG
	set_board_rev(board_rev);
#endif
#ifdef CONFIG_CMD_FACTORY_INFO
	run_command("run factory_load", 0);
#endif
	return 0;
}

#ifdef CONFIG_USB_GADGET
struct dwc2_plat_otg_data s5p6818_otg_data = {
	.phy_control	= NULL,
	.regs_phy	= PHY_BASEADDR_TIEOFF,
	.regs_otg	= PHY_BASEADDR_HSOTG,
	.usb_phy_ctrl	= NULL,
	.usb_flags	= NULL,
};

int board_usb_init(int index, enum usb_init_type init)
{
	debug("USB_udc_probe\n");
	return dwc2_udc_probe(&s5p6818_otg_data);
}
#endif
