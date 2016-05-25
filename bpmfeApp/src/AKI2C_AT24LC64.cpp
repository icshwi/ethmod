/*
 * AKI2C_AT24LC64.cpp
 *
 *  Created on: May 16, 2016
 *      Author: hinkokocevar
 */

#include "AKI2C_AT24LC64.h"

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

static const char *driverName = "AKI2C_AT24LC64";


static void exitHandler(void *drvPvt) {
	AKI2C_AT24LC64 *pPvt = (AKI2C_AT24LC64 *)drvPvt;
	delete pPvt;
}

asynStatus AKI2C_AT24LC64::write(int addr, unsigned char *data, unsigned short len, unsigned int off) {
	asynStatus status = asynSuccess;
    const char *functionName = "setData";
    int devAddr, muxAddr, muxBus;

    getIntegerParam(addr, AKI2CDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    status = setMuxBus(addr, muxAddr, muxBus);
	if (status) {
		return status;
	}

    status = xfer(addr, AK_REQ_TYPE_WRITE, devAddr, 2, data, &len, off);

    return status;
}

asynStatus AKI2C_AT24LC64::read(int addr, unsigned char *data, unsigned short *len, unsigned int off) {
	asynStatus status = asynSuccess;
    const char *functionName = "getData";
    int devAddr, muxAddr, muxBus;

    getIntegerParam(addr, AKI2CDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    status = setMuxBus(addr, muxAddr, muxBus);
	if (status) {
		return status;
	}

    status = xfer(addr, AK_REQ_TYPE_READ, devAddr, 2, data, len, off);

    return status;
}

asynStatus AKI2C_AT24LC64::readInt8Array(asynUser *pasynUser, epicsInt8 *value,
                                    size_t nElements, size_t *nIn) {
    int function = pasynUser->reason;
    int addr = 0;
    int length, offset;
    asynStatus status = asynSuccess;
    const char *functionName = "readInt8Array";

    status = getAddress(pasynUser, &addr);
    if (status != asynSuccess) {
    	return(status);
    }

    printf("%s: function %d, addr %d, nElements %ld\n", functionName, function, addr, nElements);

    if (function == AKI2C_AT24LC64_Data) {
        getIntegerParam(addr, AKI2C_AT24LC64_Length, &length);
        getIntegerParam(addr, AKI2C_AT24LC64_Offset, &offset);
		status = read(addr, (unsigned char *)value, (unsigned short *)&length, offset);
		*nIn = length;
	    printf("%s: returning length %d, from offset %d\n", functionName, length, offset);
    } else {
		status = AKI2C::readInt8Array(pasynUser, value, nElements, nIn);
    }

    if (status) {
    	asynPrint(pasynUser, ASYN_TRACE_ERROR,
              "%s:%s: error, status=%d function=%d, addr=%d\n",
              driverName, functionName, status, function, addr);
    } else {
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "%s:%s: function=%d, addr=%d\n",
              driverName, functionName, function, addr);
    }

	return status;
}

asynStatus AKI2C_AT24LC64::writeInt8Array(asynUser *pasynUser, epicsInt8 *value,
                                    size_t nElements) {
    int function = pasynUser->reason;
    int addr = 0;
    int length, offset;
    asynStatus status = asynSuccess;
    const char *functionName = "writeInt8Array";

    status = getAddress(pasynUser, &addr);
    if (status != asynSuccess) {
    	return(status);
    }

    printf("%s: function %d, addr %d, nElements %ld\n", functionName, function, addr, nElements);

    if (function == AKI2C_AT24LC64_Data) {
        getIntegerParam(addr, AKI2C_AT24LC64_Length, &length);
        getIntegerParam(addr, AKI2C_AT24LC64_Offset, &offset);
		status = write(addr, (unsigned char*)value, (unsigned short)length, offset);
    } else {
		status = AKI2C::writeInt8Array(pasynUser, value, nElements);
    }

	/* Do callbacks so higher layers see any changes */
	callParamCallbacks(addr, addr);

    if (status) {
    	asynPrint(pasynUser, ASYN_TRACE_ERROR,
              "%s:%s: error, status=%d function=%d, addr=%d\n",
              driverName, functionName, status, function, addr);
    } else {
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "%s:%s: function=%d, addr=%d\n",
              driverName, functionName, function, addr);
    }

	return status;
}

void AKI2C_AT24LC64::report(FILE *fp, int details) {

    fprintf(fp, "AKI2C_AT24LC64 %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    AKI2C::report(fp, details);
}

/** Constructor for the AKI2C_AT24LC64 class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKI2C base class.
  */
AKI2C_AT24LC64::AKI2C_AT24LC64(const char *portName, const char *ipPort,
        int devCount, const char *devAddrs,
		int muxAddr, int muxBus,
		int priority, int stackSize)
   : AKI2C(portName,
		   ipPort,
		   devCount, devAddrs, muxAddr, muxBus,
		   NUM_AKI2C_AT24LC64_PARAMS,
		   asynInt8ArrayMask,
		   asynInt8ArrayMask,
		   ASYN_CANBLOCK | ASYN_MULTIDEVICE, /* asynFlags: ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=0*/
		   1, /* autoConnect YES */
		   priority, stackSize)
{
    int status = asynSuccess;
    const char *functionName = "AKI2C_AT24LC64";

    printf("%s: Handling %d devices\n", functionName, maxAddr);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(AKI2C_AT24LC64_DataString,      asynParamInt8Array, &AKI2C_AT24LC64_Data);
    createParam(AKI2C_AT24LC64_ReadString,      asynParamInt32,     &AKI2C_AT24LC64_Read);
    createParam(AKI2C_AT24LC64_WriteString,     asynParamInt32,     &AKI2C_AT24LC64_Write);
    createParam(AKI2C_AT24LC64_SizeString,      asynParamInt32,     &AKI2C_AT24LC64_Size);
    createParam(AKI2C_AT24LC64_OffsetString,    asynParamInt32,     &AKI2C_AT24LC64_Offset);
    createParam(AKI2C_AT24LC64_LengthString,    asynParamInt32,     &AKI2C_AT24LC64_Length);

    if (status) {
    	printf("%s: failed to set parameter defaults!\n", functionName);
        printf("%s: init FAIL!\n", functionName);
    	return;
    }

    printf("%s: init complete OK!\n", functionName);
}

AKI2C_AT24LC64::~AKI2C_AT24LC64() {
    const char *functionName = "~AKI2C_AT24LC64";

    printf("%s: shutting down ...\n", functionName);

    printf("%s: shutdown complete!\n", functionName);
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CAT24LC64Configure(const char *portName, const char *ipPort,
        int devCount, const char *devAddrs,
		int muxAddr, int muxBus,
		int priority, int stackSize) {
    new AKI2C_AT24LC64(portName, ipPort, devCount, devAddrs,
    		muxAddr, muxBus, priority, stackSize);
    return(asynSuccess);
}

/* EPICS iocsh shell commands */

static const iocshArg initArg0 = { "portName",        iocshArgString};
static const iocshArg initArg1 = { "ipPort",          iocshArgString};
static const iocshArg initArg2 = { "devCount",        iocshArgInt};
static const iocshArg initArg3 = { "devAddrs",        iocshArgString};
static const iocshArg initArg4 = { "muxAddr",         iocshArgInt};
static const iocshArg initArg5 = { "muxBus",          iocshArgInt};
static const iocshArg initArg6 = { "priority",        iocshArgInt};
static const iocshArg initArg7 = { "stackSize",       iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1,
                                            &initArg2,
											&initArg3,
											&initArg4,
											&initArg5,
											&initArg6,
											&initArg7};
static const iocshFuncDef initFuncDef = {"AKI2CAT24LC64Configure", 8, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CAT24LC64Configure(args[0].sval, args[1].sval,
			args[2].ival, args[3].sval,
			args[4].ival, args[5].ival, args[6].ival, args[7].ival);
}

void AKI2CAT24LC64Register(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CAT24LC64Register);

} /* extern "C" */
