/*****************************************************************************
*  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rtc.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include "rtc-core.h"

#ifdef CONFIG_RTC_AUTO_PWRON
#include <linux/reboot.h>
#include <linux/workqueue.h>
#endif

static int dbg_mask = BCMPMU_PRINT_ERROR |
		BCMPMU_PRINT_INIT | BCMPMU_PRINT_DATA;

#define SEC_YEAR_BASE 			13  /* 2013 */

static void bcmpmu_rtc_time_fixup(struct device *dev);
#define pr_rtc(debug_level, args...) \
	do { \
		if (dbg_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)
static ssize_t
bcmpmu_rtc_show_dbgmsk(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	return snprintf(buf, 5, "%x\n", dbg_mask);
}
static ssize_t
bcmpmu_rtc_set_dbsmsk(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val > 0xFF || val == 0)
		return -EINVAL;
	dbg_mask = val;
	return n;
}
static DEVICE_ATTR(dbgmask, S_IRUGO | S_IWUSR,
		bcmpmu_rtc_show_dbgmsk, bcmpmu_rtc_set_dbsmsk);

struct bcmpmu_rtc {
	struct rtc_device *rtc;
	struct bcmpmu59xxx *bcmpmu;
	wait_queue_head_t wait;
	struct mutex lock;
	int alarm_irq_enabled;
	int update_irq_enabled;
};

#ifdef CONFIG_BCM_RTC_CAL
static bool rtc_cal_run;
extern int bcm_rtc_cal_read_time(struct bcmpmu_rtc *rdata, struct rtc_time *tm);
extern int bcm_rtc_cal_set_time(struct bcmpmu_rtc *rdata, struct rtc_time *tm);
extern void bcm_rtc_cal_init(struct bcmpmu_rtc *rdata);
extern void bcm_rtc_cal_shutdown(void);
#endif /* CONFIG_BCM_RTC_CAL*/

#ifdef CONFIG_RTC_AUTO_PWRON
static void bcmpmu_check_alarm_lpm(struct work_struct *work);
static int bcmpmu_read_time(struct device *dev, struct rtc_time *tm);
static int bcmpmu_read_alarm(struct device *dev, struct rtc_wkalrm *alarm);
extern void request_suspend_state(suspend_state_t new_state);
u32 lpcharging_boot_mode;
#define BOOT_MODE_LPM			1
#define PRE_ONE_MINUTE          	1 /* boot up 1 minute ahead before alarm time */

struct rtc_wkalrm *current_alarm_time = NULL;
struct rtc_time *current_rtc_time = NULL;
struct platform_device *info_autopower_rtc = NULL;
struct delayed_work pollling_work_alarmboot;
static int 	bcm_rtc_sleep = 0; 
static int poweron_alarm = 0; 
static struct rtc_wkalrm poweron_tm; 
extern void emergency_restart(void);
extern void arm_machine_restart(char mode, const char *cmd);
extern void kernel_restart(char *cmd);
EXPORT_SYMBOL(lpcharging_boot_mode);

static __init int setup_boot_mode(char *opt)
{
	lpcharging_boot_mode = (u32) memparse(opt, &opt);
	return 0;
}
__setup("lpcharge=", setup_boot_mode);

static void bcmpmu_check_alarm_lpm(struct work_struct *work)
{
	bcmpmu_read_time(&info_autopower_rtc->dev,current_rtc_time);
	pr_info( "%s:  time=%d.%d.%d.%d.%d.%d\n",
		__func__, 
		current_rtc_time->tm_year,current_rtc_time->tm_mon,current_rtc_time->tm_mday,
		current_rtc_time->tm_hour,current_rtc_time->tm_min,current_rtc_time->tm_sec);

	if( (current_rtc_time->tm_year==current_alarm_time->time.tm_year)
		&& (current_rtc_time->tm_mon==current_alarm_time->time.tm_mon)
		&& (current_rtc_time->tm_mday==current_alarm_time->time.tm_mday)
		&& (current_rtc_time->tm_hour==current_alarm_time->time.tm_hour)
		&& (current_rtc_time->tm_min==(current_alarm_time->time.tm_min-PRE_ONE_MINUTE )) )
	{
		kfree(current_rtc_time);
		kfree(current_alarm_time);
		pr_info("%s: auto power on alarm occurs at lpm charging\n", __func__);
		request_suspend_state(PM_SUSPEND_ON);
		kernel_restart(NULL);
	}
	schedule_delayed_work(&pollling_work_alarmboot, 3000);
}

#endif /* CONFIG_RTC_AUTO_PWRON */

static void bcmpmu_rtc_isr(enum bcmpmu59xxx_irq irq, void *data)
{
	struct bcmpmu_rtc *rdata = data;

#ifdef CONFIG_RTC_AUTO_PWRON
	if ( bcm_rtc_sleep == 0 ) {
		poweron_alarm = 0; 
	}
#endif
	
	switch (irq) {
	case PMU_IRQ_RTC_ALARM:
		rtc_update_irq(rdata->rtc, 1, RTC_IRQF | RTC_AF);
		pr_rtc(FLOW, "%s: RTC interrupt Alarm\n", __func__);
		break;
	case PMU_IRQ_RTC_SEC:
		rtc_update_irq(rdata->rtc, 1, RTC_IRQF | RTC_UF);
		pr_rtc(FLOW, "%s: RTC interrupt Sec\n", __func__);
		break;
	default:
		break;
	}
}

static int bcmpmu_alarm_irq_enable(struct device *dev,
		unsigned int enabled)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret = 0;
	if (enabled)
		ret = rdata->bcmpmu->unmask_irq(rdata->bcmpmu,
							PMU_IRQ_RTC_ALARM);
	else
		ret = rdata->bcmpmu->mask_irq(rdata->bcmpmu,
						PMU_IRQ_RTC_ALARM);
	if (unlikely(ret))
		goto err;
	rdata->alarm_irq_enabled = enabled;
err:
	return ret;
}

static int bcmpmu_read_time(struct device *dev, struct rtc_time *tm)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret = 0;
	u8 val;

	mutex_lock(&rdata->lock);
#ifdef CONFIG_BCM_RTC_CAL
	if (rtc_cal_run == false) {
		pr_rtc(DATA, "%s: rtc_cal not ready\n", __func__);
		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu,
						PMU_REG_RTCYR, &val);
		if (unlikely(ret))
			goto err;
		tm->tm_year = val + 100;

		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu,
						PMU_REG_RTCMT_WD, &val);
		if (unlikely(ret))
			goto err;
		if (val >= 1)
			tm->tm_mon = val - 1;

		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu,
						PMU_REG_RTCDT, &val);
		if (unlikely(ret))
			goto err;
		tm->tm_mday = val;

		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCHR,
						&val);
		if (unlikely(ret))
			goto err;
		tm->tm_hour = val;

		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMN,
						&val);
		if (unlikely(ret))
			goto err;
		tm->tm_min = val;

		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCSC,
						&val);
		if (unlikely(ret))
			goto err;
		tm->tm_sec = val;

		ret = rtc_valid_tm(tm);

		pr_rtc(DATA, "%s: err=%d time=%d.%d.%d.%d.%d.%d\n",
			__func__, ret, tm->tm_year, tm->tm_mon,
			tm->tm_mday, tm->tm_hour, tm->tm_min,
			tm->tm_sec);

	} else {
		ret = bcm_rtc_cal_read_time(rdata, tm);
	}
#else
	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCYR,
					&val);
	if (unlikely(ret))
		goto err;
	tm->tm_year = val + 100;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMT_WD,
					&val);
	if (unlikely(ret))
		goto err;
	if (val >= 1)
		tm->tm_mon = (val & 0xF) - 1;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCDT,
					&val);
	if (unlikely(ret))
		goto err;
	tm->tm_mday = (val & 0x1F);

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCHR,
					&val);
	if (unlikely(ret))
		goto err;
	tm->tm_hour = (val & 0x1F);

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMN,
					&val);
	if (unlikely(ret))
		goto err;
	tm->tm_min = (val & 0x3F);

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCSC,
					&val);
	if (unlikely(ret))
		goto err;
	tm->tm_sec = (val & 0x3F);

	ret = rtc_valid_tm(tm);

	pr_rtc(DATA, "%s: err=%d time=%d.%d.%d.%d.%d.%d\n",
		__func__, ret, tm->tm_year, tm->tm_mon,
		tm->tm_mday, tm->tm_hour, tm->tm_min,
		tm->tm_sec);
#endif /* CONFIG_BCM_RTC_CAL*/


err:
	mutex_unlock(&rdata->lock);
	return ret;
}

static int bcmpmu_set_time(struct device *dev, struct rtc_time *tm)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret;

	pr_rtc(DATA, "%s: time=%d.%d.%d.%d.%d.%d\n",
		__func__, tm->tm_year, tm->tm_mon,
		tm->tm_mday, tm->tm_hour,
		tm->tm_min, tm->tm_sec);

	mutex_lock(&rdata->lock);

#ifdef CONFIG_BCM_RTC_CAL
	if (rtc_cal_run == false) {
		pr_rtc(DATA, "%s: rtc_cal not ready\n", __func__);
		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR,
					tm->tm_year - 100);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT_WD,
					tm->tm_mon + 1);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT,
					tm->tm_mday);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR,
					tm->tm_hour);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN,
					tm->tm_min);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC,
					tm->tm_sec);
		if (unlikely(ret))
			goto err;
	} else {
		ret = bcm_rtc_cal_set_time(rdata, tm);
	}
#else
	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR,
				tm->tm_year - 100);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT_WD,
				tm->tm_mon + 1);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT,
				tm->tm_mday);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR,
				tm->tm_hour);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN,
				tm->tm_min);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC,
				tm->tm_sec);
	if (unlikely(ret))
		goto err;
#endif /* CONFIG_BCM_RTC_CAL*/

err:
	mutex_unlock(&rdata->lock);
	return ret;
}

static int bcmpmu_read_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret;
	u8 val;

	mutex_lock(&rdata->lock);

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCYR_A1,
					&val);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_year = val + 100;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMT_A1,
					&val);
	if (unlikely(ret))
		goto err;
	if (val >= 1)
		alarm->time.tm_mon = val - 1;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCDT_A1,
					&val);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_mday = val;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCHR_A1,
					&val);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_hour = val;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMN_A1,
					&val);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_min = val;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCSC_A1,
					&val);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_sec = val;

	alarm->enabled = rdata->alarm_irq_enabled;
	ret = rtc_valid_tm(&alarm->time);

	pr_rtc(DATA, "%s: err=%d enable=%d time=%d.%d.%d.%d.%d.%d\n",
		__func__, ret, alarm->enabled, alarm->time.tm_year, alarm->time.tm_mon,
		alarm->time.tm_mday, alarm->time.tm_hour,
		alarm->time.tm_min, alarm->time.tm_sec);

err:
	mutex_unlock(&rdata->lock);
	return ret;
}

#ifdef CONFIG_RTC_AUTO_PWRON
static int bcmpmu_reset_alarm_boot(struct device *dev)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret;
	
	pr_rtc(DATA, "%s: [SAPA] en(%d) time=%d.%d.%d %d:%d:%d\n",
		__func__, poweron_tm.enabled, poweron_tm.time.tm_year+1900, poweron_tm.time.tm_mon+1,
		poweron_tm.time.tm_mday, poweron_tm.time.tm_hour,
		poweron_tm.time.tm_min, poweron_tm.time.tm_sec);	

	mutex_lock(&rdata->lock);
	
	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR_A1,
				poweron_tm.time.tm_year - 100);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT_A1,
				poweron_tm.time.tm_mon + 1);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT_A1,
				poweron_tm.time.tm_mday);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR_A1,
				poweron_tm.time.tm_hour);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN_A1,
				poweron_tm.time.tm_min);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC_A1,
				poweron_tm.time.tm_sec);
	if (unlikely(ret))
		goto err;

	if (poweron_tm.enabled)
		bcmpmu_alarm_irq_enable(dev, poweron_tm.enabled);
err:
	mutex_unlock(&rdata->lock);
	return ret;
}

static int bcmpmu_set_alarm_boot(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret;
	u8 val;
	
	pr_rtc(DATA, "%s: [SAPA] en(%d) time=%d.%d.%d %d:%d:%d\n",
		__func__, alarm->enabled, alarm->time.tm_year+1900, alarm->time.tm_mon+1,
		alarm->time.tm_mday, alarm->time.tm_hour,
		alarm->time.tm_min, alarm->time.tm_sec);			

	mutex_lock(&rdata->lock);
	
	memcpy(&poweron_tm, alarm, sizeof(struct rtc_wkalrm));

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR_A1,
				alarm->time.tm_year - 100);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT_A1,
				alarm->time.tm_mon + 1);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT_A1,
				alarm->time.tm_mday);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR_A1,
				alarm->time.tm_hour);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN_A1,
				alarm->time.tm_min);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC_A1,
				alarm->time.tm_sec);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTC_EXTRA,
					&val);
	if (unlikely(ret))
		goto err;
	pr_info("%s PMU_REG_RTC_EXTRA = 0x%x \n", __func__, val);
	
	if (alarm->enabled)
	{
		bcmpmu_alarm_irq_enable(dev, alarm->enabled);
		val = 0x04;
		//val |= PMU_REG_RTC_EXTRA_POR_GLITCH_MASK;
	}
	else
	{
		val = 0x00;
		//val &= ~PMU_REG_RTC_EXTRA_POR_GLITCH_MASK;
	}

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTC_EXTRA,
				val);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTC_EXTRA,
					&val);
	if (unlikely(ret))
		goto err;
	
	pr_info("%s PMU_REG_RTC_EXTRA = 0x%x \n", __func__, val);
	
err:
	mutex_unlock(&rdata->lock);
	return ret;
}
#endif /* CONFIG_RTC_AUTO_PWRON */

static int bcmpmu_set_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret;

	pr_rtc(DATA, "%s: time=%d.%d.%d.%d.%d.%d\n",
		__func__, alarm->time.tm_year, alarm->time.tm_mon,
		alarm->time.tm_mday, alarm->time.tm_hour,
		alarm->time.tm_min, alarm->time.tm_sec);

	mutex_lock(&rdata->lock);

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR_A1,
				alarm->time.tm_year - 100);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT_A1,
				alarm->time.tm_mon + 1);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT_A1,
				alarm->time.tm_mday);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR_A1,
				alarm->time.tm_hour);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN_A1,
				alarm->time.tm_min);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC_A1,
				alarm->time.tm_sec);
	if (unlikely(ret))
		goto err;

#ifdef CONFIG_RTC_AUTO_PWRON
	if (alarm->enabled)
		bcmpmu_alarm_irq_enable(dev, alarm->enabled);
#else
	bcmpmu_alarm_irq_enable(dev, alarm->enabled);
#endif

err:
	mutex_unlock(&rdata->lock);
	return ret;
}

/*
static int bcmpmu_update_irq_enable(struct device *dev,
		unsigned int enabled)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret = 0;
	if (enabled)
		ret = rdata->bcmpmu->unmask_irq(rdata->bcmpmu, PMU_IRQ_RTC_SEC);
	else
		ret = rdata->bcmpmu->mask_irq(rdata->bcmpmu, PMU_IRQ_RTC_SEC);
	if (unlikely(ret))
		goto err;
	rdata->update_irq_enabled = enabled;
err:
	return ret;
}
*/


static struct rtc_class_ops bcmpmu_rtc_ops = {
	.read_time		= bcmpmu_read_time,
	.set_time		= bcmpmu_set_time,
	.read_alarm		= bcmpmu_read_alarm,
	.set_alarm		= bcmpmu_set_alarm,
	.alarm_irq_enable	= bcmpmu_alarm_irq_enable,
#ifdef CONFIG_RTC_AUTO_PWRON
	.set_bootalarm 	= bcmpmu_set_alarm_boot,
#endif	
};

static int __devinit bcmpmu_rtc_probe(struct platform_device *pdev)
{
	int ret = 0;
	u8 val;

	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_rtc *rdata;

#if 1 //#if defined(CONFIG_RTC_CHN_ALARM_BOOT)
	int alarm_en = 1;
#endif

	pr_rtc(INIT, "%s: called.\n", __func__);

	rdata = kzalloc(sizeof(struct bcmpmu_rtc), GFP_KERNEL);
	if (rdata == NULL) {
		dev_err(&pdev->dev, "failed to alloc mem\n");
		return -ENOMEM;
	}
	rdata->bcmpmu = bcmpmu;

	init_waitqueue_head(&rdata->wait);
	mutex_init(&rdata->lock);
	bcmpmu->rtcinfo = (void *)rdata;

#ifdef CONFIG_RTC_AUTO_PWRON
	current_alarm_time = kzalloc(sizeof(struct rtc_wkalrm), GFP_KERNEL);
	if(current_alarm_time == NULL)		
	{		
		pr_err("%s : memory allocation failure \n", __func__);		
		return -ENOMEM;		
	}
	current_rtc_time = kzalloc(sizeof(struct rtc_time), GFP_KERNEL);
	if(current_rtc_time == NULL)		
	{		
		pr_err("%s : memory allocation failure \n", __func__);		
		return -ENOMEM;		
	}
		rdata->update_irq_enabled = 1;
		rdata->alarm_irq_enabled = 1;		
#else
		rdata->update_irq_enabled = 0;
		rdata->alarm_irq_enabled = 0;
#endif	
	rdata->update_irq_enabled = 0;
	rdata->alarm_irq_enabled = 0;

	platform_set_drvdata(pdev, rdata);
	device_init_wakeup(&pdev->dev, 1);
	rdata->rtc = rtc_device_register(pdev->name,
			&pdev->dev, &bcmpmu_rtc_ops, THIS_MODULE);
	if (IS_ERR(rdata->rtc)) {
		ret = PTR_ERR(rdata->rtc);
		goto err;
	}

	ret = bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTC_ALARM,
			bcmpmu_rtc_isr, rdata);
	if (ret) {
		pr_rtc(ERROR, "In %s: Unable to allocate Alarm IRQ: %d.\n",
						__func__, PMU_IRQ_RTC_ALARM);
		goto err_irq_alarm;
	}
	ret = bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTC_SEC,
			bcmpmu_rtc_isr, rdata);
	if (ret) {
		pr_rtc(ERROR, "In %s: Unable to allocate SEC IRQ: %d.\n",
						__func__, PMU_IRQ_RTC_SEC);
		goto err_irq_sec;
	}
	ret = bcmpmu->mask_irq(bcmpmu, PMU_IRQ_RTC_SEC);
	if (ret)
		pr_rtc(ERROR, "%s: Failed to disable RTC 1S interrupt\n",
						__func__);
#ifdef CONFIG_BCM_RTC_CAL
	bcm_rtc_cal_init(rdata);
	rtc_cal_run = true;
#endif /*CONFIG_BCM_RTC_CAL*/

	/* Workarond the invalid value, to be removed after RTCADJ interrupt
	is handled properly */
	bcmpmu->read_dev(bcmpmu, PMU_REG_RTCDT, &val);
	if (val == 0) {
		bcmpmu->write_dev(bcmpmu, PMU_REG_RTCDT, 1);
		bcmpmu->write_dev(bcmpmu, PMU_REG_RTCYR, 0);
		bcmpmu_rtc_time_fixup(&pdev->dev);
	}

	ret = device_create_file(&rdata->rtc->dev, &dev_attr_dbgmask);
	
#if CONFIG_RTC_AUTO_PWRON
	bcmpmu_read_alarm(&pdev->dev,current_alarm_time);

	if(!(current_alarm_time->time.tm_year & 0x40))
		alarm_en = 0;

	printk( "%s: enable =%d alarm time=%d.%d.%d.%d.%d.%d\n",
		__func__, alarm_en,
		current_alarm_time->time.tm_year,current_alarm_time->time.tm_mon,current_alarm_time->time.tm_mday,
		current_alarm_time->time.tm_hour,current_alarm_time->time.tm_min,current_alarm_time->time.tm_sec);
	printk("%s : boot mode is %d \n", __func__,lpcharging_boot_mode );
	if((alarm_en )&& (BOOT_MODE_LPM == lpcharging_boot_mode))
 	{ 
		info_autopower_rtc = pdev;
		INIT_DELAYED_WORK(&pollling_work_alarmboot,
			bcmpmu_check_alarm_lpm);
		schedule_delayed_work(&pollling_work_alarmboot, 300);
 	}
	else
	{
		kfree(current_rtc_time);
		kfree(current_alarm_time);
	}
#endif /* CONFIG_RTC_AUTO_PWRON */
	
	return 0;

err_irq_sec:
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTC_ALARM);
err_irq_alarm:
	rtc_device_unregister(rdata->rtc);
err:
	platform_set_drvdata(pdev, NULL);
	kfree(rdata);
	return ret;
}

static int __devexit bcmpmu_rtc_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_rtc *rdata = (struct bcmpmu_rtc *)bcmpmu->rtcinfo ;

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTC_ALARM);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTC_SEC);
#ifdef CONFIG_BCM_RTC_CAL
	rtc_cal_run = false;
	bcm_rtc_cal_shutdown();
#else /*CONFIG_BCM_RTC_CAL*/
	rtc_device_unregister(rdata->rtc);
#endif
	kfree(bcmpmu->rtcinfo);
	return 0;
}


static void bcmpmu_rtc_time_fixup(struct device *dev)
{
	struct rtc_time current_rtc_time;
	memset(&current_rtc_time, 0 , sizeof(struct rtc_time));

	bcmpmu_read_time(dev, &current_rtc_time);
	current_rtc_time.tm_year += SEC_YEAR_BASE;
	bcmpmu_set_time(dev, &current_rtc_time);
}

static int bcmpmu_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	/* no action required */
	pr_rtc(FLOW, "%s: ####\n", __func__);
#ifdef CONFIG_RTC_AUTO_PWRON
	bcm_rtc_sleep = 1; 
#endif	
	return 0;
}

static int bcmpmu_rtc_resume(struct platform_device *pdev)
{
	/* disable the RTC ALRM interrupt
	 * as android will take care of it now.
	*/
	pr_rtc(FLOW, "%s: ####\n", __func__);
	bcmpmu_alarm_irq_enable(&pdev->dev, false);
#ifdef CONFIG_RTC_AUTO_PWRON
	bcm_rtc_sleep = 0; 
	bcmpmu_reset_alarm_boot(&pdev->dev); 
#endif	
	return 0;
}

#ifdef CONFIG_RTC_AUTO_PWRON
static int bcmpmu_rtc_shutdown(struct platform_device *pdev)
{
	pr_rtc(FLOW, "%s: ####\n", __func__);
	
	return 0;
}
#endif /* CONFIG_RTC_AUTO_PWRON */

static struct platform_driver bcmpmu_rtc_driver = {
	.driver = {
		.name = "bcmpmu59xxx_rtc",
	},
	.probe = bcmpmu_rtc_probe,
	.remove = __devexit_p(bcmpmu_rtc_remove),
	.suspend = bcmpmu_rtc_suspend,
	.resume = bcmpmu_rtc_resume,
#ifdef CONFIG_RTC_AUTO_PWRON
	.shutdown = bcmpmu_rtc_shutdown,
#endif
};

static int __init bcmpmu_rtc_init(void)
{
	return platform_driver_register(&bcmpmu_rtc_driver);
}
module_init(bcmpmu_rtc_init);

static void __exit bcmpmu_rtc_exit(void)
{
	platform_driver_unregister(&bcmpmu_rtc_driver);
}
module_exit(bcmpmu_rtc_exit);

MODULE_DESCRIPTION("BCM PMIC RTC driver");
MODULE_LICENSE("GPL");
