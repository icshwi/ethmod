/*
 * AKI2CIOExp.cpp
 *
 *  Created on: May 19, 2016
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
#include "AKI2CIOExp.h"

static const char *driverName = "AKI2CIOExp";


static void exitHandler(void *drvPvt) {
	AKI2CIOExp *pPvt = (AKI2CIOExp *)drvPvt;
	delete pPvt;
}

/* XXX: Untested! */
asynStatus AKI2CIOExp::write(int addr, unsigned char reg, unsigned char pin) {
	asynStatus status = asynSuccess;
    const char *functionName = "write";
    unsigned char data[2] = {0};
    int devAddr, muxAddr, muxBus;
    unsigned short len;
    int val = 0;

    if (addr >= AK_I2C_IOEXP_MAX_PINS) {
    	return asynError;
    }
    if (reg >= 0x08) {
    	return asynError;
    }

    getIntegerParam(addr, AKI2CDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    status = setMuxBus(addr, muxAddr, muxBus);
	if (status) {
		return status;
	}

	switch (reg) {
    case AK_I2C_IOEXP_INPUT0:
    	val = (mPinIn & ~(pin << addr)) | (pin << addr);
        setIntegerParam(addr, AKI2CIOExpPinLevel, pin);
    	break;
    case AK_I2C_IOEXP_OUTPUT0:
    	val = (mPinOut & ~(pin << addr)) | (pin << addr);
        setIntegerParam(addr, AKI2CIOExpPinLevel, pin);
    	break;
    case AK_I2C_IOEXP_POLARITY0:
    	val = (mPinPol & ~(pin << addr)) | (pin << addr);
        setIntegerParam(addr, AKI2CIOExpPinPolarity, pin);
    	break;
    case AK_I2C_IOEXP_DIRECTION0:
    	val = (mPinDir & ~(pin << addr)) | (pin << addr);
        setIntegerParam(addr, AKI2CIOExpPinDirection, pin);
    	break;
    }
	printf("%s: devAddr %d, muxAddr %d, muxBus %d reg %d ports raw %02X\n", functionName, devAddr, muxAddr, muxBus, reg, val);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d reg %d pin %d, value %d\n", functionName, devAddr, muxAddr, muxBus, reg, addr, pin);

	data[0] = val & 0xff;
	data[1] = (val >> 8) & 0xff;
	len = 2;
    status = xfer(addr, AK_REQ_TYPE_WRITE, devAddr, 1, data, &len, reg, 1.0);
    if (status) {
    	return status;
    }

    printf("%s: devAddr %d, muxAddr %d, muxBus %d resolution set to %d\n", functionName, devAddr, muxAddr, muxBus, val);

    return status;
}

/* XXX: Untested! */
asynStatus AKI2CIOExp::read(int addr, unsigned char reg) {
	asynStatus status = asynSuccess;
    const char *functionName = "read";
    unsigned char data[2] = {0};
    int devAddr, muxAddr, muxBus;
    int val;
    int pin;
    unsigned short len;

    if (addr >= AK_I2C_IOEXP_MAX_PINS) {
    	return asynError;
    }
    if (reg >= 0x08) {
    	return asynError;
    }

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
    status = xfer(addr, AK_REQ_TYPE_READ, devAddr, 1, data, &len, reg, 1.0);
    if (status) {
    	return status;
    }

    val = (mResp[3] << 8) | mResp[2];
	printf("%s: devAddr %d, muxAddr %d, muxBus %d reg %d ports raw %02X\n", functionName, devAddr, muxAddr, muxBus, reg, val);
    pin = (val & (1 << addr)) ? 1 : 0;
    printf("%s: devAddr %d, muxAddr %d, muxBus %d reg %d pin %d, value %d\n", functionName, devAddr, muxAddr, muxBus, reg, addr, pin);

	switch (reg) {
    case AK_I2C_IOEXP_INPUT0:
    	mPinIn = val;
        setIntegerParam(addr, AKI2CIOExpPinLevel, pin);
    	break;
    case AK_I2C_IOEXP_OUTPUT0:
    	mPinOut = val;
        setIntegerParam(addr, AKI2CIOExpPinLevel, pin);
    	break;
    case AK_I2C_IOEXP_POLARITY0:
    	mPinPol = val;
        setIntegerParam(addr, AKI2CIOExpPinPolarity, pin);
    	break;
    case AK_I2C_IOEXP_DIRECTION0:
    	mPinDir = val;
        setIntegerParam(addr, AKI2CIOExpPinDirection, pin);
    	break;
    }

	/* Do callbacks so higher layers see any changes */
    callParamCallbacks(addr, addr);

    return status;
}

asynStatus AKI2CIOExp::writeInt32(asynUser *pasynUser, epicsInt32 value) {

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

    if (function == AKI2CIOExpReadInput) {
    	status = read(addr, AK_I2C_IOEXP_INPUT0);
    } else if (function == AKI2CIOExpReadOutput) {
    	status = read(addr, AK_I2C_IOEXP_OUTPUT0);
    } else if (function == AKI2CIOExpReadPolarity) {
    	status = read(addr, AK_I2C_IOEXP_POLARITY0);
    } else if (function == AKI2CIOExpReadDirection) {
    	status = read(addr, AK_I2C_IOEXP_DIRECTION0);
    } else if (function == AKI2CIOExpPinLevel) {
    	status = write(addr, AK_I2C_IOEXP_OUTPUT0, value);
    } else if (function == AKI2CIOExpPinPolarity) {
    	status = write(addr, AK_I2C_IOEXP_POLARITY0, value);
    } else if (function == AKI2CIOExpPinDirection) {
    	status = write(addr, AK_I2C_IOEXP_DIRECTION0, value);
    } else if (function < FIRST_AKI2CIOEXP_PARAM) {
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

void AKI2CIOExp::report(FILE *fp, int details) {

    fprintf(fp, "AKI2CIOExp %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    AKI2C::report(fp, details);
}

/** Constructor for the AKI2CIOExp class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKI2C base class.
  */
AKI2CIOExp::AKI2CIOExp(const char *portName, const char *ipPort,
        int numDevices, int priority, int stackSize)
   : AKI2C(portName,
		   ipPort,
		   numDevices,
		   NUM_AKI2CIOEXP_PARAMS,
		   0, /* no new interface masks beyond those in AKBase */
		   0, /* no new interrupt masks beyond those in AKBase */
		   ASYN_CANBLOCK | ASYN_MULTIDEVICE, /* asynFlags: ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=0*/
		   1, /* autoConnect YES */
		   priority, stackSize)
{
//    int status = asynSuccess;
    const char *functionName = "AKI2CIOExp";

    printf("%s: Handling %d devices\n", functionName, maxAddr);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(AKI2CIOExpReadInputString,         asynParamInt32,   &AKI2CIOExpReadInput);
    createParam(AKI2CIOExpReadOutputString,        asynParamInt32,   &AKI2CIOExpReadOutput);
    createParam(AKI2CIOExpReadPolarityString,      asynParamInt32,   &AKI2CIOExpReadPolarity);
    createParam(AKI2CIOExpReadDirectionString,     asynParamInt32,   &AKI2CIOExpReadDirection);
    createParam(AKI2CIOExpPinLevelString,          asynParamInt32,   &AKI2CIOExpPinLevel);
    createParam(AKI2CIOExpPinPolarityString,       asynParamInt32,   &AKI2CIOExpPinPolarity);
    createParam(AKI2CIOExpPinDirectionString,      asynParamInt32,   &AKI2CIOExpPinDirection);

//    status = 0;
//    for (int i = 0; i < numDevices; i++) {
//    	status |= setDoubleParam(i, AKI2CTempTemperature, 0.0);
//    }
//
//    if (status) {
//    	printf("%s: failed to set parameter defaults!\n", functionName);
//        printf("%s: init FAIL!\n", functionName);
//    	return;
//    }

    printf("%s: init complete OK!\n", functionName);
}

AKI2CIOExp::~AKI2CIOExp() {
    const char *functionName = "~AKI2CIOExp";

    printf("%s: shutting down ...\n", functionName);

    printf("%s: shutdown complete!\n", functionName);
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CIOExpConfigure(const char *portName, const char *ipPort,
        int numDevices, int priority, int stackSize) {
    new AKI2CIOExp(portName, ipPort, numDevices, priority, stackSize);
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
static const iocshFuncDef initFuncDef = {"AKI2CIOExpConfigure", 5, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CIOExpConfigure(args[0].sval, args[1].sval, args[2].ival,
			args[3].ival, args[4].ival);
}

void AKI2CIOExpRegister(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CIOExpRegister);

} /* extern "C" */





