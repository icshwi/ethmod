/*
 * akNord.h
 *
 *  Created on: May 15, 2016
 *      Author: hinxx
 */

#ifndef _AKNORD_H_
#define _AKNORD_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <asynOctetSyncIO.h>

/*
 * AK-NORD XT-PICO-SXL bus support
 */

typedef enum _AKMsgType {
	AKMsgUnknown = 0,
	AKMsgRead,
	AKMsgWrite,
//	AKMsgReadOnly,
//	AKMsgWriteOnly,
//	AKMsgWriteRead,
	AKMsgLast
} AKMsgType;

#define AK_MSG_SZ			512
#define AK_MSG_HDR_SZ		2
#define AK_MSG_TOT_SZ		(AK_MSG_SZ + AK_MSG_HDR_SZ)

class AKDevice;
typedef struct _AKMsg {
	AKDevice *dev;
	int type;
	size_t nReqMax;
	size_t nRespMax;
	size_t nReq;
	size_t nResp;
	unsigned char req[AK_MSG_TOT_SZ];
	unsigned char resp[AK_MSG_TOT_SZ];
} AKMsg;

typedef enum _AKBusType {
	AKBusTypeUnknown = 0,
//	AKBusTypeAsyn,
	AKBusTypeRS232,
	AKBusTypeRS485,
	AKBusTypeLCD,
	AKBusTypeI2C,
	AKBusTypeSPI,
	AKBusTypeTTLIO,
//	AKBusTypeSDCard,	// XXX: Can not be controlled using sockets (only FTP)
//	AKBusTypeDFCard,	// XXX: Can not be controlled using sockets (only FTP)
	AKBusTypeLast
} AKBusType;

class AKBus {
public:
	AKBus(int type, asynUser *asynUserCommand)
		: mAsynUserCommand(asynUserCommand),
		  mType(type) {}
	virtual ~AKBus() {}

	int getType() { return mType; }
	virtual int write(AKMsg *msg);
	virtual int read(AKMsg *msg);
	virtual int writeRead(AKMsg *msg);

protected:

    asynUser *mAsynUserCommand;

private:
    void hexdump(void *mem, unsigned int len);

	int mType;
};

#define AK_MAX_I2C_MUXES		8

class AKBusI2C: public AKBus {
public:
	AKBusI2C(asynUser *asynUserCommand)
		: AKBus(AKBusTypeI2C, asynUserCommand) {}
	virtual ~AKBusI2C() {}

protected:
	int write(AKMsg *msg);
	int read(AKMsg *msg);
	int writeRead(AKMsg *msg);

private:
};

typedef enum _AKDeviceType {
	AKDeviceUnknown						= 0,

	AKDeviceRS232Generic				= 100,

	AKDeviceRS485Generic				= 200,

	AKDeviceLCDGeneric					= 300,

	AKDeviceI2CGeneric					= 400,
	AKDeviceI2CEeprom24LC64,
	AKDeviceI2CEepromM24M02,
	AKDeviceI2CTempSenTMP100,
	AKDeviceI2CMuxTCA9546A,
	AKDeviceI2CSerialDS28CM00,
	AKDeviceI2CPortExpTCA9555,
	AKDeviceI2CRealTimePCF85063TP,
	AKDeviceI2CVoltageMonitorLTC2991,

	AKDeviceSPIGeneric					= 500,

	AKDeviceTTLIOGeneric				= 600,


	AKDeviceLast
} AKDeviceType;

class AKDevice {
public:
	AKDevice(AKBus *bus, int type)
		: mBus(bus),
		  mType(type) {}
	virtual ~AKDevice() {}

	int getType() { return mType; }
	int getBusType() { return mBus->getType(); }
	int busWrite() { return mBus->write(&mMsg); }
	int busRead() { return mBus->read(&mMsg); }
	int busWriteRead() { return mBus->writeRead(&mMsg); }
	virtual AKDevice *getMux() { return NULL; }
	virtual unsigned char getMuxBus() { return 0; }
//	virtual void initMsg(int type);
	virtual int createMsg(int type, unsigned char *data, int len, int off);
	virtual int write(unsigned char *data, int *len, int off);
	virtual int read(unsigned char *data, int *len, int off);

	AKMsg mMsg;
	AKBus *mBus;

private:
	int mType;
};

class AKDeviceI2C: public AKDevice {
public:
	AKDeviceI2C(AKBus *bus, int type,
			unsigned char devAddr,
			unsigned char addrWidth,
			AKDevice *mux,
			unsigned char muxBus)
		: AKDevice(bus, type),
		  mDevAddr(devAddr),
		  mAddrWidth(addrWidth),
//		  mOffset(0),
		  mSize(0),
		  mMux(mux),
		  mMuxBus(muxBus) {}
	virtual ~AKDeviceI2C() {}

//	int getOffset() { return mOffset; }
	int getSize() { return mSize; }
	AKDevice *getMux() { return mMux; }
	unsigned char getMuxBus() { return mMuxBus; }
	virtual void setSize(int val) { mSize = val; }
//	virtual void setOffset(int val) {
//		if (val >= getSize()) {
//			return;
//		}
//		AKDeviceI2C::setOffset(val);
//	}

	virtual int createMsg(int type, unsigned char *data, int len, int off);
//	virtual int write(unsigned char *data, int *len, int off);
//	virtual int read(unsigned char *data, int *len, int off);

private:
	unsigned char mDevAddr;
	unsigned char mAddrWidth;
//	int mOffset;
	int mSize;
	AKDevice *mMux;
	unsigned char mMuxBus;
};

class AKDeviceI2CEeprom: public AKDeviceI2C {
public:
	AKDeviceI2CEeprom(AKBus *bus, int type,
			unsigned char devAddr,
			unsigned char addrWidth,
			AKDevice *mux,
			unsigned char muxBus)
		: AKDeviceI2C(bus, type, devAddr, addrWidth, mux, muxBus) {
		switch(type) {
		case AKDeviceI2CEeprom24LC64:
			setSize(8 * 1204);
			printf("EEPROM type 24LC64, %d bytes\n", getSize());
			break;
		case AKDeviceI2CEepromM24M02:
			setSize(256 * 1024);
			printf("EEPROM type M24M02, %d bytes\n", getSize());
			break;
		default:
			printf("Unsupported EEPROM type!\n");
			break;
		}
	}
	virtual ~AKDeviceI2CEeprom() {}

//	int readData(unsigned char *data, int *len, int off);
//	int writeData(unsigned char *data, int *len, int off);
//	virtual int write(unsigned char *data, int *len, int off);
//	virtual int read(unsigned char *data, int *len, int off);

protected:

private:
};

class AKDeviceI2CTempSens: public AKDeviceI2C {
public:
	AKDeviceI2CTempSens(AKBus *bus, int type,
			unsigned char devAddr,
			unsigned char addrWidth,
			AKDevice *mux,
			unsigned char muxBus)
		: AKDeviceI2C(bus, type, devAddr, addrWidth, mux, muxBus),
		  mTemp(0),
		  mResolution(9) {
		switch(type) {
		case AKDeviceI2CTempSenTMP100:
			setSize(4);
			printf("Temperature sensor type TMP100\n");
			break;
		default:
			printf("Unsupported temperature sensor type!\n");
			break;
		}
	}
	virtual ~AKDeviceI2CTempSens() {}

	int readTemp();
	int writeResolution(unsigned char val);
//	virtual int write(unsigned char *data, int *len, int off);
//	virtual int read(unsigned char *data, int *len, int off);

	double getTemp() { return (double)mTemp / 16.0; }
	int getRawTemp() { return mTemp; }
	int getResolution() { return mResolution; }

protected:

private:
	int mTemp;
	int mResolution;
};

class AKDeviceI2CSerial: public AKDeviceI2C {
public:
	AKDeviceI2CSerial(AKBus *bus, int type,
			unsigned char devAddr,
			unsigned char addrWidth,
			AKDevice *mux,
			unsigned char muxBus)
		: AKDeviceI2C(bus, type, devAddr, addrWidth, mux, muxBus),
		  mSerial(0) {
		switch(type) {
		case AKDeviceI2CSerialDS28CM00:
			setSize(8);
			printf("Serial number type DS28CM00\n");
			break;
		default:
			printf("Unsupported serial number type!\n");
			break;
		}
	}
	virtual ~AKDeviceI2CSerial() {}

	int readSerial();

	long long int getSerial() { return mSerial; }

protected:

private:
	/* Should be 64bits on all archs */
	long long int mSerial;
};

class AKDeviceI2CMux: public AKDeviceI2C {
public:
	AKDeviceI2CMux(AKBus *bus, int type,
			unsigned char devAddr,
			unsigned char addrWidth,
			AKDevice *mux,
			unsigned char muxBus)
		: AKDeviceI2C(bus, type, devAddr, addrWidth, mux, muxBus),
		  mCurrMuxBus(0) {
		switch(type) {
		case AKDeviceI2CMuxTCA9546A:
			setSize(1);
			printf("Mux type TCA9546A\n");
			break;
		default:
			printf("Unsupported mux type!\n");
			break;
		}
	}
	virtual ~AKDeviceI2CMux() {}

//	int readMuxBus(unsigned char *val);
	int writeMuxBus(unsigned char val);

//	unsigned char getCurrMuxBus() { return mCurrMuxBus; }
	virtual int write(unsigned char *data, int *len, int off);
//	virtual int read(unsigned char *data, int *len, int off);

protected:

private:
	unsigned char mCurrMuxBus;
};

#endif /* _AKNORD_H_ */
