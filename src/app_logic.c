/*
 * app_logic.c
 *
 *  Created on: 11 gru 2018
 *      Author: sobriodev
 */

#include "board.h"
#include "app_logic.h"
#include "bme280_spi.h"
#include "stdio.h"
#include "cdc_vcom.h"
#include "string.h"

/* ------------------------------------------------------------------------------------------- */
/* ----------------------------- Private functions and variables ----------------------------- */
/* ------------------------------------------------------------------------------------------- */

/*!
 * @brief Structure storing sensor readout data
 */
static struct bme280_data comp_data;

/*!
 * @brief Buffer used for string formatting
 */
static char sbuffer[SBUFFER_SIZE];

/*!
 * @brief Timer ticks count
 */
volatile uint32_t timer_count;

/*!
 * @brief bme280 configuration structure
 */
extern struct bme280_dev *dev;

/**
 * @brief Systick Handler for using time delay
 * @return Nothing
 */
void SysTick_Handler(void)
{
	if (timer_count) {
		timer_count--;
	}
}

/* ------------------------------------------------------------------------------------------- */
/* -------------------------------------- API functions -------------------------------------- */
/* ------------------------------------------------------------------------------------------- */

void bme280_get_default_conf()
{
    dev->dev_id = 0;
    dev->intf = BME280_SPI_INTF;
    dev->read = bme280_spi_read;
    dev->write = bme280_spi_write;
    dev->delay_ms = bme280_delay_ms;
}

void bme280_prepare_broadcast()
{
	uint8_t settings_sel;
	dev->settings.osr_h = BME280_OVERSAMPLING_1X;
	dev->settings.osr_p = BME280_OVERSAMPLING_16X;
	dev->settings.osr_t = BME280_OVERSAMPLING_2X;
	dev->settings.filter = BME280_FILTER_COEFF_16;
	dev->settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

	settings_sel = BME280_OSR_PRESS_SEL;
	settings_sel |= BME280_OSR_TEMP_SEL;
	settings_sel |= BME280_OSR_HUM_SEL;
	settings_sel |= BME280_STANDBY_SEL;
	settings_sel |= BME280_FILTER_SEL;
	bme280_set_sensor_settings(settings_sel, dev);
	bme280_set_sensor_mode(BME280_NORMAL_MODE, dev);
}

void bme280_send_data(void)
{
	bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
	sprintf(sbuffer, "T%0.2f|H%0.2f|P%0.2f", comp_data.temperature, comp_data.humidity, comp_data.pressure);
	vcom_write((uint8_t *) sbuffer, strlen(sbuffer));
	Board_LED_Toggle(0);
}

void bme280_delay_ms(uint32_t period)
{
	timer_count = period;
	while (timer_count);
}



