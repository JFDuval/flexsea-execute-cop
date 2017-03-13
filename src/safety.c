//****************************************************************************
// MIT Media Lab - Biomechatronics
// Jean-Francois (Jeff) Duval
// jfduval@mit.edu
// 03/2015
//****************************************************************************
// safety: safety checks, hazard management
//****************************************************************************


//****************************************************************************
// Include(s)
//****************************************************************************

#include "main.h"
#include "safety.h"

//****************************************************************************
// Private Function Prototype(s):
//****************************************************************************

static void init_temp_buffer(void);
static uint8 average_temp(uint8 temp);
static uint16 average_vb(uint16 vb);

//****************************************************************************
// Variable(s)
//****************************************************************************

uint8 flag_wdclk = 0;
uint8 temp_buffer[TEMP_BUF_SIZE];
uint16 vb_buffer[VB_BUF_SIZE];

//****************************************************************************
// Public Function(s)
//****************************************************************************

void init_safety(void)
{
	//Prepare buffer:
	init_temp_buffer();
}

//Safety limits. Returns qualitative result.
uint8 safety_temp(uint8 temp)
{
	uint8 avg_temp = 0, result = 0;
	
	//Filtered temp:
	avg_temp = average_temp(temp);
	
	//Limits:
	if(avg_temp < TEMP_ERROR)
	{
		if(avg_temp < TEMP_WARNING)
		{
			//Temperature is normal
			result = T_NORMAL;
		}
		else
		{
			//Temperature is in the WARNING zone
			result = T_WARNING;
		}
	}
	else
	{
		//Over or equal the TEMP_ERROR limit
		result = T_ERROR;
	}
	
	//Return qualitative result:
	return result;
}

//Is this voltage in range?
uint8 safety_volt(uint16 last_v, uint16 low, uint16 high)
{
	if(last_v < low)
	{
		return V_LOW;
	}
	else if(last_v > high)
	{
		return V_HIGH;
	}

	return V_NORMAL;
}

//Is the battery disconnected? 
uint8 safety_disconnection(uint16 last_v)
{
	uint16 avg_vb = 0, threshold = 0;
	
	//Update average:
	avg_vb = average_vb(last_v);
	
	//Threshold:
	threshold = (DISCON_GAIN * avg_vb) >> DISCON_SHIFT;
	if(last_v < threshold)
		return BATT_DISCONNECTED;
	
	//Otherwise it's fine:
	return BATT_CONNECTED;
}

//Is the WDCLK wrong? Is it time to remove Execute's control over the PWM lines?
#define MAX_INTEG	20
uint8_t safetyWDCLKpwm(uint8_t *errWDCLK)
{
	static int16_t integrator = 0;
	static uint8_t slowDown = 0;
	static uint8_t retVal = 0;
	
	slowDown++;
	slowDown %= 5;
	
	//Integrate the flag:
	if(*errWDCLK)
	{
		//Up: always
		integrator++;
	}
	else
	{
		//Down: skipping turns
		if(!slowDown)
		{
			integrator--;
		}
	}
	
	//Saturation:
	if(integrator > MAX_INTEG)
	{
		integrator = MAX_INTEG;
	}
	if(integrator < 0)
	{
		integrator = 0;
	}
	
	//Reset flag:
	(*errWDCLK) = 0;
	
	//Return value with hysteresis:
	if(integrator >= MAX_INTEG)
	{
		retVal = 1;
	}
	
	if(integrator <= 2)
	{
		retVal = 0;
	}
	
	return retVal;
}

//****************************************************************************
// Test Function(s)
//****************************************************************************

void testSafetyWDCLKpwmBlocking(void)
{
	uint8_t wdclk = 0, i = 0, res = 0;
	
	while(1)
	{
		for(i = 0; i < 20; i++)
		{
			wdclk = 1;
			res = safetyWDCLKpwm(&wdclk);
			wdclk = 0;
			res = safetyWDCLKpwm(&wdclk);
			if(res == 1)
			{
				res = 0;
			}
		}
	}
}

//****************************************************************************
// Private Function(s)
//****************************************************************************

//Fills the buffer with 0s
static void init_temp_buffer(void)
{
	uint16 i = 0;
	
	for(i = 0; i < TEMP_BUF_SIZE; i++)
	{
		temp_buffer[i] = 0;
	}
}

// Returns the average temperature for the last TEMP_BUF_SIZE samples
static uint8 average_temp(uint8 temp)
{
	uint16 i = 0;
	static uint16 pos = 0;
	//TEMP_BUF_SIZE is 128x uint8, so a maximum of 32768. uint16 would do
	//but we will use uint32 just in case we want a bigger buffer in the future
	uint32 temp_sum = 0;
	uint8 result = 0;
	
	//Store last value:
	temp_buffer[pos] = temp;
	
	//Average:
	for(i = 0; i < TEMP_BUF_SIZE; i++)
	{
		temp_sum += temp_buffer[i];
	}
	result = (uint8)((temp_sum >> TEMP_BUF_SHIFT) & 0xFF);
	
	//Increase index:
	pos++;
	if(pos > TEMP_BUF_SIZE)
	{
		pos = 0;
	}
	
	return result;
}

// Returns the average battery voltage for the last VB_BUF_SIZE samples
static uint16 average_vb(uint16 vb)
{
	uint16 i = 0;
	static uint16 pos = 0;
	//VB_BUF_SIZE is 1024x uint16, so a maximum of 67108864. uint32 will do
	uint32 vb_sum = 0;
	uint16 result = 0;
	
	//Store last value:
	vb_buffer[pos] = vb;
	
	//Average:
	for(i = 0; i < VB_BUF_SIZE; i++)
	{
		vb_sum += vb_buffer[i];
	}
	result = (uint16)((vb_sum >> VB_BUF_SHIFT) & 0xFFFF);
	
	//Increase index:
	pos++;
	if(pos > VB_BUF_SIZE)
	{
		pos = 0;
	}
	
	return result;
}
