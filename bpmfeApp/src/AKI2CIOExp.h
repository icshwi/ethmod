/*
 * AKI2CIOExp.h
 *
 *  Created on: May 19, 2016
 *      Author: hinxx
 */

#ifndef _AKI2CIOEXP_H_
#define _AKI2CIOEXP_H_


#include "AKI2C.h"

#define AK_I2C_IOEXP_INPUT0			0x00
#define AK_I2C_IOEXP_INPUT1			0x01
#define AK_I2C_IOEXP_OUTPUT0		0x02
#define AK_I2C_IOEXP_OUTPUT1		0x03
#define AK_I2C_IOEXP_POLARITY0		0x04
#define AK_I2C_IOEXP_POLARITY1		0x05
#define AK_I2C_IOEXP_DIRECTION0		0x06
#define AK_I2C_IOEXP_DIRECTION1		0x07

#define AK_I2C_IOEXP_MAX_PINS		0x10

#define AKI2CIOExpReadInputString              "AKI2CIOEXP_READ_INPUT"
#define AKI2CIOExpReadOutputString             "AKI2CIOEXP_READ_OUTPUT"
#define AKI2CIOExpReadPolarityString           "AKI2CIOEXP_READ_POLARITY"
#define AKI2CIOExpReadDirectionString          "AKI2CIOEXP_READ_DIRECTION"
#define AKI2CIOExpPinLevelString               "AKI2CIOEXP_PIN_LEVEL"
#define AKI2CIOExpPinDirectionString           "AKI2CIOEXP_PIN_DIRECTION"
#define AKI2CIOExpPinPolarityString            "AKI2CIOEXP_PIN_POLARITY"

/** Driver for AK-NORD XT-PICO-SXL I2C port expander access over TCP/IP socket */
class AKI2CIOExp: public AKI2C {
public:
	AKI2CIOExp(const char *portName, const char *ipPort,
	        int numDevices, int priority, int stackSize);
	virtual ~AKI2CIOExp();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Our parameter list */
    int AKI2CIOExpReadInput;
#define FIRST_AKI2CIOEXP_PARAM AKI2CIOExpReadInput
    int AKI2CIOExpReadOutput;
    int AKI2CIOExpReadPolarity;
    int AKI2CIOExpReadDirection;
    int AKI2CIOExpPinLevel;
    int AKI2CIOExpPinDirection;
    int AKI2CIOExpPinPolarity;
#define LAST_AKI2CIOEXP_PARAM AKI2CIOExpPinPolarity

private:
//    int clearBit(int val, int bit);
//    int setBit(int val, int bit);
    int changeBit(int val, int bit, int level);
    asynStatus read(int addr, unsigned char reg);
    asynStatus write(int addr, unsigned char reg, unsigned char val);
    asynStatus writeOutput(int addr);
    asynStatus writePolInv(int addr);
    asynStatus writeConfig(int addr);

    unsigned int mPinIn;
    unsigned int mPinOut;
    unsigned int mPinPol;
    unsigned int mPinDir;
};

#define NUM_AKI2CIOEXP_PARAMS ((int)(&LAST_AKI2CIOEXP_PARAM - &FIRST_AKI2CIOEXP_PARAM + 1))




#endif /* _AKI2CIOEXP_H_ */
