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

#include <asynPortDriver.h>
#include <asynOctetSyncIO.h>

#define MAX_MESSAGE_SIZE	512

#define BPMFEStringToServerString            "STRING_TO_SERVER"      /**< (asynOctet,    r/o) String sent to server for message-based drivers */
#define BPMFEStringFromServerString          "STRING_FROM_SERVER"    /**< (asynOctet,    r/o) String received from server for message-based drivers */

/** Driver for BPM frontend using AK-NORD XT-PICO-SXL server over TCP/IP socket */
class BPMFE : public asynPortDriver {
public:
	BPMFE(const char *portName, const char *commandPort);

    /* These are the methods that we override from asynPortDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */
    void dataTask();  /* This should be private but is called from C so must be public */

protected:
    int BPMFEStringToServer;
#define FIRST_BPMFE_PARAM BPMFEStringToServer
    int BPMFEStringFromServer;
    int BPMFEDummyEnd;
#define LAST_BPMFE_PARAM BPMFEDummyEnd

private:
    /* These are the methods that are new to this class */
    asynStatus writeReadBPMFE(double timeout);

    /* Our data */
    epicsEventId startEventId;
    epicsEventId readoutEventId;
    epicsEventId stopEventId;
    epicsTimerId timerId;
    char toBPMFE[MAX_MESSAGE_SIZE];
    char fromBPMFE[MAX_MESSAGE_SIZE];
    asynUser *pasynUserCommand;
};

#define NUM_BPMFE_PARAMS ((int)(&LAST_BPMFE_PARAM - &FIRST_BPMFE_PARAM + 1))

#endif /* BPMFEAPP_SRC_BPMFE_H_ */
