/*
 * AKI2C_AD527x.h
 *
 *  Created on: April 20, 2017
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
#include <AKI2C_AD527x.h>

static const char *driverName = "AKI2C_AD527x";


static void exitHandler(void *drvPvt) {
	AKI2C_AD527x *pPvt = (AKI2C_AD527x *)drvPvt;
	delete pPvt;
}

asynStatus AKI2C_AD527x::write(int addr, unsigned short cmd, unsigned short len) {
	asynStatus status = asynSuccess;
	unsigned char data[2] = {0};
	int devAddr;

	getIntegerParam(addr, AKI2CDevAddr, &devAddr);

	data[0] = (cmd >> 8) & 0xFF;
	data[1] = cmd & 0xFF;
	D(printf("WRITE command 0x%04X\n", cmd));
	status = xfer(addr, AK_REQ_TYPE_WRITE, devAddr, 0, data, &len, 0);
	if (status) {
		return status;
	}

	return status;
}

asynStatus AKI2C_AD527x::writeValue(int addr, double val) {
	asynStatus status = asynSuccess;
	unsigned short cmd;
	unsigned short data = 0;

	data = (unsigned short)((val * mTapPoints) / mMaxRes);
	cmd = AKI2C_AD527x_WR_RDAC_CMD | data;
	D(printf("resistance %f, maxRes %d, taps %d, val %f, D %d: cmd 0x%04X\n",
			val, mMaxRes, mTapPoints, val, data, cmd));

	status = write(addr, cmd, 2);
	if (status) {
		return status;
	}

	D(printf("wrote RDAC 0x%04X (%d)\n", data, data));

	data = 0;
	status = read(addr, AKI2C_AD527x_RD_RDAC_CMD, &data, 2);
	D(printf("readback RDAC 0x%04X (%d)\n", data, data));

	return status;
}

asynStatus AKI2C_AD527x::read(int addr, unsigned short cmd, unsigned short *val, unsigned short len) {
	asynStatus status = asynSuccess;
	unsigned char data[2] = {0};
	int devAddr;

	getIntegerParam(addr, AKI2CDevAddr, &devAddr);

	/* Read register - 2 bytes */
	status = xfer(addr, AK_REQ_TYPE_READ, devAddr, 2, data, &len, cmd);
	if (status) {
		return status;
	}

	*val = (mResp[2] & 0xFF) << 8 | (mResp[3] & 0xFF);

	D(printf("cmd 0x%04X, READ value 0x%04X\n", cmd, *val));

	return status;
}

asynStatus AKI2C_AD527x::readValue(int addr) {
	asynStatus status = asynSuccess;
	unsigned short data;
	double res;

	status = read(addr, AKI2C_AD527x_RD_RDAC_CMD, &data, 2);
	if (status) {
		return status;
	}

	res = ((double)data / mTapPoints) * mMaxRes;

	D(printf("resistance %f, maxRes %d, taps %d, D %d: cmd 0x%04X\n",
			res, mMaxRes, mTapPoints, data, AKI2C_AD527x_RD_RDAC_CMD));

	setDoubleParam(addr, AKI2C_AD527x_Value, res);

	return status;
}

asynStatus AKI2C_AD527x::writeInt32(asynUser *pasynUser, epicsInt32 value) {

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

	if (function == AKI2C_AD527x_Read) {
		status = readValue(addr);
	} else if (function == AKI2C_AD527x_Type) {
		switch (value) {
			case AKI2C_AD527x_TYPE_AD5272:
				mTapPoints = 1024;
				break;
			case AKI2C_AD527x_TYPE_AD5274:
				mTapPoints = 256;
				break;
			default:
				status = asynError;
		}
	} else if (function == AKI2C_AD527x_MaxRes) {
		switch (value) {
			case AKI2C_AD527x_MAXRES_20k:
				mMaxRes = 20000;
				break;
			case AKI2C_AD527x_MAXRES_50k:
				mMaxRes = 50000;
				break;
			case AKI2C_AD527x_MAXRES_100k:
				mMaxRes = 100000;
				break;
			default:
				status = asynError;
		}
	} else if (function < FIRST_AKI2C_AD527x_PARAM) {
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

asynStatus AKI2C_AD527x::writeFloat64(asynUser *pasynUser, epicsFloat64 value) {

	int function = pasynUser->reason;
	int addr = 0;
	asynStatus status = asynSuccess;
	const char *functionName = "writeInt32";

	status = getAddress(pasynUser, &addr);
	if (status != asynSuccess) {
		return(status);
	}

	D(printf("function %d, addr %d, value %f\n", function, addr, value));
	status = setDoubleParam(addr, function, value);

	if (function == AKI2C_AD527x_Value) {
		status = writeValue(addr, value);
	} else if (function < FIRST_AKI2C_AD527x_PARAM) {
		/* If this parameter belongs to a base class call its method */
		status = AKI2C::writeFloat64(pasynUser, value);
	}

	/* Do callbacks so higher layers see any changes */
	callParamCallbacks(addr, addr);

	if (status) {
		asynPrint(pasynUser, ASYN_TRACE_ERROR,
			"%s:%s: error, status=%d function=%d, addr=%d, value=%f\n",
			driverName, functionName, status, function, addr, value);
	} else {
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
			"%s:%s: function=%d, addr=%d, value=%f\n",
			driverName, functionName, function, addr, value);
	}

	return status;
}

void AKI2C_AD527x::report(FILE *fp, int details) {

	fprintf(fp, "AKI2C_AD527x %s\n", this->portName);
	if (details > 0) {
	}
	/* Invoke the base class method */
	AKI2C::report(fp, details);
}

/** Constructor for the AKI2C_AD527x class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKI2C base class.
  */
AKI2C_AD527x::AKI2C_AD527x(const char *portName, const char *ipPort,
		int devCount, const char *devInfos, int priority, int stackSize)
	: AKI2C(portName,
		ipPort,
		devCount, devInfos,
		NUM_AKI2C_AD527x_PARAMS,
		0, /* no new interface masks beyond those in AKBase */
		0, /* no new interrupt masks beyond those in AKBase */
		ASYN_CANBLOCK | ASYN_MULTIDEVICE,
		1, /* autoConnect YES */
		priority, stackSize)
{
	unsigned short data;
	asynStatus status = asynSuccess;

	mMaxRes = 0;
	mTapPoints = 0;

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

	createParam(AKI2C_AD527x_ValueString,		asynParamFloat64,	&AKI2C_AD527x_Value);
	createParam(AKI2C_AD527x_ReadString,		asynParamInt32,		&AKI2C_AD527x_Read);
	createParam(AKI2C_AD527x_TypeString,		asynParamInt32,		&AKI2C_AD527x_Type);
	createParam(AKI2C_AD527x_MaxResString,		asynParamInt32,		&AKI2C_AD527x_MaxRes);

	for (int i = 0; i < devCount; i++) {
		setIntegerParam(i, AKI2C_AD527x_Type, 0);
		setIntegerParam(i, AKI2C_AD527x_MaxRes, 0);
		setDoubleParam(i, AKI2C_AD527x_Value, 0.0);
		status = write(i, AKI2C_AD527x_WR_CTRL_CMD | 0x3, 2);
		data = 0;
		status = read(i, AKI2C_AD527x_RD_CTRL_CMD, &data, 2);

		/* Do callbacks so higher layers see any changes */
		callParamCallbacks(i, i);
	}

	if (status) {
		disconnect(pasynUserSelf);
		I(printf("init FAIL!\n"));
		return;
	}

	I(printf("init OK!\n"));
}

AKI2C_AD527x::~AKI2C_AD527x() {
	I(printf("shut down ..\n"));
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CAD527xConfigure(const char *portName, const char *ipPort,
		int devCount, const char *devInfos, int priority, int stackSize) {
	new AKI2C_AD527x(portName, ipPort, devCount, devInfos, priority, stackSize);
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
static const iocshFuncDef initFuncDef = {"AKI2CAD527xConfigure", 6, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CAD527xConfigure(args[0].sval, args[1].sval,
			args[2].ival, args[3].sval, args[4].ival, args[5].ival);
}

void AKI2CAD527xRegister(void) {
	iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CAD527xRegister);

} /* extern "C" */
