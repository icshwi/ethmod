/*
 * AKI2CTemp.cpp
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
#include <iocsh.h>

#include <asynPortDriver.h>
#include "AKI2CTemp.h"

static const char *driverName = "AKI2CTemp";


static void exitHandler(void *drvPvt) {
	AKI2CTemp *pPvt = (AKI2CTemp *)drvPvt;
	delete pPvt;
}

asynStatus AKI2CTemp::setResolution(int addr, unsigned char val) {
	asynStatus status = asynSuccess;
    const char *functionName = "setResolution";
    unsigned char data[2] = {0};
    int devAddr, muxAddr, muxBus;

    getIntegerParam(addr, AKI2CTempDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CTempMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CTempMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    if (getCurrentMuxBus(muxAddr) != muxBus) {
		data[0] = muxBus;
		status = xfer(AK_REQ_TYPE_WRITE, muxAddr, 1, data, 1, 0, 1.0);
		if (status) {
			return status;
		}
		setCurrentMuxBus(muxAddr, muxBus);
    }

    data[0] = val << 6;
    status = xfer(AK_REQ_TYPE_WRITE, devAddr, 1, data, 2, 0, 1.0);
    if (status) {
    	return status;
    }

    printf("%s: devAddr %d, muxAddr %d, muxBus %d resolution set to %d\n", functionName, devAddr, muxAddr, muxBus, val);

    return status;
}

asynStatus AKI2CTemp::writeInt32(asynUser *pasynUser, epicsInt32 value) {

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

    if (function == AKI2CTempResolution) {
    	status = setResolution(addr, value);
    } else if (function < FIRST_AKI2CTEMP_PARAM) {
        /* If this parameter belongs to a base class call its method */
    	status = AKBase::writeInt32(pasynUser, value);
    }

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

void AKI2CTemp::report(FILE *fp, int details) {

    fprintf(fp, "AKI2CTemp %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    AKBase::report(fp, details);
}

/** Constructor for the AKI2CTemp class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKBase base class.
  */
AKI2CTemp::AKI2CTemp(const char *portName, const char *ipPort,
        int numDevices, int priority, int stackSize)
   : AKI2C(portName,
		   ipPort,
		   numDevices,
		   NUM_AKI2CTEMP_PARAMS,
		   0, /* no new interface masks beyond those in AKBase */
		   0, /* no new interrupt masks beyond those in AKBase */
		   ASYN_CANBLOCK | ASYN_MULTIDEVICE, /* asynFlags: ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=0*/
		   1, /* autoConnect YES */
		   priority, stackSize)
{
    int status = asynSuccess;
    const char *functionName = "AKI2CTemp";

    printf("%s: Handling %d devices\n", functionName, maxAddr);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(AKI2CTempDevAddrString,          asynParamInt32,   &AKI2CTempDevAddr);
    createParam(AKI2CTempMuxAddrString,          asynParamInt32,   &AKI2CTempMuxAddr);
    createParam(AKI2CTempMuxBusString,           asynParamInt32,   &AKI2CTempMuxBus);
    createParam(AKI2CTempTemperatureString,      asynParamFloat64, &AKI2CTempTemperature);
    createParam(AKI2CTempResolutionString,       asynParamInt32,   &AKI2CTempResolution);

    status = 0;
    for (int i = 0; i < numDevices; i++) {
    	status |= setDoubleParam(i, AKI2CTempTemperature, 0.0);
    }

    if (status) {
    	printf("%s: failed to set parameter defaults!\n", functionName);
        printf("%s: init FAIL!\n", functionName);
    	return;
    }

    printf("%s: init complete OK!\n", functionName);
}

AKI2CTemp::~AKI2CTemp() {
    const char *functionName = "~AKI2CTemp";

    printf("%s: shutting down ...\n", functionName);

    printf("%s: shutdown complete!\n", functionName);
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CTempConfigure(const char *portName, const char *ipPort,
        int numDevices, int priority, int stackSize) {
    new AKI2CTemp(portName, ipPort, numDevices, priority, stackSize);
    return(asynSuccess);
}

/* EPICS iocsh shell commands */

static const iocshArg initArg0 = { "portName",        iocshArgString};
static const iocshArg initArg1 = { "ipPort",          iocshArgString};
static const iocshArg initArg2 = { "numDevices",      iocshArgInt};
static const iocshArg initArg3 = { "priority",        iocshArgInt};
static const iocshArg initArg4 = { "stackSize",       iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1,
                                            &initArg2,
											&initArg3,
											&initArg4};
static const iocshFuncDef initFuncDef = {"AKI2CTempConfigure", 5, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CTempConfigure(args[0].sval, args[1].sval, args[2].ival,
			args[3].ival, args[4].ival);
}

void AKI2CTempRegister(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CTempRegister);

} /* extern "C" */


