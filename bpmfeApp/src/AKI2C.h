/*
 * AKI2C.h
 *
 *  Created on: May 16, 2016
 *      Author: hinkokocevar
 */

#ifndef _AKI2C_H_
#define _AKI2C_H_

#include "AKBase.h"

/** Driver for AK-NORD XT-PICO-SXL I2C bus access over TCP/IP socket */
class AKI2C: public AKBase {
public:
	AKI2C(const char *portName, const char *ipPort,
			int maxAddr, int numParams, int interfaceMask, int interruptMask,
	        int asynFlags, int autoConnect, int priority, int stackSize);
	virtual ~AKI2C();

    /* These are the methods that we override from asynPortDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Override to check for response status */
    virtual asynStatus ipPortWrite(double timeout);
    virtual asynStatus ipPortRead(double timeout);
    virtual asynStatus ipPortWriteRead(double timeout);

    /* These are new methods */
    asynStatus createRequest(unsigned char type, unsigned char devAddr,
    		unsigned char addrWidth, unsigned char *data, unsigned short len,
    		unsigned int off);


    /* Our parameter list */
    int AKI2CDummy1;
#define FIRST_AKI2C_PARAM AKI2CDummy1
    int AKI2CDummy2;
#define LAST_AKI2C_PARAM AKI2CDummy2

private:



};

#define NUM_AKI2C_PARAMS ((int)(&LAST_AKI2C_PARAM - &FIRST_AKI2C_PARAM + 1))


#endif /* _AKI2C_H_ */
