/*
 * AKI2C_LTC2991.cpp
 *
 *  Created on: May 24, 2016
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
#include "AKI2C_LTC2991.h"

static const char *driverName = "AKI2C_LTC2991";


static void exitHandler(void *drvPvt) {
	AKI2C_LTC2991 *pPvt = (AKI2C_LTC2991 *)drvPvt;
	delete pPvt;
}

asynStatus AKI2C_LTC2991::write(int addr, unsigned char reg, unsigned char val) {
    asynStatus status = asynSuccess;
    const char *functionName = "write";
    unsigned char data[1] = {0};
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
	len = 1;
    status = xfer(addr, AK_REQ_TYPE_WRITE, devAddr, 1, data, &len, reg);
    if (status) {
    	return status;
    }

    printf("%s: devAddr %d, muxAddr %d, muxBus %d reg %02X = %02X\n", functionName, devAddr, muxAddr, muxBus, reg, val);

    return status;
}

void AKI2C_LTC2991::convertToVoltage(int addr, int valueParam,
		int offsetParam, int factorParam, unsigned int raw) {
    double volts = 0.0;
    double offset = 0.0;
    double factor = 1.0;

    if (offsetParam != -1) {
    	getDoubleParam(addr, offsetParam, &offset);
    }
    if (factorParam != -1) {
    	getDoubleParam(addr, factorParam, &factor);
    }

    if (raw & 0x8000) {
    	/* Data is valid */
    	raw &= 0x7FFF;
    	/* LSB = 305.18 uV ; 2.5V / 2^13 */
    	volts = factor * ((double)raw * 305.18 / 1000000.0) + offset;
    	setDoubleParam(addr, valueParam, volts);
    }
}

void AKI2C_LTC2991::convertToTemperature(int addr, int valueParam,
		unsigned int raw) {
    double temp = 0.0;

    if (raw & 0x8000) {
    	/* Data is valid */
    	raw &= 0x7FFF;
    	/* LSB = 0.0625 degrees */
    	temp = (double)raw * 0.0625;
    	setDoubleParam(addr, valueParam, temp);
    }
}

asynStatus AKI2C_LTC2991::read(int addr, unsigned char reg) {
    asynStatus status = asynSuccess;
    const char *functionName = "read";
    unsigned char data[32] = {0};
    int devAddr, muxAddr, muxBus;
    unsigned short len;
    unsigned int raw;

    getIntegerParam(addr, AKI2CDevAddr, &devAddr);
    getIntegerParam(addr, AKI2CMuxAddr, &muxAddr);
    getIntegerParam(addr, AKI2CMuxBus, &muxBus);
    printf("%s: devAddr %d, muxAddr %d, muxBus %d\n", functionName, devAddr, muxAddr, muxBus);

    status = setMuxBus(addr, muxAddr, muxBus);
	if (status) {
		return status;
	}

	/* Read all 29 registers at once */
    len = 29;
    status = xfer(addr, AK_REQ_TYPE_READ, devAddr, 1, data, &len, reg);
    if (status) {
    	return status;
    }

    /* Check for BUSY bit - if 0 then not BUSY, conversion is not in process */
    if (data[AKI2C_LTC2991_STATUS_HIGH_REG] & 0x04) {
    	return asynError;
    }

    /* Parse out the values for V1 */
    raw = ((unsigned int)data[AKI2C_LTC2991_V1_MSB_REG] << 8)
    		| (unsigned int)data[AKI2C_LTC2991_V1_LSB_REG];
    convertToVoltage(addr, AKI2C_LTC2991_V1_Value,
    		AKI2C_LTC2991_V1_Offset, AKI2C_LTC2991_V1_Factor, raw);
    raw = ((unsigned int)data[AKI2C_LTC2991_V2_MSB_REG] << 8)
    		| (unsigned int)data[AKI2C_LTC2991_V2_LSB_REG];
    convertToVoltage(addr, AKI2C_LTC2991_V2_Value,
    		AKI2C_LTC2991_V2_Offset, AKI2C_LTC2991_V2_Factor, raw);
    raw = ((unsigned int)data[AKI2C_LTC2991_V3_MSB_REG] << 8)
    		| (unsigned int)data[AKI2C_LTC2991_V3_LSB_REG];
    convertToVoltage(addr, AKI2C_LTC2991_V3_Value,
    		AKI2C_LTC2991_V3_Offset, AKI2C_LTC2991_V3_Factor, raw);
    raw = ((unsigned int)data[AKI2C_LTC2991_V4_MSB_REG] << 8)
    		| (unsigned int)data[AKI2C_LTC2991_V4_LSB_REG];
    convertToVoltage(addr, AKI2C_LTC2991_V4_Value,
    		AKI2C_LTC2991_V4_Offset, AKI2C_LTC2991_V4_Factor, raw);
    raw = ((unsigned int)data[AKI2C_LTC2991_V5_MSB_REG] << 8)
    		| (unsigned int)data[AKI2C_LTC2991_V5_LSB_REG];
    convertToVoltage(addr, AKI2C_LTC2991_V5_Value,
    		AKI2C_LTC2991_V5_Offset, AKI2C_LTC2991_V5_Factor, raw);
    raw = ((unsigned int)data[AKI2C_LTC2991_V6_MSB_REG] << 8)
    		| (unsigned int)data[AKI2C_LTC2991_V6_LSB_REG];
    convertToVoltage(addr, AKI2C_LTC2991_V6_Value,
    		AKI2C_LTC2991_V6_Offset, AKI2C_LTC2991_V6_Factor, raw);
    raw = ((unsigned int)data[AKI2C_LTC2991_V7_MSB_REG] << 8)
    		| (unsigned int)data[AKI2C_LTC2991_V7_LSB_REG];
    convertToVoltage(addr, AKI2C_LTC2991_V7_Value,
    		AKI2C_LTC2991_V7_Offset, AKI2C_LTC2991_V7_Factor, raw);
    raw = ((unsigned int)data[AKI2C_LTC2991_V8_MSB_REG] << 8)
    		| (unsigned int)data[AKI2C_LTC2991_V8_LSB_REG];
    convertToVoltage(addr, AKI2C_LTC2991_V8_Value,
    		AKI2C_LTC2991_V8_Offset, AKI2C_LTC2991_V8_Factor, raw);
    raw = ((unsigned int)data[AKI2C_LTC2991_VCC_MSB_REG] << 8)
    		| (unsigned int)data[AKI2C_LTC2991_VCC_LSB_REG];
    convertToVoltage(addr, AKI2C_LTC2991_Vcc_Value,
    		-1, -1, raw);
    raw = ((unsigned int)data[AKI2C_LTC2991_TINT_MSB_REG] << 8)
    		| (unsigned int)data[AKI2C_LTC2991_TINT_LSB_REG];
    convertToTemperature(addr, AKI2C_LTC2991_TInt_Value, raw);

	return status;
}

asynStatus AKI2C_LTC2991::writeInt32(asynUser *pasynUser, epicsInt32 value) {

    int function = pasynUser->reason;
    int addr = 0;
    asynStatus status = asynSuccess;
    const char *functionName = "writeInt32";

    status = getAddress(pasynUser, &addr);
    if (status != asynSuccess) {
    	return(status);
    }

    printf("AKI2C_LTC2991::%s: function %d, addr %d, value %d\n", functionName, function, addr, value);
    status = setIntegerParam(addr, function, value);


    if (function == AKI2C_LTC2991_Read) {
    	status = read(addr, AKI2C_LTC2991_STATUS_LOW_REG);
    } else if (function == AKI2C_LTC2991_Trigger) {
    	/* The acquisition is triggered by writing to this register */
    	status = write(addr, AKI2C_LTC2991_CH_ENABLE_REG, AKI2C_LTC2991_CH_ENABLE_VAL);
    } else if (function < FIRST_AKI2C_LTC2991_PARAM) {
        printf("AKI2C_LTC2991::%s: function %d, addr %d, value %d calling AKI2C::writeInt32 (FIRST_AKI2C_LTC2991_PARAM=%d)\n", functionName, function, addr, value, FIRST_AKI2C_LTC2991_PARAM);
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

void AKI2C_LTC2991::report(FILE *fp, int details) {

    fprintf(fp, "AKI2C_LTC2991 %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    AKI2C::report(fp, details);
}

/** Constructor for the AKI2C_LTC2991 class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKI2C base class.
  */
AKI2C_LTC2991::AKI2C_LTC2991(const char *portName, const char *ipPort,
        int devCount, const char *devAddrs,
		int muxAddr, int muxBus,
		int priority, int stackSize)
   : AKI2C(portName,
		   ipPort,
		   devCount, devAddrs, muxAddr, muxBus,
		   NUM_AKI2C_LTC2991_PARAMS,
		   0, /* no new interface masks beyond those in AKBase */
		   0, /* no new interrupt masks beyond those in AKBase */
		   ASYN_CANBLOCK | ASYN_MULTIDEVICE, /* asynFlags: ASYN_CANBLOCK=1, ASYN_MULTIDEVICE=0*/
		   1, /* autoConnect YES */
		   priority, stackSize)
{
    int status = asynSuccess;
    const char *functionName = "AKI2C_LTC2991";

    printf("%s: Handling %d devices\n", functionName, maxAddr);

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(AKI2C_LTC2991_ReadString,          asynParamInt32,   &AKI2C_LTC2991_Read);
    createParam(AKI2C_LTC2991_TriggerString,       asynParamInt32,   &AKI2C_LTC2991_Trigger);
    createParam(AKI2C_LTC2991_V1ValueString,       asynParamFloat64, &AKI2C_LTC2991_V1_Value);
    createParam(AKI2C_LTC2991_V1OffsetString,      asynParamFloat64, &AKI2C_LTC2991_V1_Offset);
    createParam(AKI2C_LTC2991_V1FactorString,      asynParamFloat64, &AKI2C_LTC2991_V1_Factor);
    createParam(AKI2C_LTC2991_V2ValueString,       asynParamFloat64, &AKI2C_LTC2991_V2_Value);
    createParam(AKI2C_LTC2991_V2OffsetString,      asynParamFloat64, &AKI2C_LTC2991_V2_Offset);
    createParam(AKI2C_LTC2991_V2FactorString,      asynParamFloat64, &AKI2C_LTC2991_V2_Factor);
    createParam(AKI2C_LTC2991_V3ValueString,       asynParamFloat64, &AKI2C_LTC2991_V3_Value);
    createParam(AKI2C_LTC2991_V3OffsetString,      asynParamFloat64, &AKI2C_LTC2991_V3_Offset);
    createParam(AKI2C_LTC2991_V3FactorString,      asynParamFloat64, &AKI2C_LTC2991_V3_Factor);
    createParam(AKI2C_LTC2991_V4ValueString,       asynParamFloat64, &AKI2C_LTC2991_V4_Value);
    createParam(AKI2C_LTC2991_V4OffsetString,      asynParamFloat64, &AKI2C_LTC2991_V4_Offset);
    createParam(AKI2C_LTC2991_V4FactorString,      asynParamFloat64, &AKI2C_LTC2991_V4_Factor);
    createParam(AKI2C_LTC2991_V5ValueString,       asynParamFloat64, &AKI2C_LTC2991_V5_Value);
    createParam(AKI2C_LTC2991_V5OffsetString,      asynParamFloat64, &AKI2C_LTC2991_V5_Offset);
    createParam(AKI2C_LTC2991_V5FactorString,      asynParamFloat64, &AKI2C_LTC2991_V5_Factor);
    createParam(AKI2C_LTC2991_V6ValueString,       asynParamFloat64, &AKI2C_LTC2991_V6_Value);
    createParam(AKI2C_LTC2991_V6OffsetString,      asynParamFloat64, &AKI2C_LTC2991_V6_Offset);
    createParam(AKI2C_LTC2991_V6FactorString,      asynParamFloat64, &AKI2C_LTC2991_V6_Factor);
    createParam(AKI2C_LTC2991_V7ValueString,       asynParamFloat64, &AKI2C_LTC2991_V7_Value);
    createParam(AKI2C_LTC2991_V7OffsetString,      asynParamFloat64, &AKI2C_LTC2991_V7_Offset);
    createParam(AKI2C_LTC2991_V7FactorString,      asynParamFloat64, &AKI2C_LTC2991_V7_Factor);
    createParam(AKI2C_LTC2991_V8ValueString,       asynParamFloat64, &AKI2C_LTC2991_V8_Value);
    createParam(AKI2C_LTC2991_V8OffsetString,      asynParamFloat64, &AKI2C_LTC2991_V8_Offset);
    createParam(AKI2C_LTC2991_V8FactorString,      asynParamFloat64, &AKI2C_LTC2991_V8_Factor);
    createParam(AKI2C_LTC2991_VccValueString,      asynParamFloat64, &AKI2C_LTC2991_Vcc_Value);
    createParam(AKI2C_LTC2991_TIntValueString,     asynParamFloat64, &AKI2C_LTC2991_TInt_Value);

    for (int i = 0; i < devCount; i++) {
    	setDoubleParam(i, AKI2C_LTC2991_V1_Value, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V1_Offset, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V1_Factor, 1.0);
    	setDoubleParam(i, AKI2C_LTC2991_V2_Value, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V2_Offset, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V2_Factor, 1.0);
    	setDoubleParam(i, AKI2C_LTC2991_V3_Value, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V3_Offset, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V3_Factor, 1.0);
    	setDoubleParam(i, AKI2C_LTC2991_V4_Value, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V4_Offset, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V4_Factor, 1.0);
    	setDoubleParam(i, AKI2C_LTC2991_V5_Value, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V5_Offset, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V5_Factor, 1.0);
    	setDoubleParam(i, AKI2C_LTC2991_V6_Value, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V6_Offset, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V6_Factor, 1.0);
    	setDoubleParam(i, AKI2C_LTC2991_V7_Value, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V7_Offset, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V7_Factor, 1.0);
    	setDoubleParam(i, AKI2C_LTC2991_V8_Value, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V8_Offset, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_V8_Factor, 1.0);
    	setDoubleParam(i, AKI2C_LTC2991_Vcc_Value, 0.0);
    	setDoubleParam(i, AKI2C_LTC2991_TInt_Value, 0.0);
    }

    /* set some defaults */
    for (int i = 0; i < devCount; i++) {
		status |= write(i, AKI2C_LTC2991_CONTROL1_REG, AKI2C_LTC2991_CONTROL1_VAL);
		status |= write(i, AKI2C_LTC2991_CONTROL2_REG, AKI2C_LTC2991_CONTROL2_VAL);
		status |= write(i, AKI2C_LTC2991_CONTROL3_REG, AKI2C_LTC2991_CONTROL3_VAL);
    }

    if (status) {
    	printf("%s: failed to set parameter defaults!\n", functionName);
        printf("%s: init FAIL!\n", functionName);
    	return;
    }

    printf("%s: init complete OK!\n", functionName);
}

AKI2C_LTC2991::~AKI2C_LTC2991() {
    const char *functionName = "~AKI2C_LTC2991";

    printf("%s: shutting down ...\n", functionName);

    printf("%s: shutdown complete!\n", functionName);
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CLTC2991Configure(const char *portName, const char *ipPort,
        int devCount, const char *devAddrs,
		int muxAddr, int muxBus,
		int priority, int stackSize) {
    new AKI2C_LTC2991(portName, ipPort, devCount, devAddrs,
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
static const iocshFuncDef initFuncDef = {"AKI2CLTC2991Configure", 8, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CLTC2991Configure(args[0].sval, args[1].sval,
			args[2].ival, args[3].sval,
			args[4].ival, args[5].ival, args[6].ival, args[7].ival);
}

void AKI2CLTC2991Register(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CLTC2991Register);

} /* extern "C" */

