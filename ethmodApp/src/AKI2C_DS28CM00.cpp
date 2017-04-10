/*
 * AKI2_DS28CM00.cpp
 *
 *  Created on: May 16, 2016
 *      Author: hinxx
 */

#include "AKI2C_DS28CM00.h"

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

static const char *driverName = "AKI2C_DS28CM00";


static void exitHandler(void *drvPvt) {
	AKI2C_DS28CM00 *pPvt = (AKI2C_DS28CM00 *)drvPvt;
	delete pPvt;
}

asynStatus AKI2C_DS28CM00::write(int addr, unsigned char reg, unsigned char val, unsigned short len) {
    asynStatus status = asynSuccess;
    unsigned char data[1] = {0};
    int devAddr;

    getIntegerParam(addr, AKI2CDevAddr, &devAddr);

    data[0] = val;
    status = xfer(addr, AK_REQ_TYPE_WRITE, devAddr, 1, data, &len, reg);
    if (status) {
    	return status;
    }

    return status;
}

asynStatus AKI2C_DS28CM00::writeConfig(int addr, unsigned short val) {
    asynStatus status = asynSuccess;

	status = write(addr, AKI2C_DS28CM00_CONTROL_REG, val, 1);
	if (status) {
		return status;
	}

	printf("%s::%s(): param %d, config 0x%02X (%d)\n",
			driverName, __func__, AKI2C_DS28CM00_Smbus, val, val);

    return status;
}

asynStatus AKI2C_DS28CM00::read(int addr, unsigned char reg, unsigned char *val, unsigned short len) {
    asynStatus status = asynSuccess;
    int devAddr;

    getIntegerParam(addr, AKI2CDevAddr, &devAddr);

    status = xfer(addr, AK_REQ_TYPE_READ, devAddr, 1, val, &len, reg);
    if (status) {
    	return status;
    }

	return status;
}

asynStatus AKI2C_DS28CM00::readId(int addr) {
    asynStatus status = asynSuccess;
    unsigned char data[8] = {0};
    char idNum[20] = {0};

	/* Read all 8 registers that constitute ID number */
	status = read(addr, AKI2C_DS28CM00_ID_REG, data, 8);
    if (status) {
    	return status;
    }

    /* Build ID number string */
	sprintf(idNum, "%02X%02X%02X%02X%02X%02X%02X%02X",
			data[0], data[1], data[2], data[3],
			data[4], data[5], data[6], data[7]);

	printf("%s::%s(): ID number %s\n", driverName, __func__, idNum);

	setStringParam(addr, AKI2C_DS28CM00_Value, idNum);

	return status;
}

asynStatus AKI2C_DS28CM00::writeInt32(asynUser *pasynUser, epicsInt32 value) {

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

    if (function == AKI2C_DS28CM00_Read) {
    	status = readId(addr);
    } else if (function == AKI2C_DS28CM00_Smbus) {
    	status = writeConfig(addr, value);
    } else if (function < FIRST_AKI2C_DS28CM00_PARAM) {
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

void AKI2C_DS28CM00::report(FILE *fp, int details) {

    fprintf(fp, "AKI2C_DS28CM00 %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    AKI2C::report(fp, details);
}

/** Constructor for the AKI2C_DS28CM00 class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKI2C base class.
  */
AKI2C_DS28CM00::AKI2C_DS28CM00(const char *portName, const char *ipPort,
        int devCount, const char *devInfos, int priority, int stackSize)
   : AKI2C(portName,
		   ipPort,
		   devCount, devInfos,
		   NUM_AKI2C_DS28CM00_PARAMS,
		   0, /* no new interface masks beyond those in AKBase */
		   0, /* no new interrupt masks beyond those in AKBase */
		   ASYN_CANBLOCK | ASYN_MULTIDEVICE, /* asynFlags: ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=0*/
		   1, /* autoConnect YES */
		   priority, stackSize)
{
    int status = asynSuccess;
    const char *functionName = "AKI2C_DS28CM00";

    printf("%s: Handling %d devices\n", functionName, maxAddr);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(AKI2C_DS28CM00_ReadString,             asynParamInt32,   &AKI2C_DS28CM00_Read);
    createParam(AKI2C_DS28CM00_ValueString,            asynParamOctet,   &AKI2C_DS28CM00_Value);
    createParam(AKI2C_DS28CM00_SmbusString,            asynParamInt32,   &AKI2C_DS28CM00_Smbus);

    for (int i = 0; i < devCount; i++) {
    	setStringParam(i, AKI2C_DS28CM00_Value, "");

        /* Do callbacks so higher layers see any changes */
    	callParamCallbacks(i, i);
    }

    if (status) {
    	printf("%s: failed to set parameter defaults!\n", functionName);
        printf("%s: init FAIL!\n", functionName);
    	return;
    }

    printf("%s: init complete OK!\n", functionName);
}

AKI2C_DS28CM00::~AKI2C_DS28CM00() {
    const char *functionName = "~AKI2C_DS28CM00";

    printf("%s: shutting down ...\n", functionName);

    printf("%s: shutdown complete!\n", functionName);
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CDS28CM00Configure(const char *portName, const char *ipPort,
        int devCount, const char *devInfos, int priority, int stackSize) {
    new AKI2C_DS28CM00(portName, ipPort, devCount, devInfos, priority, stackSize);
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
static const iocshFuncDef initFuncDef = {"AKI2CDS28CM00Configure", 6, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CDS28CM00Configure(args[0].sval, args[1].sval,
			args[2].ival, args[3].sval, args[4].ival, args[5].ival);
}

void AKI2CDS28CM00Register(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CDS28CM00Register);

} /* extern "C" */
