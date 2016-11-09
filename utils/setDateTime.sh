#!/bin/bash


echo "Setting RTC PCF85063TP date & time to:"
date

#date +"%y %m %d %H %M %S %w"
YEAR=$(date +"%y")
MONTH=$(date +"%m")
DAY=$(date +"%d")
WEEKDAY=$(date +"%w")
HOUR=$(date +"%H")
MINUTE=$(date +"%M")
SECOND=$(date +"%S")

caput BPMFE:I2C1:RTC1:Days $DAY
caput BPMFE:I2C1:RTC1:Hours $HOUR
caput BPMFE:I2C1:RTC1:Minutes $MINUTE
caput BPMFE:I2C1:RTC1:Months $MONTH
caput BPMFE:I2C1:RTC1:Seconds $SECOND
caput BPMFE:I2C1:RTC1:Weekdays $WEEKDAY
caput BPMFE:I2C1:RTC1:Years $YEAR
caput BPMFE:I2C1:RTC1:Write 1

caput BPMFE:I2C1:RTC1:Read 1

caget BPMFE:I2C1:RTC1:Years_RBV
caget BPMFE:I2C1:RTC1:Months_RBV
caget BPMFE:I2C1:RTC1:Days_RBV
caget BPMFE:I2C1:RTC1:Weekdays_RBV
caget BPMFE:I2C1:RTC1:Hours_RBV
caget BPMFE:I2C1:RTC1:Minutes_RBV
caget BPMFE:I2C1:RTC1:Seconds_RBV
caget -S BPMFE:I2C1:RTC1:Date_RBV
caget -S BPMFE:I2C1:RTC1:Time_RBV

exit 0
