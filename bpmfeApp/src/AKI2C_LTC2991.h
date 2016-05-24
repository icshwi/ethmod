/*
 * AKI2C_LTC2991.h
 *
 *  Created on: May 24, 2016
 *      Author: hinkokocevar
 */

#ifndef _AKI2C_LTC2991_H_
#define _AKI2C_LTC2991_H_



#include "AKI2C.h"

#define AKI2C_LTC2991_STATUS_LOW_REG			0x00
#define AKI2C_LTC2991_STATUS_HIGH_REG			0x01
#define AKI2C_LTC2991_CH_ENABLE_REG				0x01
//#define AKI2C_LTC2991_TRIGGER_REG				0x01
#define AKI2C_LTC2991_CONTROL1_REG				0x06
#define AKI2C_LTC2991_CONTROL2_REG				0x07
#define AKI2C_LTC2991_CONTROL3_REG				0x08
#define AKI2C_LTC2991_V1_MSB_REG				0x0A
#define AKI2C_LTC2991_V1_LSB_REG				0x0B
#define AKI2C_LTC2991_V2_MSB_REG				0x0C
#define AKI2C_LTC2991_V2_LSB_REG				0x0D
#define AKI2C_LTC2991_V3_MSB_REG				0x0E
#define AKI2C_LTC2991_V3_LSB_REG				0x0F
#define AKI2C_LTC2991_V4_MSB_REG				0x10
#define AKI2C_LTC2991_V4_LSB_REG				0x11

/* Enable all Vx channels, Internal temperature and Vcc sensoring */
#define AKI2C_LTC2991_CH_ENABLE_VAL				0xF8
/* Use filtering for Vx channels */
#define AKI2C_LTC2991_CONTROL1_VAL				0x88
#define AKI2C_LTC2991_CONTROL2_VAL				0x88
/* Use filtering for Internal Temperature */
#define AKI2C_LTC2991_CONTROL3_VAL				0x08

#define AKI2CLTC2991ReadString                  "AKI2C_LTC2991_READ"
#define AKI2CLTC2991TriggerString               "AKI2C_LTC2991_TRIGGER"

//#define AKI2CLTC2991V1V2FilterString            "AKI2C_LTC2991_V1V2_FILTER"
//#define AKI2CLTC2991V1V2TUnitString             "AKI2C_LTC2991_V1V2_TUNIT"
//#define AKI2CLTC2991V1V2TypeString              "AKI2C_LTC2991_V1V2_TYPE"
//#define AKI2CLTC2991V1V2VModeString             "AKI2C_LTC2991_V1V2_VMODE"
#define AKI2CLTC2991V1ValueString             "AKI2C_LTC2991_V1_VALUE"
#define AKI2CLTC2991V1OffsetString            "AKI2C_LTC2991_V1_OFFSET"
#define AKI2CLTC2991V1FactorString            "AKI2C_LTC2991_V1_FACTOR"

#define AKI2CLTC2991V2ValueString             "AKI2C_LTC2991_V2_VALUE"
#define AKI2CLTC2991V2OffsetString            "AKI2C_LTC2991_V2_OFFSET"
#define AKI2CLTC2991V2FactorString            "AKI2C_LTC2991_V2_FACTOR"

#define AKI2CLTC2991V3ValueString             "AKI2C_LTC2991_V3_VALUE"
#define AKI2CLTC2991V3OffsetString            "AKI2C_LTC2991_V3_OFFSET"
#define AKI2CLTC2991V3FactorString            "AKI2C_LTC2991_V3_FACTOR"

#define AKI2CLTC2991V4ValueString             "AKI2C_LTC2991_V4_VALUE"
#define AKI2CLTC2991V4OffsetString            "AKI2C_LTC2991_V4_OFFSET"
#define AKI2CLTC2991V4FactorString            "AKI2C_LTC2991_V4_FACTOR"

//#define AKI2CLTC2991V3V4FilterString            "AKI2C_LTC2991_V3V4_FILTER"
//#define AKI2CLTC2991V3V4TUnitString             "AKI2C_LTC2991_V3V4_TUNIT"
//#define AKI2CLTC2991V3V4TypeString              "AKI2C_LTC2991_V3V4_TYPE"
//#define AKI2CLTC2991V3V4VModeString             "AKI2C_LTC2991_V3V4_VMODE"
//#define AKI2CLTC2991V3V4ValueString             "AKI2C_LTC2991_V3V4_VALUE"
//#define AKI2CLTC2991V3V4EnableString            "AKI2C_LTC2991_V3V4_ENABLE"
//
//#define AKI2CLTC2991V5V6FilterString            "AKI2C_LTC2991_V5V6_FILTER"
//#define AKI2CLTC2991V5V6TUnitString             "AKI2C_LTC2991_V5V6_TUNIT"
//#define AKI2CLTC2991V5V6TypeString              "AKI2C_LTC2991_V5V6_TYPE"
//#define AKI2CLTC2991V5V6VModeString             "AKI2C_LTC2991_V5V6_VMODE"
//#define AKI2CLTC2991V5V6ValueString             "AKI2C_LTC2991_V5V6_VALUE"
//#define AKI2CLTC2991V5V6EnableString            "AKI2C_LTC2991_V5V6_ENABLE"
//
//#define AKI2CLTC2991V7V8FilterString            "AKI2C_LTC2991_V7V8_FILTER"
//#define AKI2CLTC2991V7V8TUnitString             "AKI2C_LTC2991_V7V8_TUNIT"
//#define AKI2CLTC2991V7V8TypeString              "AKI2C_LTC2991_V7V8_TYPE"
//#define AKI2CLTC2991V7V8VModeString             "AKI2C_LTC2991_V7V8_VMODE"
//#define AKI2CLTC2991V7V8ValueString             "AKI2C_LTC2991_V7V8_VALUE"
//#define AKI2CLTC2991V7V8EnableString            "AKI2C_LTC2991_V7V8_ENABLE"
//
//#define AKI2CLTC2991TIntFilterString            "AKI2C_LTC2991_TINT_FILTER"
//#define AKI2CLTC2991TIntTUnitString             "AKI2C_LTC2991_TINT_TUNIT"
//#define AKI2CLTC2991TIntValueString             "AKI2C_LTC2991_TINT_VALUE"
//#define AKI2CLTC2991TIntEnableString            "AKI2C_LTC2991_TINT_ENABLE"

/** Driver for AK-NORD XT-PICO-SXL I2C LTC2991 sensor chip access over TCP/IP socket */
class AKI2C_LTC2991: public AKI2C {
public:
	AKI2C_LTC2991(const char *portName, const char *ipPort,
	        int numDevices, int priority, int stackSize);
	virtual ~AKI2C_LTC2991();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Our parameter list */
    int AKI2C_LTC2991_Read;
#define FIRST_AKI2C_LTC2991_PARAM AKI2C_LTC2991_Read
    int AKI2C_LTC2991_Trigger;
    int AKI2C_LTC2991_V1_Value;
    int AKI2C_LTC2991_V1_Factor;
    int AKI2C_LTC2991_V1_Offset;
    int AKI2C_LTC2991_V2_Value;
    int AKI2C_LTC2991_V2_Factor;
    int AKI2C_LTC2991_V2_Offset;
    int AKI2C_LTC2991_V3_Value;
    int AKI2C_LTC2991_V3_Factor;
    int AKI2C_LTC2991_V3_Offset;
    int AKI2C_LTC2991_V4_Value;
    int AKI2C_LTC2991_V4_Factor;
    int AKI2C_LTC2991_V4_Offset;
//    int AKI2C_LTC2991_V1V2_Filter;
//    int AKI2C_LTC2991_V1V2_TUnit;
//    int AKI2C_LTC2991_V1V2_VMode;
#define LAST_AKI2C_LTC2991_PARAM AKI2C_LTC2991_V4_Offset

private:
    void convertToVoltage(int addr, int valueParam,
    		int offsetParam, int factorParam, unsigned int raw);
    asynStatus write(int addr, unsigned char reg, unsigned char val);
    asynStatus read(int addr);
};

#define NUM_AKI2C_LTC2991_PARAMS ((int)(&LAST_AKI2C_LTC2991_PARAM - &FIRST_AKI2C_LTC2991_PARAM + 1))

#endif /* _AKI2C_LTC2991_H_ */
