/*
 * AKI2CTemp.h
 *
 *  Created on: May 16, 2016
 *      Author: hinkokocevar
 */

#ifndef _I2CTEMP_H_
#define _I2CTEMP_H_

#include "AKI2C.h"

#define AKI2CTempDevAddrString                  "AKI2CTEMP_DEV_ADDR"
#define AKI2CTempMuxAddrString                  "AKI2CTEMP_MUX_ADDR"
#define AKI2CTempMuxBusString                   "AKI2CTEMP_MUX_BUS"
#define AKI2CTempTemperatureString              "AKI2CTEMP_TEMPERATURE"
#define AKI2CTempResolutionString               "AKI2CTEMP_RESOLUTION"

/** Driver for AK-NORD XT-PICO-SXL I2C temperature chip access over TCP/IP socket */
class AKI2CTemp: public AKI2C {
public:
	AKI2CTemp(const char *portName, const char *ipPort,
	        int numDevices, int priority, int stackSize);
	virtual ~AKI2CTemp();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Our parameter list */
    int AKI2CTempDevAddr;
#define FIRST_AKI2CTEMP_PARAM AKI2CTempDevAddr
    int AKI2CTempMuxAddr;
    int AKI2CTempMuxBus;
    int AKI2CTempTemperature;
    int AKI2CTempResolution;
#define LAST_AKI2CTEMP_PARAM AKI2CTempResolution

private:
    asynStatus setResolution(int addr, unsigned char val);


};

#define NUM_AKI2CTEMP_PARAMS ((int)(&LAST_AKI2CTEMP_PARAM - &FIRST_AKI2CTEMP_PARAM + 1))

#endif /* _I2CTEMP_H_ */
