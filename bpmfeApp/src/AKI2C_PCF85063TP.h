/*
 * AKI2C_PCF85063TP.h
 *
 *  Created on: May 17, 2016
 *      Author: hinxx
 */

#ifndef _AKI2C_PCF85063TP_H_
#define _AKI2C_PCF85063TP_H_

#include "AKI2C.h"

#define AKI2C_PCF85063TP_CONTROL1_REG					0x00
#define AKI2C_PCF85063TP_CONTROL2_REG					0x01
#define AKI2C_PCF85063TP_OFFSET_REG						0x02
#define AKI2C_PCF85063TP_RAMBYTE_REG					0x03
#define AKI2C_PCF85063TP_SECONDS_REG					0x04
#define AKI2C_PCF85063TP_MINUTES_REG					0x05
#define AKI2C_PCF85063TP_HOURS_REG						0x06
#define AKI2C_PCF85063TP_DAYS_REG						0x07
#define AKI2C_PCF85063TP_WEEKDAYS_REG					0x08
#define AKI2C_PCF85063TP_MONTHS_REG						0x09
#define AKI2C_PCF85063TP_YEARS_REG						0x0A

#define AKI2C_PCF85063TP_ReadString                     "AKI2C_PCF85063TP_READ"
#define AKI2C_PCF85063TP_WriteString                    "AKI2C_PCF85063TP_WRITE"
#define AKI2C_PCF85063TP_SecondsString                  "AKI2C_PCF85063TP_SECONDS"
#define AKI2C_PCF85063TP_MinutesString                  "AKI2C_PCF85063TP_MINUTES"
#define AKI2C_PCF85063TP_HoursString                    "AKI2C_PCF85063TP_HOURS"
#define AKI2C_PCF85063TP_DaysString                     "AKI2C_PCF85063TP_DAYS"
#define AKI2C_PCF85063TP_WeekdaysString                 "AKI2C_PCF85063TP_WEEKDAYS"
#define AKI2C_PCF85063TP_MonthsString                   "AKI2C_PCF85063TP_MONTHS"
#define AKI2C_PCF85063TP_YearsString                    "AKI2C_PCF85063TP_YEARS"
#define AKI2C_PCF85063TP_DateString                     "AKI2C_PCF85063TP_DATE"
#define AKI2C_PCF85063TP_TimeString                     "AKI2C_PCF85063TP_TIME"

/*
 * Chip       : NXP PCF85063TP
 * Function   : Real-time clock / calendar
 * Bus        : I2C
 * Access     : TCP/IP socket on AK-NORD XT-PICO-SX
 */
class AKI2C_PCF85063TP: public AKI2C {
public:
	AKI2C_PCF85063TP(const char *portName, const char *ipPort,
	        int devCount, const char *devInfos, int priority, int stackSize);
	virtual ~AKI2C_PCF85063TP();

    /* These are the methods that we override from AKI2C */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    void report(FILE *fp, int details);
    /* These are new methods */

protected:
    /* Our parameter list */
    int AKI2C_PCF85063TP_Read;
#define FIRST_AKI2C_PCF85063TP_PARAM AKI2C_PCF85063TP_Read
    int AKI2C_PCF85063TP_Write;
    int AKI2C_PCF85063TP_Seconds;
    int AKI2C_PCF85063TP_Minutes;
    int AKI2C_PCF85063TP_Hours;
    int AKI2C_PCF85063TP_Days;
    int AKI2C_PCF85063TP_Weekdays;
    int AKI2C_PCF85063TP_Months;
    int AKI2C_PCF85063TP_Years;
    int AKI2C_PCF85063TP_Date;
    int AKI2C_PCF85063TP_Time;
#define LAST_AKI2C_PCF85063TP_PARAM AKI2C_PCF85063TP_Time

private:
    unsigned char bcdToDec(unsigned char val);
    unsigned char decToBcd(unsigned char val);
    asynStatus write(int addr, unsigned char reg, unsigned char *val, unsigned short *len);
    asynStatus read(int addr, unsigned char reg, unsigned char *val, unsigned short *len);
    asynStatus setDateTime(int addr);
    asynStatus getDateTime(int addr);
};

#define NUM_AKI2C_PCF85063TP_PARAMS ((int)(&LAST_AKI2C_PCF85063TP_PARAM - &FIRST_AKI2C_PCF85063TP_PARAM + 1))

#endif /* _AKI2C_PCF85063TP_H_ */
