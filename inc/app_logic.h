/*
 * sensor.h
 *
 *  Created on: 11 gru 2018
 *      Author: sobriodev
 */

#ifndef APP_LOGIC_H_
#define APP_LOGIC_H_

#include "bme280.h"

#define SBUFFER_SIZE 64

/*!
 * @brief Get default configuration for bme280 sensor
 * @return Nothing
 */
void bme280_get_default_conf(void);

/**
 * @brief Set all settings and wake up sensor
 * @return Nothing
 */
void bme280_prepare_broadcast(void);

/**
 * @brief Send read data using UART
 * @return Nothing
 */
void bme280_send_data(void);

/**
 * @brief Delay function
 * @param[in] period : Time period in milliseconds
 */
void bme280_delay_ms(uint32_t period);

#endif /* APP_LOGIC_H_ */
