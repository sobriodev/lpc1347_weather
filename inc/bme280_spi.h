/*
 * bme280_spi.h
 */

#ifndef BME280_SPI_H_
#define BME280_SPI_H_

#include "chip.h"

#define SSP0_CLK_PORT 	1
#define SSP0_CLK_PIN  	29
#define SSP0_MOSI_PORT 	0
#define SSP0_MOSI_PIN 	9
#define SSP0_MISO_PORT 	0
#define SSP0_MISO_PIN 	8

#define SSP1_CLK_PORT 	1
#define SSP1_CLK_PIN 	20
#define SSP1_MOSI_PORT	1
#define SSP1_MOSI_PIN 	22
#define SSP1_MISO_PORT 	1
#define SSP1_MISO_PIN 	21

#define DEFAULT_SSEL_PORT	1
#define DEFAULT_SSEL_PIN 	19

#define BME280_ID 0
#define BME280_DUMMY_REG_ADDR 0

#define BME280_SUCCESS 0
#define BME280_FAILURE -1


/*!
 * @brief bme280 spi settings
 */
typedef struct {
	/*! SSP base address */
	LPC_SSP_T *pSSP;
	/*! SSEL pin base address (@see byte register for usage) */
	__IO uint8_t *pSSEL;
} bme280_spi;

/*!
 * @brief bme280 spi init
 * @param[in] : Configuration structure base address
 * @return Nothing
 */
void bme280_spi_init(bme280_spi *conf);

/*!
 * @brief Configure default SSEL pin, which is P1.19
 * @return Base address of default byte register
 */
__IO uint8_t *bme280_default_ssel_init(void);

/**
 * @brief Read bme280 data
 * @param[in] dev_id : Device id. Must be 0
 * @param[in] reg_addr : Register adddress
 * @param[out] reg_data : Buffer to read data in
 * @param[in] len : Data length
 * @return 0 for success, non-zero for failure
 */
int8_t bme280_spi_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);

/**
 * @brief Write bme280 data
 * @param[in] dev_id : Device id. Must be 0
 * @param[in] reg_addr : Register address
 * @param[in] reg_data : Data buffer
 * @param[in] len : Data length
 * @return 0 for success, non-zero for failure
 */
int8_t bme280_spi_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);

#endif /* BME280_SPI_H_ */
