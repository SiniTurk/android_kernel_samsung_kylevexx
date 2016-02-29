#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/gpio.h>
#include <linux/platform_data/rtmusc.h>
#include <linux/wakelock.h>
#include <linux/power_supply.h>


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

#ifdef CONFIG_I2C_GPIO
#include <linux/i2c-gpio.h>
#endif

#if defined(CONFIG_RT8969) || defined(CONFIG_RT8973)
#include <linux/mfd/bcmpmu.h>
#include <linux/power_supply.h>
#include <linux/platform_data/rtmusc.h>
#include <mach/dev-muic_rt8973.h>
#endif

#ifdef CONFIG_USB_SWITCH_TSU6111
#include <linux/mfd/bcmpmu.h>
#include <linux/power_supply.h>
#include <linux/i2c/tsu6111.h>
#include <mach/dev-muic_tsu6111.h>
#endif

#include <asm/system_info.h>


extern unsigned int system_rev;

#ifdef CONFIG_USB_SWITCH_TSU6111
int use_muic_tsu6111;
#endif

#ifdef CONFIG_RT8973
extern int use_muic_rt8973;
#endif






void check_musb_chip(void)
{
	printk(KERN_INFO "niconico in : %s\n", __func__);
	printk(KERN_INFO "niconico previously system_rev Revision: %u\n", system_rev);

#ifdef CONFIG_MACH_HAWAII_SS_KYLEVE_REV00
	system_rev = 0x0;
#endif

#ifdef CONFIG_MACH_HAWAII_SS_LOGANDS_REV01
	system_rev = 0x1;
#endif
#ifdef CONFIG_MACH_HAWAII_SS_LOGANDS_REV00
	system_rev = 0x0;
#endif
#ifdef CONFIG_MACH_HAWAII_SS_JBTLP_REV00
	system_rev = 0x0;
#endif

	printk(KERN_INFO "niconico system_rev Revision: %u\n", system_rev);
	if (system_rev == 1) {
#ifdef CONFIG_MACH_HAWAII_SS_LOGANDS
#ifdef CONFIG_RT8973
		printk(KERN_INFO "niconico LoganDS01 RT8973 MUIC\n");
		use_muic_rt8973 = 1;
#endif
#endif
	} else if (system_rev == 0) {
#ifdef CONFIG_MACH_HAWAII_SS_LOGANDS
#ifdef CONFIG_USB_SWITCH_TSU6111
		printk(KERN_INFO "niconico LoganDS00 TSU6111 MUIC\n");
		use_muic_tsu6111 = 1;
#endif
#endif
#ifdef CONFIG_MACH_HAWAII_SS_KYLEVE
#ifdef CONFIG_RT8973
		printk(KERN_INFO "niconico KyleVE00 RT8973  MUIC\n");
		use_muic_rt8973 = 1;
#endif
#endif
#ifdef CONFIG_MACH_HAWAII_SS_KYLEVESS
#ifdef CONFIG_RT8973
		printk(KERN_INFO "niconico KyleVESS00 RT8973  MUIC\n");
		use_muic_rt8973 = 1;
#endif
#endif
#ifdef CONFIG_MACH_HAWAII_SS_JBTLP
#ifdef CONFIG_RT8973
		printk(KERN_INFO "niconico JBTLP00 RT8973  MUIC\n");
		use_muic_rt8973 = 1;
#endif
#endif
	} else {
		printk(KERN_INFO "niconico not 00 nor 01 Please Check this line\n");
#ifdef CONFIG_RT8973
		use_muic_rt8973 = 1;
#endif
	}
}

void __init hawaii_muic_init(void)
{
	printk(KERN_INFO "niconico in : %s\n", __func__);
	check_musb_chip();
#ifdef CONFIG_USB_SWITCH_TSU6111
	if (use_muic_tsu6111 == 1) {
		if (system_rev == 0) {
			printk(KERN_INFO "niconico LoganDS 00 TSU6111 Please Check this\n");
			/* micro_usb_i2c_devices_info[0].addr = 0x4A >> 1;  */
		}
		/* dev_muic_tsu6111_init(); */
	}
#endif

#ifdef CONFIG_RT8973
	if (use_muic_rt8973 == 1)
		dev_muic_rt8973_init();
#endif
	return;
}

void uas_jig_force_sleep(void)
{
#ifdef CONFIG_USB_SWITCH_TSU6111
	if (use_muic_tsu6111 == 1)
		uas_jig_force_sleep_tsu6111();
#endif

#ifdef CONFIG_RT8973
	if (use_muic_rt8973 == 1)
		printk(KERN_INFO "niconico in : %s\n", __func__);
		uas_jig_force_sleep_rt8973();
#endif
	return;
}
EXPORT_SYMBOL(uas_jig_force_sleep);

/* for external charger detection  apart from PMU/BB*/
int bcm_ext_bc_status(void)
{
#ifdef CONFIG_USB_SWITCH_TSU6111
	if (use_muic_tsu6111 == 1)
		return bcm_ext_bc_status_tsu6111();
#endif

#ifdef CONFIG_RT8973
	if (use_muic_rt8973 == 1)
		printk(KERN_INFO "niconico in : %s\n", __func__);
		return bcm_ext_bc_status_rt8973();
#endif
	return 0;
}
EXPORT_SYMBOL(bcm_ext_bc_status);

void musb_vbus_changed(int state)
{
#ifdef CONFIG_USB_SWITCH_TSU6111
	if (use_muic_tsu6111 == 1)
		return musb_vbus_changed_tsu6111(state);
#endif

#ifdef CONFIG_RT8973
	if (use_muic_rt8973 == 1)
		printk(KERN_INFO "niconico in : %s\n", __func__);
		return musb_vbus_changed_rt8973(state);
#endif
}
EXPORT_SYMBOL(musb_vbus_changed);

unsigned int musb_get_charger_type(void)
{
#ifdef CONFIG_USB_SWITCH_TSU6111
	if (use_muic_tsu6111 == 1)
		return musb_get_charger_type_tsu6111();
#endif

#ifdef CONFIG_RT8973
	if (use_muic_rt8973 == 1)
		printk(KERN_INFO "niconico in : %s\n", __func__);
		return musb_get_charger_type_rt8973();
#endif

	return 0;
}
EXPORT_SYMBOL(musb_get_charger_type);
