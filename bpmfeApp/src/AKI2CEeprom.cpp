/*
 * AKI2CEeprom.cpp
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
#include "AKI2CEeprom.h"

static const char *driverName = "AKI2CEeprom";


static void exitHandler(void *drvPvt) {
	AKI2CEeprom *pPvt = (AKI2CEeprom *)drvPvt;
	delete pPvt;
}

/* XXX: Untested! */
asynStatus AKI2CEeprom::setData(int addr, unsigned char *data, unsigned short len, unsigned int off) {
	asynStatus status = asynSuccess;
    const char *functionName = "setData";
    int devAddr, muxAddr, muxBus;

    getIntegerParam(addr, AKI2CEepromDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CEepromMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CEepromMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    if (getCurrentMuxBus(muxAddr) != muxBus) {
		data[0] = muxBus;
		status = xfer(AK_REQ_TYPE_WRITE, muxAddr, 1, data, 1, 0, 1.0);
		if (status) {
			return status;
		}
		setCurrentMuxBus(muxAddr, muxBus);
    }

    status = xfer(AK_REQ_TYPE_WRITE, devAddr, 2, data, len, off, 1.0);
    if (status) {
    	return status;
    }

    return status;
}

/* XXX: Untested! */
asynStatus AKI2CEeprom::getData(int addr, unsigned char *data, unsigned short *len, unsigned int off) {
	asynStatus status = asynSuccess;
    const char *functionName = "getData";
    int devAddr, muxAddr, muxBus;

    getIntegerParam(addr, AKI2CEepromDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CEepromMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CEepromMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    if (getCurrentMuxBus(muxAddr) != muxBus) {
		data[0] = muxBus;
		status = xfer(AK_REQ_TYPE_WRITE, muxAddr, 1, data, 1, 0, 1.0);
		if (status) {
			return status;
		}
		setCurrentMuxBus(muxAddr, muxBus);
    }

    status = xfer(AK_REQ_TYPE_READ, devAddr, 2, NULL, *len, off, 1.0);
    if (status) {
    	return status;
    }

//    setDoubleParam(addr, AKI2CTempTemperature, temp);
    /* Do callbacks so higher layers see any changes */
//    callParamCallbacks(addr, addr);

    return status;
}

asynStatus AKI2CEeprom::writeInt32(asynUser *pasynUser, epicsInt32 value) {

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

    if (0) {
//    if (function == AKI2CEepromRead) {
//    	status = setData(addr, mData, mDataLen, 0);
//    } else if (function == AKI2CEepromWrite) {
//    	status = getData(addr, mData, &mDataLen, 0);
    } else if (function < FIRST_AKI2CEEPROM_PARAM) {
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

asynStatus AKI2CEeprom::readInt8Array(asynUser *pasynUser, epicsInt8 *value,
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

    if (function == AKI2CEepromData) {
        getIntegerParam(addr, AKI2CEepromLength, &length);
        getIntegerParam(addr, AKI2CEepromOffset, &offset);
		status = getData(addr, (unsigned char *)value, (unsigned short *)&length, offset);

//        if (nElements <= AK_I2C_EEPROM_MAX_SZ) {
//			/* XXX: offset should be configurable? */
//			status = getData(addr, (unsigned char *)value, (unsigned short *)nIn, 0);
//			memcpy(value, mData, nElements*sizeof(unsigned char));
//			*nIn = nElements;
//    	} else {
//    		status = asynError;
//    	}
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

asynStatus AKI2CEeprom::writeInt8Array(asynUser *pasynUser, epicsInt8 *value,
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

    if (function == AKI2CEepromData) {
        getIntegerParam(addr, AKI2CEepromLength, &length);
        getIntegerParam(addr, AKI2CEepromOffset, &offset);
		status = setData(addr, (unsigned char*)value, (unsigned short)length, offset);

//    	if (nElements <= AK_I2C_EEPROM_MAX_SZ) {
//			memcpy(mData, value, nElements*sizeof(unsigned char));
//			/* XXX: offset should be configurable? */
//			status = setData(addr, (unsigned char*)value, (unsigned short)nElements, 0);
//    	} else {
//    		status = asynError;
//    	}
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

void AKI2CEeprom::report(FILE *fp, int details) {

    fprintf(fp, "AKI2CEeprom %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    AKI2C::report(fp, details);
}

/** Constructor for the AKI2CEeprom class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKI2C base class.
  */
AKI2CEeprom::AKI2CEeprom(const char *portName, const char *ipPort,
        int numDevices, int priority, int stackSize)
   : AKI2C(portName,
		   ipPort,
		   numDevices,
		   NUM_AKI2CEEPROM_PARAMS,
		   asynInt8ArrayMask,
		   asynInt8ArrayMask,
		   ASYN_CANBLOCK | ASYN_MULTIDEVICE, /* asynFlags: ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=0*/
		   1, /* autoConnect YES */
		   priority, stackSize)
{
    int status = asynSuccess;
    const char *functionName = "AKI2CEeprom";

    printf("%s: Handling %d devices\n", functionName, maxAddr);

//    mData = (unsigned char *)calloc(1, AK_I2C_EEPROM_MAX_SZ);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(AKI2CEepromDevAddrString,          asynParamInt32,     &AKI2CEepromDevAddr);
    createParam(AKI2CEepromMuxAddrString,          asynParamInt32,     &AKI2CEepromMuxAddr);
    createParam(AKI2CEepromMuxBusString,           asynParamInt32,     &AKI2CEepromMuxBus);
    createParam(AKI2CEepromDataString,             asynParamInt8Array, &AKI2CEepromData);
    createParam(AKI2CEepromReadString,             asynParamInt32,     &AKI2CEepromRead);
    createParam(AKI2CEepromWriteString,            asynParamInt32,     &AKI2CEepromWrite);
    createParam(AKI2CEepromSizeString,             asynParamInt32,     &AKI2CEepromSize);
    createParam(AKI2CEepromOffsetString,           asynParamInt32,     &AKI2CEepromOffset);
    createParam(AKI2CEepromLengthString,           asynParamInt32,     &AKI2CEepromLength);

    if (status) {
    	printf("%s: failed to set parameter defaults!\n", functionName);
        printf("%s: init FAIL!\n", functionName);
    	return;
    }

    printf("%s: init complete OK!\n", functionName);
}

AKI2CEeprom::~AKI2CEeprom() {
    const char *functionName = "~AKI2CEeprom";

    printf("%s: shutting down ...\n", functionName);

//    free(mData);

    printf("%s: shutdown complete!\n", functionName);
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CEepromConfigure(const char *portName, const char *ipPort,
        int numDevices, int priority, int stackSize) {
    new AKI2CEeprom(portName, ipPort, numDevices, priority, stackSize);
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
static const iocshFuncDef initFuncDef = {"AKI2CEepromConfigure", 5, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CEepromConfigure(args[0].sval, args[1].sval, args[2].ival,
			args[3].ival, args[4].ival);
}

void AKI2CEepromRegister(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CEepromRegister);

} /* extern "C" */





