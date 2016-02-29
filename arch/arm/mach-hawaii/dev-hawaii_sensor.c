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
#include <asm/system_info.h>

#if defined  (CONFIG_SENSORS_BMC150) || (CONFIG_SENSORS_BMA2X2)
#include <linux/bst_sensor_common.h>
#endif

#if defined(CONFIG_SENSORS_GP2AP002)
#include <linux/gp2ap002_dev.h>
#endif

#if defined  (CONFIG_SENSORS_BMC150)
#define ACC_INT_GPIO_PIN  92
static struct bosch_sensor_specific bss_bma2x2 = {
	.name = "bma2x2" ,
        .place = 4,
        .irq = ACC_INT_GPIO_PIN,        
};

static struct bosch_sensor_specific bss_bmm050 = {
	.name = "bmm050" ,
        .place = 4,
};
#endif

#if defined  (CONFIG_SENSORS_BMA2X2)
#define ACC_INT_GPIO_PIN  92
static struct bosch_sensor_specific bss_bma2x2 = {
	.name = "bma2x2" ,
        .place = 4,
        .irq = ACC_INT_GPIO_PIN,        
};
#endif

#if defined(CONFIG_SENSORS_GP2AP002)
static void gp2ap002_power_onoff(bool onoff)
{                 
	if (onoff) {
            struct regulator *power_regulator = NULL;
            int ret=0;
            power_regulator = regulator_get(NULL, "tcxldo_uc");
            if (IS_ERR(power_regulator)){
                printk(KERN_ERR "[GP2A] can not get prox_regulator (SENSOR_3.3V) \n");
            } else {
                ret = regulator_set_voltage(power_regulator,3300000,3300000);
                printk(KERN_INFO "[GP2A] regulator_set_voltage : %d\n", ret);
                ret = regulator_enable(power_regulator);
                printk(KERN_INFO "[GP2A] regulator_enable : %d\n", ret);
                regulator_put(power_regulator);
                mdelay(5);
            }
	}
}

static void gp2ap002_led_onoff(bool onoff)
{            
        struct regulator *led_regulator = NULL;
        int ret=0;
            
	if (onoff) {	
                led_regulator = regulator_get(NULL, "gpldo1_uc");
                if (IS_ERR(led_regulator)){
                    printk(KERN_ERR "[GP2A] can not get prox_regulator (SENSOR_LED_3.3V) \n");
                } else {
                    ret = regulator_set_voltage(led_regulator,3300000,3300000);
                    printk(KERN_INFO "[GP2A] regulator_set_voltage : %d\n", ret);
                    ret = regulator_enable(led_regulator);
                    printk(KERN_INFO "[GP2A] regulator_enable : %d\n", ret);
                    regulator_put(led_regulator);
                    mdelay(5);
                }
	} else {
                led_regulator = regulator_get(NULL, "gpldo1_uc");
		ret = regulator_disable(led_regulator); 
                printk(KERN_INFO "[GP2A] regulator_disable : %d\n", ret);
                regulator_put(led_regulator);
	}
}

#define PROXI_INT_GPIO_PIN  89
static struct gp2ap002_platform_data gp2ap002_platform_data = {
         .power_on = gp2ap002_power_onoff,
         .led_on = gp2ap002_led_onoff,
         .irq_gpio = PROXI_INT_GPIO_PIN,
         .irq = gpio_to_irq(PROXI_INT_GPIO_PIN),
};
#endif

static struct i2c_board_info __initdata bsc3_i2c_boardinfo[] =
{
#if defined(CONFIG_SENSORS_BMC150)
        {
        	I2C_BOARD_INFO("bma2x2", 0x10),
		.platform_data = &bss_bma2x2,
        },
        
        {
        	I2C_BOARD_INFO("bmm050", 0x12),
		.platform_data = &bss_bmm050,                
        },
#endif

#if defined(CONFIG_SENSORS_BMA2X2)
        {
        	I2C_BOARD_INFO("bma2x2", 0x18),
		.platform_data = &bss_bma2x2,
        },
#endif

#if defined(CONFIG_SENSORS_GP2AP002)
        {
             I2C_BOARD_INFO("gp2ap002",0x44),
             .platform_data = &gp2ap002_platform_data,
        },
#endif
};

void __init hawaii_sensor_init()
{
	i2c_register_board_info(2, bsc3_i2c_boardinfo, ARRAY_SIZE(bsc3_i2c_boardinfo));
	return;
}