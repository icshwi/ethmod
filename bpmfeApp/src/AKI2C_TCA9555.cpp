/*
 * AKI2C_TCA9555.cpp
 *
 *  Created on: May 19, 2016
 *      Author: hinxx
 */


#include "AKI2C_TCA9555.h"

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

static const char *driverName = "AKI2C_TCA9555";


static void exitHandler(void *drvPvt) {
	AKI2C_TCA9555 *pPvt = (AKI2C_TCA9555 *)drvPvt;
	delete pPvt;
}

int AKI2C_TCA9555::changeBit(int val, int bit, int level) {
	if (level) {
		return (val | (1 << bit));
	} else {
		return (val & ~(1 << bit));
	}
}

asynStatus AKI2C_TCA9555::write(int addr, unsigned char reg, unsigned short val) {
	asynStatus status = asynSuccess;
    const char *functionName = "write";
    unsigned char data[2] = {0};
    int devAddr, muxAddr, muxBus;
    unsigned short len;

    getIntegerParam(addr, AKI2CDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    status = setMuxBus(addr, muxAddr, muxBus);
	if (status) {
		return status;
	}

	printf("%s: devAddr %d, muxAddr %d, muxBus %d reg %d ports raw %08X\n",
			functionName, devAddr, muxAddr, muxBus, reg, val);

	data[0] = val & 0xff;
	data[1] = (val >> 8) & 0xff;
	len = 2;
    status = xfer(addr, AK_REQ_TYPE_WRITE, devAddr, 1, data, &len, reg);
    if (status) {
    	return status;
    }

    return status;
}

asynStatus AKI2C_TCA9555::writeLevel(int addr, unsigned char param, unsigned short val) {
    asynStatus status = asynSuccess;
    const char *functionName = "writeOutput";
    unsigned short regVal;
    int pin;

    pin = param - AKI2C_TCA9555_Pin0;

    if (AKI2C_TCA9555_DIRECTION_VAL & (1 << pin)) {
    	/* This pin is input, so do nothing */
        printf("%s: pin %d is input - will not change level!\n", functionName, pin);
    	status = asynError;
    } else {
    	/* This pin is output, so write output registers */
    	regVal = changeBit(mPinLevel, pin, val);
    	status = write(addr, AKI2C_TCA9555_OUTPUT0_REG, regVal);
        if (status) {
        	return status;
        }
        printf("%s: pin %d, value %d\n", functionName, pin, val);
        /* Update the local copy of the value */
        mPinLevel = regVal;
        /* Update asyn parameters */
        setIntegerParam(addr, AKI2C_TCA9555_Level, regVal);
    }

    return status;
}

asynStatus AKI2C_TCA9555::readLevelAll(int addr) {
    asynStatus status = asynSuccess;
    const char *functionName = "readLevelAll";
    unsigned short regVal;
    int pin;
    int level;

	status = read(addr, AKI2C_TCA9555_INPUT0_REG, &regVal);
    if (status) {
    	return status;
    }

	for (pin = AKI2C_TCA9555_Pin0; pin <= AKI2C_TCA9555_Pin15; pin++) {
	    if (AKI2C_TCA9555_DIRECTION_VAL & (1 << pin)) {
	    	/* This pin is input, update individual pin params */
	        level = (regVal >> pin) & 1;
	    	setIntegerParam(addr, pin, level);
			printf("%s: pin %d, value %d\n", functionName, pin, level);
	    }
	}

    /* Update the local copy of the value */
    mPinLevel = regVal;
    /* Update asyn parameters */
    setIntegerParam(addr, AKI2C_TCA9555_Level, regVal);

    return status;
}

asynStatus AKI2C_TCA9555::read(int addr, unsigned char reg, unsigned short *val) {
	asynStatus status = asynSuccess;
    const char *functionName = "read";
    unsigned char data[2] = {0};
    int devAddr, muxAddr, muxBus;
    unsigned short len;

    getIntegerParam(addr, AKI2CDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    status = setMuxBus(addr, muxAddr, muxBus);
	if (status) {
		return status;
	}

	/* read both port 0 and port 1 in one transfer - hence len == 2 */
    len = 2;
    status = xfer(addr, AK_REQ_TYPE_READ, devAddr, 1, data, &len, reg);
    if (status) {
    	return status;
    }

    *val = (mResp[3] << 8) | mResp[2];

    return status;
}

asynStatus AKI2C_TCA9555::writeInt32(asynUser *pasynUser, epicsInt32 value) {

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

    if (function == AKI2C_TCA9555_Read) {
    	status = readLevelAll(addr);
    } else if ((function >= AKI2C_TCA9555_Pin0) && (function <= AKI2C_TCA9555_Pin15)) {
    	status = writeLevel(addr, function, value);
    } else if (function < FIRST_AKI2C_TCA9555_PARAM) {
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

void AKI2C_TCA9555::report(FILE *fp, int details) {

    fprintf(fp, "AKI2C_TCA9555 %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    AKI2C::report(fp, details);
}

/** Constructor for the AKI2C_TCA9555 class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKI2C base class.
  */
AKI2C_TCA9555::AKI2C_TCA9555(const char *portName, const char *ipPort,
        int devCount, const char *devAddrs,
		int muxAddr, int muxBus,
		int priority, int stackSize)
   : AKI2C(portName,
		   ipPort,
		   devCount, devAddrs, muxAddr, muxBus,
		   NUM_AKI2C_TCA9555_PARAMS,
		   0, /* no new interface masks beyond those in AKBase */
		   0, /* no new interrupt masks beyond those in AKBase */
		   ASYN_CANBLOCK | ASYN_MULTIDEVICE, /* asynFlags: ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=0*/
		   1, /* autoConnect YES */
		   priority, stackSize)
{
    int status = asynSuccess;
    const char *functionName = "AKI2C_TCA9555";

    printf("%s: Handling %d devices\n", functionName, maxAddr);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(AKI2C_TCA9555_ReadString,             asynParamInt32,   &AKI2C_TCA9555_Read);
    createParam(AKI2C_TCA9555_LevelString,            asynParamInt32,   &AKI2C_TCA9555_Level);
    createParam(AKI2C_TCA9555_PolarityString,         asynParamInt32,   &AKI2C_TCA9555_Polarity);
    createParam(AKI2C_TCA9555_DirectionString,        asynParamInt32,   &AKI2C_TCA9555_Direction);
    /* These need to be in sequence! */
    createParam(AKI2C_TCA9555_Pin0String,             asynParamInt32,   &AKI2C_TCA9555_Pin0);
    createParam(AKI2C_TCA9555_Pin1String,             asynParamInt32,   &AKI2C_TCA9555_Pin1);
    createParam(AKI2C_TCA9555_Pin2String,             asynParamInt32,   &AKI2C_TCA9555_Pin2);
    createParam(AKI2C_TCA9555_Pin3String,             asynParamInt32,   &AKI2C_TCA9555_Pin3);
    createParam(AKI2C_TCA9555_Pin4String,             asynParamInt32,   &AKI2C_TCA9555_Pin4);
    createParam(AKI2C_TCA9555_Pin5String,             asynParamInt32,   &AKI2C_TCA9555_Pin5);
    createParam(AKI2C_TCA9555_Pin6String,             asynParamInt32,   &AKI2C_TCA9555_Pin6);
    createParam(AKI2C_TCA9555_Pin7String,             asynParamInt32,   &AKI2C_TCA9555_Pin7);
    createParam(AKI2C_TCA9555_Pin8String,             asynParamInt32,   &AKI2C_TCA9555_Pin8);
    createParam(AKI2C_TCA9555_Pin9String,             asynParamInt32,   &AKI2C_TCA9555_Pin9);
    createParam(AKI2C_TCA9555_Pin10String,            asynParamInt32,   &AKI2C_TCA9555_Pin10);
    createParam(AKI2C_TCA9555_Pin11String,            asynParamInt32,   &AKI2C_TCA9555_Pin11);
    createParam(AKI2C_TCA9555_Pin12String,            asynParamInt32,   &AKI2C_TCA9555_Pin12);
    createParam(AKI2C_TCA9555_Pin13String,            asynParamInt32,   &AKI2C_TCA9555_Pin13);
    createParam(AKI2C_TCA9555_Pin14String,            asynParamInt32,   &AKI2C_TCA9555_Pin14);
    createParam(AKI2C_TCA9555_Pin15String,            asynParamInt32,   &AKI2C_TCA9555_Pin15);

    /* set some defaults */
    for (int i = 0; i < devCount; i++) {
		status |= write(i, AKI2C_TCA9555_POLARITY0_REG, AKI2C_TCA9555_POLARITY_VAL);
		status |= write(i, AKI2C_TCA9555_DIRECTION0_REG, AKI2C_TCA9555_DIRECTION_VAL);
		status |= write(i, AKI2C_TCA9555_OUTPUT0_REG, AKI2C_TCA9555_OUTPUT_VAL);
		setIntegerParam(i, AKI2C_TCA9555_Polarity, AKI2C_TCA9555_POLARITY_VAL);
		setIntegerParam(i, AKI2C_TCA9555_Direction, AKI2C_TCA9555_DIRECTION_VAL);
		/* read actual level from the ports */
		status |= readLevelAll(i);
    	/* update port params */
        callParamCallbacks(i, i);
    }

    if (status) {
    	printf("%s: failed to set parameter defaults!\n", functionName);
        printf("%s: init FAIL!\n", functionName);
    	return;
    }

    printf("%s: init complete OK!\n", functionName);
}

AKI2C_TCA9555::~AKI2C_TCA9555() {
    const char *functionName = "~AKI2C_TCA9555";

    printf("%s: shutting down ...\n", functionName);

    printf("%s: shutdown complete!\n", functionName);
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CTCA9555Configure(const char *portName, const char *ipPort,
        int devCount, const char *devAddrs,
		int muxAddr, int muxBus,
		int priority, int stackSize) {
    new AKI2C_TCA9555(portName, ipPort, devCount, devAddrs,
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
static const iocshFuncDef initFuncDef = {"AKI2CTCA9555Configure", 8, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CTCA9555Configure(args[0].sval, args[1].sval,
			args[2].ival, args[3].sval,
			args[4].ival, args[5].ival, args[6].ival, args[7].ival);
}

void AKI2CTCA9555Register(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CTCA9555Register);

} /* extern "C" */





