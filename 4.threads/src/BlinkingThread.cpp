#include "BlinkingThread.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(BlinkingThread);

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

#define STACK_SIZE 500
#define THREAD_PRIORITY 5 

K_THREAD_STACK_DEFINE(threadStack, STACK_SIZE);

BlinkingThread::BlinkingThread()
{
}

void BlinkingThread::setup(const struct gpio_dt_spec *l)
{
	led = l;
}

void BlinkingThread::start()
{
	threadId = k_thread_create(&threadData, 
		threadStack,
		K_THREAD_STACK_SIZEOF(threadStack),
		BlinkingThread::threadTrampoline,
		this, NULL, NULL,
		THREAD_PRIORITY, 0, K_NO_WAIT);
}

void BlinkingThread::wait()
{
	k_thread_join(&threadData, K_FOREVER);
}

void BlinkingThread::threadTrampoline(void *object, void *, void *)
{
	auto threadObject = reinterpret_cast<BlinkingThread*>(object);
	threadObject->main();
}

void BlinkingThread::main()
{
	int ret;

	while (1) {
		LOG_INF("Blinking");
		ret = gpio_pin_toggle_dt(led);
		if (ret < 0) {
			return;
		}
		k_msleep(SLEEP_TIME_MS);
	}

}

