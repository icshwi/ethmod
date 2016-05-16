/*
 * AKI2CEeprom.h
 *
 *  Created on: May 16, 2016
 *      Author: hinkokocevar
 */

#ifndef _AKI2CEEPROM_H_
#define _AKI2CEEPROM_H_

#include "AKI2C.h"

#define AK_I2C_EEPROM_MAX_SZ			65536

#define AKI2CEepromDevAddrString                  "AKI2CEEPROM_DEV_ADDR"
#define AKI2CEepromMuxAddrString                  "AKI2CEEPROM_MUX_ADDR"
#define AKI2CEepromMuxBusString                   "AKI2CEEPROM_MUX_BUS"
#define AKI2CEepromDataString                     "AKI2CEEPROM_DATA"
#define AKI2CEepromReadString                     "AKI2CEEPROM_READ"
#define AKI2CEepromWriteString                    "AKI2CEEPROM_WRITE"

/** Driver for AK-NORD XT-PICO-SXL I2C EEPROM chip access over TCP/IP socket */
class AKI2CEeprom: public AKI2C {
public:
	AKI2CEeprom(const char *portName, const char *ipPort,
	        int numDevices, int priority, int stackSize);
	virtual ~AKI2CEeprom();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus readInt8Array(asynUser *pasynUser, epicsInt8 *value,
                                        size_t nElements, size_t *nIn);
    virtual asynStatus writeInt8Array(asynUser *pasynUser, epicsInt8 *value,
                                        size_t nElements);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Our parameter list */
    int AKI2CEepromDevAddr;
#define FIRST_AKI2CEEPROM_PARAM AKI2CEepromDevAddr
    int AKI2CEepromMuxAddr;
    int AKI2CEepromMuxBus;
    int AKI2CEepromData;
    int AKI2CEepromRead;
    int AKI2CEepromWrite;
#define LAST_AKI2CEEPROM_PARAM AKI2CEepromWrite

private:
    asynStatus setData(int addr, unsigned char *data, unsigned short len, unsigned int off);
    asynStatus getData(int addr, unsigned char *data, unsigned short *len, unsigned int off);

    unsigned char *mData;
    unsigned short mDataLen;
};

#define NUM_AKI2CEEPROM_PARAMS ((int)(&LAST_AKI2CEEPROM_PARAM - &FIRST_AKI2CEEPROM_PARAM + 1))

#endif /* _AKI2CEEPROM_H_ */
