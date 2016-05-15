/*
 * akNord.cpp
 *
 *  Created on: May 15, 2016
 *      Author: hinxx
 */

#include "akNord.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>



#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 8
#endif
void AKBus::hexdump(void *mem, unsigned int len) {
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

int AKBus::read(AKMsg *msg) {
    int eomReason;
    int ret;

    printf("%s: request:\n", __func__);
	hexdump(msg->req, msg->nReqMax);

    ret = pasynOctetSyncIO->read(mAsynUserCommand,
    		(char *)msg->req, msg->nReqMax,
			1.0, &msg->nResp, &eomReason);

    printf("%s: response:\n", __func__);
	hexdump(msg->resp, msg->nResp);

    return ret;
}

int AKBus::write(AKMsg *msg) {
    int ret;

    printf("%s: request:\n", __func__);
	hexdump(msg->req, msg->nReqMax);

    ret = pasynOctetSyncIO->write(mAsynUserCommand,
    		(const char *)msg->req, msg->nReqMax, 1.0, &msg->nReq);

    return ret;
}

int AKBus::writeRead(AKMsg *msg) {
    int eomReason;
    int ret;

    printf("%s: request:\n", __func__);
	hexdump(msg->req, msg->nReqMax);

    ret = pasynOctetSyncIO->writeRead(mAsynUserCommand,
    		(const char *)msg->req, msg->nReqMax,
			(char *)msg->resp, msg->nRespMax,
			1.0, &msg->nReq, &msg->nResp, &eomReason);

    printf("%s: response:\n", __func__);
	hexdump(msg->resp, msg->nResp);

    return ret;
}

int AKBusI2C::read(AKMsg *msg) {
    int ret;
    AKDevice *dev = msg->dev;
    AKDevice *mux = dev->getMux();
    unsigned char data[1];
    int len = 1;

    if (mux) {
    	ret = mux->write(data, &len, 0);
    	if (ret) {
    		return ret;
    	}
    }

    ret = AKBus::read(msg);

    return ret;
}

int AKBusI2C::write(AKMsg *msg) {
    int ret;
    AKDevice *dev = msg->dev;
    AKDevice *mux = dev->getMux();
    unsigned char data[1];
    int len = 1;

    if (mux) {
    	ret = mux->write(data, &len, 0);
    	if (ret) {
    		return ret;
    	}
    }

    ret = AKBus::write(msg);

    return ret;
}

int AKBusI2C::writeRead(AKMsg *msg) {
    int ret;
    AKDevice *dev = msg->dev;
    AKDevice *mux = dev->getMux();
    unsigned char data[1];
    int len = 1;

    if (mux) {
    	ret = mux->write(data, &len, 0);
    	if (ret) {
    		return ret;
    	}
    }

    ret = AKBus::writeRead(msg);

    return ret;
}

//void AKDevice::initMsg(int type) {
//	mMsg.dev = this;
//	mMsg.type = type;
//	mMsg.nReq = 0;
//	mMsg.nResp = 0;
//	mMsg.nReqMax = 0;
//	mMsg.nRespMax = 0;
//	memset(mMsg.req, 0, AK_MSG_TOT_SZ);
//	memset(mMsg.resp, 0, AK_MSG_TOT_SZ);
//}

int AKDevice::createMsg(int type, unsigned char *data, int len, int off) {
	int i, l;
	unsigned char *req;

	if ((type != AKMsgRead) && (type != AKMsgWrite)) {
		return -1;
	}
	if (len <= 0) {
		return -1;
	}
	if ((type == AKMsgWrite) && (!data)) {
		return -1;
	}
	if (len > AK_MSG_SZ) {
		return -1;
	}

	mMsg.dev = this;
	mMsg.type = type;
	mMsg.nReq = 0;
	mMsg.nResp = 0;
	mMsg.nReqMax = len;
	mMsg.nRespMax = len;
	memset(mMsg.req, 0, AK_MSG_TOT_SZ);
	memset(mMsg.resp, 0, AK_MSG_TOT_SZ);

	// build the request
	l = 0;
	req = (unsigned char *)mMsg.req;
	if (mMsg.type == AKMsgWrite) {
		for (i = 0; i < len; i++) {
			req[l++] = *(data + i);		// data byte[i]
		}
	}

    return 0;
}

int AKDeviceI2C::createMsg(int type, unsigned char *data, int len, int off) {
	int i, l, msgLen;
	unsigned char *req;

	if ((type != AKMsgRead) && (type != AKMsgWrite)) {
		return -1;
	}
	if ((type == AKMsgWrite) && (!data)) {
		return -1;
	}
	if ((type == AKMsgWrite) && (len <= 0)) {
		return -1;
	}
	if ((type == AKMsgRead) && ((len + AK_MSG_HDR_SZ) > AK_MSG_TOT_SZ)) {
		return -1;
	}

	// calculate the message length wo/ STX and length included (3 bytes)
	msgLen =  1							// function code
			+ 1							// slave address
			+ 1							// count internal address
			+ mAddrWidth				// internal address width (0 - 4)
			+ 1							// W - WRITE or R - READ
			+ 2							// bytes to write
			+ len						// data length
			+ 1;						// ETX
	if (msgLen > AK_MSG_TOT_SZ) {
		return -1;
	}

	mMsg.dev = this;
	mMsg.type = type;
	mMsg.nReq = 0;
	mMsg.nResp = 0;
	mMsg.nReqMax = msgLen;
	mMsg.nRespMax = len + AK_MSG_HDR_SZ;
	memset(mMsg.req, 0, AK_MSG_TOT_SZ);
	memset(mMsg.resp, 0, AK_MSG_TOT_SZ);

	// build the request
	l = 0;
	req = (unsigned char *)mMsg.req;

	req[l++] = 0x02;					// STX
	req[l++] = (msgLen >> 8) & 0xFF;	// high byte of message length that follows
	req[l++] = (msgLen & 0xFF);			// high byte of message length that follows
	req[l++] = 0x03;					// function code
	req[l++] = mDevAddr;				// slave I2C address
	req[l++] = mAddrWidth;				// count internal address
	// add specified addrWidth amount of bytes to the message
	switch(mAddrWidth) {
	case 0:
		// no additional bytes
		break;
	case 1:
		req[l++] = off & 0xFF;
		break;
	case 2:
		req[l++] = (off >> 8) & 0xFF;
		req[l++] = off & 0xFF;
		break;
	case 3:
		req[l++] = (off >> 16) & 0xFF;
		req[l++] = (off >> 8) & 0xFF;
		req[l++] = off & 0xFF;
		break;
	case 4:
		req[l++] = (off >> 24) & 0xFF;
		req[l++] = (off >> 16) & 0xFF;
		req[l++] = (off >> 8) & 0xFF;
		req[l++] = off & 0xFF;
		break;
	}
	if (mMsg.type == AKMsgRead) {
		req[l++] = 0x52;				// R - READ
	} else if (mMsg.type == AKMsgWrite) {
		req[l++] = 0x57;				// W - WRITE
	}
	req[l++] = (len >> 8) & 0xFF;		// high byte count to write
	req[l++] = len & 0xFF;				// low byte count to write
	if (mMsg.type == AKMsgWrite) {
		for (i = 0; i < len; i++) {
			req[l++] = *(data + i);		// data byte[i]
		}
	}
	req[l++] = 0x03;					// ETX

    return 0;
}

int AKDevice::read(unsigned char *data, int *len, int off) {
	int ret;

//	initMsg(AKMsgRead);
//	setOffset(off);
	ret = createMsg(AKMsgRead, data, *len, off);
	if (ret) {
		return ret;
	}
	ret = busWriteRead();
	if (ret) {
		return ret;
	}

	if (data) {
		memcpy(data, mMsg.resp, mMsg.nResp);
	}
	*len = mMsg.nResp;

	return ret;
}

int AKDevice::write(unsigned char *data, int *len, int off) {
	int ret;

//	initMsg(AKMsgWrite);
//	setOffset(off);
	ret = createMsg(AKMsgWrite, data, *len, off);
	if (ret) {
		return ret;
	}
	ret = busWriteRead();
	if (ret) {
		return ret;
	}

	*len = mMsg.nReq;

	return ret;
}

//int AKDeviceI2CEeprom::read(unsigned char *data, int *len, int off) {
//	int ret;
//
//	initMsg(AKMsgRead);
//	setOffset(off);
//	ret = createMsg(NULL, *len);
//	if (ret) {
//		return ret;
//	}
//	ret = busWriteRead();
//	if (ret) {
//		return ret;
//	}
//
//	memcpy(data, mMsg.resp, mMsg.nResp);
//	*len = mMsg.nResp;
//
//	return ret;
//}
//
//int AKDeviceI2CEeprom::write(unsigned char *data, int *len, int off) {
//	int ret;
//
//	initMsg(AKMsgWrite);
//	setOffset(off);
//	ret = createMsg(data, *len);
//	if (ret) {
//		return ret;
//	}
//	ret = busWriteRead();
//	if (ret) {
//		return ret;
//	}
//
//	*len = mMsg.nReq;
//
//	return ret;
//}

int AKDeviceI2CTempSens::readTemp() {
	int ret;
	int len = 2;
//	initMsg(AKMsgRead);
//	setOffset(0);
//	ret = createMsg(NULL, 2);
//	if (ret) {
//		return ret;
//	}
//	ret = busWriteRead();
//	if (ret) {
//		return ret;
//	}

	ret = read(NULL, &len, 0);
	if (ret) {
		return ret;
	}

	mTemp = mMsg.resp[2] << 8 | mMsg.resp[3];
	mTemp = mTemp >> 4;

	return ret;
}

int AKDeviceI2CTempSens::writeResolution(unsigned char val) {
	int ret;
	unsigned char data[2] = {0};
	int len = 2;

//	initMsg(AKMsgWrite);
//	setOffset(1);
	if (val == 9) {
		data[0] = 0x00 << 6;
	} else if (val == 10) {
		data[0] = 0x01 << 6;
	} else if (val == 11) {
		data[0] = 0x02 << 6;
	} else if (val == 12) {
		data[0] = 0x03 << 6;
	} else {
		return -1;
	}
//	ret = createMsg(data, 2);
//	if (ret) {
//		return ret;
//	}
//	ret = busWriteRead();
//	if (ret) {
//		return ret;
//	}

	ret = write(data, &len, 1);
	if (ret) {
		return ret;
	}

	mResolution = val;

	return ret;
}

int AKDeviceI2CSerial::readSerial() {
	int ret;
	int len = 8;

//	initMsg(AKMsgRead);
//	setOffset(0);
//	ret = createMsg(NULL, 8);
//	if (ret) {
//		return ret;
//	}
//	ret = busWriteRead();
//	if (ret) {
//		return ret;
//	}

	ret = read(NULL, &len, 0);
	if (ret) {
		return ret;
	}

	mSerial = (long long int)mMsg.resp[0] << 56
			| (long long int)mMsg.resp[1] << 48
			| (long long int)mMsg.resp[2] << 40
			| (long long int)mMsg.resp[3] << 32
			| (long long int)mMsg.resp[4] << 24
			| (long long int)mMsg.resp[5] << 16
			| (long long int)mMsg.resp[6] << 8
			| (long long int)mMsg.resp[7];

	return ret;
}

//int AKDeviceI2CMux::writeMuxBus(unsigned char val) {
//	int ret;
//    unsigned char data[1] = {0};
//
//	if (val == mCurrMuxBus) {
//		return 0;
//	}
//
//	initMsg(AKMsgWrite);
//	setOffset(0);
//	data[0] = val;
//	ret = createMsg(data, 1);
//	if (ret) {
//		return ret;
//	}
//	ret = busWriteRead();
//	if (ret) {
//		return ret;
//	}
//
//	return 0;
//}

int AKDeviceI2CMux::write(unsigned char *data, int *len, int off) {
	int ret;

	if (data[0] == mCurrMuxBus) {
		return 0;
	}

	ret = AKDeviceI2C::write(data, len, 0);
	if (ret) {
		return ret;
	}

	mCurrMuxBus = data[0];

	return ret;
}
