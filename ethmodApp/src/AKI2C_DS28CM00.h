/*
 * AKI2C_DS28CM00.h
 *
 *  Created on: May 16, 2016
 *      Author: hinxx
 */

#ifndef _AKI2C_DS28CM00_H_
#define _AKI2C_DS28CM00_H_

#include "AKI2C.h"

#define AKI2C_DS28CM00_ID_REG					0x00
#define AKI2C_DS28CM00_CONTROL_REG				0x08
/* Use I2C protocol instead of SMBUS */
#define AKI2C_DS28CM00_CONTROL_VAL				0x00

#define AKI2C_DS28CM00_ReadString				"AKI2C_DS28CM00_READ"
#define AKI2C_DS28CM00_ValueString				"AKI2C_DS28CM00_VALUE"
#define AKI2C_DS28CM00_SmbusString				"AKI2C_DS28CM00_SMBUS"

/*
 * Chip			: Maxim DS28CM00
 * Function		: ID number
 * Bus			: I2C
 * Access		: TCP/IP socket on AK-NORD XT-PICO-SX
 */
class AKI2C_DS28CM00: public AKI2C {
public:
	AKI2C_DS28CM00(const char *portName, const char *ipPort,
		int devCount, const char *devInfos, int priority, int stackSize);
	virtual ~AKI2C_DS28CM00();

	/* These are the methods that we override from AKI2C */
	virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
	void report(FILE *fp, int details);
	/* These are new methods */

protected:
	/* Our parameter list */
	int AKI2C_DS28CM00_Read;
#define FIRST_AKI2C_DS28CM00_PARAM AKI2C_DS28CM00_Read
	int AKI2C_DS28CM00_Value;
	int AKI2C_DS28CM00_Smbus;
#define LAST_AKI2C_DS28CM00_PARAM AKI2C_DS28CM00_Smbus

private:
	asynStatus write(int addr, unsigned char reg, unsigned char val, unsigned short len);
	asynStatus read(int addr, unsigned char reg, unsigned char *val, unsigned short len);
	asynStatus readId(int addr);
	asynStatus writeConfig(int addr, unsigned short val);
};

#define NUM_AKI2C_DS28CM00_PARAMS ((int)(&LAST_AKI2C_DS28CM00_PARAM - &FIRST_AKI2C_DS28CM00_PARAM + 1))

#endif /* _AKI2C_DS28CM00_H_ */
