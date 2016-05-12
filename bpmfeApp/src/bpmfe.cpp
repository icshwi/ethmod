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
#include <iocsh.h>

#include "bpmfe.h"
#include <epicsExport.h>

static const char *driverName = "bpmfe";
void bpmfeTaskC(void *drvPvt);

void bpmfeTaskC(void *drvPvt) {
    BPMFE *pPvt = (BPMFE *)drvPvt;
    pPvt->dataTask();
}

void BPMFE::dataTask(void) {
    lock();

    while (1) {
    	unlock();
    	(void) epicsEventWait(startEventId);
    	sleep(1);
    	lock();

        callParamCallbacks();

    }
    unlock();
}

asynStatus BPMFE::writeInt32(asynUser *pasynUser, epicsInt32 value) {
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char* functionName = "writeInt32";

    /* Set the parameter in the parameter library. */
    status = (asynStatus) setIntegerParam(function, value);

    return status;
}

asynStatus BPMFE::writeReadBPMFE(double timeout)
{
    size_t nwrite, nread;
    int eomReason;
    asynStatus status;
    const char *functionName="writeReadBPMFE";

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

void BPMFE::report(FILE *fp, int details) {

    fprintf(fp, "BPM frontend %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    asynPortDriver::report(fp, details);
}

/** Constructor for the BPMFE class.
  * Calls constructor for the asynPortDriver base class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] commandPort The name of the TCP/IP server port */
BPMFE::BPMFE(const char *portName, const char *commandPort)
   : asynPortDriver(portName,
                    1, /* maxAddr */
                    (int)NUM_BPMFE_PARAMS,
                    asynInt32Mask | asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask,  /* Interrupt mask */
                    0, /* asynFlags.  This driver does not block and it is not multi-device, so flag is 0 */
                    1, /* Autoconnect */
                    0, /* Default priority */
                    0) /* Default stack size*/
{
    int status = asynSuccess;
    const char *functionName = "BPMFE";


    createParam(BPMFEStringToServerString,      asynParamOctet, &BPMFEStringToServer);
    createParam(BPMFEStringFromServerString,    asynParamOctet, &BPMFEStringFromServer);

    /* Connect to BPM FE */
    status = pasynOctetSyncIO->connect(commandPort, 0, &this->pasynUserCommand, NULL);

    setStringParam (BPMFEStringToServer,         "");
    setStringParam (BPMFEStringFromServer,       "");

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


/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int bpmfeConfigure(const char *portName, const char *commandPort) {
    new BPMFE(portName, commandPort);
    return(asynSuccess);
}

/* EPICS iocsh shell commands */

static const iocshArg initArg0 = { "portName",     iocshArgString};
static const iocshArg initArg1 = { "commandPort",  iocshArgString};
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1};
static const iocshFuncDef initFuncDef = {"BPMFEConfigure", 2, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
    bpmfeConfigure(args[0].sval, args[1].sval);
}

void BPMFERegister(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(BPMFERegister);

} /* extern "C" */

