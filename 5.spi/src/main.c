/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/spi.h>

LOG_MODULE_REGISTER(main);

#define CC1101_REG_VERSION                            0x31
#define CC1101_VERSION_CURRENT                        0x14
#define CC1101_VERSION_LEGACY                         0x04
#define CC1101_VERSION_CLONE                          0x17

int strobe (const struct device *spidev, struct spi_config *spi_cfg,uint8_t cmd)
{
	cmd = cmd & 0x3f;

	uint8_t wr_buf[1] = {cmd};
	struct spi_buf wr_spibuf[1];
	
	wr_spibuf[0].buf = wr_buf;
	wr_spibuf[0].len = 1;

	const struct spi_buf_set tx_buff = { wr_spibuf, 1 };

	return spi_transceive(spidev, spi_cfg, &tx_buff, 0);
}

void main(void)
{
	int err;

	const struct device *spidev = DEVICE_DT_GET(DT_ALIAS(spi));
	struct spi_config spi_cfg = {0};

	if (!device_is_ready(spidev)) {
		LOG_INF("%s: device not ready.\n", spidev->name);
		return;
	}

	LOG_INF("\n%s SPI testing\n", spidev->name);

	spi_cfg.operation = SPI_WORD_SET(8);
	spi_cfg.frequency = 256000U;

	LOG_INF("Resetting chip\n");
	strobe(spidev, &spi_cfg,0x30);
	k_msleep(500);

	while (true) {
		uint8_t wr_buf[2] = {0x80 | 0x40 | CC1101_REG_VERSION, 0};
		struct spi_buf wr_spibuf[1];
		
		wr_spibuf[0].buf = wr_buf;
		wr_spibuf[0].len = 2;

		const struct spi_buf_set tx_buff = { wr_spibuf, 1 };

		uint8_t rd_buf[2] = { 0, 0};
		struct spi_buf rd_spibuf[1];
		
		rd_spibuf[0].buf = rd_buf;
		rd_spibuf[0].len = 2;

		const struct spi_buf_set rx_buff = { rd_spibuf, 1 };

		err = spi_transceive(spidev, &spi_cfg, &tx_buff, &rx_buff);
		if (err == 0) {
			if (rd_buf[1] != CC1101_VERSION_CURRENT && 
				rd_buf[1] != CC1101_VERSION_CLONE &&
				rd_buf[1] != CC1101_VERSION_LEGACY) {
				LOG_ERR("Invalid value (%02x)", rd_buf[1]);
			} else {
				LOG_INFO("Version: %02x", rd_buf[1]);
				return;
			}
		}

		LOG_INF("Error reading SPI\n");
		k_msleep(500);
	}
}
