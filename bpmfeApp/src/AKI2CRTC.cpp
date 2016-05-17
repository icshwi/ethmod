/*
 * AKI2CRTC.cpp
 *
 *  Created on: May 17, 2016
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
#include "AKI2CRTC.h"

static const char *driverName = "AKI2CRTC";


static void exitHandler(void *drvPvt) {
	AKI2CRTC *pPvt = (AKI2CRTC *)drvPvt;
	delete pPvt;
}

/* XXX: Untested! */
asynStatus AKI2CRTC::setDateTime(int addr, unsigned char year, unsigned char month,
		unsigned char weekday, unsigned char day, unsigned char hour, unsigned char minute,
		unsigned char second) {
	asynStatus status = asynSuccess;
    const char *functionName = "setResolution";
    unsigned char data[7] = {0};
    int devAddr, muxAddr, muxBus;
    unsigned short len;

    getIntegerParam(addr, AKI2CRTCDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CRTCMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CRTCMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    status = setMuxBus(muxAddr, muxBus);
	if (status) {
		return status;
	}

    data[0] = second;
    data[1] = minute;
    data[2] = hour;
    data[3] = day;
    data[4] = weekday;
    data[5] = month;
    data[6] = year;
	len = 7;
    status = xfer(AK_REQ_TYPE_WRITE, devAddr, 1, data, &len, 4, 1.0);
    if (status) {
    	return status;
    }

    printf("%s: devAddr %d, muxAddr %d, muxBus %d datetime set to %d-%d-%d %d:%d:%d\n",
    		functionName, devAddr, muxAddr, muxBus, year, month, day, hour, minute, second);

    return status;
}

/* XXX: Untested! */
asynStatus AKI2CRTC::getDateTime(int addr, unsigned char *year, unsigned char *month,
		unsigned char *weekday, unsigned char *day, unsigned char *hour, unsigned char *minute,
		unsigned char *second) {
	asynStatus status = asynSuccess;
    const char *functionName = "getRTCerature";
    unsigned char data[7] = {0};
    int devAddr, muxAddr, muxBus;
    unsigned short len;

    getIntegerParam(addr, AKI2CRTCDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CRTCMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CRTCMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    status = setMuxBus(muxAddr, muxBus);
	if (status) {
		return status;
	}

    len = 7;
    status = xfer(AK_REQ_TYPE_READ, devAddr, 1, data, &len, 4, 1.0);
    if (status) {
    	return status;
    }

    printf("%s: devAddr %d, muxAddr %d, muxBus %d datetime set to %d-%d-%d %d:%d:%d\n",
    		functionName, devAddr, muxAddr, muxBus, data[6], data[5], data[4], data[2], data[1], data[0]);

    setIntegerParam(addr, AKI2CRTCSeconds, data[0]);
    setIntegerParam(addr, AKI2CRTCMinutes, data[1]);
    setIntegerParam(addr, AKI2CRTCHours, data[2]);
    setIntegerParam(addr, AKI2CRTCDays, data[3]);
    setIntegerParam(addr, AKI2CRTCWeekdays, data[4]);
    setIntegerParam(addr, AKI2CRTCMonths, data[5]);
    setIntegerParam(addr, AKI2CRTCYears, data[6]);
    /* Do callbacks so higher layers see any changes */
    callParamCallbacks(addr, addr);

    return status;
}

asynStatus AKI2CRTC::writeInt32(asynUser *pasynUser, epicsInt32 value) {

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

    if (function == AKI2CRTCRead) {
    	//status = getDateTime(addr, value);
    } else if (function == AKI2CRTCWrite) {
    	//status = setDateTime(addr, value);
    } else if (function < FIRST_AKI2CRTC_PARAM) {
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

void AKI2CRTC::report(FILE *fp, int details) {

    fprintf(fp, "AKI2CRTC %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    AKI2C::report(fp, details);
}

/** Constructor for the AKI2CRTC class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKI2C base class.
  */
AKI2CRTC::AKI2CRTC(const char *portName, const char *ipPort,
        int numDevices, int priority, int stackSize)
   : AKI2C(portName,
		   ipPort,
		   numDevices,
		   NUM_AKI2CRTC_PARAMS,
		   0, /* no new interface masks beyond those in AKBase */
		   0, /* no new interrupt masks beyond those in AKBase */
		   ASYN_CANBLOCK | ASYN_MULTIDEVICE, /* asynFlags: ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=0*/
		   1, /* autoConnect YES */
		   priority, stackSize)
{
    int status = asynSuccess;
    const char *functionName = "AKI2CRTC";

    printf("%s: Handling %d devices\n", functionName, maxAddr);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(AKI2CRTCDevAddrString,          asynParamInt32,   &AKI2CRTCDevAddr);
    createParam(AKI2CRTCMuxAddrString,          asynParamInt32,   &AKI2CRTCMuxAddr);
    createParam(AKI2CRTCMuxBusString,           asynParamInt32,   &AKI2CRTCMuxBus);
    createParam(AKI2CRTCReadString,             asynParamInt32,   &AKI2CRTCRead);
    createParam(AKI2CRTCWriteString,            asynParamInt32,   &AKI2CRTCWrite);
    createParam(AKI2CRTCSecondsString,          asynParamInt32,   &AKI2CRTCSeconds);
    createParam(AKI2CRTCMinutesString,          asynParamInt32,   &AKI2CRTCMinutes);
    createParam(AKI2CRTCHoursString,            asynParamInt32,   &AKI2CRTCHours);
    createParam(AKI2CRTCDaysString,             asynParamInt32,   &AKI2CRTCDays);
    createParam(AKI2CRTCWeekDaysString,         asynParamInt32,   &AKI2CRTCWeekdays);
    createParam(AKI2CRTCMonthsString,           asynParamInt32,   &AKI2CRTCMonths);
    createParam(AKI2CRTCYearsString,            asynParamInt32,   &AKI2CRTCYears);

//    status = 0;
//    for (int i = 0; i < numDevices; i++) {
//    	status |= setDoubleParam(i, AKI2CRTCRTCerature, 0.0);
//    }

    if (status) {
    	printf("%s: failed to set parameter defaults!\n", functionName);
        printf("%s: init FAIL!\n", functionName);
    	return;
    }

    printf("%s: init complete OK!\n", functionName);
}

AKI2CRTC::~AKI2CRTC() {
    const char *functionName = "~AKI2CRTC";

    printf("%s: shutting down ...\n", functionName);

    printf("%s: shutdown complete!\n", functionName);
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CRTCConfigure(const char *portName, const char *ipPort,
        int numDevices, int priority, int stackSize) {
    new AKI2CRTC(portName, ipPort, numDevices, priority, stackSize);
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
static const iocshFuncDef initFuncDef = {"AKI2CRTCConfigure", 5, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CRTCConfigure(args[0].sval, args[1].sval, args[2].ival,
			args[3].ival, args[4].ival);
}

void AKI2CRTCRegister(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CRTCRegister);

} /* extern "C" */




