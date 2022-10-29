#include "TMP117.h"

static int8_t TMP117_WriteRegisters(TMP117 *dev, uint8_t reg, uint8_t *data, uint8_t length);
static int8_t TMP117_WriteRegister(TMP117 *dev, uint8_t reg, uint8_t *data);
static int8_t TMP117_ReadRegisters(TMP117 *dev, uint8_t reg, uint8_t *data, uint8_t length);
static int8_t TMP117_ReadRegister(TMP117 *dev, uint8_t reg, uint8_t *data);

static void TMP117_Delay(uint32_t msec);

int8_t TMP117_Init(TMP117 *dev, I2C_HandleTypeDef * i2cHandle)
{
	if(dev == NULL || i2cHandle == NULL)
	{
		return -1;
	}
	
	dev->i2cHandle = i2cHandle;
	dev->isAvalible = TMP117_is_present(dev);

	return 0;
}

/**
 * @brief  Check if there is device with adress coresponding to TMP117 adress
 * @param  none
 * @retval 1 if device was found and 0 if dev wasn't found
 */
bool TMP117_is_present(TMP117 *dev)
{
	if(dev == NULL)
	{
		return false;
	}	
	
	const uint32_t REQUEST_TIMEOUT_MS = 10;
	const uint32_t	MAX_AMOUNT_OF_IS_READY_TRIALS	= 20;

	if(HAL_I2C_IsDeviceReady(dev->i2cHandle, TMP117_I2C_ADDR, MAX_AMOUNT_OF_IS_READY_TRIALS, REQUEST_TIMEOUT_MS) == HAL_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}


/**
 * @brief  Checks if conversion is done and temperature register can be read
 * @param  none
 * @retval 1 if conversion is done and 0 if not
 */
bool TMP117_is_conversion_done(TMP117 *dev)
{
	if(dev == NULL)
	{
		return false;
	}
	
	bool ret = false;

	uint8_t val[2] = {0x00};
	TMP117_ReadRegisters(dev, CONFIG_REG, &val[0], 2);
	uint16_t val_16 = (val[0] << 8) | val[1];

	if((val_16 & DATA_READY) == DATA_READY)
	{
		ret = true;
	}
	return ret;
}

/**
 * @brief  Set temp conversion mode
 * @param  none
 * @retval -1 if operation is done and 0 if not
 */
int8_t TMP117_set_conversion_mode(TMP117 *dev, Conv_modes_t conv_mode)
{
	if(dev == NULL)
	{
		return -1;
	}
	
	int8_t status = -1;

	uint8_t val[2] = {0x00};
	TMP117_ReadRegisters(dev, CONFIG_REG, &val[0], 2);
	uint16_t val_16 = (val[0] << 8) | val[1];

	if((val_16 & conv_mode) != conv_mode)
	{
		uint16_t mod_val_16 = (val_16 | conv_mode);
		uint8_t mod_val[2] = {(mod_val_16 & 0xFF00)<<8, (mod_val_16 & 0x00FF)};

		TMP117_WriteRegisters(dev, CONFIG_REG, &mod_val[0], 2);

		TMP117_ReadRegisters(dev, CONFIG_REG, &val[0], 2);
		val_16 = (val[0] << 8) | val[1];
		if((val_16 & conv_mode) == conv_mode)
		{
			status = 0;
		}
	}
	else
	{
		status = 0;
	}
	return status;
}

/**
 * @brief  Set temp conversion cycle mode
 * @param  none
 * @retval -1 if operation is done and 0 if not
 */
int8_t TMP117_set_conversion_cycle(TMP117 *dev, Conv_cycle_modes_t conv_cyc_mode)
{
	if(dev == NULL)
	{
		return -1;
	}
	
	int8_t status = -1;

	uint8_t val[2] = {0x00};
	TMP117_ReadRegisters(dev, CONFIG_REG, &val[0], 2);
	uint16_t val_16 = (val[0] << 8) | val[1];

	if((val_16 & conv_cyc_mode) != conv_cyc_mode)
	{
		uint16_t mod_val_16 = (val_16 | conv_cyc_mode);
		uint8_t mod_val[2] = {(mod_val_16 & 0xFF00)<<8, (mod_val_16 & 0x00FF)};

		TMP117_WriteRegisters(dev, CONFIG_REG, &mod_val[0], 2);

		TMP117_ReadRegisters(dev, CONFIG_REG, &val[0], 2);
		val_16 = (val[0] << 8) | val[1];
		if((val_16 & conv_cyc_mode) == conv_cyc_mode)
		{
			status = 0;
		}
	}
	else
	{
		status = 0;
	}
	return status;
}

/**
 * @brief  Get temperature
 * @param  none
 * @retval temperature
 */
float TMP117_get_temp(TMP117 *dev)
{
	const float LSB = 0.0078125;

	float		temperature = -255; // dummy value
	uint8_t		buff_8[3] 	= {0};

	if (dev->isAvalible)
	{
		TMP117_set_conversion_mode(dev, CONTINOUS_CONV_MODE);
		TMP117_set_conversion_cycle(dev, CYCE_125_MS_2);

		/* 8 samples avg with 125ms cycle */
		for(uint8_t i = 0; i < 10; i++)
		{
			if(TMP117_is_conversion_done(dev) == true)
			{
				break;
			}
			else
			{
				TMP117_Delay(25);
			}
		}
		buff_8[0] = TEMP_RESULT_REG;
		TMP117_ReadRegisters(dev, TEMP_RESULT_REG, &buff_8[0], 2);

		int16_t buff_16 = (buff_8[0] << 8) | buff_8[1]; 		// raw value
		temperature = buff_16 * LSB;												// calculated value in *C
	}
	return temperature;
}

int8_t TMP117_WriteRegisters(TMP117 *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	if(dev == NULL || data == NULL || length <= 0)
	{
		return -1;
	}
	HAL_StatusTypeDef status =  HAL_I2C_Mem_Write(dev->i2cHandle, TMP117_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, length, TMP117_WAIT_TIME_MAX);
	if(status != HAL_OK)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int8_t TMP117_WriteRegister(TMP117 *dev, uint8_t reg, uint8_t *data)
{
	if(dev == NULL || data == NULL)
	{
		return -1;
	}
	HAL_StatusTypeDef status =  HAL_I2C_Mem_Write(dev->i2cHandle, TMP117_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TMP117_WAIT_TIME_MAX);
	if(status != HAL_OK)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int8_t TMP117_ReadRegisters(TMP117 *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	if(dev == NULL || data == NULL || length <= 0)
	{
		return -1;
	}
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(dev->i2cHandle, TMP117_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, length, TMP117_WAIT_TIME_MAX);
	if(status != HAL_OK)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int8_t TMP117_ReadRegister(TMP117 *dev, uint8_t reg, uint8_t *data)
{
	if(dev == NULL || data == NULL)
	{
		return -1;
	}
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(dev->i2cHandle, TMP117_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TMP117_WAIT_TIME_MAX);
	if(status != HAL_OK)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

/**
 * @brief  Creates time delay
 * @param  msec number of miliseconds to wait
 * @retval none
 */
static void TMP117_Delay(uint32_t msec)
{
	HAL_Delay(msec);
	//osDelay(msec);
}
