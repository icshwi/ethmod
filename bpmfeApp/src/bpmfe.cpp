/*
 * bpmfe.cpp
 *
 *  Created on: May 12, 2016
 *      Author: hinxx
 */



#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>

#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsExit.h>
#include <iocsh.h>

#include "bpmfe.h"
#include <epicsExport.h>

static const char *driverName = "bpmfe";
void bpmfeTaskC(void *drvPvt);

void bpmfeTaskC(void *drvPvt) {
    BPMFE *pPvt = (BPMFE *)drvPvt;
    pPvt->dataTask();
}

static void exitHandler(void *drvPvt) {
    BPMFE *pPvt = (BPMFE *)drvPvt;
	delete pPvt;
}

void BPMFE::dataTask(void) {
	int status;
	int counter;
	static const char *functionName = "dataTask";

	counter = 0;

	sleep(3);

	lock();

    while (1) {

        /* Has acquisition been stopped? */
        status = epicsEventTryWait(this->stopEventId);
        if (status == epicsEventWaitOK) {
			printf("%s: ending thread for port %s\n", functionName, portName);
            break;
        }

    	unlock();
    	sleep(1);
    	lock();

    	memset(this->toBPMFE, 0, sizeof(this->toBPMFE));
        epicsSnprintf(this->toBPMFE, sizeof(this->toBPMFE),
            "%s [%d]", portName, counter);
    	writeBPMFE(1.0);

    	counter++;

    	unlock();
        callParamCallbacks();
    	lock();

    }
	printf("%s: data thread finished!\n", functionName);
    unlock();
}

asynStatus BPMFE::writeInt32(asynUser *pasynUser, epicsInt32 value) {
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char* functionName = "writeInt32";

    printf("%s: function %d, value %d\n", functionName, function, value);

    /* Set the parameter in the parameter library. */
    status = (asynStatus) setIntegerParam(function, value);

    return status;
}

asynStatus BPMFE::writeReadBPMFE(double timeout) {
    size_t nwrite, nread;
    int eomReason;
    asynStatus status;
    const char *functionName="writeReadBPMFE";

    printf("%s: called..\n", functionName);

    status = pasynOctetSyncIO->writeRead(this->pasynUserCommand,
                                         this->toBPMFE, strlen(this->toBPMFE),
                                         this->fromBPMFE, sizeof(this->fromBPMFE),
                                         timeout, &nwrite, &nread, &eomReason);

    if (status) {
    	asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "%s:%s, status=%d, sent\n%s\n",
                driverName, functionName, status, this->toBPMFE);
    }

    /* Set output string so it can get back to EPICS */
    setStringParam(BPMFEStringToServer, this->toBPMFE);
    setStringParam(BPMFEStringFromServer, this->fromBPMFE);

    return(status);
}

asynStatus BPMFE::writeBPMFE(double timeout) {
    size_t nwrite;
    asynStatus status;
    const char *functionName="writeBPMFE";

    printf("%s: called..\n", functionName);

    status = pasynOctetSyncIO->write(this->pasynUserCommand,
                                     this->toBPMFE, strlen(this->toBPMFE),
                                     timeout, &nwrite);

    if (status) {
    	asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "%s:%s, status=%d, sent\n%s\n",
                driverName, functionName, status, this->toBPMFE);
    }

    /* Set output string so it can get back to EPICS */
    setStringParam(BPMFEStringToServer, this->toBPMFE);
    //setStringParam(BPMFEStringFromServer, this->fromBPMFE);

    return(status);
}

void BPMFE::report(FILE *fp, int details) {

    fprintf(fp, "BPM FE %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    asynPortDriver::report(fp, details);
}

/** Constructor for the BPMFE class.
  * Calls constructor for the asynPortDriver base class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] commandPort The name of the TCP/IP server port */
BPMFE::BPMFE(const char *portName, const char *ipPort, int ipPortType)
   : asynPortDriver(portName,
                    1, /* maxAddr */
                    (int)NUM_BPMFE_PARAMS,
                    asynInt32Mask | asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask | asynOctetMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask | asynOctetMask,  /* Interrupt mask */
                    0, /* asynFlags.  This driver does not block and it is not multi-device, so flag is 0 */
                    1, /* Autoconnect */
                    0, /* Default priority */
                    0) /* Default stack size*/
{
    int status = asynSuccess;
    const char *functionName = "BPMFE";

    mIpPort = strdup(ipPort);
    mIpPortType = ipPortType;
    printf("%s: starting IP port %s, type %d\n", functionName, mIpPort, mIpPortType);

    this->stopEventId = epicsEventCreate(epicsEventEmpty);
    if (!this->stopEventId) {
        printf("%s:%s epicsEventCreate failure for stop event\n",
            driverName, functionName);
        return;
    }

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(BPMFEReadStatusString,          asynParamInt32, &BPMFEReadStatus);
    createParam(BPMFEStatusMessageString,       asynParamOctet, &BPMFEStatusMessage);
    createParam(BPMFEStringToServerString,      asynParamOctet, &BPMFEStringToServer);
    createParam(BPMFEStringFromServerString,    asynParamOctet, &BPMFEStringFromServer);

    /* Connect to desired BPM FE IP port */
    status = pasynOctetSyncIO->connect(mIpPort, 0, &this->pasynUserCommand, NULL);

    setStringParam(BPMFEStatusMessage,          "");
    setStringParam(BPMFEStringToServer,         "");
    setStringParam(BPMFEStringFromServer,       "");

    /* Create the thread that computes the waveforms in the background */
    status = (asynStatus)(epicsThreadCreate("bpmfeTask",
                          epicsThreadPriorityMedium,
                          epicsThreadGetStackSize(epicsThreadStackMedium),
                          (EPICSTHREADFUNC)::bpmfeTaskC,
                          this) == NULL);
    if (status) {
        printf("%s:%s: epicsThreadCreate failure\n", driverName, functionName);
        return;
    }

    printf("%s: init OK!\n", functionName);
}

BPMFE::~BPMFE() {
    const char *functionName = "BPMFE";

    printf("%s: shutting down ...\n", functionName);

    epicsEventSignal(this->stopEventId);
    sleep(1);
	pasynOctetSyncIO->disconnect(pasynUserCommand);

    printf("%s: shutdown complete!\n", functionName);
}


/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int bpmfeConfigure(const char *portName, const char *ipPort, int ipPortType) {
    new BPMFE(portName, ipPort, ipPortType);
    return(asynSuccess);
}

/* EPICS iocsh shell commands */

static const iocshArg initArg0 = { "portName",        iocshArgString};
static const iocshArg initArg1 = { "ipPort",          iocshArgString};
static const iocshArg initArg2 = { "ipPortType",      iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1,
                                            &initArg2};
static const iocshFuncDef initFuncDef = {"BPMFEConfigure", 3, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
    bpmfeConfigure(args[0].sval, args[1].sval, args[2].ival);
}

void BPMFERegister(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(BPMFERegister);

} /* extern "C" */

