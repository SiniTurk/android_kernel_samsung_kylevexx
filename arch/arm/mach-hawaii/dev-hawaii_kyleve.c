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

#ifdef CONFIG_KEYBOARD_BCM
#include <mach/bcm_keypad.h>
#endif
#ifdef CONFIG_DMAC_PL330
#include <mach/irqs.h>
#include <plat/pl330-pdata.h>
#include <linux/dma-mapping.h>
#endif

#if defined(CONFIG_SPI_GPIO)
#include <linux/spi/spi_gpio.h>
#endif

#if defined(CONFIG_HAPTIC)
#include <linux/haptic.h>
#endif

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE) || defined(CONFIG_BCM_BZHW) || defined(CONFIG_BCM_BT_LPM))
#include <mach/dev-hawaii_kyleve_bt.h>
#endif

#if defined(CONFIG_BCMI2CNFC)
#include <linux/bcmi2cnfc.h>
#endif

#if defined (CONFIG_VIDEO_UNICAM_CAMERA)
#include <mach/dev-hawaii_kyleve_camera.h>
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
#include <linux/input.h>
#include <linux/gpio_keys.h>
#endif

#ifdef CONFIG_BACKLIGHT_PWM
#include <linux/pwm_backlight.h>
#endif

#ifdef CONFIG_BACKLIGHT_KTD259B
#include <linux/ktd259b_bl.h>
#endif

#ifdef CONFIG_BACKLIGHT_TPS61158
#include <linux/tps61158_pwm_bl.h>
#endif
#if defined(CONFIG_BCM_ALSA_SOUND)
#include <mach/caph_platform.h>
#include <mach/caph_settings.h>
#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
#include "hawaii_wifi.h"
#endif

/* NFC */
#ifndef CONFIG_ISDBT
#define NFC_INT	90
#define NFC_WAKE 25
#define NFC_ENABLE 100
#endif


extern struct ion_platform_data ion_system_data;
extern struct ion_platform_data ion_system_extra_data;

extern struct platform_device hawaii_camera;

#if defined(CONFIG_BCMI2CNFC)
static int bcmi2cnfc_gpio_setup(void *);
static int bcmi2cnfc_gpio_clear(void *);
static struct bcmi2cnfc_i2c_platform_data bcmi2cnfc_pdata = {
	.i2c_pdata	= {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_400K),
		SET_CLIENT_FUNC(TX_FIFO_ENABLE | RX_FIFO_ENABLE)},
	.irq_gpio = NFC_INT,
	.en_gpio = NFC_ENABLE,
	.wake_gpio = NFC_WAKE,
	.init = bcmi2cnfc_gpio_setup,
	.reset = bcmi2cnfc_gpio_clear,
};

static int bcmi2cnfc_gpio_setup(void *this)
{
	return 0;
}
static int bcmi2cnfc_gpio_clear(void *this)
{
	return 0;
}

static struct i2c_board_info __initdata bcmi2cnfc[] = {
	{
		I2C_BOARD_INFO("bcmi2cnfc", 0x1F0),
		.flags = I2C_CLIENT_TEN,
		.platform_data = (void *)&bcmi2cnfc_pdata,
		.irq = gpio_to_irq(NFC_INT),
	},
};
#endif

#ifdef CONFIG_KONA_HEADSET_MULTI_BUTTON

#define HS_IRQ		gpio_to_irq(121)
#define HSB_IRQ		BCM_INT_ID_AUXMIC_COMP2
#define HSB_REL_IRQ	BCM_INT_ID_AUXMIC_COMP2_INV


static unsigned int hawaii_button_adc_values_2_1[3][2] = {
	/* SEND/END Min, Max*/
	{0,     150},
	/* Volume Up  Min, Max*/
	{151,   310},
	/* Volue Down Min, Max*/
	{311,   610},
};
static struct kona_headset_pd hawaii_headset_data = {
	/* GPIO state read is 0 on HS insert and 1 for
	 * HS remove
	 */

	.hs_default_state = 1,
	/*
	 * Because of the presence of the resistor in the MIC_IN line.
	 * The actual ground may not be 0, but a small offset is added to it.
	 * This needs to be subtracted from the measured voltage to determine
	 * the correct value. This will vary for different HW based on the
	 * resistor values used.
	 *
	 * if there is a resistor present on this line, please measure the load
	 * value and put it here, otherwise 0.
	 *
	 */
	.phone_ref_offset = 0,

	/*
	 * Inform the driver whether there is a GPIO present on the board to
	 * detect accessory insertion/removal _OR_ should the driver use the
	 * COMP1 for the same.
	 */
	.gpio_for_accessory_detection = 1,

	.button_adc_values_low = 0,

	/*
	 * Pass the board specific button detection range
	 */
	.button_adc_values_high = hawaii_button_adc_values_2_1,

};
#endif /* CONFIG_KONA_HEADSET_MULTI_BUTTON */

/*
 * SPI board info for the slaves
 */
static struct spi_board_info spi_slave_board_info[] __initdata = {
#ifdef CONFIG_SPI_SPIDEV

  #ifdef CONFIG_ISDBT
  {
   .modalias = "fc8150_spi",  /* use spidev generic driver */
   .max_speed_hz = 6500000,  /* use max speed */
   .bus_num = 0,	  /* framework bus number */
   .chip_select = 0,  /* for each slave */
   .platform_data = NULL, /* no spi_driver specific */
   .irq = 0,	  /* IRQ for this device */
   .mode = SPI_MODE_0,  /* SPI mode 0 */
   },
  #else
	{
	 .modalias = "spidev",	/* use spidev generic driver */
	 .max_speed_hz = 13000000,	/* use max speed */
	 .bus_num = 0,		/* framework bus number */
	 .chip_select = 0,	/* for each slave */
	 .platform_data = NULL,	/* no spi_driver specific */
	 .irq = 0,		/* IRQ for this device */
	 .mode = SPI_LOOP,	/* SPI mode 0 */
	 },
#endif
#endif
};

static struct spi_kona_platform_data hawaii_ssp0_info = {
#ifdef CONFIG_DMAC_PL330
	.enable_dma = 1,
#else
	.enable_dma = 0,
#endif
	.cs_line = 1,
#ifdef CONFIG_ISDBT
	.mode = SPI_MODE_0,
#else
	.mode = SPI_LOOP | SPI_MODE_3,
#endif
};

static struct platform_device h264_device = {
	.name = "h264-device",
	.id = -1,
};

#ifdef CONFIG_BYPASS_WIFI_DEVTREE
static struct board_wifi_info brcm_wifi_data = {
	.wl_reset_gpio = 3,
	.host_wake_gpio = 74,
	.board_nvram_file = "/system/vendor/firmware/fw_wifi_nvram_4330.txt",
	.module_name = "bcm-wifi",
};
static struct platform_device board_wifi_driver_device = {

	.name = "bcm_wifi",
	.id = -1,
	.dev = {
		.platform_data = &brcm_wifi_data,
	},
};
#endif

#if defined(CONFIG_HAPTIC_SAMSUNG_PWM)
void haptic_gpio_setup(void)
{
	/* Board specific configuration like pin mux & GPIO */
}

static struct haptic_platform_data haptic_control_data = {
	/* Haptic device name: can be device-specific name like ISA1000 */
	.name = "pwm_vibra",
	/* PWM interface name to request */
	.pwm_id = 2,
	/* Invalid gpio for now, pass valid gpio number if connected */
	.gpio = ARCH_NR_GPIOS,
	.setup_pin = haptic_gpio_setup,
};

struct platform_device haptic_pwm_device = {
	.name   = "samsung_pwm_haptic",
	.id     = -1,
	.dev	=	 {	.platform_data = &haptic_control_data,}
};

#endif /* CONFIG_HAPTIC_SAMSUNG_PWM */



#ifdef CONFIG_BACKLIGHT_PWM

static struct platform_pwm_backlight_data hawaii_backlight_data = {
/* backlight */
	.pwm_id		= 2,
	.max_brightness	= 32,   /* Android calibrates to 32 levels*/
	.dft_brightness	= 32,
	.polarity	= 1,    /* Inverted polarity */
	.pwm_period_ns	= 1000000,
};

#endif /*CONFIG_BACKLIGHT_PWM */

#ifdef CONFIG_BACKLIGHT_TPS61158

static struct platform_tps61158_backlight_data bcm_tps61158_backlight_data = {
	.max_brightness = 255,
	.dft_brightness = 160,
	.ctrl_pin = 24,
};

struct platform_device hawaii_backlight_device = {
	.name           = "panel",
	.id 		= -1,
	.dev 	= {
		.platform_data  =  &bcm_tps61158_backlight_data,
	},
};

#endif /* CONFIG_BACKLIGHT_TPS61158 */
/* Remove this comment when camera data for Hawaii is updated */


#if defined(CONFIG_BCM_ALSA_SOUND)
static struct caph_platform_cfg board_caph_platform_cfg =
#ifdef HW_CFG_CAPH
HW_CFG_CAPH;
#else
{
	.aud_ctrl_plat_cfg = {
				.ext_aud_plat_cfg = {
					.ihf_ext_amp_gpio = -1,
					.dock_aud_route_gpio = -1,
					.mic_sel_aud_route_gpio = -1,
					}
				}
};
#endif

static struct platform_device board_caph_device = {
	.name = "brcm_caph_device",
	.id = -1, /*Indicates only one device */
	.dev = {
		.platform_data = &board_caph_platform_cfg,
	},
};

#endif /* CONFIG_BCM_ALSA_SOUND */

#ifdef CONFIG_BCM_SS_VIBRA
struct platform_device bcm_vibrator_device = {
	.name = "vibrator",
	.id = 0,
	.dev = {
		.platform_data = "vibldo_uc",
	},
};
#endif

#ifdef CONFIG_LCD_NT35510_SUPPORT
static struct platform_device nt35510_panel_device = {
	.name = "nt35510",
	.id = -1,
};
#endif

#ifdef CONFIG_LCD_HX8369_SUPPORT
static struct platform_device hx8369_panel_device = {
	.name = "HX8369",
	.id = -1,
};
#endif

#define GPIO_KEYS_SETTINGS { \
	{ KEY_HOME, 10, 1, "HOME", EV_KEY, 0, 64},	\
}

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
static struct gpio_keys_button board_gpio_keys[] = GPIO_KEYS_SETTINGS;

static struct gpio_keys_platform_data gpio_keys_data = {
		.nbuttons = ARRAY_SIZE(board_gpio_keys),
		.buttons = board_gpio_keys,
};

static struct platform_device board_gpio_keys_device = {
		.name = "gpio-keys",
		.id = -1,
		.dev = {
				.platform_data = &gpio_keys_data,
		},
};
#endif

#ifdef CONFIG_IOMMU_API
struct bcm_iommu_pdata iommu_mm_pdata = {
	.name        = "iommu-mm",
	.iova_begin  = 0x80000000,
	.iova_size   = 0x80000000,
	.errbuf_size = 0x1000,
	.skip_enable = 1,
};
#endif
#ifdef CONFIG_BCM_IOVMM
struct bcm_iovmm_pdata iovmm_mm_pdata = {
	.name = "iovmm-mm",
	.base = 0xc0000000,
	.size = 0x30000000,
	.order = 0,
};
struct bcm_iovmm_pdata iovmm_mm_256mb_pdata = {
	.name = "iovmm-mm-256mb",
	.base = 0xf0000000,
	.size = 0x0ff00000,
	.order = 0,
};
#endif



static struct platform_device *hawaii_kyleve_devices[] __initdata = {
#ifdef CONFIG_KEYBOARD_BCM
	&hawaii_kp_device,
#endif
#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
	&board_gpio_keys_device,
#endif
#ifdef CONFIG_KONA_HEADSET_MULTI_BUTTON
	&hawaii_headset_device,
#endif
#if	defined(CONFIG_BACKLIGHT_PWM) || defined(CONFIG_BACKLIGHT_KTD259B) || defined(CONFIG_BACKLIGHT_TPS61158)
	&hawaii_backlight_device,
#endif
#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
	&hawaii_bcmbt_rfkill_device,
#endif
#ifdef CONFIG_BCM_BZHW
	&hawaii_bcm_bzhw_device,
#endif
#ifdef CONFIG_BCM_BT_LPM
	&board_bcmbt_lpm_device,
#endif
#ifdef CONFIG_BYPASS_WIFI_DEVTREE
	&board_wifi_driver_device,
#endif
#if defined(CONFIG_BCM_ALSA_SOUND)
	&board_caph_device,
#endif
	&hawaii_ssp0_device,
#ifdef CONFIG_VIDEO_UNICAM_CAMERA
	&hawaii_camera_device,
	&hawaii_camera,
#ifdef CONFIG_SOC_SUB_CAMERA
	&hawaii_camera_sub,
#endif
#endif /* CONFIG_VIDEO_UNICAM_CAMERA */
#ifdef CONFIG_LCD_NT35510_SUPPORT
	&nt35510_panel_device,
#endif
#ifdef CONFIG_LCD_HX8369_SUPPORT
	&hx8369_panel_device,
#endif	
#ifdef CONFIG_HAWAII_MM_H264
	&h264_device,
#endif
#ifdef CONFIG_BCM_SS_VIBRA
	&bcm_vibrator_device,
#endif
};

extern void hawaii_keyboard_init(void);
extern void hawaii_sensor_init(void);
extern void hawaii_muic_init(void);
extern void hawaii_touchscreen_init(void);

void __init hawaii_kyleve_init(void)
{
	hawaii_keyboard_init();
	hawaii_ssp0_device.dev.platform_data = &hawaii_ssp0_info;
	hawaii_headset_device.dev.platform_data = &hawaii_headset_data;
#ifdef CONFIG_BACKLIGHT_PWM
	hawaii_backlight_device.dev.platform_data = &hawaii_backlight_data;
#endif


#ifdef CONFIG_IOMMU_API
	iommu_mm_device.dev.platform_data = &iommu_mm_pdata;
#endif
#ifdef CONFIG_BCM_IOVMM
	iovmm_mm_device.dev.platform_data = &iovmm_mm_pdata;
	iovmm_mm_256mb_device.dev.platform_data = &iovmm_mm_256mb_pdata;
	ion_system_device.dev.platform_data = &ion_system_data;
	ion_system_extra_device.dev.platform_data = &ion_system_extra_data;
#endif
#ifdef CONFIG_IOMMU_API
	platform_device_register(&iommu_mm_device);
#endif
#ifdef CONFIG_BCM_IOVMM
	platform_device_register(&iovmm_mm_device);
	platform_device_register(&iovmm_mm_256mb_device);
	platform_device_register(&ion_system_device);
	platform_device_register(&ion_system_extra_device);
#endif



	spi_register_board_info(spi_slave_board_info, ARRAY_SIZE(spi_slave_board_info));
	platform_add_devices(hawaii_kyleve_devices, ARRAY_SIZE(hawaii_kyleve_devices));
	hawaii_sensor_init();
	hawaii_muic_init();
	hawaii_touchscreen_init();
	return;
}
