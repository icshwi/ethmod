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
	msg[l++] = type;					// W - WRITE, R- READ
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
    const char *functionName = "unpack";

    if (mRespActSz >= 2) {
    	if (mResp[0] == AK_I2C_STATUS_FAIL) {
    		printf("%s: FAIL - we got NAK! %02X %02X\n", functionName, mResp[0], mResp[1]);
	    	*len = 0;
    	} else if (mResp[0] == AK_I2C_STATUS_OK) {
    		printf("%s:  OK  - we got ACK! %02X %02X\n", functionName, mResp[0], mResp[1]);
    	    if ((type == AK_REQ_TYPE_READ) && data && len) {
    	    	memcpy(data, &mResp[AK_I2C_RESP_HDR_SZ], *len * sizeof(unsigned char));
    	    }
    		status = asynSuccess;
    	} else {
    		printf("%s: FAIL - unknown response code! %02X %02X\n", functionName, mResp[0], mResp[1]);
    	}
    	sprintf(mStatusMsg, "%s", status2Msg(mResp[0], mResp[1]));
    } else {
    	sprintf(mStatusMsg, "Invalid I2C response size received");
    }

	return status;
}

asynStatus AKI2C::xfer(int asynAddr, unsigned char type, unsigned char devAddr,
		unsigned char addrWidth, unsigned char *data, unsigned short *len,
		unsigned int off, double timeout) {
	asynStatus status = asynSuccess;
    const char *functionName = "xfer";

	if ((type != AK_REQ_TYPE_WRITE) && (type != AK_REQ_TYPE_READ)) {
		sprintf(mStatusMsg, "Invalid request type");
	    status = asynError;
	}

	timeout = 0.3;

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
                "%s:%s, status=%d\n",
                driverName, functionName, status);
    }

    setStringParam(asynAddr, AKStatusMessage, mStatusMsg);
    char m[AK_MAX_MSG_SZ] = {0};
    getStringParam(asynAddr, AKStatusMessage, AK_MAX_MSG_SZ, m);
    printf("Status message: '%s'\n", m);

    /* Do callbacks so higher layers see any changes */
    callParamCallbacks(asynAddr, asynAddr);

	return status;
}

// XXX: Can not do MUX bus changes like this since different chips inherit this
// class and they work with their own copies of the mux info struct!
// Find a better way or just set the mux bus every time when accessing the chip..


//void AKI2C::updateMuxBus(int muxAddr, int muxBus) {
//	for (int i = 0; i < AK_I2C_MUX_MAX; i++) {
//		if (mMuxInfo[i].addr == muxAddr) {
//			mMuxInfo[i].bus = muxBus;
//		}
//	}
//}
//
//int AKI2C::getMuxBus(int muxAddr) {
//	for (int i = 0; i < AK_I2C_MUX_MAX; i++) {
//		if (mMuxInfo[i].addr == muxAddr) {
//			return mMuxInfo[i].bus;
//		}
//	}
//	return -1;
//}

asynStatus AKI2C::setMuxBus(int asynAddr, int muxAddr, int muxBus) {
    unsigned char data;
    unsigned short len;
	asynStatus status = asynSuccess;

//	printf("%s: MUX %02X want bus %d\n", __func__, muxAddr, muxBus);

//	if (getMuxBus(muxAddr) == muxBus) {
//		return asynSuccess;
//	}

	printf("%s: SETTING MUX %02X bus %d\n", __func__, muxAddr, muxBus);
	data = muxBus;
	len = 1;
	status = xfer(asynAddr, AK_REQ_TYPE_WRITE, muxAddr, 1, &data, &len, 0);
	if (status) {
		return status;
	}
//	updateMuxBus(muxAddr, muxBus);

//	printf("%s: MUX %02X have bus %d\n", __func__, muxAddr, muxBus);

//	sleep(1);

	return status;
}

asynStatus AKI2C::writeInt32(asynUser *pasynUser, epicsInt32 value) {

    int function = pasynUser->reason;
    int addr = 0;
    asynStatus status = asynSuccess;
    const char *functionName = "writeInt32";

    status = getAddress(pasynUser, &addr);
    if (status != asynSuccess) {
    	return(status);
    }

    printf("AKI2C::%s: function %d, addr %d, value %d\n", functionName, function, addr, value);
    status = setIntegerParam(addr, function, value);

    if (0) {

    } else if (function < FIRST_AKI2C_PARAM) {
        printf("AKI2C::%s: function %d, addr %d, value %d calling AKBase::writeInt32\n", functionName, function, addr, value);
        /* If this parameter belongs to a base class call its method */
    	status = AKBase::writeInt32(pasynUser, value);
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
		int maxAddr, int numParams, int interfaceMask, int interruptMask,
        int asynFlags, int autoConnect, int priority, int stackSize)
   : AKBase(portName,
		   ipPort,
		   AK_IP_PORT_I2C,
		   maxAddr,
		   numParams + NUM_AKI2C_PARAMS,
		   interfaceMask,
		   interruptMask,
		   asynFlags, autoConnect, priority, stackSize)
{
//    int status = asynSuccess;
    const char *functionName = "AKI2C";

//	for (int i = 0; i< AK_I2C_MUX_MAX; i++) {
//		mMuxInfo[i].addr = 0x70 + i;
//		mMuxInfo[i].bus = 0;
//	}

    createParam(AKI2CDevAddrString,          asynParamInt32,   &AKI2CDevAddr);
    createParam(AKI2CMuxAddrString,          asynParamInt32,   &AKI2CMuxAddr);
    createParam(AKI2CMuxBusString,           asynParamInt32,   &AKI2CMuxBus);

    printf("%s: init complete OK!\n", functionName);
}

AKI2C::~AKI2C() {
    const char *functionName = "~AKI2C";

    printf("%s: shutting down ...\n", functionName);

    printf("%s: shutdown complete!\n", functionName);
}
