/*
 * AKI2C_TMP100.h
 *
 *  Created on: May 16, 2016
 *      Author: hinkokocevar
 */

#ifndef _I2C_TMP100_H_
#define _I2C_TMP100_H_

#include "AKI2C.h"

#define AKI2C_TMP100_TEMPERATURE_REG			0x00
#define AKI2C_TMP100_CONFIG_REG					0x01

#define AKI2C_TMP100_RESOLUTION_SHIFT			5
#define AKI2C_TMP100_RESOLUTION_9BIT			0
#define AKI2C_TMP100_RESOLUTION_10BIT			1
#define AKI2C_TMP100_RESOLUTION_11BIT			2
#define AKI2C_TMP100_RESOLUTION_12BIT			3

#define AKI2C_TMP100_ValueString                "AKI2C_TMP100_VALUE"
#define AKI2C_TMP100_ReadString                 "AKI2C_TMP100_READ"
#define AKI2C_TMP100_ResolutionString           "AKI2C_TMP100_RESOLUTION"

/*
 * Chip       : TI TMP100
 * Function   : temperature sensor
 * Bus        : I2C
 * Access     : TCP/IP socket on AK-NORD XT-PICO-SX
 */
class AKI2C_TMP100: public AKI2C {
public:
	AKI2C_TMP100(const char *portName, const char *ipPort,
	        int devCount, const char *devInfos, int priority, int stackSize);
	virtual ~AKI2C_TMP100();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Our parameter list */
    int AKI2C_TMP100_Read;
#define FIRST_AKI2C_TMP100_PARAM AKI2C_TMP100_Read
    int AKI2C_TMP100_Value;
    int AKI2C_TMP100_Resolution;
#define LAST_AKI2C_TMP100_PARAM AKI2C_TMP100_Resolution

private:
    asynStatus write(int addr, unsigned char reg, unsigned short val, unsigned short len);
    asynStatus read(int addr, unsigned char reg, unsigned short *val, unsigned short len);
    asynStatus readTemperature(int addr);
    asynStatus writeResolution(int addr, unsigned short val);
};

#define NUM_AKI2C_TMP100_PARAMS ((int)(&LAST_AKI2C_TMP100_PARAM - &FIRST_AKI2C_TMP100_PARAM + 1))

#endif /* _I2C_TMP100_H_ */
