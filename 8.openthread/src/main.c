/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/openthread.h>
#include <openthread/thread.h>

LOG_MODULE_REGISTER(main);

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* OT events works */

typedef void (*ot_connection_cb_t)(struct k_work *item);
typedef void (*ot_disconnection_cb_t)(struct k_work *item);

static struct k_work on_connect_work;
static struct k_work on_disconnect_work;

static void on_ot_connect(struct k_work *item)
{
	ARG_UNUSED(item);

	gpio_pin_set_dt(&led, 1);
}

static void on_ot_disconnect(struct k_work *item)
{
	ARG_UNUSED(item);

	gpio_pin_set_dt(&led, 0);
}

static bool is_connected = false;

/* Open Thread callback */

static void on_thread_state_changed(otChangedFlags flags, struct openthread_context *ot_context,
				    void *user_data)
{
	if (flags & OT_CHANGED_THREAD_ROLE) {
		switch (otThreadGetDeviceRole(ot_context->instance)) {
		case OT_DEVICE_ROLE_CHILD:
		case OT_DEVICE_ROLE_ROUTER:
		case OT_DEVICE_ROLE_LEADER:
			k_work_submit(&on_connect_work);
			is_connected = true;
			break;

		case OT_DEVICE_ROLE_DISABLED:
		case OT_DEVICE_ROLE_DETACHED:
		default:
			k_work_submit(&on_disconnect_work);
			is_connected = false;
			break;
		}
	}
}

static struct openthread_state_changed_cb ot_state_chaged_cb = {
	.state_changed_cb = on_thread_state_changed
};

void main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led)) {
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

	ret = settings_subsys_init();
	if (ret) {
		printk("settings subsys initialization: fail (err %d)\n", ret);
		return;
	}

	printk("Settings subsystem init: OK.\n");

	k_work_init(&on_connect_work, on_ot_connect);
	k_work_init(&on_disconnect_work, on_ot_disconnect);

	openthread_state_changed_cb_register(openthread_get_default_context(), &ot_state_chaged_cb);
	openthread_start(openthread_get_default_context());

	printk("Openthread subsystem init: OK.\n");

}
