/*
 * AKBase.h
 *
 *  Created on: May 16, 2016
 *      Author: hinkokocevar
 */

#ifndef _AKBASE_H_
#define _AKBASE_H_

#include <asynPortDriver.h>
#include <asynOctetSyncIO.h>


#define AK_IP_PORT_INVALID				0
#define AK_IP_PORT_RS232				1
#define AK_IP_PORT_RS485				2
#define AK_IP_PORT_LCD					3
#define AK_IP_PORT_I2C					4
#define AK_IP_PORT_SPI					5
#define AK_IP_PORT_TTLIO				6

#define AK_MAX_MSG_SZ					512

#define AK_REQ_TYPE_READ				'R'
#define AK_REQ_TYPE_WRITE				'W'

#define AKStatusMessageString           "AK_STATUS_MESSAGE"        /**< (asynOctet,    r/o) Status message */


/** Driver for AK-NORD XT-PICO-SXL server over TCP/IP socket */
class AKBase : public asynPortDriver {
public:
	AKBase(const char *portName, const char *ipPort, int ipPortType,
			int maxAddr, int numParams, int interfaceMask, int interruptMask,
	        int asynFlags, int autoConnect, int priority, int stackSize);
	virtual ~AKBase();

    /* These are the methods that we override from asynPortDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    virtual asynStatus ipPortWrite(double timeout);
    virtual asynStatus ipPortRead(double timeout);
    virtual asynStatus ipPortWriteRead(double timeout);
    /* Derived classes need access to these members */
    char mReq[AK_MAX_MSG_SZ];
    size_t mReqSz;
    size_t mReqActSz;
    char mResp[AK_MAX_MSG_SZ];
    size_t mRespSz;
    size_t mRespActSz;
    char mStatusMsg[AK_MAX_MSG_SZ];

    /* Our parameter list */
    int AKStatusMessage;
#define FIRST_AKBASE_PARAM AKStatusMessage
#define LAST_AKBASE_PARAM AKStatusMessage

private:
    /* These are the methods that are new to this class */
    void hexdump(void *mem, unsigned int len);

    char *mIpPort;
    int mIpPortType;
    asynUser *mAsynUserCommand;
};

#define NUM_AKBASE_PARAMS ((int)(&LAST_AKBASE_PARAM - &FIRST_AKBASE_PARAM + 1))

#endif /* _AKBASE_H_ */
