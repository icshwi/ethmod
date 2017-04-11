/*
 * AKI2C_TMP100.cpp
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
#include <AKI2C_TMP100.h>

static const char *driverName = "AKI2C_TMP100";


static void exitHandler(void *drvPvt) {
	AKI2C_TMP100 *pPvt = (AKI2C_TMP100 *)drvPvt;
	delete pPvt;
}

asynStatus AKI2C_TMP100::write(int addr, unsigned char reg, unsigned short val, unsigned short len) {
	asynStatus status = asynSuccess;
	unsigned char data[2] = {0};
	int devAddr;

	getIntegerParam(addr, AKI2CDevAddr, &devAddr);

	/* Even if writing only one byte, we can use two byte buffer with proper
	 * length set! */
	data[0] = val & 0xFF;
	data[1] = (val >> 8) & 0xFF;
	D(printf("reg %d, WRITE value 0x%04X\n", reg, val));
	status = xfer(addr, AK_REQ_TYPE_WRITE, devAddr, 1, data, &len, reg);
	if (status) {
		return status;
	}

	return status;
}

asynStatus AKI2C_TMP100::writeResolution(int addr, unsigned short val) {
	asynStatus status = asynSuccess;
	unsigned short regVal;

	status = read(addr, AKI2C_TMP100_CONFIG_REG, &regVal, 1);
	if (status) {
		return status;
	}

	/* Preserve the other config register bits! */
	regVal = regVal & ~(0x03 << AKI2C_TMP100_RESOLUTION_SHIFT);
	regVal |= val << AKI2C_TMP100_RESOLUTION_SHIFT;
	status = write(addr, AKI2C_TMP100_CONFIG_REG, regVal, 1);
	if (status) {
		return status;
	}

	D(printf("param %d, resolution 0x%02X (%d)\n",
		AKI2C_TMP100_Resolution, val, val));

	return status;
}

asynStatus AKI2C_TMP100::read(int addr, unsigned char reg, unsigned short *val, unsigned short len) {
	asynStatus status = asynSuccess;
	unsigned char data[2] = {0};
	int devAddr;

	getIntegerParam(addr, AKI2CDevAddr, &devAddr);

	/* Read register - config is 1 byte long, others 2 bytes */
	status = xfer(addr, AK_REQ_TYPE_READ, devAddr, 1, data, &len, reg);
	if (status) {
		return status;
	}

	*val = mResp[2] & 0xFF;
	if (len == 2) {
		*val = (mResp[2] & 0xFF) << 8 | (mResp[3] & 0xFF);
	}
	D(printf("reg %d, READ value 0x%04X\n", reg, *val));

	return status;
}

asynStatus AKI2C_TMP100::readTemperature(int addr) {
	asynStatus status = asynSuccess;
	unsigned short regVal;
	int val;
	int res;
	int msb;
	double temp;
	double fact;

	status = read(addr, AKI2C_TMP100_TEMPERATURE_REG, &regVal, 2);
	if (status) {
		return status;
	}

	/* Some LSB bits are unused, also set proper conversion factor dpending
	 * on the resolution. */
	getIntegerParam(addr, AKI2C_TMP100_Resolution, &res);
	switch (res) {
	case AKI2C_TMP100_RESOLUTION_9BIT:
		msb = 8;
		regVal >>= 7;
		fact = 0.5;
		break;
	case AKI2C_TMP100_RESOLUTION_10BIT:
		msb = 9;
		regVal >>= 6;
		fact = 0.25;
		break;
	case AKI2C_TMP100_RESOLUTION_11BIT:
		msb = 10;
		regVal >>= 5;
		fact = 0.125;
		break;
	case AKI2C_TMP100_RESOLUTION_12BIT:
		msb = 11;
		regVal >>= 4;
		fact = 0.0625;
		break;
	default:
		return asynError;
	}

	/* if msb == 1 we have negative value */
	if (regVal & (1 << msb)) {
		val = (regVal & ((1 << (msb + 1)) - 1)) | ~((1 << msb) - 1);
	} else {
		val = (regVal & ((1 << (msb + 1)) - 1));
	}

	/* Convert to degrees using proper resolution factor. */
	temp = (double)val * fact;

	D(printf("param %d, temperature raw %d, converted %f C\n",
			AKI2C_TMP100_Value, val, temp));

	setDoubleParam(addr, AKI2C_TMP100_Value, temp);

	return status;
}

asynStatus AKI2C_TMP100::writeInt32(asynUser *pasynUser, epicsInt32 value) {

	int function = pasynUser->reason;
	int addr = 0;
	asynStatus status = asynSuccess;
	const char *functionName = "writeInt32";

	status = getAddress(pasynUser, &addr);
	if (status != asynSuccess) {
		return(status);
	}

	D(printf("function %d, addr %d, value %d\n", function, addr, value));
	status = setIntegerParam(addr, function, value);

	if (function == AKI2C_TMP100_Read) {
		status = readTemperature(addr);
	} else if (function == AKI2C_TMP100_Resolution) {
		status = writeResolution(addr, value & 0x3);
	} else if (function < FIRST_AKI2C_TMP100_PARAM) {
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

void AKI2C_TMP100::report(FILE *fp, int details) {

	fprintf(fp, "AKI2C_TMP100 %s\n", this->portName);
	if (details > 0) {
	}
	/* Invoke the base class method */
	AKI2C::report(fp, details);
}

/** Constructor for the AKI2C_TMP100 class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKI2C base class.
  */
AKI2C_TMP100::AKI2C_TMP100(const char *portName, const char *ipPort,
		int devCount, const char *devInfos, int priority, int stackSize)
	: AKI2C(portName,
		ipPort,
		devCount, devInfos,
		NUM_AKI2C_TMP100_PARAMS,
		0, /* no new interface masks beyond those in AKBase */
		0, /* no new interrupt masks beyond those in AKBase */
		ASYN_CANBLOCK | ASYN_MULTIDEVICE,
		1, /* autoConnect YES */
		priority, stackSize)
{
	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

	createParam(AKI2C_TMP100_ReadString,		asynParamInt32,		&AKI2C_TMP100_Read);
	createParam(AKI2C_TMP100_ValueString,		asynParamFloat64,	&AKI2C_TMP100_Value);
	createParam(AKI2C_TMP100_ResolutionString,	asynParamInt32,		&AKI2C_TMP100_Resolution);

	for (int i = 0; i < devCount; i++) {
		setDoubleParam(i, AKI2C_TMP100_Value, 0.0);

		/* Do callbacks so higher layers see any changes */
		callParamCallbacks(i, i);
	}

	I(printf("init OK!\n"));
}

AKI2C_TMP100::~AKI2C_TMP100() {
	I(printf("shut down ..\n"));
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CTMP100Configure(const char *portName, const char *ipPort,
		int devCount, const char *devInfos, int priority, int stackSize) {
	new AKI2C_TMP100(portName, ipPort, devCount, devInfos, priority, stackSize);
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
static const iocshFuncDef initFuncDef = {"AKI2CTMP100Configure", 6, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CTMP100Configure(args[0].sval, args[1].sval,
			args[2].ival, args[3].sval, args[4].ival, args[5].ival);
}

void AKI2CTMP100Register(void) {
	iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CTMP100Register);

} /* extern "C" */
