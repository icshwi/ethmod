/*
 * AKI2C_M24M02.cpp
 *
 *  Created on: April 10, 2017
 *      Author: hinkokocevar
 */

#include "AKI2C_M24M02.h"

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

static const char *driverName = "AKI2C_M24M02";


static void exitHandler(void *drvPvt) {
	AKI2C_M24M02 *pPvt = (AKI2C_M24M02 *)drvPvt;
	delete pPvt;
}

asynStatus AKI2C_M24M02::write(int addr, unsigned char *data, unsigned short len, unsigned int off) {
	asynStatus status = asynSuccess;
	int devAddr, muxAddr, muxBus;

	getIntegerParam(addr, AKI2CDevAddr, &devAddr);
	getIntegerParam(addr, AKI2CMuxAddr, &muxAddr);
	getIntegerParam(addr, AKI2CMuxBus, &muxBus);
	D(printf("devAddr %d, muxAddr %d, muxBus %d\n", devAddr, muxAddr, muxBus));

	status = xfer(addr, AK_REQ_TYPE_WRITE, devAddr, 2, data, &len, off);

	return status;
}

asynStatus AKI2C_M24M02::read(int addr, unsigned char *data, unsigned short *len, unsigned int off) {
	asynStatus status = asynSuccess;
	int devAddr, muxAddr, muxBus;

	getIntegerParam(addr, AKI2CDevAddr, &devAddr);
	getIntegerParam(addr, AKI2CMuxAddr, &muxAddr);
	getIntegerParam(addr, AKI2CMuxBus, &muxBus);
	D(printf("devAddr %d, muxAddr %d, muxBus %d\n", devAddr, muxAddr, muxBus));

	status = xfer(addr, AK_REQ_TYPE_READ, devAddr, 2, data, len, off);

	return status;
}

asynStatus AKI2C_M24M02::readInt8Array(asynUser *pasynUser, epicsInt8 *value,
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

	D(printf("function %d, addr %d, nElements %ld\n", function, addr, nElements));

	if (function == AKI2C_M24M02_Data) {
		getIntegerParam(addr, AKI2C_M24M02_Length, &length);
		getIntegerParam(addr, AKI2C_M24M02_Offset, &offset);
		status = read(addr, (unsigned char *)value, (unsigned short *)&length, offset);
		*nIn = length;
		D(printf("returning length %d, from offset %d\n", length, offset));
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

asynStatus AKI2C_M24M02::writeInt8Array(asynUser *pasynUser, epicsInt8 *value,
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

	D(printf("function %d, addr %d, nElements %ld\n", function, addr, nElements));

	if (function == AKI2C_M24M02_Data) {
		getIntegerParam(addr, AKI2C_M24M02_Length, &length);
		getIntegerParam(addr, AKI2C_M24M02_Offset, &offset);
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

void AKI2C_M24M02::report(FILE *fp, int details) {

	fprintf(fp, "AKI2C_M24M02 %s\n", this->portName);
	if (details > 0) {
	}
	/* Invoke the base class method */
	AKI2C::report(fp, details);
}

/** Constructor for the AKI2C_M24M02 class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKI2C base class.
  */
AKI2C_M24M02::AKI2C_M24M02(const char *portName, const char *ipPort,
		int devCount, const char *devInfos,
		int priority, int stackSize)
	: AKI2C(portName,
		ipPort,
		devCount, devInfos,
		NUM_AKI2C_M24M02_PARAMS,
		asynInt8ArrayMask,
		asynInt8ArrayMask,
		ASYN_CANBLOCK | ASYN_MULTIDEVICE,
		1, /* autoConnect YES */
		priority, stackSize)
{
	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

	createParam(AKI2C_M24M02_DataString,	asynParamInt8Array,	&AKI2C_M24M02_Data);
	createParam(AKI2C_M24M02_ReadString,	asynParamInt32,		&AKI2C_M24M02_Read);
	createParam(AKI2C_M24M02_WriteString,	asynParamInt32,		&AKI2C_M24M02_Write);
	createParam(AKI2C_M24M02_SizeString,	asynParamInt32,		&AKI2C_M24M02_Size);
	createParam(AKI2C_M24M02_OffsetString,	asynParamInt32,		&AKI2C_M24M02_Offset);
	createParam(AKI2C_M24M02_LengthString,	asynParamInt32,		&AKI2C_M24M02_Length);

	I(printf("init OK!\n"));
}

AKI2C_M24M02::~AKI2C_M24M02() {
	I(printf("shut down ..\n"));
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CM24M02Configure(const char *portName, const char *ipPort,
		int devCount, const char *devInfos,
		int priority, int stackSize) {
	new AKI2C_M24M02(portName, ipPort, devCount, devInfos,
			priority, stackSize);
	return(asynSuccess);
}

/* EPICS iocsh shell commands */

static const iocshArg initArg0 = { "portName",		iocshArgString};
static const iocshArg initArg1 = { "ipPort",		iocshArgString};
static const iocshArg initArg2 = { "devCount",		iocshArgInt};
static const iocshArg initArg3 = { "devInfos",		iocshArgString};
static const iocshArg initArg4 = { "priority",		iocshArgInt};
static const iocshArg initArg5 = { "stackSize",		iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
											&initArg1,
											&initArg2,
											&initArg3,
											&initArg4,
											&initArg5};
static const iocshFuncDef initFuncDef = {"AKI2CM24M02Configure", 6, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CM24M02Configure(args[0].sval, args[1].sval,
			args[2].ival, args[3].sval, args[4].ival, args[5].ival);
}

void AKI2CM24M02Register(void) {
	iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CM24M02Register);

} /* extern "C" */
