//****************************************************************************
// MIT Media Lab - Biomechatronics
// Jean-Francois (Jeff) Duval
// jfduval@mit.edu
// 03/2015
//****************************************************************************
// safety: safety checks, hazard management
//****************************************************************************
	
#ifndef INC_SAFETY_H
#define INC_SAFETY_H

//****************************************************************************
// Include(s)
//****************************************************************************	
	
#include "main.h"
#include <stdint.h>

//****************************************************************************
// Structure(s)
//****************************************************************************	
	
extern uint8 flag_wdclk;		
	
//****************************************************************************
// Public Function Prototype(s):
//****************************************************************************

void init_safety(void);
uint8_t safety_temp(uint8 temp);
uint8_t safety_volt(uint16 last_v, uint16 low, uint16 high);
uint8_t safety_disconnection(uint16 last_v);
uint8_t safetyWDCLKpwm(uint8_t *errWDCLK);
void testSafetyWDCLKpwmBlocking(void);

//****************************************************************************
// Definition(s):
//****************************************************************************

//WatchDog Clock (WDCLK) uses a 100kHz clock
//1 = 10us, 255 = 2.55ms
#define WDCLK_PULSE			150		//1.5ms/667Hz

//Temperature:
#define TEMP_WARNING		179		//75C
#define TEMP_ERROR			187		//80C
//based on components rated for a minimum of +85C
	
//Temperature filtering:
#define TEMP_BUF_SIZE		64		//10ms/sample, 64 = 0.64s
#define TEMP_BUF_SHIFT		6		//2^TEMP_BUF_SHIFT = TEMP_BUF_SIZE
	
//+3V3: MCP1700 ±3%, we will use ±7.5% (±248mV, 3.0525-3.5475V)
//(using a higher margin to catch big errors, not just conversion gains)
#define M_3V3_LOW			200
#define M_3V3_NORMAL		216
#define M_3V3_HIGH			232	

//+VG:  
//Normal is 10V, range is 8.0-11.0V.
#define M_VG_LOW			112
#define M_VG_NORMAL			152
#define M_VG_HIGH			173
	
//+VB: from 15 to 50V.
#define M_VB_LOW			28
#define M_VB_HIGH			227

//+VB filtering (for the disconnected battery detection):
#define VB_BUF_SIZE			1024	//10ms/sample, 1024 = 10.24s
#define VB_BUF_SHIFT		10		//2^VB_BUF_SHIFT = VB_BUF_SIZE

//Disconnected battery threshold: 80% of the average
//Approximated by (value*13)>>4 = 81.25% of value
#define DISCON_GAIN			13
#define DISCON_SHIFT		4

//Qualitative:	
#define V_LOW				1
#define V_NORMAL			0
#define V_HIGH				2	
#define T_NORMAL			0
#define T_WARNING			1
#define T_ERROR				2
#define BATT_CONNECTED		0
#define BATT_DISCONNECTED	1
//If everything is normal STATUS1 == 0

//****************************************************************************
// Macro(s):
//****************************************************************************

//STATUS1 = [WDCLK, DISCON, TEMPH, TEMPL, VBH, VBL, VGH, VGL]
#define CMB_FLAGS_STATUS1(err_wdclk, err_discon, err_temp, err_v_vb, err_v_vg) \
	(((err_wdclk & 0x01) << 7) | ((err_discon & 0x01) << 6) | ((err_temp & 0x03) << 4) | \
	((err_v_vb & 0x03) << 2) | (err_v_vg & 0x03))
	
//STATUS2 = [0, 0, 0, 0, 0, 0, 3V3H, 3V3L]
#define CMB_FLAGS_STATUS2(err_v_3v3)		(err_v_3v3 & 0x03)
	
#endif	//INC_SAFETY_H
	