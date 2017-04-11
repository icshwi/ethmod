/*
 * AKI2C.cpp
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

#include <asynPortDriver.h>
#include "AKI2C.h"

static const char *driverName = "AKI2C";

I2CMuxInfo AKI2C::mMuxInfo[AK_I2C_MUX_MAX] = {
		{-1, -1},
		{-1, -1},
		{-1, -1},
		{-1, -1},
		{-1, -1},
		{-1, -1},
		{-1, -1},
		{-1, -1}
};

/*
 *
 * See design_guide_sxl_en.pdf, page 51.
 *
WRITE: 0x02,0x00,0x0A,0x03,0x50,0x02,0x00,0x00,0x57,0x00,0x01,0xnn,0x03
BREAK DOWN:
	0x02		STX
	0x00,0x0A	Len (10 Bytes) follows, always 2 bytes long
	0x03		function code (with all messages)
	0x50		Slave Address
	0x02		Count Internal Address
	0x00,0x00	Internal Address 0x00,0x00, Count(0-4 Byte)
	0x57		W = WRITE
	0x00,0x01	write 1 byte, always 2 bytes
	0xnn		Byte to write
	0x03		ETX


READ: 0x02,0x00,0x09,0x03,0x50,0x02,0x00,0x00,0x52,0x00,0x02,0x03
BREAK DOWN:
	0x02		STX
	0x00,0x09	Len (9 Bytes) follows, always 2 bytes long
	0x03		function code (with all messages)
	0x50		Slave Address
	0x02		Count Internal Address
	0x00,0x00	Internal Address 0x00,0x00, Count(0-4 Byte)
	0x52		R = READ
	0x00,0x02	read 2 byte, always 2 bytes
	0x03		ETX

Responses:

	MSG: (relates to the 'function code' above)
		no ACK/NAK			(function code = 0x00)
		NAK only			(function code = 0x01)
		ACK only			(function code = 0x02)
		ACK and NAK			(function code = 0x03)

	NAK
		0x15,'S'			NAK STX
		0x15,'E'			NAK ETX
		0x15,'A'			NAK Slave Address
		0x15,'C'			NAK Command
		0x15,'L'			NAK Len
		0x15,'B'			NAK Buffer
		0x15,'R',..			NAK Read and Data we could read
		0x15,'W',nn,nn		NAK Write and nn,nn = Data we could write

	ACK
		0x06,'R'...			ACK Read and Data
		0x06,'W'			ACK Write

*/

char const *AKI2C::status2Msg(unsigned char status, unsigned char code) {
	if (status == AK_I2C_STATUS_OK) {
		switch(code) {
		case 'R':
			return "Read OK";
			break;
		case 'W':
			return "Write OK";
			break;
		default:
			return "Unknown OK code";
			break;
		}
	} else if (status == AK_I2C_STATUS_FAIL) {
		switch(code) {
		case 'R':
			return "Read error";
			break;
		case 'W':
			return "Write error";
			break;
		case 'S':
			return "STX error";
			break;
		case 'E':
			return "ETX error";
			break;
		case 'A':
			return "Slave address error";
			break;
		case 'C':
			return "Command error";
			break;
		case 'L':
			return "Length error";
			break;
		case 'B':
			return "Buffer error";
			break;
		default:
			return "Unknown FAIL code";
			break;
		}
	} else {
		return "Unknown status";
	}

	return "Unknown!";
}

asynStatus AKI2C::pack(unsigned char type, unsigned char devAddr,
		unsigned char addrWidth, unsigned char *data, unsigned short len,
		unsigned int off) {
	asynStatus status = asynSuccess;
	int i, l, msgLen;
	unsigned char msg[AK_MAX_MSG_SZ] = {0};
	l = 0;

	if (devAddr > 0x7F) {
		sprintf(mStatusMsg, "Invalid I2C address");
		return asynError;
	}
	if (addrWidth > 4) {
		sprintf(mStatusMsg, "Invalid I2C address width");
		return asynError;
	}

	// calculate the request length wo/ STX and length included (3 bytes)
	msgLen =  1							// function code
			+ 1							// slave address
			+ 1							// count internal address
			+ addrWidth					// internal address width (0 - 4)
			+ 1							// W - WRITE, R - READ
			+ 2							// bytes to write
			+ 1;						// ETX
	if (type == AK_REQ_TYPE_WRITE) {
		msgLen += len;					// data length
	}
	if (msgLen > AK_MAX_MSG_SZ) {
		sprintf(mStatusMsg, "Invalid I2C request length");
		return asynError;
	}
	if ((type == AK_REQ_TYPE_READ) && (len + 2 > AK_MAX_MSG_SZ)) {
		sprintf(mStatusMsg, "Invalid I2C response length");
		return asynError;
	}

	// build the request
	msg[l++] = 0x02;					// STX
	msg[l++] = (msgLen >> 8) & 0xFF;	// high byte of message length that follows
	msg[l++] = (msgLen & 0xFF);			// high byte of message length that follows
	msg[l++] = 0x03;					// function code
	msg[l++] = devAddr;					// slave address
	msg[l++] = addrWidth;				// count internal address
	// add specified intAddrWidth amount of bytes to the message
	switch(addrWidth) {
	case 0:
		// no additional bytes
		break;
	case 1:
		msg[l++] = off & 0xFF;
		break;
	case 2:
		msg[l++] = (off >> 8) & 0xFF;
		msg[l++] = off & 0xFF;
		break;
	case 3:
		msg[l++] = (off >> 16) & 0xFF;
		msg[l++] = (off >> 8) & 0xFF;
		msg[l++] = off & 0xFF;
		break;
	case 4:
		msg[l++] = (off >> 24) & 0xFF;
		msg[l++] = (off >> 16) & 0xFF;
		msg[l++] = (off >> 8) & 0xFF;
		msg[l++] = off & 0xFF;
		break;
	}
	msg[l++] = type;					// W - WRITE, R - READ
	msg[l++] = (len >> 8) & 0xFF;		// high byte count to write
	msg[l++] = len & 0xFF;				// low byte count to write
	if (type == AK_REQ_TYPE_WRITE) {
		for (i = 0; i < len; i++) {
			msg[l++] = *(data + i);		// data byte[i]
		}
	}
	msg[l++] = 0x03;					// ETX

	memset(mReq, 0, AK_MAX_MSG_SZ);
	memset(mResp, 0, AK_MAX_MSG_SZ);
	mReqActSz = 0;
	mRespActSz = 0;
	mReqSz = l;
	// maximum bytes received:
	//  READ  two status bytes + data (in case of ACK,NAK)
	//  WRITE two status bytes + data (in case of NAK)
	mRespSz = len + 2;
	memcpy(mReq, msg, mReqSz);

	return status;
}

asynStatus AKI2C::unpack(unsigned char type, unsigned char *data,
		unsigned short *len, asynStatus status) {

	if (mRespActSz >= 2) {
		if (mResp[0] == AK_I2C_STATUS_FAIL) {
			E(printf("FAIL - we got NAK! %02X %02X\n", mResp[0], mResp[1]));
			*len = 0;
		} else if (mResp[0] == AK_I2C_STATUS_OK) {
			D(printf("OK  - we got ACK! %02X %02X\n", mResp[0], mResp[1]));
			if ((type == AK_REQ_TYPE_READ) && data && len) {
				memcpy(data, &mResp[AK_I2C_RESP_HDR_SZ],
					*len * sizeof(unsigned char));
			}
			status = asynSuccess;
		} else {
			E(printf("FAIL - unknown response code! %02X %02X\n",
				mResp[0], mResp[1]));
		}
		sprintf(mStatusMsg, "%s", status2Msg(mResp[0], mResp[1]));
	} else {
		sprintf(mStatusMsg, "Invalid I2C response size received");
	}

	return status;
}

asynStatus AKI2C::doXfer(int asynAddr, unsigned char type, unsigned char devAddr,
		unsigned char addrWidth, unsigned char *data, unsigned short *len,
		unsigned int off, double timeout) {
	asynStatus status = asynSuccess;

	if ((type != AK_REQ_TYPE_WRITE) && (type != AK_REQ_TYPE_READ)) {
		sprintf(mStatusMsg, "Invalid request type");
		status = asynError;
	}

	if (status == asynSuccess) {
		status = pack(type, devAddr, addrWidth, data, *len, off);
	}
	if (status == asynSuccess) {
		status = ipPortWriteRead(timeout);
		// status is checked in the unpack() below
		status = unpack(type, data, len, status);
	}

	if (status) {
		asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
			"%s::%s, status=%d\n",
			driverName, __func__, status);
	}

	setStringParam(asynAddr, AKStatusMessage, mStatusMsg);
	char m[AK_MAX_MSG_SZ] = {0};
	getStringParam(asynAddr, AKStatusMessage, AK_MAX_MSG_SZ, m);
	D(printf("Status message: '%s'\n", m));

	/* Do callbacks so higher layers see any changes */
	callParamCallbacks(asynAddr, asynAddr);

	return status;
}

asynStatus AKI2C::xfer(int asynAddr, unsigned char type, unsigned char devAddr,
		unsigned char addrWidth, unsigned char *data, unsigned short *len,
		unsigned int off, double timeout) {
	asynStatus status = asynSuccess;

	status = setMuxBus(asynAddr, devAddr);
	if (status) {
		return status;
	}

	return doXfer(asynAddr, type, devAddr, addrWidth, data, len, off, timeout);
}

asynStatus AKI2C::setMuxBus(int asynAddr, int devAddr) {
	unsigned char data;
	unsigned short len;
	int muxAddr;
	int muxBus;
	asynStatus status = asynSuccess;


	getIntegerParam(asynAddr, AKI2CMuxAddr, &muxAddr);
	getIntegerParam(asynAddr, AKI2CMuxBus, &muxBus);
	D(printf("DEV 0x%02X WANTS MUX 0x%02X : %d\n", devAddr, muxAddr, muxBus));

	/* No MUX bus to setup */
	if (muxAddr == 0) {
		D(printf("MUX setup NOT required (MUX addr == 0)\n"));
		return asynSuccess;
	}

	D(printf("Known MUXes:\n"));
	for (int i = 0; i < AK_I2C_MUX_MAX; i++) {
		if (mMuxInfo[i].addr != -1) {
			D(printf("[%d] MUX 0x%02X : %d\n",
				i, mMuxInfo[i].addr, mMuxInfo[i].bus));
		}
	}

	/* Do we need to setup MUX bus? */
	for (int i = 0; i < AK_I2C_MUX_MAX; i++) {
		if ((mMuxInfo[i].addr == muxAddr) && (mMuxInfo[i].bus == muxBus)) {
			/* MUX bus already setup */
			D(printf("DEV 0x%02X ALREADY USING MUX 0x%02X : %d\n",
				devAddr, muxAddr, muxBus));
			return asynSuccess;
		}
	}

	/* We need to change the MUX bus */
	data = (1 << muxBus);
	len = 1;
	status = doXfer(asynAddr, AK_REQ_TYPE_WRITE, muxAddr, 0, &data, &len, 0);
	if (status) {
		return status;
	}

	/* Update the currently selected MUX bus */
	for (int i = 0; i < AK_I2C_MUX_MAX; i++) {
		if (mMuxInfo[i].addr == muxAddr) {
			mMuxInfo[i].bus = muxBus;
			D(printf("[%d] UPDATED MUX 0x%02X : %d\n",
				i, mMuxInfo[i].addr, mMuxInfo[i].bus));
		} else {
			if (mMuxInfo[i].addr != -1) {
				D(printf("[%d] MUX 0x%02X : %d\n",
					i, mMuxInfo[i].addr, mMuxInfo[i].bus));
			}
		}
	}

	D(printf("DEV 0x%02X NOW USING MUX 0x%02X : %d\n",
		devAddr, muxAddr, muxBus));

	return status;
}

void AKI2C::report(FILE *fp, int details) {

	fprintf(fp, "AKI2C %s\n", this->portName);
	if (details > 0) {
	}
	/* Invoke the base class method */
	AKBase::report(fp, details);
}

/** Constructor for the AKI2C class.
  * Calls constructor for the AKBase base class.
  * All the arguments are simply passed to the AKBase base class.
  */
AKI2C::AKI2C(const char *portName, const char *ipPort,
		int devCount, const char *devInfos,
		int numParams, int interfaceMask, int interruptMask,
		int asynFlags, int autoConnect, int priority, int stackSize)
	: AKBase(portName,
		ipPort,
		AK_IP_PORT_I2C,
		devCount,
		numParams + NUM_AKI2C_PARAMS,
		interfaceMask,
		interruptMask,
		asynFlags, autoConnect, priority, stackSize)
{
	int devAddr;
	int muxAddr;
	int muxBusNr;
	char *end_str, *end_token;
	char *devs = strdup(devInfos);
	char *devTuple = NULL;
	char *dev = NULL;
	char *devStr = NULL;
	char *muxStr = NULL;
	char *muxBusStr = NULL;
	bool muxStored = false;

	D(printf("Handling %d devices: '%s'\n", maxAddr, devInfos));

	createParam(AKI2CDevAddrString,		asynParamInt32,		&AKI2CDevAddr);
	createParam(AKI2CMuxAddrString,		asynParamInt32,		&AKI2CMuxAddr);
	createParam(AKI2CMuxBusString,		asynParamInt32,		&AKI2CMuxBus);

	if (devCount < 1) {
		E(printf("Device count must be > 0!\n"));
		disconnect(pasynUserSelf);
		return;
	}
	
	/* Device address, MUX address and MUX bus number are packed in tuples
	 * as follows, with ',' as a separator:
	 * A: 'dev_addr, mux_addr, mux_busnr' (MUX present)
	 * B: 'dev_addr, mux_addr' (MUX present, assume MUX bus number = 0)
	 * C: 'dev_addr' (no MUX present)
	 *
	 * Multiple tuples are separated using ';' as follows:
	 * "A1; C1; B1; A2; C2"
	 */
	for (int i = 0; i < devCount; i++) {
		if (devStr == NULL) {
			devTuple = strtok_r(devs, ";", &end_str);
		} else {
			devTuple = strtok_r(NULL, ";", &end_str);
		}
		assert(devTuple != NULL);

		dev = strdup(devTuple);
		devStr = strtok_r(dev, ", ", &end_token);
		muxStr = strtok_r(NULL, ", ", &end_token);
		muxBusStr = strtok_r(NULL, ", ", &end_token);
		devAddr = strtol(devStr, NULL, 0);
		/* MUX address can be missing; set to invalid MUX address */
		muxAddr = 0;
		if (muxStr) {
			muxAddr = strtol(muxStr, NULL, 0);
		}
		/* MUX bus can be missing; default to bus 0 */
		muxBusNr = 0;
		if (muxBusStr) {
			muxBusNr = strtol(muxBusStr, NULL, 0);
		}
		free(dev);

		/* Store the MUX address if one is specified (non zero value). */
		muxStored = false;
		if (muxAddr > 0) {
			for (int j = 0; j < AK_I2C_MUX_MAX; j++) {
				/* If MUX address is already listed there is nothing to do.. */
				if (mMuxInfo[j].addr == muxAddr) {
					muxStored = true;
					break;
				}
				/* Store MUX address in first empty slot.. */
				if (mMuxInfo[j].addr == -1) {
					mMuxInfo[j].addr = muxAddr;
					muxStored = true;
					break;
				}
			}
			assert(muxStored == true);
		}

		setIntegerParam(i, AKI2CDevAddr, devAddr);
		setIntegerParam(i, AKI2CMuxAddr, muxAddr);
		setIntegerParam(i, AKI2CMuxBus, muxBusNr);
		D(printf("PORT %s DEV 0x%02X, MUX 0x%02X : %d\n",
				portName, devAddr, muxAddr, muxBusNr));

		/* Do callbacks so higher layers see any changes */
		callParamCallbacks(i, i);
	}

	free(devs);

	I(printf("init OK!\n"));
}

AKI2C::~AKI2C() {
	I(printf("shut down ..\n"));
}
