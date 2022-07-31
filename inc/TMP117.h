#ifndef TMP117_DRIVER_H
#define TMP117_DRIVER_H

#include "stm32h7XX_hal.h"
#include "stdbool.h"

#define TMP117_I2C_ADDR   		(0x48 << 1) /* 0b1010000x */
#define TMP117_WAIT_TIME_MAX 	0xFFFF

/****************** DEV ADDR ***************/	
#define TMP117_ID						0x0117

/**************** REGISTERS ***************/
#define TEMP_RESULT_REG				0x00
#define CONFIG_REG						0x01
#define DEVICE_ID_REG					0x0F
#define LOW_LIMIT_REG					0x03
#define HIGH_LIMIT_RE					0x02
#define DEVICE_ID_REG					0x0F

typedef struct
{
    I2C_HandleTypeDef *i2cHandle;
    bool isAvalible;
}TMP117;

typedef enum 
{
	DATA_READY			= (1 << 13),
}Data_ready_states_t;

typedef enum 
{
	CONTINOUS_CONV_MODE		= (0 << 10),
	SHUTDOWN_MODE					= (1 << 10),
	CONTINOUS_CONV_2_MODE	= (2 << 10),
	ONE_SHOT 							= (3 << 10),
}Conv_modes_t;

typedef enum // values valid if avg mode is default
{
	CYCE_125_MS		= (0 << 7),
	CYCE_125_MS_2	= (1 << 7),
	CYCE_250_MS		= (2 << 7),
	CYCE_500_MS		= (3 << 7),
	CYCE_1_MS			= (4 << 7),
	CYCE_4_MS			= (5 << 7),
	CYCE_8_MS			= (6 << 7),
	CYCE_16_MS		= (7 << 7),
}Conv_cycle_modes_t;

bool TMP117_is_present(TMP117 *dev);
bool TMP117_is_conversion_done(TMP117 *dev);
HAL_StatusTypeDef TMP117_set_conversion_mode(TMP117 *dev, Conv_modes_t conv_mode);
HAL_StatusTypeDef TMP117_set_conversion_cycle(TMP117 *dev, Conv_cycle_modes_t conv_cyc_mode);
float TMP117_get_temp(TMP117 *dev);

#endif /* TMP117_DRIVER_H */
