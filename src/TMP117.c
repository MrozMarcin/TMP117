#include "TMP117.h"

static HAL_StatusTypeDef TMP117_WriteRegisters(TMP117 *dev, uint8_t reg, uint8_t *data, uint8_t length);
static HAL_StatusTypeDef TMP117_WriteRegister(TMP117 *dev, uint8_t reg, uint8_t *data);
static HAL_StatusTypeDef TMP117_ReadRegisters(TMP117 *dev, uint8_t reg, uint8_t *data, uint8_t length);
static HAL_StatusTypeDef TMP117_ReadRegister(TMP117 *dev, uint8_t reg, uint8_t *data);

int8_t TMP117_Init(TMP117 *dev, I2C_HandleTypeDef * i2cHandle)
{
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
	const uint32_t REQUEST_TIMEOUT_MS = 10;
	const uint8_t 	MAX_AMOUNT_OF_FULL_RETRIES		= 5;
	const uint32_t	MAX_AMOUNT_OF_IS_READY_TRIALS	= 20;
	uint8_t retry = 0;
	bool ret = false;
	
	do
	{
		if(HAL_I2C_IsDeviceReady(dev->i2cHandle, TMP117_I2C_ADDR, MAX_AMOUNT_OF_IS_READY_TRIALS, REQUEST_TIMEOUT_MS) == HAL_OK)
		{
			ret = true;
			break;
		}
		else
		{
			retry++;
		}
	}while(retry < MAX_AMOUNT_OF_FULL_RETRIES);
	return ret;
}


/**
 * @brief  Checks if conversion is done and temperature register can be read
 * @param  none
 * @retval 1 if conversion is done and 0 if not
 */
bool TMP117_is_conversion_done(TMP117 *dev)
{
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
HAL_StatusTypeDef TMP117_set_conversion_mode(TMP117 *dev, Conv_modes_t conv_mode)
{
	HAL_StatusTypeDef status = HAL_ERROR;
	
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
			status = HAL_OK;
		}
	}
	else
	{
		status = HAL_OK;
	}
	return status;
}

/**
 * @brief  Set temp conversion cycle mode
 * @param  none
 * @retval -1 if operation is done and 0 if not
 */
HAL_StatusTypeDef TMP117_set_conversion_cycle(TMP117 *dev, Conv_cycle_modes_t conv_cyc_mode)
{
	HAL_StatusTypeDef status = HAL_ERROR;
	
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
			status = HAL_OK;
		}
	}
	else
	{
		status = HAL_OK;
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
		while(TMP117_is_conversion_done(dev)==false){;};
		buff_8[0] = TEMP_RESULT_REG;		
		TMP117_ReadRegisters(dev, TEMP_RESULT_REG, &buff_8[0], 2);
		
		int16_t buff_16 = (buff_8[0] << 8) | buff_8[1]; 		// raw value
		temperature = buff_16 * LSB;							// calculated value in *C
	}
	return temperature;
}

HAL_StatusTypeDef TMP117_WriteRegisters(TMP117 *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, TMP117_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, length, TMP117_WAIT_TIME_MAX);
}

HAL_StatusTypeDef TMP117_WriteRegister(TMP117 *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, TMP117_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TMP117_WAIT_TIME_MAX);
}

HAL_StatusTypeDef TMP117_ReadRegisters(TMP117 *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, TMP117_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, length, TMP117_WAIT_TIME_MAX);
}

HAL_StatusTypeDef TMP117_ReadRegister(TMP117 *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, TMP117_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TMP117_WAIT_TIME_MAX);
}