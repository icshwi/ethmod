/*
 * bpmfe.cpp
 *
 *  Created on: May 12, 2016
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
#include <iocsh.h>

#include "bpmfe.h"
#include <epicsExport.h>

static const char *driverName = "bpmfe";
void bpmfeTaskC(void *drvPvt);

void bpmfeTaskC(void *drvPvt) {
    BPMFE *pPvt = (BPMFE *)drvPvt;
    pPvt->dataTask();
}

static void exitHandler(void *drvPvt) {
    BPMFE *pPvt = (BPMFE *)drvPvt;
	delete pPvt;
}

#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 8
#endif
void hexdump(void *mem, unsigned int len) {
	unsigned int i, j;

	for (i = 0;	i < len + ((len % HEXDUMP_COLS) ?
							(HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++) {
		/* print offset */
		if (i % HEXDUMP_COLS == 0) {
			printf("0x%06x: ", i);
		}

		/* print hex data */
		if (i < len) {
			printf("%02x ", 0xFF & ((char*) mem)[i]);
		} else {
			/* end of block, just aligning for ASCII dump */
			printf("   ");
		}

		/* print ASCII dump */
		if (i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
			for (j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
				if (j >= len) {
					/* end of block, not really printing */
					putchar(' ');
				} else if (isprint(((char*) mem)[j])) {
					/* printable char */
					putchar(0xFF & ((char*) mem)[j]);
				} else {
					/* other char */
					putchar('.');
				}
			}
			putchar('\n');
		}
	}
}

/*
 *
Here is a preliminary list of I2C chips that will need support:
	Temp sensor			TMP100NA/3K
	Port expander		TCA9555
	EEPROM				M24M02-DRMN6TP
	Real time clk		PCF85063TP/1Z
	Serial Number		DS28CM00
	Voltage monitor		LTC2991
	I2C mux				TCA9546A
*/

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

asynStatus BPMFE::I2CWrite(unsigned char i2cAddr, unsigned char intAddrWidth, unsigned int intAddr, unsigned char *data, unsigned short len) {
	asynStatus status = asynSuccess;
	int i, l, msgLen;
	unsigned char msg[MAX_MESSAGE_SIZE] = {0};
	l = 0;

	if (i2cAddr > 0x7F) {
		return asynError;
	}
	if (intAddrWidth > 4) {
		return asynError;
	}
	if (len > (MAX_MESSAGE_SIZE - intAddrWidth - 10)) {
		return asynError;
	}

	// calculate the message length wo/ STX and length included (3 bytes)
	msgLen =  1							// function code
			+ 1							// slave address
			+ 1							// count internal address
			+ intAddrWidth				// internal address width (0 - 4)
			+ 1							// W - WRITE
			+ 2							// bytes to write
			+ len						// data length
			+ 1;						// ETX
	// build the message
	msg[l++] = 0x02;					// STX
	msg[l++] = (msgLen >> 8) & 0xFF;	// high byte of message length that follows
	msg[l++] = (msgLen & 0xFF);			// high byte of message length that follows
	msg[l++] = 0x03;					// function code
	msg[l++] = i2cAddr;					// slave address
	msg[l++] = intAddrWidth;			// count internal address
	// add specified intAddrWidth amount of bytes to the message
	switch(intAddrWidth) {
	case 0:
		// no additional bytes
		break;
	case 1:
		msg[l++] = intAddr & 0xFF;
		break;
	case 2:
		msg[l++] = (intAddr >> 8) & 0xFF;
		msg[l++] = intAddr & 0xFF;
		break;
	case 3:
		msg[l++] = (intAddr >> 16) & 0xFF;
		msg[l++] = (intAddr >> 8) & 0xFF;
		msg[l++] = intAddr & 0xFF;
		break;
	case 4:
		msg[l++] = (intAddr >> 24) & 0xFF;
		msg[l++] = (intAddr >> 16) & 0xFF;
		msg[l++] = (intAddr >> 8) & 0xFF;
		msg[l++] = intAddr & 0xFF;
		break;
	}
	msg[l++] = 0x57;					// W - WRITE
	msg[l++] = (len >> 8) & 0xFF;		// high byte count to write
	msg[l++] = len & 0xFF;				// low byte count to write
	for (i = 0; i < len; i++) {
		msg[l++] = *(data + i);			// data byte[i]
	}
	msg[l++] = 0x03;					// ETX

	memset(this->toBPMFE, 0, sizeof(this->toBPMFE));
	memcpy(this->toBPMFE, msg, l);
	this->toBPMFELen = l;
	// two status bytes to be read back
	this->fromBPMFELen = 2;

    return status;
}

asynStatus BPMFE::I2CRead(unsigned char i2cAddr, unsigned char intAddrWidth, unsigned int intAddr, unsigned char *data, unsigned short len) {
	asynStatus status = asynSuccess;
	int l, msgLen;
	unsigned char msg[MAX_MESSAGE_SIZE] = {0};
	l = 0;

	if (i2cAddr > 0x7F) {
		return asynError;
	}
	if (intAddrWidth > 4) {
		return asynError;
	}
	if (len > (MAX_MESSAGE_SIZE - intAddrWidth - 10)) {
		return asynError;
	}

	// calculate the message length wo/ STX and length included (3 bytes)
	msgLen =  1							// function code
			+ 1							// slave address
			+ 1							// count internal address
			+ intAddrWidth				// internal address width (0 - 4)
			+ 1							// R - READ
			+ 2							// number of bytes to read
			+ 1;						// ETX
	// build the message
	msg[l++] = 0x02;					// STX
	msg[l++] = (msgLen >> 8) & 0xFF;	// high byte of message length that follows
	msg[l++] = (msgLen & 0xFF);			// high byte of message length that follows
	msg[l++] = 0x03;					// function code
	msg[l++] = i2cAddr;					// slave address
	msg[l++] = intAddrWidth;			// count internal address
	// add specified intAddrWidth amount of bytes to the message
	switch(intAddrWidth) {
	case 0:
		// no additional bytes
		break;
	case 1:
		msg[l++] = intAddr & 0xFF;
		break;
	case 2:
		msg[l++] = (intAddr >> 8) & 0xFF;
		msg[l++] = intAddr & 0xFF;
		break;
	case 3:
		msg[l++] = (intAddr >> 16) & 0xFF;
		msg[l++] = (intAddr >> 8) & 0xFF;
		msg[l++] = intAddr & 0xFF;
		break;
	case 4:
		msg[l++] = (intAddr >> 24) & 0xFF;
		msg[l++] = (intAddr >> 16) & 0xFF;
		msg[l++] = (intAddr >> 8) & 0xFF;
		msg[l++] = intAddr & 0xFF;
		break;
	}
	msg[l++] = 0x52;					// R - READ
	msg[l++] = (len >> 8) & 0xFF;		// high byte count to read
	msg[l++] = len & 0xFF;				// low byte count to read
	msg[l++] = 0x03;					// ETX

	memset(this->toBPMFE, 0, sizeof(this->toBPMFE));
	memcpy(this->toBPMFE, msg, l);
	this->toBPMFELen = l;
	// data length and two status bytes to be read back
	this->fromBPMFELen = len + 2;

    return status;
}

asynStatus BPMFE::handleI2CTempSensor(void) {
	asynStatus status = asynSuccess;

	unsigned char data[2];
	unsigned short len;
/*
	data[0] = 0x00;
	len = 1;
	status = I2CWrite(0x48, 0, 0, data, len);
	if (status) {
		return status;
	}
//	status = writeBPMFE(1.0);
	if (status) {
		return status;
	}
*/

	data[0] = 0x00;
	data[1] = 0x00;
	len = 2;
	// supply pointer register value in single byte, value 0x00 -> temperature
	// register
	status = I2CRead(0x48, 1, 0, data, len);
	if (status) {
		return status;
	}
	printf("I2C READ\n");
	status = writeReadBPMFE(1.0);
	if (status) {
		return status;
	}

	return status;
}

asynStatus BPMFE::handleI2CEeprom(void) {
	asynStatus status = asynSuccess;
	static unsigned char c = '0';

	unsigned char data[5];
	unsigned short len;

	data[0] = 'H';
	data[1] = 'i';
	data[2] = 'n';
	data[3] = 'x';
	data[4] = c++;
	len = 5;
	status = I2CWrite(0x50, 2, 0, data, len);
	if (status) {
		return status;
	}
	printf("I2C WRITE\n");
	status = writeReadBPMFE(1.0);
	if (status) {
		return status;
	}

	/* XXX: This is mandatory if we do write, then read - otherwise
	 * the device is busy and NAKs the read! */
	usleep(10000);

	data[0] = 0x00;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0x00;
	data[4] = 0x00;
	len = 5;
	status = I2CRead(0x50, 2, 0, data, len);
	if (status) {
		return status;
	}
	printf("I2C READ\n");
	status = writeReadBPMFE(1.0);
	if (status) {
		return status;
	}

	return status;
}

asynStatus BPMFE::handleI2CBus(void) {
	asynStatus status = asynSuccess;

	memset(this->toBPMFE, 0, sizeof(this->toBPMFE));

	//status = handleI2CTempSensor();
	status = handleI2CEeprom();

	if (status) {
		return status;
	}


	return status;
}

asynStatus BPMFE::TTLIOWrite(unsigned char func, unsigned char bit) {
	asynStatus status = asynSuccess;
	int l;
	unsigned char msg[2] = {0};
	l = 0;

	if (func > 0x06) {
		return asynError;
	}
	// build the message
	msg[l++] = func;					// TTL IO function
	msg[l++] = bit;						// bit number

	memset(this->toBPMFE, 0, sizeof(this->toBPMFE));
	memcpy(this->toBPMFE, msg, l);
	this->toBPMFELen = l;
	// 0 bytes to be read back
	this->fromBPMFELen = 0;

    return status;
}

asynStatus BPMFE::TTLIOSetLevel(unsigned char bit, unsigned char val) {
	asynStatus status = asynSuccess;
	if (val == 0) {
		status = TTLIOWrite(TTLIO_FUNC_CLEAR_PIN, bit);
	} else if (val == 1) {
		status = TTLIOWrite(TTLIO_FUNC_SET_PIN, bit);
	}
	if (status) {
		return status;
	}
	printf("TTLIO WRITE\n");
	status = writeBPMFE(1.0);
	if (status) {
		return status;
	}
    return status;
}

asynStatus BPMFE::TTLIOSetDirection(unsigned char bit, unsigned char val) {
	asynStatus status = asynSuccess;
	if (val == 0) {
		status = TTLIOWrite(TTLIO_FUNC_INPUT_PIN, bit);
	} else if (val == 1) {
		status = TTLIOWrite(TTLIO_FUNC_OUTPUT_PIN, bit);
	}
	if (status) {
		return status;
	}
	printf("TTLIO WRITE\n");
	status = writeBPMFE(1.0);
	if (status) {
		return status;
	}
    return status;
}

asynStatus BPMFE::TTLIOSetPullup(unsigned char bit, unsigned char val) {
	asynStatus status = asynSuccess;
	if (val == 0) {
		status = TTLIOWrite(TTLIO_FUNC_CLEAR_PULLUP, bit);
	} else if (val == 1) {
		status = TTLIOWrite(TTLIO_FUNC_SET_PULLUP, bit);
	}
	if (status) {
		return status;
	}
	printf("TTLIO WRITE\n");
	status = writeBPMFE(1.0);
	if (status) {
		return status;
	}
    return status;
}

asynStatus BPMFE::handleTTLOutput(void) {
	asynStatus status = asynSuccess;
//	static unsigned char c = '0';

	unsigned char func;
	unsigned char bit;

	func = 0x04;
	bit = 0x01;
	status = TTLIOWrite(func, bit);
	if (status) {
		return status;
	}
	printf("TTLIO WRITE\n");
	status = writeBPMFE(1.0);
	if (status) {
		return status;
	}

	sleep(1);

	func = 0x02;
	bit = 0x01;
	status = TTLIOWrite(func, bit);
	if (status) {
		return status;
	}
	printf("TTLIO WRITE\n");
	status = writeBPMFE(1.0);
	if (status) {
		return status;
	}

#if 0
	/* XXX: This is mandatory if we do write, then read - otherwise
	 * the device is busy and NAKs the read! */
	usleep(10000);

	data[0] = 0x00;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0x00;
	data[4] = 0x00;
	len = 5;
	status = I2CRead(0x50, 2, 0, data, len);
	if (status) {
		return status;
	}
	printf("I2C READ\n");
	status = writeReadBPMFE(1.0);
	if (status) {
		return status;
	}
#endif

	return status;
}

asynStatus BPMFE::handleTTLIOBus(void) {
	asynStatus status = asynSuccess;

	memset(this->toBPMFE, 0, sizeof(this->toBPMFE));

	status = handleTTLOutput();

	if (status) {
		return status;
	}


	return status;
}

void BPMFE::dataTask(void) {
	int status;
	int counter;
	static const char *functionName = "dataTask";

	counter = 0;

	sleep(3);

	lock();

    while (1) {

        /* Has acquisition been stopped? */
        status = epicsEventTryWait(this->stopEventId);
        if (status == epicsEventWaitOK) {
			printf("%s: ending thread for port %s\n", functionName, portName);
            break;
        }

    	unlock();
    	sleep(1);
    	lock();

    	if (mIpPortType == BPMFE_IP_PORT_I2C) {
    		handleI2CBus();
    	} else if (mIpPortType == BPMFE_IP_PORT_TTLIO) {
//    		handleTTLIOBus();
    	} else {
			memset(this->toBPMFE, 0, sizeof(this->toBPMFE));
			epicsSnprintf(this->toBPMFE, sizeof(this->toBPMFE),
				"%s [%d]", portName, counter);
			writeBPMFE(1.0);
    	}

    	counter++;

    	unlock();
        callParamCallbacks();
    	lock();

    }
	printf("%s: data thread finished!\n", functionName);
    unlock();
}

asynStatus BPMFE::writeInt32(asynUser *pasynUser, epicsInt32 value) {

    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *functionName = "writeInt32";

    printf("%s: function %d, value %d\n", functionName, function, value);
    status = setIntegerParam(function, value);

    if (function == mTTLIO1Level
			|| function == mTTLIO2Level
			|| function == mTTLIO3Level
			|| function == mTTLIO4Level
			|| function == mTTLIO5Level
			|| function == mTTLIO6Level
			|| function == mTTLIO7Level
			|| function == mTTLIO8Level) {
    	TTLIOSetLevel(function - mTTLIO1Level + 1, value);
    } else if (function == mTTLIO1Direction
			|| function == mTTLIO2Direction
			|| function == mTTLIO3Direction
			|| function == mTTLIO4Direction
			|| function == mTTLIO5Direction
			|| function == mTTLIO6Direction
			|| function == mTTLIO7Direction
			|| function == mTTLIO8Direction) {
    	TTLIOSetDirection(function - mTTLIO1Direction + 1, value);
    } else if (function == mTTLIO1Pullup
			|| function == mTTLIO2Pullup
			|| function == mTTLIO3Pullup
			|| function == mTTLIO4Pullup
			|| function == mTTLIO5Pullup
			|| function == mTTLIO6Pullup
			|| function == mTTLIO7Pullup
			|| function == mTTLIO8Pullup) {
    	TTLIOSetPullup(function - mTTLIO1Pullup + 1, value);
    } else {
    	status = asynError;
    }

    /* Do callbacks so higher layers see any changes */
    callParamCallbacks();

    if (status)
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
              "%s:%s: error, status=%d function=%d, value=%d\n",
              driverName, functionName, status, function, value);
    else
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "%s:%s: function=%d, value=%d\n",
              driverName, functionName, function, value);

    return status;
}

asynStatus BPMFE::writeReadBPMFE(double timeout) {
    size_t nwrite, nread;
    int eomReason;
    asynStatus status;
    const char *functionName="writeReadBPMFE";

    printf("%s: request:\n", functionName);
	hexdump(this->toBPMFE, this->toBPMFELen);
	nread = 0;
//	this->fromBPMFELen = 2;

    status = pasynOctetSyncIO->writeRead(this->pasynUserCommand,
                                         this->toBPMFE, this->toBPMFELen,
                                         this->fromBPMFE, this->fromBPMFELen,
                                         timeout, &nwrite, &nread, &eomReason);
    printf("%s: response:\n", functionName);
    this->fromBPMFELen = nread;
	hexdump(this->fromBPMFE, this->fromBPMFELen);

    if (status) {
    	asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "%s:%s, status=%d, sent\n%s\n",
                driverName, functionName, status, this->toBPMFE);
    }

    /* Set output string so it can get back to EPICS */
//    setStringParam(BPMFEStringToServer, this->toBPMFE);
//    setStringParam(BPMFEStringFromServer, this->fromBPMFE);

    return(status);
}

asynStatus BPMFE::writeBPMFE(double timeout) {
    size_t nwrite;
    asynStatus status;
    const char *functionName="writeBPMFE";

//    printf("%s: called..\n", functionName);
    printf("%s: request:\n", functionName);
	hexdump(this->toBPMFE, this->toBPMFELen);

    status = pasynOctetSyncIO->write(this->pasynUserCommand,
                                     this->toBPMFE, this->toBPMFELen,
                                     timeout, &nwrite);
    if (status) {
    	asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "%s:%s, status=%d, sent\n%s\n",
                driverName, functionName, status, this->toBPMFE);
    }

    /* Set output string so it can get back to EPICS */
//    setStringParam(BPMFEStringToServer, this->toBPMFE);
    //setStringParam(BPMFEStringFromServer, this->fromBPMFE);

    return(status);
}

asynStatus BPMFE::readBPMFE(double timeout) {
    size_t nread;
    int eomReason;
    asynStatus status;
    const char *functionName="readBPMFE";

    printf("%s: called..\n", functionName);
    this->fromBPMFELen = MAX_MESSAGE_SIZE;
    status = pasynOctetSyncIO->read(this->pasynUserCommand,
                                     this->fromBPMFE, this->fromBPMFELen,
                                     timeout, &nread, &eomReason);

    printf("%s: response:\n", functionName);
    this->fromBPMFELen = nread;
	hexdump(this->fromBPMFE, this->fromBPMFELen);

    if (status) {
    	asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                "%s:%s, status=%d, sent\n%s\n",
                driverName, functionName, status, this->toBPMFE);
    }

    /* Set output string so it can get back to EPICS */
    //setStringParam(BPMFEStringToServer, this->toBPMFE);
//    setStringParam(BPMFEStringFromServer, this->fromBPMFE);

    return(status);
}

void BPMFE::report(FILE *fp, int details) {

    fprintf(fp, "BPM FE %s\n", this->portName);
    if (details > 0) {
    }
    /* Invoke the base class method */
    asynPortDriver::report(fp, details);
}

/** Constructor for the BPMFE class.
  * Calls constructor for the asynPortDriver base class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] commandPort The name of the TCP/IP server port */
BPMFE::BPMFE(const char *portName, const char *ipPort, int ipPortType)
   : asynPortDriver(portName,
                    1, /* maxAddr */
                    (int)NUM_BPMFE_PARAMS,
                    asynInt32Mask | asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask | asynOctetMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask | asynOctetMask,  /* Interrupt mask */
                    0, /* asynFlags.  This driver does not block and it is not multi-device, so flag is 0 */
                    1, /* Autoconnect */
                    0, /* Default priority */
                    0) /* Default stack size*/
{
    int status = asynSuccess;
    const char *functionName = "BPMFE";

    mIpPort = strdup(ipPort);
    mIpPortType = ipPortType;
    printf("%s: starting IP port %s, type %d\n", functionName, mIpPort, mIpPortType);

    this->stopEventId = epicsEventCreate(epicsEventEmpty);
    if (!this->stopEventId) {
        printf("%s:%s epicsEventCreate failure for stop event\n",
            driverName, functionName);
        return;
    }

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);

    createParam(BPMFEReadStatusString,          asynParamInt32, &BPMFEReadStatus);
    createParam(BPMFEStatusMessageString,       asynParamOctet, &BPMFEStatusMessage);
    createParam(BPMFEStringToServerString,      asynParamOctet, &BPMFEStringToServer);
    createParam(BPMFEStringFromServerString,    asynParamOctet, &BPMFEStringFromServer);

    /* TTL IO parameters */
    createParam(TTLIO1LevelString,              asynParamInt32, &mTTLIO1Level);
    createParam(TTLIO2LevelString,              asynParamInt32, &mTTLIO2Level);
    createParam(TTLIO3LevelString,              asynParamInt32, &mTTLIO3Level);
    createParam(TTLIO4LevelString,              asynParamInt32, &mTTLIO4Level);
    createParam(TTLIO5LevelString,              asynParamInt32, &mTTLIO5Level);
    createParam(TTLIO6LevelString,              asynParamInt32, &mTTLIO6Level);
    createParam(TTLIO7LevelString,              asynParamInt32, &mTTLIO7Level);
    createParam(TTLIO8LevelString,              asynParamInt32, &mTTLIO8Level);
    createParam(TTLIO1DirectionString,          asynParamInt32, &mTTLIO1Direction);
    createParam(TTLIO2DirectionString,          asynParamInt32, &mTTLIO2Direction);
    createParam(TTLIO3DirectionString,          asynParamInt32, &mTTLIO3Direction);
    createParam(TTLIO4DirectionString,          asynParamInt32, &mTTLIO4Direction);
    createParam(TTLIO5DirectionString,          asynParamInt32, &mTTLIO5Direction);
    createParam(TTLIO6DirectionString,          asynParamInt32, &mTTLIO6Direction);
    createParam(TTLIO7DirectionString,          asynParamInt32, &mTTLIO7Direction);
    createParam(TTLIO8DirectionString,          asynParamInt32, &mTTLIO8Direction);
    createParam(TTLIO1PullupString,             asynParamInt32, &mTTLIO1Pullup);
    createParam(TTLIO2PullupString,             asynParamInt32, &mTTLIO2Pullup);
    createParam(TTLIO3PullupString,             asynParamInt32, &mTTLIO3Pullup);
    createParam(TTLIO4PullupString,             asynParamInt32, &mTTLIO4Pullup);
    createParam(TTLIO5PullupString,             asynParamInt32, &mTTLIO5Pullup);
    createParam(TTLIO6PullupString,             asynParamInt32, &mTTLIO6Pullup);
    createParam(TTLIO7PullupString,             asynParamInt32, &mTTLIO7Pullup);
    createParam(TTLIO8PullupString,             asynParamInt32, &mTTLIO8Pullup);

    /* Connect to desired BPM FE IP port */
    status = pasynOctetSyncIO->connect(mIpPort, 0, &this->pasynUserCommand, NULL);

    setStringParam(BPMFEStatusMessage,          "");
    setStringParam(BPMFEStringToServer,         "");
    setStringParam(BPMFEStringFromServer,       "");

    /* Create the thread that computes the waveforms in the background */
    status = (asynStatus)(epicsThreadCreate("bpmfeTask",
                          epicsThreadPriorityMedium,
                          epicsThreadGetStackSize(epicsThreadStackMedium),
                          (EPICSTHREADFUNC)::bpmfeTaskC,
                          this) == NULL);
    if (status) {
        printf("%s:%s: epicsThreadCreate failure\n", driverName, functionName);
        return;
    }

    printf("%s: init OK!\n", functionName);
}

BPMFE::~BPMFE() {
    const char *functionName = "BPMFE";

    printf("%s: shutting down ...\n", functionName);

    epicsEventSignal(this->stopEventId);
    sleep(1);
	pasynOctetSyncIO->disconnect(pasynUserCommand);

    printf("%s: shutdown complete!\n", functionName);
}


/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

int bpmfeConfigure(const char *portName, const char *ipPort, int ipPortType) {
    new BPMFE(portName, ipPort, ipPortType);
    return(asynSuccess);
}

/* EPICS iocsh shell commands */

static const iocshArg initArg0 = { "portName",        iocshArgString};
static const iocshArg initArg1 = { "ipPort",          iocshArgString};
static const iocshArg initArg2 = { "ipPortType",      iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1,
                                            &initArg2};
static const iocshFuncDef initFuncDef = {"BPMFEConfigure", 3, initArgs};
static void initCallFunc(const iocshArgBuf *args) {
    bpmfeConfigure(args[0].sval, args[1].sval, args[2].ival);
}

void BPMFERegister(void) {
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(BPMFERegister);

} /* extern "C" */

