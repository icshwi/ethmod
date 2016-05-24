/*
 * AKI2CTemp.h
 *
 *  Created on: May 16, 2016
 *      Author: hinkokocevar
 */

#ifndef _I2CTEMP_H_
#define _I2CTEMP_H_

#include "AKI2C.h"

#define AKI2C_TMP100_TEMPERATURE_REG			0x00
#define AKI2C_TMP100_CONFIG_REG					0x01

#define AKI2C_TMP100_RESOLUTION_9BIT			(0 << 6)
#define AKI2C_TMP100_RESOLUTION_10BIT			(1 << 6)
#define AKI2C_TMP100_RESOLUTION_11BIT			(2 << 6)
#define AKI2C_TMP100_RESOLUTION_12BIT			(3 << 6)

#define AKI2CTempValueString                    "AKI2CTEMP_VALUE"
#define AKI2CTempReadString                     "AKI2CTEMP_READ"

/** Driver for AK-NORD XT-PICO-SXL I2C temperature chip access over TCP/IP socket */
class AKI2CTemp: public AKI2C {
public:
	AKI2CTemp(const char *portName, const char *ipPort,
	        int devCount, const char *devAddrs,
			int muxAddr, int muxBus,
			int priority, int stackSize);
	virtual ~AKI2CTemp();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Our parameter list */
    int AKI2CTempRead;
#define FIRST_AKI2CTEMP_PARAM AKI2CTempRead
    int AKI2CTempValue;
#define LAST_AKI2CTEMP_PARAM AKI2CTempValue

private:
    asynStatus write(int addr, unsigned char reg, unsigned char val);
    asynStatus read(int addr, unsigned char reg);
};

#define NUM_AKI2CTEMP_PARAMS ((int)(&LAST_AKI2CTEMP_PARAM - &FIRST_AKI2CTEMP_PARAM + 1))

#endif /* _I2CTEMP_H_ */
