

#include "tempSensor.h"
#include "Platform.h"
#include "stdint.h"
#include "main.h"
#include "tim.h"
#include "math.h"

uint8_t DS18B20_Start(void);
void DS18B20_Write(uint8_t data);
uint8_t DS18B20_Read(void);
void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
float read_Temp();
void delay(uint32_t us) ;


int8_t Presence = 0;
  uint8_t Temp_byte1=0, Temp_byte2=0;
float _tempFinal;


float getTemp(void){

	return _tempFinal;
}
void delay(uint32_t us) {
	__HAL_TIM_SET_COUNTER(&htim5, 0);
	while ((__HAL_TIM_GET_COUNTER(&htim5)) < us)
		;
}

void readTempSensor(uint16_t sCalib)
{
  u8 u8StrDate[100];
  static stcU16Cnt=0;
  f32 f32Data=0;
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
		HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);

		Presence = DS18B20_Start();
		HAL_Delay(1);
		DS18B20_Write(0xCC);  // skip ROM
		DS18B20_Write(0x44);  // convert t
		HAL_Delay(800);

		Presence = DS18B20_Start();
		HAL_Delay(1);
		DS18B20_Write(0xCC);  // skip ROM
		DS18B20_Write(0xBE);  // Read Scratch-pad

		Temp_byte1 = DS18B20_Read();
		Temp_byte2 = DS18B20_Read();
		//if(0!=read_Temp())
		if(sCalib==0)
		{
			sCalib=100;
		}

		f32Data= ((float) sCalib / 100) * read_Temp();
		_tempFinal=((_tempFinal*stcU16Cnt)+f32Data)/(stcU16Cnt+1);
		if(stcU16Cnt++>100)
		{
			stcU16Cnt=1;
		}
		//snprintf(u8StrDate,100,"%0.1f\r",_tempFinal);
		//TransmitCMDResponse(u8StrDate);

  
}


uint8_t DS18B20_Read(void) {
	uint8_t value = 0;
	Set_Pin_Input(DS18B20_GPIO_Port, DS18B20_Pin);

	for (int i = 0; i < 8; i++) {
		Set_Pin_Output(DS18B20_GPIO_Port, DS18B20_Pin);
		// set as output

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);  // pull the data pin LOW
		delay(2);  // wait for 2 us

		Set_Pin_Input(DS18B20_GPIO_Port, DS18B20_Pin);  // set as input
		if (HAL_GPIO_ReadPin(DS18B20_GPIO_Port, DS18B20_Pin)) // if the pin is HIGH
				{
			value |= 1 << i;  // read = 1
		}
		delay(60);  // wait for 60 us
	}
	return value;
}


void DS18B20_Write(uint8_t data) {
	Set_Pin_Output(DS18B20_GPIO_Port, DS18B20_Pin);  // set as output

	for (int i = 0; i < 8; i++) {

		if ((data & (1 << i)) != 0)  // if the bit is high
				{
			// write 1

			Set_Pin_Output(DS18B20_GPIO_Port, DS18B20_Pin);  // set as output
			HAL_GPIO_WritePin(DS18B20_GPIO_Port, DS18B20_Pin, GPIO_PIN_RESET); // pull the pin LOW
			delay(1);  // wait for 1 us

			Set_Pin_Input(DS18B20_GPIO_Port, DS18B20_Pin);  // set as input
			delay(50);  // wait for 60 us
		}

		else  // if the bit is low
		{
			// write 0

			Set_Pin_Output(DS18B20_GPIO_Port, DS18B20_Pin);
			HAL_GPIO_WritePin(DS18B20_GPIO_Port, DS18B20_Pin, GPIO_PIN_RESET); // pull the pin LOW
			delay(50);  // wait for 60 us

			Set_Pin_Input(DS18B20_GPIO_Port, DS18B20_Pin);
		}
	}
}


uint8_t DS18B20_Start(void) {
	uint8_t Response = 0;
	Set_Pin_Output(DS18B20_GPIO_Port, DS18B20_Pin);   // set the pin as output
	HAL_GPIO_WritePin(DS18B20_GPIO_Port, DS18B20_Pin, GPIO_PIN_RESET);  // pull the pin low
	delay(480);   // delay according to datasheet

	Set_Pin_Input(DS18B20_GPIO_Port, DS18B20_Pin);    // set the pin as input
	delay(80);    // delay according to datasheet

	if (!(HAL_GPIO_ReadPin(DS18B20_GPIO_Port, DS18B20_Pin)))
		Response = 1;    // if the pin is low i.e the presence pulse is detected
	else
		Response = -1;

	delay(400); // 480 us delay totally.

	return Response;
}



void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
	GPIO_InitTypeDef DS18B202 = { 0 };
	DS18B202.Pin = GPIO_Pin;
	DS18B202.Mode = GPIO_MODE_OUTPUT_PP;
	DS18B202.Pull = GPIO_PULLUP;
	DS18B202.Speed = GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(GPIOx, &DS18B202);
}

void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
	GPIO_InitTypeDef DS18B203 = { 0 };
	DS18B203.Pin = GPIO_Pin;
	DS18B203.Mode = GPIO_MODE_INPUT;
	DS18B203.Pull = GPIO_PULLUP;
	DS18B203.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOx, &DS18B203);
}


float read_Temp() {
	// Wake up every 30 seconds and write to the server
	int  sign_bit = 0, a, b;
	int  individual_bits[11] = { 0 };
	float tempCelsius = 0, temp_value = 0, ex = 0;
	long raw = 0, temp_raw = 0;
	//uint32

	// Calculate the temperature from LSB and MSB, making sure we
	// sign-extend the signed 16-bit temperature readinf ('raw')
	// to a Squirrel 32-bit signed integer ('tempCelsius')
	raw = ((Temp_byte2 << 8) | Temp_byte1);
	if (raw != 0xffff)
	{
		sign_bit = (raw & 0xf000);
		sign_bit = sign_bit >> 15;

		for (a = 0; a < 11; a++) {
			temp_raw = raw >> a;
			individual_bits[a] = (temp_raw & 0x01);
		}
		tempCelsius = 0;
		for (b = 0; b < 11; b++) {
			a = (-4 + b);
			ex = pow(2, a);
			temp_value = ((float)individual_bits[b] * ex);
			tempCelsius = (tempCelsius + temp_value);
		}

		if (sign_bit == 1) {
			tempCelsius -= 128;
		} else {
			tempCelsius += 0;
		}
	}

	return tempCelsius;

}
