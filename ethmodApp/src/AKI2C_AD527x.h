/*
 * AKI2C_AD527x.h
 *
 *  Created on: April 20, 2017
 *      Author: hinkokocevar
 */

#ifndef _I2C_AD527x_H_
#define _I2C_AD527x_H_

#include "AKI2C.h"

/* Two different device types */
#define AKI2C_AD527x_TYPE_AD5272				0
#define AKI2C_AD527x_TYPE_AD5274				1

/* Three different nominal resistances */
#define AKI2C_AD527x_MAXRES_20k					0
#define AKI2C_AD527x_MAXRES_50k					1
#define AKI2C_AD527x_MAXRES_100k				2

/* AD527x datasheet Table 12 */
#define AKI2C_AD527x_WR_RDAC_CMD				0x0400
#define AKI2C_AD527x_RD_RDAC_CMD				0x0800
#define AKI2C_AD527x_WR_CTRL_CMD				0x1C00
#define AKI2C_AD527x_RD_CTRL_CMD				0x2000

/* AD527x datasheet Table 15 */
#define AKI2C_AD527x_TP50_CTRL_BIT				0
#define AKI2C_AD527x_RDAC_CTRL_BIT				1
#define AKI2C_AD527x_RES_PERF_CTRL_BIT			2
#define AKI2C_AD527x_TP50_STAT_BIT				3

#define AKI2C_AD527x_ValueString				"AKI2C_AD527x_VALUE"
#define AKI2C_AD527x_ReadString					"AKI2C_AD527x_READ"
#define AKI2C_AD527x_TypeString					"AKI2C_AD527x_TYPE"
#define AKI2C_AD527x_MaxResString				"AKI2C_AD527x_MAXRES"

/*
 * Chip			: Analog AD527x
 * Function		: digital resistor
 * Bus			: I2C
 * Access		: TCP/IP socket on AK-NORD XT-PICO-SX
 */
class AKI2C_AD527x: public AKI2C {
public:
	AKI2C_AD527x(const char *portName, const char *ipPort,
			int devCount, const char *devInfos, int priority, int stackSize);
	virtual ~AKI2C_AD527x();

	/* These are the methods that we override from AKI2C */
	virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
	virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
	void report(FILE *fp, int details);
	/* These are new methods */

protected:
	/* Our parameter list */
	int AKI2C_AD527x_Value;
#define FIRST_AKI2C_AD527x_PARAM AKI2C_AD527x_Value
	int AKI2C_AD527x_Read;
	int AKI2C_AD527x_Type;
	int AKI2C_AD527x_MaxRes;
#define LAST_AKI2C_AD527x_PARAM AKI2C_AD527x_MaxRes

private:
	asynStatus write(int addr, unsigned short cmd, unsigned short len);
	asynStatus read(int addr, unsigned short cmd, unsigned short *val, unsigned short len);
	asynStatus readValue(int addr);
	asynStatus writeValue(int addr, double val);

	unsigned int mTapPoints;
	unsigned int mMaxRes;
};

#define NUM_AKI2C_AD527x_PARAMS ((int)(&LAST_AKI2C_AD527x_PARAM - &FIRST_AKI2C_AD527x_PARAM + 1))

#endif /* _I2C_AD527x_H_ */
