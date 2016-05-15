/*
 * bpmfe.h
 *
 *  Created on: May 12, 2016
 *      Author: hinxx
 */

#ifndef BPMFEAPP_SRC_BPMFE_H_
#define BPMFEAPP_SRC_BPMFE_H_

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <epicsMutex.h>
//#include <ellLib.h>

#include <asynPortDriver.h>
#include <asynOctetSyncIO.h>

#define BPMFE_IP_PORT_INVALID		0
#define BPMFE_IP_PORT_RS232			1
#define BPMFE_IP_PORT_RS485			2
#define BPMFE_IP_PORT_LCD			3
#define BPMFE_IP_PORT_I2C			4
#define BPMFE_IP_PORT_SPI			5
#define BPMFE_IP_PORT_TTLIO			6
#define BPMFE_IP_PORT_SDCARD		7
#define BPMFE_IP_PORT_DFCARD		8

#define TTLIO_FUNC_READ_PIN			0x00
#define TTLIO_FUNC_CLEAR_PIN		0x01
#define TTLIO_FUNC_SET_PIN			0x02
#define TTLIO_FUNC_INPUT_PIN		0x03
#define TTLIO_FUNC_OUTPUT_PIN		0x04
#define TTLIO_FUNC_CLEAR_PULLUP		0x05
#define TTLIO_FUNC_SET_PULLUP		0x06

#define MAX_MESSAGE_SIZE	512

#define BPMFEReadStatusString                "READ_STATUS"           /**< (asynInt32,    r/w) Write 1 to force a read of status */
#define BPMFEStatusMessageString             "STATUS_MESSAGE"        /**< (asynOctet,    r/o) Status message */
#define BPMFEStringToServerString            "STRING_TO_SERVER"      /**< (asynOctet,    r/o) String sent to server for message-based drivers */
#define BPMFEStringFromServerString          "STRING_FROM_SERVER"    /**< (asynOctet,    r/o) String received from server for message-based drivers */

#define TTLIO1LevelString                    "TTLIO_1_LEVEL"
#define TTLIO2LevelString                    "TTLIO_2_LEVEL"
#define TTLIO3LevelString                    "TTLIO_3_LEVEL"
#define TTLIO4LevelString                    "TTLIO_4_LEVEL"
#define TTLIO5LevelString                    "TTLIO_5_LEVEL"
#define TTLIO6LevelString                    "TTLIO_6_LEVEL"
#define TTLIO7LevelString                    "TTLIO_7_LEVEL"
#define TTLIO8LevelString                    "TTLIO_8_LEVEL"
#define TTLIO1DirectionString                "TTLIO_1_DIR"
#define TTLIO2DirectionString                "TTLIO_2_DIR"
#define TTLIO3DirectionString                "TTLIO_3_DIR"
#define TTLIO4DirectionString                "TTLIO_4_DIR"
#define TTLIO5DirectionString                "TTLIO_5_DIR"
#define TTLIO6DirectionString                "TTLIO_6_DIR"
#define TTLIO7DirectionString                "TTLIO_7_DIR"
#define TTLIO8DirectionString                "TTLIO_8_DIR"
#define TTLIO1PullupString                   "TTLIO_1_PULLUP"
#define TTLIO2PullupString                   "TTLIO_2_PULLUP"
#define TTLIO3PullupString                   "TTLIO_3_PULLUP"
#define TTLIO4PullupString                   "TTLIO_4_PULLUP"
#define TTLIO5PullupString                   "TTLIO_5_PULLUP"
#define TTLIO6PullupString                   "TTLIO_6_PULLUP"
#define TTLIO7PullupString                   "TTLIO_7_PULLUP"
#define TTLIO8PullupString                   "TTLIO_8_PULLUP"

/** Driver for BPM frontend using AK-NORD XT-PICO-SXL server over TCP/IP socket */
class BPMFE : public asynPortDriver {
public:
	BPMFE(const char *portName, const char *ipPort, int ipPortType);
	virtual ~BPMFE();

    /* These are the methods that we override from asynPortDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */
    void dataTask();  /* This should be private but is called from C so must be public */
    ELLNODE node;  /* This should be private but is called from C so must be public */

protected:
    int BPMFEReadStatus;
#define FIRST_BPMFE_PARAM BPMFEReadStatus
    int BPMFEStatusMessage;
    int BPMFEStringToServer;
    int BPMFEStringFromServer;
    int mTTLIO1Level;
    int mTTLIO2Level;
    int mTTLIO3Level;
    int mTTLIO4Level;
    int mTTLIO5Level;
    int mTTLIO6Level;
    int mTTLIO7Level;
    int mTTLIO8Level;
    int mTTLIO1Direction;
    int mTTLIO2Direction;
    int mTTLIO3Direction;
    int mTTLIO4Direction;
    int mTTLIO5Direction;
    int mTTLIO6Direction;
    int mTTLIO7Direction;
    int mTTLIO8Direction;
    int mTTLIO1Pullup;
    int mTTLIO2Pullup;
    int mTTLIO3Pullup;
    int mTTLIO4Pullup;
    int mTTLIO5Pullup;
    int mTTLIO6Pullup;
    int mTTLIO7Pullup;
    int mTTLIO8Pullup;
    int BPMFEDummyEnd;
#define LAST_BPMFE_PARAM BPMFEDummyEnd

private:
    /* These are the methods that are new to this class */
    asynStatus writeBPMFE(double timeout);
    asynStatus readBPMFE(double timeout);
    asynStatus writeReadBPMFE(double timeout);

    asynStatus I2CWrite(unsigned char i2cAddr, unsigned char intAddrWidth, unsigned int intAddr, unsigned char *data, unsigned short len);
    asynStatus I2CRead(unsigned char i2cAddr, unsigned char intAddrWidth, unsigned int intAddr, unsigned char *data, unsigned short len);
    asynStatus handleI2CBus();
    asynStatus handleI2CTempSensor();
    asynStatus handleI2CEeprom();

    asynStatus handleTTLIOBus();
    asynStatus handleTTLOutput();
    asynStatus TTLIOWrite(unsigned char func, unsigned char bit);
    asynStatus TTLIOSetLevel(unsigned char bit, unsigned char val);
    asynStatus TTLIOSetDirection(unsigned char bit, unsigned char val);
    asynStatus TTLIOSetPullup(unsigned char bit, unsigned char val);


    char *mIpPort;
    int mIpPortType;

    /* Our data */
//    epicsEventId startEventId;
//    epicsEventId readoutEventId;
    epicsEventId stopEventId;
//    epicsTimerId timerId;
    char toBPMFE[MAX_MESSAGE_SIZE];
    int toBPMFELen;
    char fromBPMFE[MAX_MESSAGE_SIZE];
    int fromBPMFELen;
    asynUser *pasynUserCommand;
};

#define NUM_BPMFE_PARAMS ((int)(&LAST_BPMFE_PARAM - &FIRST_BPMFE_PARAM + 1))

#endif /* BPMFEAPP_SRC_BPMFE_H_ */
