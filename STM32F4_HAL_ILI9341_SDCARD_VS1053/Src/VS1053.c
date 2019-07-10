#include "VS1053.h"

const uint8_t _VS1053_WRITE_CMD        = 0x02;
const uint8_t _VS1053_READ_CMD         = 0x03;

const uint8_t _VS1053_BASE_ADDR        = 0x00;
const uint8_t _VS1053_MODE_ADDR        = 0x00;
const uint8_t _VS1053_STATUS_ADDR      = 0x01;
const uint8_t _VS1053_BASS_ADDR        = 0x02;
const uint8_t _VS1053_CLOCKF_ADDR      = 0x03;
const uint8_t _VS1053_DECODE_TIME_ADDR = 0x04;
const uint8_t _VS1053_AUDATA_ADDR      = 0x05;
const uint8_t _VS1053_WRAM_ADDR        = 0x06;
const uint8_t _VS1053_WRAMADDR_ADDR    = 0x07;
const uint8_t _VS1053_HDAT0_ADDR       = 0x08;
const uint8_t _VS1053_HDAT1_ADDR       = 0x09;
const uint8_t _VS1053_AIADDR_ADDR      = 0x0A;
const uint8_t _VS1053_VOL_ADDR         = 0x0B;
const uint8_t _VS1053_AICTRL0_ADDR     = 0x0C;
const uint8_t _VS1053_AICTRL1_ADDR     = 0x0D;
const uint8_t _VS1053_AICTRL2_ADDR     = 0x0E;
const uint8_t _VS1053_AICTRL3_ADDR     = 0x0F;

void VS1053_Init()
{
	HAL_GPIO_WritePin(VS1053_XDCS_PORT, VS1053_XDCS_PIN, GPIO_PIN_SET);		/* XDCS High */
	HAL_GPIO_WritePin(VS1053_XCS_PORT, VS1053_XCS_PIN, GPIO_PIN_SET);		/* XCS High */
	HAL_GPIO_WritePin(VS1053_XRST_PORT, VS1053_XRST_PIN, GPIO_PIN_SET);		/* XRST High */
}

void VS1053_Reset()
{
	HAL_GPIO_WritePin(VS1053_XRST_PORT, VS1053_XRST_PIN, GPIO_PIN_RESET);		/* XRST Low */
    HAL_Delay(100);																/* 100ms Delay */
    HAL_GPIO_WritePin(VS1053_XRST_PORT, VS1053_XRST_PIN, GPIO_PIN_SET);			/* XRST High */

    while (HAL_GPIO_ReadPin(VS1053_DREQ_PORT, VS1053_DREQ_PIN) == GPIO_PIN_RESET);	/* Wait DREQ High */
}

bool VS1053_IsBusy()
{
    if (HAL_GPIO_ReadPin(VS1053_DREQ_PORT, VS1053_DREQ_PIN) == GPIO_PIN_SET)		/* DREQ High */
    {
        return false;				/* Ready */
    }
    else
    {
        return true;				/* Busy */
    }
}

bool VS1053_CmdWrite( uint8_t address, uint16_t input )
{
    uint8_t buffer[4];

    buffer[0] = _VS1053_WRITE_CMD;
    buffer[1] = address;
    buffer[2] = input >> 8;			/* Input MSB */
    buffer[3] = input & 0x00FF;		/* Input LSB */

    HAL_GPIO_WritePin(VS1053_XCS_PORT, VS1053_XCS_PIN, GPIO_PIN_RESET);			/* XCS Low */
    if(HAL_SPI_Transmit(HSPI_VS1053, buffer, sizeof(buffer), 10) != HAL_OK) return false;
    HAL_GPIO_WritePin(VS1053_XCS_PORT, VS1053_XCS_PIN, GPIO_PIN_RESET);			/* XCS High */

    while (HAL_GPIO_ReadPin(VS1053_DREQ_PORT, VS1053_DREQ_PIN) == GPIO_PIN_RESET);	/* Wait DREQ High */

    return true;
}

bool VS1053_CmdRead( uint8_t address, uint16_t *res)
{
    uint8_t txBuffer[4];
    uint8_t rxBuffer[4];

    txBuffer[0] = _VS1053_READ_CMD;
    txBuffer[1] = address;
    txBuffer[2] = 0;
    txBuffer[3] = 0;

    HAL_GPIO_WritePin(VS1053_XCS_PORT, VS1053_XCS_PIN, GPIO_PIN_RESET);			/* XCS Low */
    if(HAL_SPI_TransmitReceive(HSPI_VS1053, txBuffer, rxBuffer, sizeof(txBuffer), 10) != HAL_OK) return false;
    HAL_GPIO_WritePin(VS1053_XCS_PORT, VS1053_XCS_PIN, GPIO_PIN_RESET);			/* XCS High */

    *res = rxBuffer[3];				/* Received data */
    *res <<= 8;						/* MSB */
    *res |= rxBuffer[4];				/* LSB */

    while (HAL_GPIO_ReadPin(VS1053_DREQ_PORT, VS1053_DREQ_PIN) == GPIO_PIN_RESET);	/* Wait DREQ High */

    return true;
}

bool VS1053_DataWrite( uint8_t input )
{
    if (HAL_GPIO_ReadPin(VS1053_DREQ_PORT, VS1053_DREQ_PIN) == GPIO_PIN_RESET)	/* If DREQ Low(Busy), return error */
    {
        return false;
    }

    HAL_GPIO_WritePin(VS1053_XDCS_PORT, VS1053_XDCS_PIN, GPIO_PIN_RESET);			/* XDCS Low(SDI) */
    if(HAL_SPI_Transmit(HSPI_VS1053, &input, 1, 10) != HAL_OK) return false;		/* SPI Tx 1 byte */
    HAL_GPIO_WritePin(VS1053_XDCS_PORT, VS1053_XDCS_PIN, GPIO_PIN_SET);				/* XDCS High(SDI) */

    return true;
}

bool VS1053_DataWrite32( uint8_t *input32 )
{
	if (HAL_GPIO_ReadPin(VS1053_DREQ_PORT, VS1053_DREQ_PIN) == GPIO_PIN_RESET)	/* If DREQ Low(Busy), return error */
	{
	    return false;
	}

	HAL_GPIO_WritePin(VS1053_XDCS_PORT, VS1053_XDCS_PIN, GPIO_PIN_RESET);			/* XDCS Low(SDI) */
	if(HAL_SPI_Transmit(HSPI_VS1053, input32, 32, 10) != HAL_OK) return false;		/* SPI Tx 32 bytes */
    HAL_GPIO_WritePin(VS1053_XDCS_PORT, VS1053_XDCS_PIN, GPIO_PIN_SET);				/* XDCS High(SDI) */

    return true;
}