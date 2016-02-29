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

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#include <linux/broadcom/bcmbt_rfkill.h>
#endif

#ifdef CONFIG_BCM_BZHW
#include <linux/broadcom/bcm_bzhw.h>
#endif

#ifdef CONFIG_BCM_BT_LPM
#include <linux/broadcom/bcmbt_lpm.h>
#endif

#if defined(CONFIG_BCMI2CNFC)
#include <linux/bcmi2cnfc.h>
#endif

#if defined(CONFIG_SENSORS_BMA2X2)
#include <linux/bma222.h>
#endif

#if defined(CONFIG_BMP18X) || defined(CONFIG_BMP18X_I2C)
	|| defined(CONFIG_BMP18X_I2C_MODULE)
#include <linux/bmp18x.h>
#include <mach/bmp18x_i2c_settings.h>
#endif

#if defined(CONFIG_AL3006) || defined(CONFIG_AL3006_MODULE)
#include <linux/al3006.h>
#include <mach/al3006_i2c_settings.h>
#endif

#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
	|| defined(CONFIG_MPU_SENSORS_MPU6050B1_MODULE)
#include <linux/mpu.h>
#include <mach/mpu6050_settings.h>
#endif

#if defined(CONFIG_MPU_SENSORS_AMI306)
	|| defined(CONFIG_MPU_SENSORS_AMI306_MODULE)
#include <linux/ami306_def.h>
#include <linux/ami_sensor.h>
#include <mach/ami306_settings.h>
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

#ifdef CONFIG_FB_BRCM_KONA
#include <video/kona_fb.h>
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

#ifdef CONFIG_I2C_GPIO
#include <linux/i2c-gpio.h>
#endif

#if defined(CONFIG_RT8969) || defined(CONFIG_RT8973)
#include <linux/mfd/bcmpmu.h>
#include <linux/power_supply.h>
#include <linux/platform_data/rtmusc.h>
#endif

#ifdef CONFIG_USB_SWITCH_TSU6111
#include <linux/mfd/bcmpmu.h>
#include <linux/power_supply.h>
#include <linux/i2c/tsu6111.h>
#endif
#if defined(CONFIG_SEC_CHARGING_FEATURE)
/* Samsung charging feature */
#include <linux/spa_power.h>
#endif

#if defined(CONFIG_TOUCHSCREEN_BCM915500)
	|| defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
#include <linux/i2c/bcm15500_i2c_ts.h>
#endif

#ifdef CONFIG_USB_DWC_OTG
#include <linux/usb/bcm_hsotgctrl.h>
#include <linux/usb/otg.h>
#endif

#ifdef CONFIG_MOBICORE_DRIVER
#include <linux/broadcom/mobicore.h>
#endif

#include <linux/i2c/tsu6111.h>

void send_usb_insert_event(enum bcmpmu_event_t event, void *para);
void send_chrgr_insert_event(enum bcmpmu_event_t event, void *para);

#ifdef CONFIG_HAS_WAKELOCK
static struct wake_lock rt8973_jig_wakelock;
#endif
#ifdef CONFIG_KONA_PI_MGR
static struct pi_mgr_qos_node qos_node;
#endif

enum cable_type_t{
		CABLE_TYPE_USB,
		CABLE_TYPE_AC,
		CABLE_TYPE_NONE
};
static void rt8973_wakelock_init(void)
{
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&rt8973_jig_wakelock, WAKE_LOCK_SUSPEND,
	"rt8973_jig_wakelock");
#endif

#ifdef CONFIG_KONA_PI_MGR
	pi_mgr_qos_add_request(&qos_node, "rt8973_jig_qos",
		PI_MGR_PI_ID_ARM_SUB_SYSTEM, PI_MGR_QOS_DEFAULT_VALUE);
#endif
}
#define FSA9485_I2C_BUS_ID 8
#define GPIO_USB_I2C_SDA 113
#define GPIO_USB_I2C_SCL 114
#define GPIO_USB_INT 56
#ifdef CONFIG_HAS_WAKELOCK
static struct wake_lock fsa9485_jig_wakelock;
#endif
#ifdef CONFIG_KONA_PI_MGR
static struct pi_mgr_qos_node qos_node;
#endif

static void fsa9485_wakelock_init(void)
{
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&fsa9485_jig_wakelock, WAKE_LOCK_SUSPEND,
		"fsa9485_jig_wakelock");
#endif

#ifdef CONFIG_KONA_PI_MGR
	pi_mgr_qos_add_request(&qos_node, "fsa9485_jig_qos",
		PI_MGR_PI_ID_ARM_SUB_SYSTEM, PI_MGR_QOS_DEFAULT_VALUE);
#endif
}

static void fsa9485_usb_cb(bool attached)
{
	pr_info("fsa9485_usb_cb attached %d\n", attached);
}
static void fsa9485_charger_cb(bool attached)
{
	pr_info("fsa9480_charger_cb attached %d\n", attached);
}
static void fsa9485_jig_cb(bool attached)
{
	pr_info("fsa9480_jig_cb attached %d\n", attached);
	if (attached) {
		#ifdef CONFIG_HAS_WAKELOCK
			if (!wake_lock_active(&fsa9485_jig_wakelock))
				wake_lock(&fsa9485_jig_wakelock);
		#endif
		#ifdef CONFIG_KONA_PI_MGR
			pi_mgr_qos_request_update(&qos_node, 0);
		#endif
	} else {
		#ifdef CONFIG_HAS_WAKELOCK
			if (wake_lock_active(&fsa9485_jig_wakelock))
				wake_unlock(&fsa9485_jig_wakelock);
		#endif
		#ifdef CONFIG_KONA_PI_MGR
			pi_mgr_qos_request_update(&qos_node,
				PI_MGR_QOS_DEFAULT_VALUE);
		#endif
	}
}
static void fsa9485_uart_cb(bool attached)
{
	pr_info("fsa9485_uart_cb attached %d\n", attached);
	if (attached) {
		#ifdef CONFIG_HAS_WAKELOCK
		if (!wake_lock_active(&fsa9485_jig_wakelock))
			wake_lock(&fsa9485_jig_wakelock);
		#endif
		#ifdef CONFIG_KONA_PI_MGR
			pi_mgr_qos_request_update(&qos_node, 0);
		#endif
	} else {
		#ifdef CONFIG_HAS_WAKELOCK
		if (wake_lock_active(&fsa9485_jig_wakelock))
			wake_unlock(&fsa9485_jig_wakelock);
		#endif
		#ifdef CONFIG_KONA_PI_MGR
			pi_mgr_qos_request_update(&qos_node,
				PI_MGR_QOS_DEFAULT_VALUE);
		#endif
		}
}

static enum cable_type_t set_cable_status;

#if defined (CONFIG_TOUCHSCREEN_IST30XX)
extern void ist30xx_set_ta_mode(bool charging);
#endif

static void usb_attach(uint8_t attached)
{
	enum bcmpmu_chrgr_type_t chrgr_type;
	enum bcmpmu_usb_type_t usb_type;

	printk (attached ? "USB attached\n" : "USB detached\n");

#if defined(CONFIG_SEC_CHARGING_FEATURE)
	int spa_data = POWER_SUPPLY_TYPE_BATTERY;
#endif
	pr_info("ftsu6111_usb_cb attached %d\n", attached);

	set_cable_status = attached ? CABLE_TYPE_USB : CABLE_TYPE_NONE;

	switch (set_cable_status) {
	case CABLE_TYPE_USB:
			usb_type = PMU_USB_TYPE_SDP;
			chrgr_type = PMU_CHRGR_TYPE_SDP;
#if defined(CONFIG_SEC_CHARGING_FEATURE)
		spa_data = POWER_SUPPLY_TYPE_USB_CDP;
#endif
#if defined (CONFIG_TOUCHSCREEN_IST30XX)
		ist30xx_set_ta_mode(1);
#endif
			pr_info("%s USB attached\n" , __func__);
		/* send_usb_insert_event(BCMPMU_USB_EVENT_USB_DETECTION,
			&usb_type); */
			break;
	case CABLE_TYPE_NONE:
			usb_type = PMU_USB_TYPE_NONE;
			chrgr_type = PMU_CHRGR_TYPE_NONE;
			spa_data = POWER_SUPPLY_TYPE_BATTERY;
#if defined (CONFIG_TOUCHSCREEN_IST30XX)
		ist30xx_set_ta_mode(0);
#endif			
			pr_info("%s USB removed\n" , __func__);
		/* set_usb_connection_status(&usb_type);  // only set status */
			break;
	}
	send_chrgr_insert_event(BCMPMU_CHRGR_EVENT_CHGR_DETECTION , &chrgr_type);
#if defined(CONFIG_SEC_CHARGING_FEATURE)
	spa_event_handler(SPA_EVT_CHARGER, spa_data);
#endif
	pr_info("tsu6111_usb_cb attached %d\n", attached);
}

extern int musb_info_handler(struct notifier_block *nb, unsigned long event, void *para);

static void uart_attach(uint8_t attached)
{
	printk(attached ? "UART attached\n" : "UART detached\n");

	pr_info("%s attached %d\n", __func__, attached);
		if (attached) {
#ifndef CONFIG_SEC_MAKE_LCD_TEST
#ifdef CONFIG_HAS_WAKELOCK
			if (!wake_lock_active(&rt8973_jig_wakelock))
			wake_lock(&rt8973_jig_wakelock);
#endif
#ifdef CONFIG_KONA_PI_MGR
			pi_mgr_qos_request_update(&qos_node, 0);
#endif
#endif
			musb_info_handler(NULL, 0, 1);	
	} else {
#ifndef CONFIG_SEC_MAKE_LCD_TEST
#ifdef CONFIG_HAS_WAKELOCK
			if (wake_lock_active(&rt8973_jig_wakelock))
			wake_unlock(&rt8973_jig_wakelock);
#endif
#ifdef CONFIG_KONA_PI_MGR
			pi_mgr_qos_request_update(&qos_node,
				PI_MGR_QOS_DEFAULT_VALUE);
#endif
#endif
			musb_info_handler(NULL, 0, 0);
		}
}

static void charger_attach(uint8_t attached)
{
	enum bcmpmu_chrgr_type_t chrgr_type;
	enum cable_type_t set_cable_status;
	printk(attached ? "Charger attached\n" : "Charger detached\n");
#if defined(CONFIG_SEC_CHARGING_FEATURE)
	int spa_data = POWER_SUPPLY_TYPE_BATTERY;
#endif
	pr_info("tsu6111_charger_cb attached %d\n", attached);
	set_cable_status = attached ? CABLE_TYPE_AC : CABLE_TYPE_NONE;
	switch (set_cable_status) {
	case CABLE_TYPE_AC:
			chrgr_type = PMU_CHRGR_TYPE_DCP;
			pr_info("%s TA attached\n", __func__);
			#if defined (CONFIG_TOUCHSCREEN_IST30XX)
				ist30xx_set_ta_mode(1);
			#endif
#if defined(CONFIG_SEC_CHARGING_FEATURE)
		spa_data = POWER_SUPPLY_TYPE_USB_DCP;
#endif
			break;
	case CABLE_TYPE_NONE:
			chrgr_type = PMU_CHRGR_TYPE_NONE;
			pr_info("%s TA removed\n", __func__);
			#if defined (CONFIG_TOUCHSCREEN_IST30XX)
				ist30xx_set_ta_mode(0);
			#endif
			break;
	default:
			break;
	}
	send_chrgr_insert_event(BCMPMU_CHRGR_EVENT_CHGR_DETECTION,
			&chrgr_type);
#if defined(CONFIG_SEC_CHARGING_FEATURE)
	spa_event_handler(SPA_EVT_CHARGER, spa_data);
#endif
	pr_info("tsu6111_charger_cb attached %d\n", attached);
}

static void jig_attach(uint8_t attached, uint8_t factory_mode)
{
	switch (factory_mode) {
	case RTMUSC_FM_BOOT_OFF_UART:
		printk("JIG BOOT OFF UART\n");
		break;
	case RTMUSC_FM_BOOT_OFF_USB:
		printk("JIG BOOT OFF USB\n");
		break;
	case RTMUSC_FM_BOOT_ON_UART:
		printk("JIG BOOT ON UART\n");
		break;
	case RTMUSC_FM_BOOT_ON_USB:
		printk("JIG BOOT ON USB\n");
		break;
	default:
		break;
		}
	pr_info("tsu6111_jig_cb attached %d\n", attached);
	if (attached) {
#ifndef CONFIG_SEC_MAKE_LCD_TEST
#ifdef CONFIG_HAS_WAKELOCK
		if (!wake_lock_active(&rt8973_jig_wakelock))
			wake_lock(&rt8973_jig_wakelock);
#endif
#ifdef CONFIG_KONA_PI_MGR
			pi_mgr_qos_request_update(&qos_node, 0);
#endif
#endif
	} else {
#ifndef CONFIG_SEC_MAKE_LCD_TEST
#ifdef CONFIG_HAS_WAKELOCK
		if (wake_lock_active(&rt8973_jig_wakelock))
			wake_unlock(&rt8973_jig_wakelock);
#endif
#ifdef CONFIG_KONA_PI_MGR
		pi_mgr_qos_request_update(&qos_node,
			PI_MGR_QOS_DEFAULT_VALUE);
#endif
#endif
	}
printk(attached ? "Jig attached\n" : "Jig detached\n");
}

static void over_temperature(uint8_t detected)
{
	printk("over temperature detected = %d!\n", detected);
}

static void over_voltage(uint8_t detected)
{
	printk("over voltage = %d\n", (int32_t)detected);
	printk("OVP triggered by musb - %d\n", detected);
	spa_event_handler(SPA_EVT_OVP, detected);
}
static void set_usb_power(uint8_t on)
{
	printk(on ? "on resume() : Set USB on\n" :
		"on suspend() : Set USB off\n");
}

void uas_jig_force_sleep_rt8973(void)
{
#ifdef CONFIG_HAS_WAKELOCK
	if (wake_lock_active(&rt8973_jig_wakelock)) {
		wake_unlock(&rt8973_jig_wakelock);
		pr_info("Force unlock jig_uart_wl\n");
	}
#else
	pr_info("Warning : %s - Empty function!!!\n", __func__);
#endif
#ifdef CONFIG_KONA_PI_MGR
	pi_mgr_qos_request_update(&qos_node, PI_MGR_QOS_DEFAULT_VALUE);
#endif
	return;
}

static struct rtmus_platform_data __initdata rtmus_pdata = {
	.usb_callback = &usb_attach,
	.uart_callback = &uart_attach,
	.charger_callback = &charger_attach,
	.jig_callback = &jig_attach,
	.over_temperature_callback = &over_temperature,
	.charging_complete_callback = NULL,
	.over_voltage_callback = &over_voltage,
	.usb_power = &set_usb_power,
};

/* For you device setting */
static struct i2c_board_info __initdata micro_usb_i2c_devices_info[]  = {
/* Add for Ricktek RT8969 */
#ifdef CONFIG_RT8973
	{I2C_BOARD_INFO("rt8973", 0x28>>1),
	.irq = -1,
	.platform_data = &rtmus_pdata,},
#endif
};

static struct i2c_gpio_platform_data mUSB_i2c_gpio_data = {
	.sda_pin        = GPIO_USB_I2C_SDA,
	.scl_pin = GPIO_USB_I2C_SCL,
	.udelay                 = 2,
	};

static struct platform_device mUSB_i2c_gpio_device = {
		.name					= "i2c-gpio",
	.id						= RT_I2C_BUS_ID,
		.dev = {
				.platform_data	= &mUSB_i2c_gpio_data,
	},
};
static struct platform_device *mUSB_i2c_devices[] __initdata = {
		&mUSB_i2c_gpio_device,
};
void __init dev_muic_rt8973_init(void)
{
	pr_info("rt8973: micro_usb_i2c_devices_info\n");
	rt8973_wakelock_init();
	i2c_register_board_info(RT_I2C_BUS_ID, micro_usb_i2c_devices_info,
		ARRAY_SIZE(micro_usb_i2c_devices_info));
	pr_info("mUSB_i2c_devices\n");
	platform_add_devices(mUSB_i2c_devices, ARRAY_SIZE(mUSB_i2c_devices));
	return;
}
