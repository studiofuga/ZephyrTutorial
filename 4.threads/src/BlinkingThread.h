#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

class BlinkingThread {
public:
	explicit BlinkingThread();

	void setup(const struct gpio_dt_spec *led);

	void start();

	void wait();

private:
	const struct gpio_dt_spec *led;

	k_tid_t threadId;
	struct k_thread threadData;

	void main();

	static void threadTrampoline(void *, void *, void *);
};
