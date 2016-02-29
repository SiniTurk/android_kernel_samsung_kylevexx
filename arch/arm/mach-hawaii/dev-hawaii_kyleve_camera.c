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


#ifdef CONFIG_VIDEO_UNICAM_CAMERA
#include <media/soc_camera.h>
#include "../../../drivers/media/video/camdrv_ss.h"
#endif

#ifdef CONFIG_VIDEO_KONA
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/kona-unicam.h>
#endif

#ifdef CONFIG_VIDEO_UNICAM_CAMERA

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

struct platform_device hawaii_camera = {
	.name = "soc-camera-pdrv",
	 .id = 0,
	 .dev = {
		 .platform_data = &iclink_sr300pc20,
	 },
};

#endif /* CONFIG_VIDEO_UNICAM_CAMERA */