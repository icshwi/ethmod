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
#define AK_I2C_STATUS_MSG_SZ		512
#define AK_I2C_STATUS_OK			0x06
#define AK_I2C_STATUS_FAIL			0x15

#define AK_I2C_MUX_MAX				8
typedef struct _I2CMuxInfo {
	int addr;
	int bus;
} I2CMuxInfo;

#define AKI2CDevAddrString				"AKI2C_DEV_ADDR"
#define AKI2CMuxAddrString				"AKI2C_MUX_ADDR"
#define AKI2CMuxBusString				"AKI2C_MUX_BUS"

/** Driver for AK-NORD XT-PICO-SXL I2C bus access over TCP/IP socket */
class AKI2C: public AKBase {
public:
	AKI2C(const char *portName, const char *ipPort,
		int devCount, const char *devInfos,
		int numParams, int interfaceMask, int interruptMask,
		int asynFlags, int autoConnect, int priority, int stackSize);
	virtual ~AKI2C();

	/* These are the methods that we override from AKBase */
	void report(FILE *fp, int details);
	/* These are new methods */

protected:
	/* These are new methods */
	asynStatus xfer(int asynAddr, unsigned char type, unsigned char devAddr,
		unsigned char addrWidth, unsigned char *data, unsigned short *len,
		unsigned int off, double timeout = 0.3);
	asynStatus setMuxBus(int asynAddr, int devAddr);

	/* Our parameter list */
	int AKI2CDevAddr;
#define FIRST_AKI2C_PARAM AKI2CDevAddr
	int AKI2CMuxAddr;
	int AKI2CMuxBus;
#define LAST_AKI2C_PARAM AKI2CMuxBus

private:
	/* These are new methods */
	char const *status2Msg(unsigned char status, unsigned char code);
	asynStatus pack(unsigned char type, unsigned char devAddr,
		unsigned char addrWidth, unsigned char *data, unsigned short len,
		unsigned int off);
	asynStatus unpack(unsigned char type, unsigned char *data,
		unsigned short *len, asynStatus status);
	asynStatus doXfer(int asynAddr, unsigned char type, unsigned char devAddr,
		unsigned char addrWidth, unsigned char *data, unsigned short *len,
		unsigned int off, double timeout = 0.3);

	static I2CMuxInfo mMuxInfo[AK_I2C_MUX_MAX];
};

#define NUM_AKI2C_PARAMS ((int)(&LAST_AKI2C_PARAM - &FIRST_AKI2C_PARAM + 1))

#endif /* _AKI2C_H_ */
