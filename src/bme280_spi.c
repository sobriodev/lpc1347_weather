/*
 * bme280_spi.c
 */

#include "bme280_spi.h"

/* ------------------------------------------------------------------------------------------- */
/* ----------------------------- Private functions and variables ----------------------------- */
/* ------------------------------------------------------------------------------------------- */

/*!
 * @brief Base address of the current config structure
 */
static bme280_spi *settings;

/*!
 * @brief Set pin muxing for SSP pins
 * @param[in] pSSP Base address of desired SSP
 * @return Nothing
 */
static void set_pin_muxing(const LPC_SSP_T *pSSP)
{
	if (settings->pSSP == LPC_SSP0) {
		/* Set pin muxing for SSP0 */
		Chip_IOCON_PinMuxSet(LPC_IOCON, SSP0_CLK_PORT, SSP0_CLK_PIN, IOCON_FUNC1);
		Chip_IOCON_PinMuxSet(LPC_IOCON, SSP0_MISO_PORT, SSP0_MISO_PIN, IOCON_FUNC1);
		Chip_IOCON_PinMuxSet(LPC_IOCON, SSP0_MOSI_PORT, SSP0_MOSI_PIN, IOCON_FUNC1);
	} else {
		/* Set pin muxing for SSP1 */
		Chip_IOCON_PinMuxSet(LPC_IOCON, SSP1_CLK_PORT, SSP1_CLK_PIN, IOCON_FUNC2);
		Chip_IOCON_PinMuxSet(LPC_IOCON, SSP1_MISO_PORT, SSP1_MISO_PIN, IOCON_FUNC2);
		Chip_IOCON_PinMuxSet(LPC_IOCON, SSP1_MOSI_PORT, SSP1_MOSI_PIN, IOCON_FUNC2);
	}
}

/*!
 * @brief Set SSEL pin state to low
 * @return Nothing
 */
static inline void ssel_set(void)
{
	*(settings->pSSEL) = false;
}

/*!
 * @brief Set SSEL pin state to high
 * @return Nothing
 */
static inline void ssel_reset(void)
{
	*(settings->pSSEL) = true;
}

/**
 * @brief Calculate total number of 8 byte data chunks based on data length
 * @param[in] len : Data length
 * @return Number of chunks
 */
static uint16_t calculate_chunks(uint16_t len)
{
    uint16_t chunks = 1;
    bool add = false;
    for (int i = 1; i <= len; i++) {
        chunks += add;
        add = ((i % 8) == 0);
    }
    return chunks;
}

/**
 * @brief Flush data in DR register
 * @return Nothing
 */
static inline void flush_spi_data_buffer(void) {
	while(Chip_SSP_GetStatus(settings->pSSP, SSP_STAT_RNE)) {
		Chip_SSP_ReceiveFrame(settings->pSSP);
	}
}

/**
 * Clear 7th bit of register address for write operation
 * @param[in] addr : Register address
 * @return New address
 */
static inline uint8_t write_reg_addr(uint8_t addr) {
	return addr & ~_BIT(7);
}

/* ------------------------------------------------------------------------------------------- */
/* -------------------------------------- API functions -------------------------------------- */
/* ------------------------------------------------------------------------------------------- */


void bme280_spi_init(bme280_spi *conf)
{
	settings = conf;
	set_pin_muxing(settings->pSSP);
	Chip_SSP_Init(settings->pSSP);
	Chip_SSP_SetMaster(settings->pSSP, 1);
	Chip_SSP_SetClockRate(settings->pSSP, 0, 8); /* 9 MHz clock */
	Chip_SSP_SetFormat(settings->pSSP, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_CPHA0_CPOL0);
	Chip_SSP_Enable(settings->pSSP);
	ssel_reset();
}

__IO uint8_t *bme280_default_ssel_init(void)
{
	Chip_IOCON_PinMuxSet(LPC_IOCON, DEFAULT_SSEL_PORT, DEFAULT_SSEL_PIN, IOCON_FUNC0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, DEFAULT_SSEL_PORT, DEFAULT_SSEL_PIN);
	return &LPC_GPIO_PORT->B[DEFAULT_SSEL_PORT][DEFAULT_SSEL_PIN];
}

int8_t bme280_spi_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	if (dev_id != BME280_ID) {
		return BME280_FAILURE;
	}

	uint16_t chunks = calculate_chunks(len);

	ssel_set();
	while(!Chip_SSP_GetStatus(settings->pSSP, SSP_STAT_TNF));
	Chip_SSP_SendFrame(settings->pSSP, reg_addr);
	while(Chip_SSP_GetStatus(settings->pSSP, SSP_STAT_BSY));

	flush_spi_data_buffer();

	for (int i = 1; i <= chunks; i++) {
		uint8_t startOffset = (i-1)*8;
		uint8_t stopOffset;
		if (i == chunks) {
			stopOffset = len-1;
		} else {
			stopOffset = i*8-1;
		}

		while(!Chip_SSP_GetStatus(settings->pSSP, SSP_STAT_TFE));
		for (int i = startOffset; i <= stopOffset; i++) {
			Chip_SSP_SendFrame(settings->pSSP, BME280_DUMMY_REG_ADDR);
		}
		while(Chip_SSP_GetStatus(settings->pSSP, SSP_STAT_BSY));
		for (int i = startOffset; i<= stopOffset; i++) {
			reg_data[i] = Chip_SSP_ReceiveFrame(settings->pSSP);
		}
	}

	ssel_reset();
	return BME280_SUCCESS;
}

int8_t bme280_spi_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	if (dev_id != BME280_ID) {
		return BME280_FAILURE;
	}

	ssel_set();
	while(!Chip_SSP_GetStatus(settings->pSSP, SSP_STAT_TNF));
	Chip_SSP_SendFrame(settings->pSSP, write_reg_addr(reg_addr));
	for (int i =0; i < len; i++) {
		while(!Chip_SSP_GetStatus(settings->pSSP, SSP_STAT_TNF));
		Chip_SSP_SendFrame(settings->pSSP, reg_data[i]);
	}
	while(Chip_SSP_GetStatus(settings->pSSP, SSP_STAT_BSY));
	ssel_reset();
	return BME280_SUCCESS;
}
