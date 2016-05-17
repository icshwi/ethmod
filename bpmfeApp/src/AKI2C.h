/*
 * AKI2C.h
 *
 *  Created on: May 16, 2016
 *      Author: hinkokocevar
 */

#ifndef _AKI2C_H_
#define _AKI2C_H_

#include "AKBase.h"

#define AK_I2C_RESP_HDR_SZ			2
#define AK_I2C_STATUS_MSG_SZ		32
#define AK_I2C_STATUS_OK			0x06
#define AK_I2C_STATUS_FAIL			0x15

#define AK_I2C_MUX_MAX				8
typedef struct _I2CMuxInfo {
	int addr;
	int bus;
} I2CMuxInfo;

/** Driver for AK-NORD XT-PICO-SXL I2C bus access over TCP/IP socket */
class AKI2C: public AKBase {
public:
	AKI2C(const char *portName, const char *ipPort,
			int maxAddr, int numParams, int interfaceMask, int interruptMask,
	        int asynFlags, int autoConnect, int priority, int stackSize);
	virtual ~AKI2C();

    /* These are the methods that we override from AKBase */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* These are new methods */
    asynStatus xfer(unsigned char type, unsigned char devAddr,
    		unsigned char addrWidth, unsigned char *data, unsigned short *len,
    		unsigned int off, double timeout);

    void updateMuxBus(int muxAddr, int muxBus);
    int getMuxBus(int muxAddr);
    asynStatus setMuxBus(int muxAddr, int muxBus);

    /* Our parameter list */
    int AKI2CDummy1;
#define FIRST_AKI2C_PARAM AKI2CDummy1
    int AKI2CDummy2;
#define LAST_AKI2C_PARAM AKI2CDummy2

private:
    /* These are new methods */
    char const *status2Msg(unsigned char status, unsigned char code);
    asynStatus pack(unsigned char type, unsigned char devAddr,
    		unsigned char addrWidth, unsigned char *data, unsigned short len,
    		unsigned int off);
    asynStatus unpack(unsigned char type, unsigned char *data, unsigned short *len);

    I2CMuxInfo mMuxInfo[AK_I2C_MUX_MAX];
//    unsigned char mStatus;
    char mStatusMsg[AK_I2C_STATUS_MSG_SZ];
};

#define NUM_AKI2C_PARAMS ((int)(&LAST_AKI2C_PARAM - &FIRST_AKI2C_PARAM + 1))


#endif /* _AKI2C_H_ */
