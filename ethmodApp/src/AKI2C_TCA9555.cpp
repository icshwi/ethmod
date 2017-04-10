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
    unsigned char data[2] = {0};
    int devAddr;
    unsigned short len;

    getIntegerParam(addr, AKI2CDevAddr, &devAddr);

	/* write both port 0 & 1 in one transfer - hence len == 2 */
	data[0] = val & 0xff;
	data[1] = (val >> 8) & 0xff;
	len = 2;
    printf("%s::%s(): reg %d, WRITE value 0x%04X\n", driverName, __func__, reg, val);
    status = xfer(addr, AK_REQ_TYPE_WRITE, devAddr, 1, data, &len, reg);
    if (status) {
    	return status;
    }

    return status;
}

asynStatus AKI2C_TCA9555::read(int addr, unsigned char reg, unsigned short *val) {
	asynStatus status = asynSuccess;
    unsigned char data[2] = {0};
    int devAddr;
    unsigned short len;

    getIntegerParam(addr, AKI2CDevAddr, &devAddr);

	/* read both port 0 & 1 in one transfer - hence len == 2 */
    len = 2;
    status = xfer(addr, AK_REQ_TYPE_READ, devAddr, 1, data, &len, reg);
    if (status) {
    	return status;
    }

    *val = (mResp[3] & 0xFF) << 8 | (mResp[2] & 0xFF);
    printf("%s::%s(): reg %d, READ value 0x%04X\n", driverName, __func__, reg, *val);

    return status;
}

asynStatus AKI2C_TCA9555::writeLevel(int addr, unsigned char param, unsigned short val) {
    asynStatus status = asynSuccess;
    unsigned short regVal;
    int pin;

	status = read(addr, AKI2C_TCA9555_OUTPUT0_REG, &regVal);
    if (status) {
    	return status;
    }

    pin = param - AKI2C_TCA9555_LevelPin0;

	/* Write to output have no effect to the pin level if pin direction is set
	 * to input - see TCA9555 datasheet. */
	regVal = changeBit(regVal, pin, val);
	status = write(addr, AKI2C_TCA9555_OUTPUT0_REG, regVal);
	if (status) {
		return status;
	}

	printf("%s::%s(): param %d, pin %d, level %d\n", driverName, __func__, param, pin, val);
	setIntegerParam(addr, AKI2C_TCA9555_Level, regVal);

    return status;
}

asynStatus AKI2C_TCA9555::readLevelAll(int addr) {
    asynStatus status = asynSuccess;
    unsigned short regVal;
    int param;
    int pin;
    int val;

	status = read(addr, AKI2C_TCA9555_INPUT0_REG, &regVal);
    if (status) {
    	return status;
    }

	for (param = AKI2C_TCA9555_LevelPin0; param <= AKI2C_TCA9555_LevelPin15; param++) {
		/* Input register holds value of the pin level regardless of the
		 * direction - see TCA9555 datasheet. */
		pin = param - AKI2C_TCA9555_LevelPin0;
		val = (regVal >> pin) & 1;
		setIntegerParam(addr, param, val);
		printf("%s::%s(): param %d, pin %d, level %d\n", driverName, __func__, param, pin, val);
	}

    setIntegerParam(addr, AKI2C_TCA9555_Level, regVal);

    return status;
}

asynStatus AKI2C_TCA9555::writeDirection(int addr, unsigned char param, unsigned short val) {
    asynStatus status = asynSuccess;
    unsigned short regVal;
    int pin;

	status = read(addr, AKI2C_TCA9555_DIRECTION0_REG, &regVal);
    if (status) {
    	return status;
    }

    pin = param - AKI2C_TCA9555_DirPin0;

	regVal = changeBit(regVal, pin, val);
	status = write(addr, AKI2C_TCA9555_DIRECTION0_REG, regVal);
	if (status) {
		return status;
	}

	printf("%s::%s(): param %d, pin %d, direction %d\n", driverName, __func__, param, pin, val);
	setIntegerParam(addr, AKI2C_TCA9555_Direction, regVal);

    return status;
}

asynStatus AKI2C_TCA9555::readDirectionAll(int addr) {
    asynStatus status = asynSuccess;
    unsigned short regVal;
    int param;
    int pin;
    int val;

	status = read(addr, AKI2C_TCA9555_DIRECTION0_REG, &regVal);
    if (status) {
    	return status;
    }

	for (param = AKI2C_TCA9555_DirPin0; param <= AKI2C_TCA9555_DirPin15; param++) {
		pin = param - AKI2C_TCA9555_DirPin0;
		val = (regVal >> pin) & 1;
		setIntegerParam(addr, param, val);
		printf("%s::%s(): param %d, pin %d, direction %d\n", driverName, __func__, param, pin, val);
	}

    setIntegerParam(addr, AKI2C_TCA9555_Direction, regVal);

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
    	if (status == asynSuccess) {
    		status = readDirectionAll(addr);
    	}
    } else if ((function >= AKI2C_TCA9555_LevelPin0) && (function <= AKI2C_TCA9555_LevelPin15)) {
    	status = writeLevel(addr, function, value);
    } else if ((function >= AKI2C_TCA9555_DirPin0) && (function <= AKI2C_TCA9555_DirPin15)) {
    	status = writeDirection(addr, function, value);
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
        int devCount, const char *devInfos, int priority, int stackSize)
   : AKI2C(portName,
		   ipPort,
		   devCount, devInfos,
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
    createParam(AKI2C_TCA9555_LevelPin0String,        asynParamInt32,   &AKI2C_TCA9555_LevelPin0);
    createParam(AKI2C_TCA9555_LevelPin1String,        asynParamInt32,   &AKI2C_TCA9555_LevelPin1);
    createParam(AKI2C_TCA9555_LevelPin2String,        asynParamInt32,   &AKI2C_TCA9555_LevelPin2);
    createParam(AKI2C_TCA9555_LevelPin3String,        asynParamInt32,   &AKI2C_TCA9555_LevelPin3);
    createParam(AKI2C_TCA9555_LevelPin4String,        asynParamInt32,   &AKI2C_TCA9555_LevelPin4);
    createParam(AKI2C_TCA9555_LevelPin5String,        asynParamInt32,   &AKI2C_TCA9555_LevelPin5);
    createParam(AKI2C_TCA9555_LevelPin6String,        asynParamInt32,   &AKI2C_TCA9555_LevelPin6);
    createParam(AKI2C_TCA9555_LevelPin7String,        asynParamInt32,   &AKI2C_TCA9555_LevelPin7);
    createParam(AKI2C_TCA9555_LevelPin8String,        asynParamInt32,   &AKI2C_TCA9555_LevelPin8);
    createParam(AKI2C_TCA9555_LevelPin9String,        asynParamInt32,   &AKI2C_TCA9555_LevelPin9);
    createParam(AKI2C_TCA9555_LevelPin10String,       asynParamInt32,   &AKI2C_TCA9555_LevelPin10);
    createParam(AKI2C_TCA9555_LevelPin11String,       asynParamInt32,   &AKI2C_TCA9555_LevelPin11);
    createParam(AKI2C_TCA9555_LevelPin12String,       asynParamInt32,   &AKI2C_TCA9555_LevelPin12);
    createParam(AKI2C_TCA9555_LevelPin13String,       asynParamInt32,   &AKI2C_TCA9555_LevelPin13);
    createParam(AKI2C_TCA9555_LevelPin14String,       asynParamInt32,   &AKI2C_TCA9555_LevelPin14);
    createParam(AKI2C_TCA9555_LevelPin15String,       asynParamInt32,   &AKI2C_TCA9555_LevelPin15);
    /* These need to be in sequence! */
    createParam(AKI2C_TCA9555_DirPin0String,          asynParamInt32,   &AKI2C_TCA9555_DirPin0);
    createParam(AKI2C_TCA9555_DirPin1String,          asynParamInt32,   &AKI2C_TCA9555_DirPin1);
    createParam(AKI2C_TCA9555_DirPin2String,          asynParamInt32,   &AKI2C_TCA9555_DirPin2);
    createParam(AKI2C_TCA9555_DirPin3String,          asynParamInt32,   &AKI2C_TCA9555_DirPin3);
    createParam(AKI2C_TCA9555_DirPin4String,          asynParamInt32,   &AKI2C_TCA9555_DirPin4);
    createParam(AKI2C_TCA9555_DirPin5String,          asynParamInt32,   &AKI2C_TCA9555_DirPin5);
    createParam(AKI2C_TCA9555_DirPin6String,          asynParamInt32,   &AKI2C_TCA9555_DirPin6);
    createParam(AKI2C_TCA9555_DirPin7String,          asynParamInt32,   &AKI2C_TCA9555_DirPin7);
    createParam(AKI2C_TCA9555_DirPin8String,          asynParamInt32,   &AKI2C_TCA9555_DirPin8);
    createParam(AKI2C_TCA9555_DirPin9String,          asynParamInt32,   &AKI2C_TCA9555_DirPin9);
    createParam(AKI2C_TCA9555_DirPin10String,         asynParamInt32,   &AKI2C_TCA9555_DirPin10);
    createParam(AKI2C_TCA9555_DirPin11String,         asynParamInt32,   &AKI2C_TCA9555_DirPin11);
    createParam(AKI2C_TCA9555_DirPin12String,         asynParamInt32,   &AKI2C_TCA9555_DirPin12);
    createParam(AKI2C_TCA9555_DirPin13String,         asynParamInt32,   &AKI2C_TCA9555_DirPin13);
    createParam(AKI2C_TCA9555_DirPin14String,         asynParamInt32,   &AKI2C_TCA9555_DirPin14);
    createParam(AKI2C_TCA9555_DirPin15String,         asynParamInt32,   &AKI2C_TCA9555_DirPin15);

    /* set some defaults */
    for (int i = 0; i < devCount; i++) {
    	/* Normal polarity */
		status |= write(i, AKI2C_TCA9555_POLARITY0_REG, 0x0000);
		/* All pins are inputs */
		status |= write(i, AKI2C_TCA9555_DIRECTION0_REG, 0xFFFF);
		setIntegerParam(i, AKI2C_TCA9555_Polarity, 0x0000);
		setIntegerParam(i, AKI2C_TCA9555_Direction, 0xFFFF);
		/* read actual direction from the ports */
		status |= readDirectionAll(i);
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
        int devCount, const char *devInfos, int priority, int stackSize) {
    new AKI2C_TCA9555(portName, ipPort, devCount, devInfos, priority, stackSize);
    return(asynSuccess);
}

/* EPICS iocsh shell commands */

static const iocshArg initArg0 = { "portName",        iocshArgString};
static const iocshArg initArg1 = { "ipPort",          iocshArgString};
static const iocshArg initArg2 = { "devCount",        iocshArgInt};
static const iocshArg initArg3 = { "devInfos",        iocshArgString};
static const iocshArg initArg4 = { "priority",        iocshArgInt};
static const iocshArg initArg5 = { "stackSize",       iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1,
                                            &initArg2,
											&initArg3,
											&initArg4,
											&initArg5};
static const iocshFuncDef initFuncDef = {"AKI2CTCA9555Configure", 6, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CTCA9555Configure(args[0].sval, args[1].sval,
			args[2].ival, args[3].sval, args[4].ival, args[5].ival);
}

void AKI2CTCA9555Register(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CTCA9555Register);

} /* extern "C" */
