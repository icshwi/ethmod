/*
 * AKBase.cpp
 *
 *  Created on: May 16, 2016
 *      Author: hinkokocevar
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>

#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsExit.h>
#include <epicsExport.h>

#include <asynPortDriver.h>
#include "AKBase.h"

static const char *driverName = "AKBase";


static void exitHandler(void *drvPvt) {
	AKBase *pPvt = (AKBase *)drvPvt;
	delete pPvt;
}

#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 8
#endif
void AKBase::hexdump(void *mem, unsigned int len) {
	unsigned int i, j;

	for (i = 0;	i < len + ((len % HEXDUMP_COLS) ?
							(HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++) {
		/* print offset */
		if (i % HEXDUMP_COLS == 0) {
			printf("0x%06x: ", i);
		}

		/* print hex data */
		if (i < len) {
			printf("%02x ", 0xFF & ((char*) mem)[i]);
		} else {
			/* end of block, just aligning for ASCII dump */
			printf("   ");
		}

		/* print ASCII dump */
		if (i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
			for (j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
				if (j >= len) {
					/* end of block, not really printing */
					putchar(' ');
				} else if (isprint(((char*) mem)[j])) {
					/* printable char */
					putchar(0xFF & ((char*) mem)[j]);
				} else {
					/* other char */
					putchar('.');
				}
			}
			putchar('\n');
		}
	}
}

asynStatus AKBase::writeInt32(asynUser *pasynUser, epicsInt32 value) {

    int function = pasynUser->reason;
    int addr = 0;
    asynStatus status = asynSuccess;
    const char *functionName = "writeInt32";

    status = getAddress(pasynUser, &addr);
    if (status != asynSuccess) {
    	return(status);
    }

    printf("%s: function %d, addr %d, value %d\n", functionName, function, addr, value);
    status = setIntegerParam(addr, function, value);

    /* Do callbacks so higher layers see any changes */
    callParamCallbacks(addr, addr);

    if (status) {
    	asynPrint(pasynUser, ASYN_TRACE_ERROR,
              "%s:%s: error, status=%d function=%d, addr=%d, value=%d\n",
              driverName, functionName, status, function, addr, value);
    } else {
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "%s:%s: function=%d, addr=%d, value=%d\n",
              driverName, functionName, function, addr, value);
    }

    return status;
}

asynStatus AKBase::ipPortWriteRead(double timeout) {
    int eomReason;
    asynStatus status;
    const char *functionName="ipPortWriteRead";

    printf("%s: request:\n", functionName);
	hexdump(mReq, mReqSz);

    status = pasynOctetSyncIO->writeRead(mAsynUserCommand,
    		mReq, mReqSz, mResp, mRespSz,
			timeout, &mReqActSz, &mRespActSz, &eomReason);

    printf("%s: response:\n", functionName);
	hexdump(mResp, mRespActSz);

    if (status) {
    	asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
                "%s:%s, status=%d",
                driverName, functionName, status);
    }

    /* Set output string so it can get back to EPICS */
//    setStringParam(BPMFEStringToServer, this->toBPMFE);
//    setStringParam(BPMFEStringFromServer, this->fromBPMFE);

    return status;
}

asynStatus AKBase::ipPortWrite(double timeout) {
    asynStatus status;
    const char *functionName="ipPortWrite";

    printf("%s: request:\n", functionName);
	hexdump(mReq, mReqSz);

    status = pasynOctetSyncIO->write(mAsynUserCommand,
    		mReq, mReqSz,
			timeout, &mReqActSz);

    if (status) {
    	asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
                "%s:%s, status=%d",
                driverName, functionName, status);
    }

    /* Set output string so it can get back to EPICS */
//    setStringParam(BPMFEStringToServer, this->toBPMFE);
//    setStringParam(BPMFEStringFromServer, this->fromBPMFE);

    return status;
}

asynStatus AKBase::ipPortRead(double timeout) {
    int eomReason;
    asynStatus status;
    const char *functionName = "ipPortRead";

    status = pasynOctetSyncIO->read(mAsynUserCommand,
    		mResp, mRespSz,
			timeout, &mRespActSz, &eomReason);


    printf("%s: response:\n", functionName);
	hexdump(mResp, mRespActSz);

    if (status) {
    	asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "%s:%s, status=%d",
                driverName, functionName, status);
    }

    /* Set output string so it can get back to EPICS */
//    setStringParam(BPMFEStringToServer, this->toBPMFE);
//    setStringParam(BPMFEStringFromServer, this->fromBPMFE);

    return status;
}

void AKBase::report(FILE *fp, int details) {

    fprintf(fp, "AKBase %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    asynPortDriver::report(fp, details);
}

/** Constructor for the AKBase class.
  * Calls constructor for the asynPortDriver base class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] commandPort The name of the TCP/IP server port
  *
  * The other arguments are simply passed to the asynPortDriver base class.
  */
AKBase::AKBase(const char *portName, const char *ipPort, int ipPortType,
		int maxAddr, int numParams, int interfaceMask, int interruptMask,
        int asynFlags, int autoConnect, int priority, int stackSize)
   : asynPortDriver(portName,
		   maxAddr,
		   numParams + NUM_AKBASE_PARAMS,
		   interfaceMask | asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask,
		   interruptMask | asynInt32Mask | asynFloat64Mask | asynOctetMask,
		   asynFlags, autoConnect, priority, stackSize)
{
    int status = asynSuccess;
    const char *functionName = "AKBase";

    mIpPort = strdup(ipPort);
    mIpPortType = ipPortType;
    printf("%s: starting IP port %s, type %d\n", functionName, mIpPort, mIpPortType);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(AKReadStatusString,          asynParamInt32, &AKReadStatus);
    createParam(AKStatusMessageString,       asynParamOctet, &AKStatusMessage);
    createParam(AKStringToServerString,      asynParamOctet, &AKStringToServer);
    createParam(AKStringFromServerString,    asynParamOctet, &AKStringFromServer);

    setStringParam(AKStatusMessage,          "");
    setStringParam(AKStringToServer,         "");
    setStringParam(AKStringFromServer,       "");

    /* Connect to desired IP port */
    status = pasynOctetSyncIO->connect(mIpPort, 0, &mAsynUserCommand, NULL);
    if (status) {
        printf("%s:%s: pasynOctetSyncIO->connect failure\n", driverName, functionName);
        printf("%s: init FAIL!\n", functionName);
        return;
    }

    printf("%s: init complete OK!\n", functionName);
}

AKBase::~AKBase() {
    const char *functionName = "~AKBase";

    printf("%s: shutting down ...\n", functionName);

	pasynOctetSyncIO->disconnect(mAsynUserCommand);

    printf("%s: shutdown complete!\n", functionName);
}

