/*
 * AKI2CIdNum.h
 *
 *  Created on: May 16, 2016
 *      Author: hinxx
 */

#ifndef _AKI2CIDNUM_H_
#define _AKI2CIDNUM_H_

#include "AKI2C.h"

#define AKI2CIdNumReadString                     "AKI2CIDNUM_READ"
#define AKI2CIdNumValueString                    "AKI2CIDNUM_VALUE"

/** Driver for AK-NORD XT-PICO-SXL I2C serial number chip access over TCP/IP socket */
class AKI2CIdNum: public AKI2C {
public:
	AKI2CIdNum(const char *portName, const char *ipPort,
	        int numDevices, int priority, int stackSize);
	virtual ~AKI2CIdNum();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Our parameter list */
    int AKI2CIdNumRead;
#define FIRST_AKI2CIDNUM_PARAM AKI2CIdNumRead
    int AKI2CIdNumValue;
#define LAST_AKI2CIDNUM_PARAM AKI2CIdNumValue

private:
    asynStatus getIdNumber(int addr);
};

#define NUM_AKI2CIDNUM_PARAMS ((int)(&LAST_AKI2CIDNUM_PARAM - &FIRST_AKI2CIDNUM_PARAM + 1))

#endif /* _AKI2CIDNUM_H_ */
