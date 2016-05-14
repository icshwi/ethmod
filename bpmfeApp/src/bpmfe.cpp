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
//	hexdump(this->toBPMFE, l);

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
//	hexdump(this->toBPMFE, l);

    return status;
}

asynStatus BPMFE::handleI2CTempSensor(void) {
	asynStatus status = asynSuccess;

	unsigned char data[2];
	unsigned short len;
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

	data[0] = 0x00;
	data[1] = 0x00;
	len = 2;
	status = I2CRead(0x48, 0, 0, data, len);
	if (status) {
		return status;
	}
//	status = readBPMFE(1.0);
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
	this->fromBPMFELen = 2;
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
	this->fromBPMFELen = 5 + 2;
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
    const char* functionName = "writeInt32";

    printf("%s: function %d, value %d\n", functionName, function, value);

    /* Set the parameter in the parameter library. */
    status = (asynStatus) setIntegerParam(function, value);

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
	hexdump(this->fromBPMFE, this->fromBPMFELen);

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

