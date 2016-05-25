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

asynStatus AKI2C_TMP100::write(int addr, unsigned char reg, unsigned char val) {
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

    data[0] = val;
	len = 2;
    status = xfer(addr, AK_REQ_TYPE_WRITE, devAddr, 1, data, &len, reg);
    if (status) {
    	return status;
    }

    printf("%s: devAddr %d, muxAddr %d, muxBus %d reg %02X = %02X\n", functionName, devAddr, muxAddr, muxBus, reg, val);

    return status;
}

asynStatus AKI2C_TMP100::read(int addr, unsigned char reg) {
    asynStatus status = asynSuccess;
    const char *functionName = "read";
    unsigned char data[2] = {0};
    int devAddr, muxAddr, muxBus;
    unsigned short len;
    unsigned int raw;
    double temp;

    getIntegerParam(addr, AKI2CDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    status = setMuxBus(addr, muxAddr, muxBus);
	if (status) {
		return status;
	}

	/* Read only 1 register - temperature */
    len = 2;
    status = xfer(addr, AK_REQ_TYPE_READ, devAddr, 1, data, &len, reg);
    if (status) {
    	return status;
    }

    /* Convert to degrees */
    raw = (mResp[2] << 8 | mResp[3]) >> 4;
    temp = (double)raw / 16.0;

    printf("%s: devAddr %d, muxAddr %d, muxBus %d temperature %d, %f C\n", functionName, devAddr, muxAddr, muxBus, raw, temp);

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

    printf("AKI2C_TMP100::%s: function %d, addr %d, value %d\n", functionName, function, addr, value);
    status = setIntegerParam(addr, function, value);

    if (function == AKI2C_TMP100_Read) {
    	status = read(addr, AKI2C_TMP100_TEMPERATURE_REG);
    } else if (function < FIRST_AKI2C_TMP100_PARAM) {
        printf("AKI2C_TMP100::%s: function %d, addr %d, value %d calling AKI2C::writeInt32 (FIRST_AKI2C_TMP100_PARAM=%d)\n", functionName, function, addr, value, FIRST_AKI2C_TMP100_PARAM);
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
        int devCount, const char *devAddrs,
		int muxAddr, int muxBus,
		int priority, int stackSize)
   : AKI2C(portName,
		   ipPort,
		   devCount, devAddrs, muxAddr, muxBus,
		   NUM_AKI2C_TMP100_PARAMS,
		   0, /* no new interface masks beyond those in AKBase */
		   0, /* no new interrupt masks beyond those in AKBase */
		   ASYN_CANBLOCK | ASYN_MULTIDEVICE, /* asynFlags: ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=0*/
		   1, /* autoConnect YES */
		   priority, stackSize)
{
    int status = asynSuccess;
    const char *functionName = "AKI2C_TMP100";

    printf("%s: Handling %d devices\n", functionName, maxAddr);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(AKI2C_TMP100_ReadString,   asynParamInt32,   &AKI2C_TMP100_Read);
    createParam(AKI2C_TMP100_ValueString,  asynParamFloat64, &AKI2C_TMP100_Value);

    for (int i = 0; i < devCount; i++) {
    	setDoubleParam(i, AKI2C_TMP100_Value, 0.0);
    }

    /* set some defaults */
    for (int i = 0; i < devCount; i++) {
		status |= write(i, AKI2C_TMP100_CONFIG_REG, AKI2C_TMP100_RESOLUTION_12BIT);
    }

    if (status) {
    	printf("%s: failed to set parameter defaults!\n", functionName);
        printf("%s: init FAIL!\n", functionName);
    	return;
    }

    printf("%s: init complete OK!\n", functionName);
}

AKI2C_TMP100::~AKI2C_TMP100() {
    const char *functionName = "~AKI2C_TMP100";

    printf("%s: shutting down ...\n", functionName);

    printf("%s: shutdown complete!\n", functionName);
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CTMP100Configure(const char *portName, const char *ipPort,
        int devCount, const char *devAddrs,
		int muxAddr, int muxBus,
		int priority, int stackSize) {
    new AKI2C_TMP100(portName, ipPort, devCount, devAddrs,
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
static const iocshFuncDef initFuncDef = {"AKI2CTMP100Configure", 8, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CTMP100Configure(args[0].sval, args[1].sval,
			args[2].ival, args[3].sval,
			args[4].ival, args[5].ival, args[6].ival, args[7].ival);
}

void AKI2CTMP100Register(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CTMP100Register);

} /* extern "C" */


