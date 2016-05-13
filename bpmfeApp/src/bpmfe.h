/*
 * bpmfe.h
 *
 *  Created on: May 12, 2016
 *      Author: hinxx
 */

#ifndef BPMFEAPP_SRC_BPMFE_H_
#define BPMFEAPP_SRC_BPMFE_H_

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <epicsMutex.h>
//#include <ellLib.h>

#include <asynPortDriver.h>
#include <asynOctetSyncIO.h>

#define BPMFE_IP_PORT_INVALID		0
#define BPMFE_IP_PORT_RS232			1
#define BPMFE_IP_PORT_RS485			2
#define BPMFE_IP_PORT_LCD			3
#define BPMFE_IP_PORT_I2C			4
#define BPMFE_IP_PORT_SPI			5
#define BPMFE_IP_PORT_TTLIO			6
#define BPMFE_IP_PORT_SDCARD		7
#define BPMFE_IP_PORT_DFCARD		8

#define MAX_MESSAGE_SIZE	512

#define BPMFEReadStatusString                "READ_STATUS"           /**< (asynInt32,    r/w) Write 1 to force a read of status */
#define BPMFEStatusMessageString             "STATUS_MESSAGE"        /**< (asynOctet,    r/o) Status message */
#define BPMFEStringToServerString            "STRING_TO_SERVER"      /**< (asynOctet,    r/o) String sent to server for message-based drivers */
#define BPMFEStringFromServerString          "STRING_FROM_SERVER"    /**< (asynOctet,    r/o) String received from server for message-based drivers */

/** Driver for BPM frontend using AK-NORD XT-PICO-SXL server over TCP/IP socket */
class BPMFE : public asynPortDriver {
public:
	BPMFE(const char *portName, const char *ipPort, int ipPortType);
	virtual ~BPMFE();

    /* These are the methods that we override from asynPortDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */
    void dataTask();  /* This should be private but is called from C so must be public */
    ELLNODE node;  /* This should be private but is called from C so must be public */

protected:
    int BPMFEReadStatus;
#define FIRST_BPMFE_PARAM BPMFEReadStatus
    int BPMFEStatusMessage;
    int BPMFEStringToServer;
    int BPMFEStringFromServer;
    int BPMFEDummyEnd;
#define LAST_BPMFE_PARAM BPMFEDummyEnd

private:
    /* These are the methods that are new to this class */
    asynStatus writeBPMFE(double timeout);
    asynStatus writeReadBPMFE(double timeout);

    char *mIpPort;
    int mIpPortType;

    /* Our data */
//    epicsEventId startEventId;
//    epicsEventId readoutEventId;
    epicsEventId stopEventId;
//    epicsTimerId timerId;
    char toBPMFE[MAX_MESSAGE_SIZE];
    char fromBPMFE[MAX_MESSAGE_SIZE];
    asynUser *pasynUserCommand;
};

#define NUM_BPMFE_PARAMS ((int)(&LAST_BPMFE_PARAM - &FIRST_BPMFE_PARAM + 1))

#endif /* BPMFEAPP_SRC_BPMFE_H_ */
