/*
 * AKI2C_PCF85063TP.cpp
 *
 *  Created on: May 17, 2016
 *      Author: hinxx
 */



#include "AKI2C_PCF85063TP.h"

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

static const char *driverName = "AKI2C_PCF85063TP";


static void exitHandler(void *drvPvt) {
	AKI2C_PCF85063TP *pPvt = (AKI2C_PCF85063TP *)drvPvt;
	delete pPvt;
}

unsigned char AKI2C_PCF85063TP::bcdToDec(unsigned char val) {
	return ( (val/16*10) + (val%16) );
}

unsigned char AKI2C_PCF85063TP::decToBcd(unsigned char val) {
  return ( (val/10*16) + (val%10) );
}

asynStatus AKI2C_PCF85063TP::write(int addr, unsigned char reg, unsigned char *val, unsigned short *len) {
	asynStatus status = asynSuccess;
	int devAddr;

	getIntegerParam(addr, AKI2CDevAddr, &devAddr);

	status = xfer(addr, AK_REQ_TYPE_WRITE, devAddr, 1, val, len, reg);
	if (status) {
		return status;
	}

	return status;
}

asynStatus AKI2C_PCF85063TP::setDateTime(int addr) {
	asynStatus status = asynSuccess;
	unsigned char data[7] = {0};
	int devAddr;
	unsigned short len;
	int years, months, weekdays, days, hours, minutes, seconds;

	getIntegerParam(addr, AKI2CDevAddr, &devAddr);
	getIntegerParam(addr, AKI2C_PCF85063TP_Seconds, &seconds);
	getIntegerParam(addr, AKI2C_PCF85063TP_Minutes, &minutes);
	getIntegerParam(addr, AKI2C_PCF85063TP_Hours, &hours);
	getIntegerParam(addr, AKI2C_PCF85063TP_Days, &days);
	getIntegerParam(addr, AKI2C_PCF85063TP_Weekdays, &weekdays);
	getIntegerParam(addr, AKI2C_PCF85063TP_Months, &months);
	getIntegerParam(addr, AKI2C_PCF85063TP_Years, &years);
	D(printf("NEW DATE&TIME %d-%d-%d %d %d:%d:%d\n",
			years, months, weekdays, days, hours, minutes, seconds));

	/* bit 7 is OS flag; set to 0 */
	data[0] = decToBcd((unsigned char)(seconds & 0x7F));
	data[1] = decToBcd((unsigned char)(minutes & 0x7F));
	data[2] = decToBcd((unsigned char)(hours & 0x3F));
	data[3] = decToBcd((unsigned char)(days & 0x3F));
	data[4] = decToBcd((unsigned char)(weekdays & 0x07));
	data[5] = decToBcd((unsigned char)(months & 0x1F));
	data[6] = decToBcd((unsigned char)(years & 0xFF));
	len = 7;
	status = write(addr, AKI2C_PCF85063TP_SECONDS_REG, data, &len);
	if (status) {
		return status;
	}

	return status;
}

asynStatus AKI2C_PCF85063TP::read(int addr, unsigned char reg, unsigned char *val, unsigned short *len) {
	asynStatus status = asynSuccess;
	int devAddr;

	getIntegerParam(addr, AKI2CDevAddr, &devAddr);

	status = xfer(addr, AK_REQ_TYPE_READ, devAddr, 1, val, len, reg);
	if (status) {
		return status;
	}

	return status;
}

asynStatus AKI2C_PCF85063TP::getDateTime(int addr) {
	asynStatus status = asynSuccess;
	unsigned char data[7] = {0};
	int devAddr;
	unsigned short len;
	char dateTime[16];
	int years, months, weekdays, days, hours, minutes, seconds;

	getIntegerParam(addr, AKI2CDevAddr, &devAddr);

	/* Read all 7 registers that constitute date and time */
	len = 7;
	status = read(addr, AKI2C_PCF85063TP_SECONDS_REG, data, &len);
	if (status) {
		return status;
	}

	D(printf("RAW 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
			data[0], data[1], data[2], data[3], data[4], data[5], data[6]));

	seconds = bcdToDec(data[0] & 0xFF);
	minutes = bcdToDec(data[1] & 0xFF);
	hours = bcdToDec(data[2] & 0xFF);
	days = bcdToDec(data[3] & 0xFF);
	weekdays = bcdToDec(data[4] & 0xFF);
	months = bcdToDec(data[5] & 0xFF);
	years = bcdToDec(data[6] & 0xFF) + 2000;

	D(printf("DATE&TIME %d-%d-%d %d %d:%d:%d\n",
			years, months, days, weekdays, hours, minutes, seconds));

	setIntegerParam(addr, AKI2C_PCF85063TP_Seconds, seconds);
	setIntegerParam(addr, AKI2C_PCF85063TP_Minutes, minutes);
	setIntegerParam(addr, AKI2C_PCF85063TP_Hours, hours);
	setIntegerParam(addr, AKI2C_PCF85063TP_Days, days);
	setIntegerParam(addr, AKI2C_PCF85063TP_Weekdays, weekdays);
	setIntegerParam(addr, AKI2C_PCF85063TP_Months, months);
	setIntegerParam(addr, AKI2C_PCF85063TP_Years, years);
	memset(dateTime, 0, sizeof(dateTime));
	sprintf(dateTime, "%d-%d-%d", years, months, days);
	setStringParam(addr, AKI2C_PCF85063TP_Date, dateTime);
	memset(dateTime, 0, sizeof(dateTime));
	sprintf(dateTime, "%d:%d:%d", hours, minutes, seconds);
	setStringParam(addr, AKI2C_PCF85063TP_Time, dateTime);

	/* Do callbacks so higher layers see any changes */
	callParamCallbacks(addr, addr);

	return status;
}

asynStatus AKI2C_PCF85063TP::writeInt32(asynUser *pasynUser, epicsInt32 value) {

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

	if (function == AKI2C_PCF85063TP_Read) {
		status = getDateTime(addr);
	} else if (function == AKI2C_PCF85063TP_Write) {
		status = setDateTime(addr);
	} else if (function < FIRST_AKI2C_PCF85063TP_PARAM) {
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

void AKI2C_PCF85063TP::report(FILE *fp, int details) {

	fprintf(fp, "AKI2C_PCF85063TP %s\n", this->portName);
	if (details > 0) {
	}
	/* Invoke the base class method */
	AKI2C::report(fp, details);
}

/** Constructor for the AKI2C_PCF85063TP class.
  * Calls constructor for the AKI2C base class.
  * All the arguments are simply passed to the AKI2C base class.
  */
AKI2C_PCF85063TP::AKI2C_PCF85063TP(const char *portName, const char *ipPort,
		int devCount, const char *devInfos, int priority, int stackSize)
	: AKI2C(portName,
		ipPort,
		devCount, devInfos,
		NUM_AKI2C_PCF85063TP_PARAMS,
		0, /* no new interface masks beyond those in AKBase */
		0, /* no new interrupt masks beyond those in AKBase */
		ASYN_CANBLOCK | ASYN_MULTIDEVICE,
		1, /* autoConnect YES */
		priority, stackSize)
{
	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

	createParam(AKI2C_PCF85063TP_ReadString,		asynParamInt32,	&AKI2C_PCF85063TP_Read);
	createParam(AKI2C_PCF85063TP_WriteString,		asynParamInt32,	&AKI2C_PCF85063TP_Write);
	createParam(AKI2C_PCF85063TP_SecondsString,		asynParamInt32,	&AKI2C_PCF85063TP_Seconds);
	createParam(AKI2C_PCF85063TP_MinutesString,		asynParamInt32,	&AKI2C_PCF85063TP_Minutes);
	createParam(AKI2C_PCF85063TP_HoursString,		asynParamInt32,	&AKI2C_PCF85063TP_Hours);
	createParam(AKI2C_PCF85063TP_DaysString,		asynParamInt32,	&AKI2C_PCF85063TP_Days);
	createParam(AKI2C_PCF85063TP_WeekdaysString,	asynParamInt32,	&AKI2C_PCF85063TP_Weekdays);
	createParam(AKI2C_PCF85063TP_MonthsString,		asynParamInt32,	&AKI2C_PCF85063TP_Months);
	createParam(AKI2C_PCF85063TP_YearsString,		asynParamInt32,	&AKI2C_PCF85063TP_Years);
	createParam(AKI2C_PCF85063TP_DateString,		asynParamOctet,	&AKI2C_PCF85063TP_Date);
	createParam(AKI2C_PCF85063TP_TimeString,		asynParamOctet,	&AKI2C_PCF85063TP_Time);

	for (int i = 0; i < devCount; i++) {
		setStringParam(i, AKI2C_PCF85063TP_Date, "");
		setStringParam(i, AKI2C_PCF85063TP_Time, "");
		setIntegerParam(i, AKI2C_PCF85063TP_Seconds, 0);
		setIntegerParam(i, AKI2C_PCF85063TP_Minutes, 0);
		setIntegerParam(i, AKI2C_PCF85063TP_Hours, 0);
		setIntegerParam(i, AKI2C_PCF85063TP_Days, 0);
		setIntegerParam(i, AKI2C_PCF85063TP_Weekdays, 0);
		setIntegerParam(i, AKI2C_PCF85063TP_Months, 0);
		setIntegerParam(i, AKI2C_PCF85063TP_Years, 0);

		/* Do callbacks so higher layers see any changes */
		callParamCallbacks(i, i);
	}

	I(printf("init OK!\n"));
}

AKI2C_PCF85063TP::~AKI2C_PCF85063TP() {
	I(printf("shut down ..\n"));
}

/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int AKI2CPCF85063TPConfigure(const char *portName, const char *ipPort,
		int devCount, const char *devInfos, int priority, int stackSize) {
	new AKI2C_PCF85063TP(portName, ipPort, devCount, devInfos, priority, stackSize);
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
static const iocshFuncDef initFuncDef = {"AKI2CPCF85063TPConfigure", 6, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
	AKI2CPCF85063TPConfigure(args[0].sval, args[1].sval,
			args[2].ival, args[3].sval, args[4].ival, args[5].ival);
}

void AKI2CPCF85063TPRegister(void) {
	iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(AKI2CPCF85063TPRegister);

} /* extern "C" */

