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

#ifdef CONFIG_DMAC_PL330
#include <mach/irqs.h>
#include <plat/pl330-pdata.h>
#include <linux/dma-mapping.h>
#endif

#if defined(CONFIG_HAPTIC)
#include <linux/haptic.h>
#endif


#if defined(CONFIG_BCM_ALSA_SOUND)
#include <mach/caph_platform.h>
#include <mach/caph_settings.h>
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

#ifdef CONFIG_WD_TAPPER
#include <linux/broadcom/wd-tapper.h>
#endif

#ifdef CONFIG_USB_DWC_OTG
#include <linux/usb/bcm_hsotgctrl.h>
#include <linux/usb/otg.h>
#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
#include "hawaii_wifi.h"

extern int hawaii_wifi_status_register(
		void (*callback)(int card_present, void *dev_id),
		void *dev_id);
#endif


int reset_pwm_padcntrl(void)
{
	struct pin_config new_pin_config;
	int ret;
	new_pin_config.name = PN_GPIO24;
	new_pin_config.func = PF_GPIO24;
	ret = pinmux_set_pin_config(&new_pin_config);
	return ret;
}

#ifdef CONFIG_ANDROID_PMEM
struct android_pmem_platform_data android_pmem_data = {
	.name = "pmem",
	.cmasize = 0,
	.carveout_base = 0,
	.carveout_size = 0,
};
#endif

#ifdef CONFIG_ION_BCM_NO_DT
struct ion_platform_data ion_system_data = {
	.nr = 1,
#ifdef CONFIG_IOMMU_API
	.pdev_iommu = &iommu_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
	.pdev_iovmm = &iovmm_mm_256mb_device,
#endif
	.heaps = {
		[0] = {
			.id    = 0,
			.type  = ION_HEAP_TYPE_SYSTEM,
			.name  = "ion-system",
			.base  = 0,
			.limit = 0,
			.size  = 0,
		},
	},
};
struct ion_platform_data ion_system_extra_data = {
	.nr = 1,
#ifdef CONFIG_IOMMU_API
	.pdev_iommu = &iommu_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
	.pdev_iovmm = &iovmm_mm_device,
#endif
	.heaps = {
		[0] = {
			.id    = 1,
			.type  = ION_HEAP_TYPE_SYSTEM,
			.name  = "ion-system-extra",
			.base  = 0,
			.limit = 0,
			.size  = 0,
		},
	},
};

struct ion_platform_data ion_carveout_data = {
	.nr = 2,
#ifdef CONFIG_IOMMU_API
	.pdev_iommu = &iommu_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
	.pdev_iovmm = &iovmm_mm_256mb_device,
#endif
	.heaps = {
		[0] = {
			.id    = 3,
			.type  = ION_HEAP_TYPE_CARVEOUT,
			.name  = "ion-carveout",
			.base  = 0x90000000,
			.limit = 0xa0000000,
			.size  = (20 * SZ_1M),
#ifdef CONFIG_ION_OOM_KILLER
			.lmk_enable = 0,
			.lmk_min_score_adj = 411,
			.lmk_min_free = 32,
#endif
		},
		[1] = {
			.id    = 1,
			.type  = ION_HEAP_TYPE_CARVEOUT,
			.name  = "ion-carveout-extra",
			.base  = 0,
			.limit = 0xa0000000,
			.size  = (0 * SZ_1M),
#ifdef CONFIG_ION_OOM_KILLER
			.lmk_enable = 0,
			.lmk_min_score_adj = 411,
			.lmk_min_free = 32,
#endif
		},
	},
};

#ifdef CONFIG_CMA
struct ion_platform_data ion_cma_data = {
	.nr = 2,
#ifdef CONFIG_IOMMU_API
	.pdev_iommu = &iommu_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
	.pdev_iovmm = &iovmm_mm_256mb_device,
#endif
	.heaps = {
		[0] = {
			.id = 2,
			.type  = ION_HEAP_TYPE_DMA,
			.name  = "ion-cma",
			.base  = 0x90000000,
			.limit = 0xa0000000,
			.size  = (0 * SZ_1M),
#ifdef CONFIG_ION_OOM_KILLER
			.lmk_enable = 1,
			.lmk_min_score_adj = 411,
			.lmk_min_free = 32,
#endif
		},
		[1] = {
			.id = 0,
			.type  = ION_HEAP_TYPE_DMA,
			.name  = "ion-cma-extra",
			.base  = 0x00000000,
			.limit = 0xa0000000,
			.size  = (0 * SZ_1M),
#ifdef CONFIG_ION_OOM_KILLER
			.lmk_enable = 1,
			.lmk_min_score_adj = 411,
			.lmk_min_free = 32,
#endif
		},
	},
};
#endif /* CONFIG_CMA */
#endif /* CONFIG_ION_BCM_NO_DT */

#define KONA_UART0_PA   UARTB_BASE_ADDR
#define KONA_UART1_PA   UARTB2_BASE_ADDR
#define KONA_UART2_PA   UARTB3_BASE_ADDR


#define HAWAII_8250PORT(name, clk, freq, uart_name, power_save_en)\
{								\
	.membase    = (void __iomem *)(KONA_##name##_VA),	\
	.mapbase    = (resource_size_t)(KONA_##name##_PA),	\
	.irq        = BCM_INT_ID_##name,			\
	.uartclk    = freq,					\
	.regshift   = 2,					\
	.iotype     = UPIO_MEM32,				\
	.type       = PORT_16550A,				\
	.flags      = UPF_BOOT_AUTOCONF | UPF_BUG_THRE |	\
			UPF_FIXED_TYPE | UPF_SKIP_TEST,		\
	.private_data = power_save_en,				\
	.clk_name = clk,					\
	.port_name = uart_name,					\
}

/*This flag is added for saving power for UART GPS. If you want to save power
 * pass the address of this flag as a parameter to power_save_en*/
static bool power_save_enable = 1;


static struct plat_serial8250_port hawaii_uart_platform_data[] = {
	HAWAII_8250PORT(UART0, UARTB_PERI_CLK_NAME_STR, 48000000,
					"bluetooth", NULL),
	HAWAII_8250PORT(UART1, UARTB2_PERI_CLK_NAME_STR, 26000000,
				"gps", &power_save_enable),
	HAWAII_8250PORT(UART2, UARTB3_PERI_CLK_NAME_STR, 26000000,
				"console", NULL),
	{
		.flags = 0,
	},
};

#if defined (CONFIG_TOUCHKEY_BACKLIGHT)
static struct platform_device touchkeyled_device = {
	.name 		= "touchkey-led",
	.id 		= -1,
};
#endif

#ifdef CONFIG_WD_TAPPER
static struct wd_tapper_platform_data wd_tapper_data = {
	/* Set the count to the time equivalent to the time-out in seconds
	 * required to pet the PMU watchdog to overcome the problem of reset in
	 * suspend*/
	.count = 300,
	.ch_num = 1,
	.name = "aon-timer",
};

static struct platform_device wd_tapper = {
	.name = "wd_tapper",
	.id = 0,
	.dev = {
		.platform_data = &wd_tapper_data,
	},
};
#endif

static struct platform_device *hawaii_devices[] __initdata = {

#ifdef CONFIG_DMAC_PL330
	&hawaii_pl330_dmac_device,
#endif

#ifdef CONFIG_HAPTIC_SAMSUNG_PWM
	&haptic_pwm_device,
#endif

#ifdef CONFIG_VIDEO_KONA
	&hawaii_unicam_device,
#endif

#ifdef CONFIG_WD_TAPPER
	&wd_tapper,
#endif

#if defined(CONFIG_TOUCHKEY_BACKLIGHT)
	&touchkeyled_device,
#endif
	&hawaii_serial_device,
	&hawaii_i2c_adap_devices[0],
	&hawaii_i2c_adap_devices[1],
	&hawaii_i2c_adap_devices[2],
	&hawaii_i2c_adap_devices[3],
	&hawaii_i2c_adap_devices[4],
	&pmu_device,
	&hawaii_pwm_device,
#ifdef CONFIG_SENSORS_KONA
	&thermal_device,
#endif

#ifdef CONFIG_STM_TRACE
/*	&hawaii_stm_device, */
#endif

#if defined(CONFIG_HW_RANDOM_KONA)
	&rng_device,
#endif

#if defined(CONFIG_USB_DWC_OTG)
	&hawaii_usb_phy_platform_device,
	&hawaii_hsotgctrl_platform_device,
	&hawaii_otg_platform_device,
#endif

#ifdef CONFIG_KONA_AVS
	&avs_device,
#endif

#ifdef CONFIG_KONA_CPU_FREQ_DRV
	&kona_cpufreq_device,
#endif

#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_HASH
	&hawaii_spum_device,
#endif

#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_AES
	&hawaii_spum_aes_device,
#endif

#ifdef CONFIG_UNICAM
	&hawaii_unicam_device,
#endif

#ifdef CONFIG_SND_BCM_SOC
	&hawaii_audio_device,
	&caph_i2s_device,
	&caph_pcm_device,
	&spdif_dit_device,

#endif

#ifdef CONFIG_KONA_MEMC
	&kona_memc_device,
#endif
#ifdef CONFIG_ION_BCM_NO_DT
	&ion_carveout_device,
#ifdef CONFIG_CMA
	&ion_cma_device,
#endif
#endif
#ifdef CONFIG_ANDROID_PMEM
	&android_pmem,
#endif
};

static struct bsc_adap_cfg bsc_i2c_cfg[] = {
	{
		.speed = BSC_BUS_SPEED_400K,
		.dynamic_speed = 1,
		.bsc_clk = "bsc1_clk",
		.bsc_apb_clk = "bsc1_apb_clk",
		.retries = 1,
		.is_pmu_i2c = false,
		.fs_ref = BSC_BUS_REF_13MHZ,
		.hs_ref = BSC_BUS_REF_104MHZ,
	},

	{
		.speed = BSC_BUS_SPEED_400K,
		.dynamic_speed = 1,
		.bsc_clk = "bsc2_clk",
		.bsc_apb_clk = "bsc2_apb_clk",
		.retries = 3,
		.is_pmu_i2c = false,
		.fs_ref = BSC_BUS_REF_13MHZ,
		.hs_ref = BSC_BUS_REF_104MHZ,
	},

	{
		.speed = BSC_BUS_SPEED_400K,
		.dynamic_speed = 1,
		.bsc_clk = "bsc3_clk",
		.bsc_apb_clk = "bsc3_apb_clk",
		.retries = 1,
		.is_pmu_i2c = false,
		.fs_ref = BSC_BUS_REF_13MHZ,
		.hs_ref = BSC_BUS_REF_104MHZ,
	},

	{
		.speed = BSC_BUS_SPEED_400K,
		.dynamic_speed = 1,
		.bsc_clk = "bsc4_clk",
		.bsc_apb_clk = "bsc4_apb_clk",
		.retries = 1,
		.is_pmu_i2c = false,
		.fs_ref = BSC_BUS_REF_13MHZ,
		.hs_ref = BSC_BUS_REF_104MHZ,
	},

	{
#if defined(CONFIG_KONA_PMU_BSC_HS_MODE)
		.speed = BSC_BUS_SPEED_HS,
		/* No dynamic speed in HS mode */
		.dynamic_speed = 0,
		/*
		 * PMU can NAK certain I2C read commands, while write
		 * is in progress; and it takes a while to synchronise
		 * writes between HS clock domain(3.25MHz) and
		 * internal clock domains (32k). In such cases, we retry
		 * PMU reads until the writes are through. PMU need more
		 * retry counts in HS mode to handle this.
		 */
		.retries = 5,
#elif defined(CONFIG_KONA_PMU_BSC_HS_1MHZ)
		.speed = BSC_BUS_SPEED_HS_1MHZ,
		.dynamic_speed = 0,
		.retries = 5,
#elif defined(CONFIG_KONA_PMU_BSC_HS_1625KHZ)
		.speed = BSC_BUS_SPEED_HS_1625KHZ,
		.dynamic_speed = 0,
		.retries = 5,
#else
		.speed = BSC_BUS_SPEED_50K,
		.dynamic_speed = 1,
		.retries = 3,
#endif
		.bsc_clk = "pmu_bsc_clk",
		.bsc_apb_clk = "pmu_bsc_apb",
		.is_pmu_i2c = true,
		.fs_ref = BSC_BUS_REF_13MHZ,
		.hs_ref = BSC_BUS_REF_26MHZ,
	 },
};

/* __weak int musb_info_handler(struct notifier_block *nb,
	unsigned long event, void *para)
{
	return 0;
}
 */

/* SD */
#ifdef CONFIG_ISDBT
#define SD_CARDDET_GPIO_PIN	44
#else
#define SD_CARDDET_GPIO_PIN	91
#endif

static struct sdio_platform_cfg hawaii_sdio_param[] = {
		{
				.id = 0,
				.data_pullup = 0,
				.cd_gpio = SD_CARDDET_GPIO_PIN,
				.devtype = SDIO_DEV_TYPE_SDMMC,
				.flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE,
				.peri_clk_name = "sdio1_clk",
				.ahb_clk_name = "sdio1_ahb_clk",
				.sleep_clk_name = "sdio1_sleep_clk",
				.peri_clk_rate = 48000000,
				/*The SD card regulator*/
				.vddo_regulator_name = "vdd_sdio",
				/*The SD controller regulator*/
				.vddsdxc_regulator_name = "vdd_sdxc",
				.configure_sdio_pullup = configure_sdio_pullup,
		},
		{
				.id = 1,
				.data_pullup = 0,
				.is_8bit = 1,
				.devtype = SDIO_DEV_TYPE_EMMC,
				.flags = KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE ,
				.peri_clk_name = "sdio2_clk",
				.ahb_clk_name = "sdio2_ahb_clk",
				.sleep_clk_name = "sdio2_sleep_clk",
				.peri_clk_rate = 52000000,
		},
		{
				.id = 2,
				.data_pullup = 0,
				.devtype = SDIO_DEV_TYPE_WIFI,
				.flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE,
				.peri_clk_name = "sdio3_clk",
				.ahb_clk_name = "sdio3_ahb_clk",
				.sleep_clk_name = "sdio3_sleep_clk",
				.peri_clk_rate = 48000000,
#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
				.wifi_gpio = {
						.reset			= 3,
						.reg			= -1,
						.host_wake		= 74,
						.shutdown		= -1,
				},
		.register_status_notify = hawaii_wifi_status_register,
#endif
		},
};

static struct spi_kona_platform_data hawaii_ssp1_info = {
#ifdef CONFIG_DMAC_PL330
	.enable_dma = 1,
#else
	.enable_dma = 0,
#endif
};

#ifdef CONFIG_STM_TRACE
static struct stm_platform_data hawaii_stm_pdata = {
	.regs_phys_base = STM_BASE_ADDR,
	.channels_phys_base = SWSTM_BASE_ADDR,
	.id_mask = 0x0,		/* Skip ID check/match */
	.final_funnel = CHAL_TRACE_FIN_FUNNEL,
};
#endif

#ifdef CONFIG_DMAC_PL330
static struct kona_pl330_data hawaii_pl330_pdata =	{
	/* Non Secure DMAC virtual base address */
	.dmac_ns_base = KONA_DMAC_NS_VA,
	/* Secure DMAC virtual base address */
	.dmac_s_base = KONA_DMAC_S_VA,
	/* # of PL330 dmac channels 'configurable' */
	.num_pl330_chans = 8,
	/* irq number to use */
	.irq_base = BCM_INT_ID_DMAC0,
	/* # of PL330 Interrupt lines connected to GIC */
	.irq_line_count = 8,
};
#endif

#if defined(CONFIG_USB_DWC_OTG)
static struct bcm_hsotgctrl_platform_data hsotgctrl_plat_data = {
	.hsotgctrl_virtual_mem_base = KONA_USB_HSOTG_CTRL_VA,
	.chipreg_virtual_mem_base = KONA_CHIPREG_VA,
	.irq = BCM_INT_ID_HSOTG_WAKEUP,
	.usb_ahb_clk_name = USB_OTG_AHB_BUS_CLK_NAME_STR,
	.mdio_mstr_clk_name = MDIOMASTER_PERI_CLK_NAME_STR,
};
#endif

static void hawaii_add_pdata(void)
{
	hawaii_serial_device.dev.platform_data = &hawaii_uart_platform_data;
	hawaii_i2c_adap_devices[0].dev.platform_data = &bsc_i2c_cfg[0];
	hawaii_i2c_adap_devices[1].dev.platform_data = &bsc_i2c_cfg[1];
	hawaii_i2c_adap_devices[2].dev.platform_data = &bsc_i2c_cfg[2];
	hawaii_i2c_adap_devices[3].dev.platform_data = &bsc_i2c_cfg[3];
	hawaii_i2c_adap_devices[4].dev.platform_data = &bsc_i2c_cfg[4];
	hawaii_sdio1_device.dev.platform_data = &hawaii_sdio_param[0];
	hawaii_sdio2_device.dev.platform_data = &hawaii_sdio_param[1];
	hawaii_sdio3_device.dev.platform_data = &hawaii_sdio_param[2];
	hawaii_ssp1_device.dev.platform_data = &hawaii_ssp1_info;
	hawaii_stm_device.dev.platform_data = &hawaii_stm_pdata;
	hawaii_pl330_dmac_device.dev.platform_data = &hawaii_pl330_pdata;
#ifdef CONFIG_USB_DWC_OTG
	hawaii_hsotgctrl_platform_device.dev.platform_data = &hsotgctrl_plat_data;
	hawaii_usb_phy_platform_device.dev.platform_data = &hsotgctrl_plat_data;
#endif
}

void __init hawaii_init(void)
{
	hawaii_add_pdata();
	platform_add_devices(hawaii_devices, ARRAY_SIZE(hawaii_devices));
	return;
}

