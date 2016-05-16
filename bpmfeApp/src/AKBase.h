/*
 * AKBase.h
 *
 *  Created on: May 16, 2016
 *      Author: hinkokocevar
 */

#ifndef _AKBASE_H_
#define _AKBASE_H_

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <epicsMutex.h>

#include <asynPortDriver.h>
#include <asynOctetSyncIO.h>


#define AK_IP_PORT_INVALID			0
#define AK_IP_PORT_RS232			1
#define AK_IP_PORT_RS485			2
#define AK_IP_PORT_LCD				3
#define AK_IP_PORT_I2C				4
#define AK_IP_PORT_SPI				5
#define AK_IP_PORT_TTLIO			6
//#define AK_IP_PORT_SDCARD			7
//#define AK_IP_PORT_DFCARD			8

#define MAX_MESSAGE_SIZE			512

#define AKReadStatusString                "AK_READ_STATUS"           /**< (asynInt32,    r/w) Write 1 to force a read of status */
#define AKStatusMessageString             "AK_STATUS_MESSAGE"        /**< (asynOctet,    r/o) Status message */
#define AKStringToServerString            "AK_STRING_TO_SERVER"      /**< (asynOctet,    r/o) String sent to server for message-based drivers */
#define AKStringFromServerString          "AK_STRING_FROM_SERVER"    /**< (asynOctet,    r/o) String received from server for message-based drivers */


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
    void dataTask();  /* This should be private but is called from C so must be public */

protected:
    int AKReadStatus;
#define FIRST_AKBASE_PARAM AKReadStatus
    int AKStringToServer;
    int AKStringFromServer;
    int AKStatusMessage;
#define LAST_AKBASE_PARAM AKStatusMessage

private:
    /* These are the methods that are new to this class */
    asynStatus ipPortWrite(double timeout);
    asynStatus ipPortRead(double timeout);
    asynStatus ipPortWriteRead(double timeout);
    void hexdump(void *mem, unsigned int len);

    char *mIpPort;
    int mIpPortType;

    char mReq[MAX_MESSAGE_SIZE];
    size_t mReqLen;
    char mResp[MAX_MESSAGE_SIZE];
    size_t mRespLen;
    asynUser *mAsynUserCommand;
};

#define NUM_AKBASE_PARAMS ((int)(&LAST_AKBASE_PARAM - &FIRST_AKBASE_PARAM + 1))

#endif /* _AKBASE_H_ */
