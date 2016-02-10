/************************************************************************/
/*                                                                      */
/*  Copyright 2012  Broadcom Corporation                                */
/*                                                                      */
/* Unless you and Broadcom execute a separate written software license  */
/* agreement governing use of this software, this software is licensed  */
/* to you under the terms of the GNU General Public License version 2   */
/* (the GPL), available at						*/
/*                                                                      */
/*          http://www.broadcom.com/licenses/GPLv2.php                  */
/*                                                                      */
/*  with the following added to such license:                           */
/*                                                                      */
/*  As a special exception, the copyright holders of this software give */
/*  you permission to link this software with independent modules, and  */
/*  to copy and distribute the resulting executable under terms of your */
/*  choice, provided that you also meet, for each linked independent    */
/*  module, the terms and conditions of the license of that module. An  */
/*  independent module is a module which is not derived from this       */
/*  software.  The special   exception does not apply to any            */
/*  modifications of the software.					*/
/*									*/
/*  Notwithstanding the above, under no circumstances may you combine	*/
/*  this software in any way with any other Broadcom software provided	*/
/*  under a license other than the GPL, without Broadcom's express	*/
/*  prior written consent.						*/
/*									*/
/************************************************************************/

#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/of_platform.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <mach/pinmux.h>
#ifdef CONFIG_IOMMU_API
#include <plat/bcm_iommu.h>
#endif
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#ifdef CONFIG_ION_BCM_NO_DT
#include <linux/ion.h>
#include <linux/broadcom/bcm_ion.h>
#endif
#include <linux/serial_8250.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <linux/spi/spi.h>
#include <linux/clk.h>
#include <linux/bootmem.h>
#include <linux/input.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/i2c-kona.h>
#include <linux/i2c.h>
#include <linux/i2c/tango_ts.h>
#include <linux/i2c/melfas_ts.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <asm/mach/map.h>
#include <asm/hardware/gic.h>
#include <mach/hardware.h>
#include <mach/hardware.h>
#include <mach/kona_headset_pd.h>
#include <mach/kona.h>
#include <mach/sdio_platform.h>
#include <mach/hawaii.h>
#include <mach/io_map.h>
#include <mach/irqs.h>
#include <mach/rdb/brcm_rdb_uartb.h>
#include <mach/clock.h>
#include <plat/spi_kona.h>
#include <plat/chal/chal_trace.h>
#include <plat/pi_mgr.h>
#include <plat/spi_kona.h>

#include <trace/stm.h>

#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif

#include <mach/pinmux.h>

#include "devices.h"

#if defined(CONFIG_SPI_GPIO)
#include <linux/spi/spi_gpio.h>
#endif

#ifdef CONFIG_FB_BRCM_KONA
#include <video/kona_fb.h>
#endif

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
#include <media/soc_camera.h>
#include "../../../drivers/media/video/camdrv_ss.h"
#endif

#ifdef CONFIG_VIDEO_KONA
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/kona-unicam.h>
#endif

#ifdef CONFIG_I2C_GPIO
#include <linux/i2c-gpio.h>
#endif

#ifdef CONFIG_MOBICORE_DRIVER
#include <linux/broadcom/mobicore.h>
#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
#include "hawaii_wifi.h"

extern int hawaii_wifi_status_register(
		void (*callback)(int card_present, void *dev_id),
		void *dev_id);
#endif

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
#ifdef CONFIG_SOC_CAMERA_SR352

#define SR352_I2C_ADDRESS (0x40>>1)


#define SENSOR_0_GPIO_PWRDN             (002)
#define SENSOR_0_GPIO_RST               (111)
#define SENSOR_0_CLK                    "dig_ch0_clk" /* DCLK1 */
#define SENSOR_0_CLK_FREQ               (26000000)

#define SENSOR_1_CLK                    "dig_ch0_clk" /* DCLK1 */
#define SENSOR_1_CLK_FREQ               (26000000)

#define SENSOR_1_GPIO_PWRDN             (005)

static struct i2c_board_info hawaii_i2c_camera[] = {
	{
		I2C_BOARD_INFO("camdrv_ss", SR352_I2C_ADDRESS),
	},

};

static int hawaii_camera_power(struct device *dev, int on)
{
	static struct pi_mgr_dfs_node unicam_dfs_node;
	int ret;

	printk(KERN_INFO "%s:camera power %s, %d\n", __func__,
		(on ? "on" : "off"), unicam_dfs_node.valid);

	if (!unicam_dfs_node.valid) {
		ret = pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam",
						PI_MGR_PI_ID_MM,
						PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			printk(
			KERN_ERR "%s: failed to register PI DFS request\n",
			__func__
			);
			return -1;
		}
	}

	if (on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO)) {
			printk(
			KERN_ERR "%s:failed to update dfs request for unicam\n",
			__func__
			);
			return -1;
		}
	}

	if (!camdrv_ss_power(0, (bool)on)) {
		printk(
		KERN_ERR "%s,camdrv_ss_power failed for MAIN CAM!!\n",
		__func__
		);
		return -1;
	}

	if (!on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node,
						PI_MGR_DFS_MIN_VALUE)) {
			printk(
			KERN_ERR"%s: failed to update dfs request for unicam\n",
			__func__);
		}
	}

	return 0;
}
static int hawaii_camera_reset(struct device *dev)
{
	/* reset the camera gpio */
	printk(KERN_INFO "%s:camera reset\n", __func__);
	return 0;
}


static struct v4l2_subdev_sensor_interface_parms sr352_if_params = {
	.if_type = V4L2_SUBDEV_SENSOR_SERIAL,
	.if_mode = V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2,
	.orientation = V4L2_SUBDEV_SENSOR_ORIENT_90,
	.facing = V4L2_SUBDEV_SENSOR_BACK,
	.parms.serial = {
		 .lanes = 1,
		 .channel = 0,
		 .phy_rate = 0,
		 .pix_clk = 0,
		 .hs_term_time = 0x2,
	},
};

static struct soc_camera_link iclink_sr352 = {
	.bus_id = 0,
	.board_info = &hawaii_i2c_camera[0],
	.i2c_adapter_id = 0,
	.module_name = "camdrv_ss",
	.power = &hawaii_camera_power,
	.reset = &hawaii_camera_reset,
	.priv =  &sr352_if_params,
};

static struct platform_device hawaii_camera = {
	.name = "soc-camera-pdrv",
	 .id = 0,
	 .dev = {
		 .platform_data = &iclink_sr352,
	 },
};


#endif /* CONFIG_VIDEO_UNICAM_CAMERA */

#ifdef CONFIG_SOC_CAMERA_SR300PC20

#define SR300PC20_I2C_ADDRESS (0x40>>1)


#define SENSOR_0_GPIO_PWRDN             (002)
#define SENSOR_0_GPIO_RST               (111)
#define SENSOR_0_CLK                    "dig_ch0_clk" /* DCLK1 */
#define SENSOR_0_CLK_FREQ               (13000000)

#define SENSOR_1_CLK                    "dig_ch0_clk" /* DCLK1 */
#define SENSOR_1_CLK_FREQ               (26000000)

#define SENSOR_1_GPIO_PWRDN             (005)

static struct i2c_board_info hawaii_i2c_camera[] = {
	{
		I2C_BOARD_INFO("camdrv_ss", SR300PC20_I2C_ADDRESS),
	},

};

static int hawaii_camera_power(struct device *dev, int on)
{
	static struct pi_mgr_dfs_node unicam_dfs_node;
	int ret;

	printk(KERN_INFO "%s:camera power %s, %d\n", __func__,
		(on ? "on" : "off"), unicam_dfs_node.valid);

	if (!unicam_dfs_node.valid) {
		ret = pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam",
						PI_MGR_PI_ID_MM,
						PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			printk(
			KERN_ERR "%s: failed to register PI DFS request\n",
			__func__
			);
			return -1;
		}
	}

	if (on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO)) {
			printk(
			KERN_ERR "%s:failed to update dfs request for unicam\n",
			__func__
			);
			return -1;
		}
	}

	if (!camdrv_ss_power(0, (bool)on)) {
		printk(
		KERN_ERR "%s,camdrv_ss_power failed for MAIN CAM!!\n",
		__func__
		);
		return -1;
	}

	if (!on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node,
						PI_MGR_DFS_MIN_VALUE)) {
			printk(
			KERN_ERR"%s: failed to update dfs request for unicam\n",
			__func__);
		}
	}

	return 0;
}
static int hawaii_camera_reset(struct device *dev)
{
	/* reset the camera gpio */
	printk(KERN_INFO "%s:camera reset\n", __func__);
	return 0;
}


static struct v4l2_subdev_sensor_interface_parms sr300pc20_if_params = {
	.if_type = V4L2_SUBDEV_SENSOR_SERIAL,
	.if_mode = V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2,
	.orientation = V4L2_SUBDEV_SENSOR_ORIENT_90,
	.facing = V4L2_SUBDEV_SENSOR_BACK,
	.parms.serial = {
		 .lanes = 1,
		 .channel = 0,
		 .phy_rate = 0,
		 .pix_clk = 0,
		 .hs_term_time = 0x7,
		     .hs_settle_time = 0x5
	},
};

static struct soc_camera_link iclink_sr300pc20 = {
	.bus_id = 0,
	.board_info = &hawaii_i2c_camera[0],
	.i2c_adapter_id = 0,
	.module_name = "camdrv_ss",
	.power = &hawaii_camera_power,
	.reset = &hawaii_camera_reset,
	.priv =  &sr300pc20_if_params,
};

static struct platform_device hawaii_camera = {
	.name = "soc-camera-pdrv",
	 .id = 0,
	 .dev = {
		 .platform_data = &iclink_sr300pc20,
	 },
};

#endif

#endif /* CONFIG_VIDEO_UNICAM_CAMERA */


struct regulator_consumer_supply hv6_supply[] = {
	{.supply = "vdd_sdxc"},
	{.supply = "sddat_debug_bus"},
};

struct regulator_consumer_supply sd_supply[] = {
	{.supply = "sdldo_uc"},
	REGULATOR_SUPPLY("vddmmc", "sdhci.3"), /* 0x3f1b0000.sdhci */
	{.supply = "vdd_sdio"},
};

struct regulator_consumer_supply sdx_supply[] = {
	{.supply = "sdxldo_uc"},
	REGULATOR_SUPPLY("vddo", "sdhci.3"), /* 0x3f1b0000.sdhci */
	{.supply = "vdd_sdxc"},
};


struct platform_device *hawaii_sdio_devices[] __initdata = {
	&hawaii_sdio2_device,
	&hawaii_sdio3_device,
	&hawaii_sdio1_device,
};

#ifdef CONFIG_MOBICORE_OS
static void hawaii_mem_reserve(void)
{
	mobicore_device.dev.platform_data = &mobicore_plat_data;
	hawaii_reserve();
}
#endif

static void __init hawaii_add_sdio_devices(void)
{
		platform_add_devices(hawaii_sdio_devices,
				ARRAY_SIZE(hawaii_sdio_devices));
}

#ifdef CONFIG_FB_BRCM_KONA
/*
 * KONA FRAME BUFFER DISPLAY DRIVER PLATFORM CONFIG
 */
struct kona_fb_platform_data konafb_devices[] __initdata = {
	{
		.name = "HX8369",
		.reg_name = "cam2",
		.rst =  {
			.gpio = 22,
			.setup = 5,
			.pulse = 20,
			.hold = 10000,
			.active = false,
		},
		.vmode = true,
		.vburst = true,
		.cmnd_LP = true,
		.te_ctrl = false,
		.col_mod_i = 2,  //DISPDRV_FB_FORMAT_xRGB8888
		.col_mod_o = 2, //DISPDRV_FB_FORMAT_xRGB8888
		.width = 480,
		.height = 800,
		.fps = 60,
		.lanes = 2,
		.hs_bps = 490000000,//350000000
		.lp_bps = 8000000,
#ifdef CONFIG_IOMMU_API
		.pdev_iommu = &iommu_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
		.pdev_iovmm = &iovmm_mm_256mb_device,
#endif
	},
};

#include "kona_fb_init.c"
#endif /* #ifdef CONFIG_FB_BRCM_KONA */

static int __init hawaii_add_lateinit_devices(void)
{
	hawaii_add_sdio_devices();

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	hawaii_wlan_init();
#endif
	return 0;
}

late_initcall(hawaii_add_lateinit_devices);

extern void hawaii_init(void);
extern void hawaii_kyleve_init(void);

#ifdef CONFIG_ISDBT
extern int isdbt_dev_init(void);
#endif

static void __init hawaii_kylevess_board_init(void)
{
	hawaii_init();

#ifdef CONFIG_FB_BRCM_KONA
	konafb_init();
#endif
	hawaii_kyleve_init();
#ifdef CONFIG_ISDBT
	isdbt_dev_init();
#endif

	return;
}

MACHINE_START(HAWAII, "hawaii_ss_kylevess")
	.atag_offset = 0x100,
	.map_io = hawaii_map_io,
	.init_irq = kona_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &kona_timer,
	.init_machine = hawaii_kylevess_board_init,
#ifdef CONFIG_MOBICORE_OS
	.reserve = hawaii_mem_reserve,
#else
	.reserve = hawaii_reserve,
#endif
	.restart = hawaii_restart,
MACHINE_END
