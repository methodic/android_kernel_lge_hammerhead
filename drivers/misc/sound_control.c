/*
 * Copyright 2013 Francisco Franco
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

extern void update_enable_control(int value);
extern void update_headphones_boost(int volume);
extern void update_speaker_boost(int volume);

int enable_control = 0;
int headphones_boost = 0;
int speaker_boost = 0;

static ssize_t enable_control_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", enable_control);
}

static ssize_t enable_control_store(struct device * dev, struct device_attribute * attr, const char * buf, size_t size)
{
    int val;
	sscanf(buf, "%d", &val);

	if (val != enable_control) {
		if (val < 0 || val > 1)
			return -EINVAL;

		enable_control = val;
		update_enable_control(enable_control);

		if (enable_control == 1) {
			update_headphones_boost(headphones_boost);
			update_speaker_boost(speaker_boost);
		}
	}

    return size;
}

static ssize_t headphones_boost_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", headphones_boost);
}

static ssize_t headphones_boost_store(struct device * dev, struct device_attribute * attr, const char * buf, size_t size)
{
    int val;

	sscanf(buf, "%d", &val);

	if (val != headphones_boost) {
		if (val < -20 || val > 20)
			return -EINVAL;

		headphones_boost = val;

		if (enable_control == 1)
			update_headphones_boost(headphones_boost);
	}

    return size;
}

static ssize_t speaker_boost_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", speaker_boost);
}

static ssize_t speaker_boost_store(struct device * dev, struct device_attribute * attr, const char * buf, size_t size)
{
    int val;

	sscanf(buf, "%d", &val);

	if (val != speaker_boost) {
		if (val < -20 || val > 20)
			return -EINVAL;

		speaker_boost = val;

		if (enable_control == 1)
			update_speaker_boost(speaker_boost);
	}

    return size;
}

static DEVICE_ATTR(enable_control, 0664, enable_control_show, enable_control_store);
static DEVICE_ATTR(headphones_boost, 0664, headphones_boost_show, headphones_boost_store);
static DEVICE_ATTR(speaker_boost, 0664, speaker_boost_show, speaker_boost_store);

static struct attribute *soundcontrol_attributes[] = 
{
	&dev_attr_enable_control.attr,
	&dev_attr_headphones_boost.attr,
	&dev_attr_speaker_boost.attr,
	NULL
};

static struct attribute_group soundcontrol_group = 
{
	.attrs  = soundcontrol_attributes,
};

static struct miscdevice soundcontrol_device = 
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "soundcontrol",
};

static int __init soundcontrol_init(void)
{
    int ret;

    pr_info("%s misc_register(%s)\n", __FUNCTION__, soundcontrol_device.name);

    ret = misc_register(&soundcontrol_device);

    if (ret) {
	    pr_err("%s misc_register(%s) fail\n", __FUNCTION__, soundcontrol_device.name);
	    return 1;
	}

    if (sysfs_create_group(&soundcontrol_device.this_device->kobj, &soundcontrol_group) < 0) {
	    pr_err("%s sysfs_create_group fail\n", __FUNCTION__);
	    pr_err("Failed to create sysfs group for device (%s)!\n", soundcontrol_device.name);
	}

    return 0;
}
late_initcall(soundcontrol_init);
