/*
 * AKI2IdNum.cpp
 *
 *  Created on: May 16, 2016
 *      Author: hinxx
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
#include "AKI2CIdNum.h"

static const char *driverName = "AKI2CIdNum";


static void exitHandler(void *drvPvt) {
	AKI2CIdNum *pPvt = (AKI2CIdNum *)drvPvt;
	delete pPvt;
}

/* XXX: Untested! */
asynStatus AKI2CIdNum::getIdNumber(int addr) {
	asynStatus status = asynSuccess;
    const char *functionName = "getIdNum";
    unsigned char data[8] = {0};
    int devAddr, muxAddr, muxBus;
    unsigned short len;
    long long int rawIdNum;
    double idNum;

    getIntegerParam(addr, AKI2CDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    status = setMuxBus(addr, muxAddr, muxBus);
	if (status) {
		return status;
	}

	len = 8;
    status = xfer(addr, AK_REQ_TYPE_READ, devAddr, 1, data, &len, 0, 1.0);
    if (status) {
    	return status;
    }

    rawIdNum = (long long int)data[0] << 56
			| (long long int)data[1] << 48
			| (long long int)data[2] << 40
			| (long long int)data[3] << 32
			| (long long int)data[4] << 24
			| (long long int)data[5] << 16
			| (long long int)data[6] << 8
			| (long long int)data[7];
    idNum = (double)rawIdNum;

    printf("%s: devAddr %d, muxAddr %d, muxBus %d serial %lld, %08llX, %f\n", functionName, devAddr, muxAddr, muxBus, rawIdNum, rawIdNum, idNum);

    setDoubleParam(addr, AKI2CIdNumValue, idNum);
    /* Do callbacks so higher layers see any changes */
    callParamCallbacks(addr, addr);

    return status;
}

asynStatus AKI2CIdNum::writeInt32(asynUser *pasynUser, epicsInt32 value) {

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

    if (function == AKI2CIdNumRead) {
    	status = getIdNumber(addr);
    } else if (function < FIRST_AKI2CIDNUM_PARAM) {
        /* If this parameter belongs to a base class call its method */
    	status = AKI2C::writeInt32(pasynUser, value);
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

void AKI2CIdNum::report(FILE *fp, int details) {

    fprintf(fp, "AKI2CIdNum %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    AKI2C::report(fp, details);
}

/** Constructor for the AKI2CIdNum class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKI2C base class.
  */
AKI2CIdNum::AKI2CIdNum(const char *portName, const char *ipPort,
        int numDevices, int priority, int stackSize)
   : AKI2C(portName,
		   ipPort,
		   numDevices,
		   NUM_AKI2CIDNUM_PARAMS,
		   0, /* no new interface masks beyond those in AKBase */
		   0, /* no new interrupt masks beyond those in AKBase */
		   ASYN_CANBLOCK | ASYN_MULTIDEVICE, /* asynFlags: ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=0*/
		   1, /* autoConnect YES */
		   priority, stackSize)
{
    int status = asynSuccess;
    const char *functionName = "AKI2CIdNum";

    printf("%s: Handling %d devices\n", functionName, maxAddr);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(AKI2CIdNumReadString,             asynParamInt32,   &AKI2CIdNumRead);
    createParam(AKI2CIdNumValueString,            asynParamFloat64, &AKI2CIdNumValue);

    for (int i = 0; i < numDevices; i++) {
    	status |= setDoubleParam(i, AKI2CIdNumValue , 0.0);
    }

    if (status) {
    	printf("%s: failed to set parameter defaults!\n", functionName);
        printf("%s: init FAIL!\n", functionName);
    	return;
    }

    printf("%s: init complete OK!\n", functionName);
}

AKI2CIdNum::~AKI2CIdNum() {
    const char *functionName = "~AKI2CIdNum";

    printf("%s: shutting down ...\n", functionName);

    printf("%s: shutdown complete!\n", functionName);
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CIdNumConfigure(const char *portName, const char *ipPort,
        int numDevices, int priority, int stackSize) {
    new AKI2CIdNum(portName, ipPort, numDevices, priority, stackSize);
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
static const iocshFuncDef initFuncDef = {"AKI2CIdNumConfigure", 5, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CIdNumConfigure(args[0].sval, args[1].sval, args[2].ival,
			args[3].ival, args[4].ival);
}

void AKI2CIdNumRegister(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CIdNumRegister);

} /* extern "C" */



