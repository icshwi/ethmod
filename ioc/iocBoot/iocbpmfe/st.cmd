< envPaths
errlogInit(20000)

#cd "${TOP}"

## Register all support components
dbLoadDatabase("$(TOP)/dbd/bpmfeDemoApp.dbd")
bpmfeDemoApp_registerRecordDeviceDriver(pdbbase)
#epicsEnvSet("EPICS_CA_ADDR_LIST", "127.0.0.1")
epicsEnvSet("EPICS_CA_ADDR_LIST", "192.168.100.100 192.168.100.1")
epicsEnvSet("EPICS_CA_AUTO_ADDR_LIST", "NO")
epicsEnvSet("EPICS_CA_MAX_ARRAY_BYTES", "10000000")
# The search path for database files
epicsEnvSet("EPICS_DB_INCLUDE_PATH", "$(BPMFE)/db")

# Prefix for all records
epicsEnvSet("PREFIX", "BPMFE:")
#epicsEnvSet("PORT",             "BPMFE")

epicsEnvSet("RS232_PORT",        "AK_RS232")
epicsEnvSet("RS232_IP_PORT",     "AK_RS232_COMM")

epicsEnvSet("LCD_PORT",          "AK_LCD")
epicsEnvSet("LCD_IP_PORT",       "AK_LCD_COMM")

#epicsEnvSet("I2C_PORT",          "AK_I2C")
epicsEnvSet("I2C_TEMP_PORT",     "AK_I2C_TEMP")
epicsEnvSet("I2C_EEPROM_PORT",   "AK_I2C_EEPROM")
epicsEnvSet("I2C_IDNUM_PORT",    "AK_I2C_IDNUM")
epicsEnvSet("I2C_RTC_PORT",      "AK_I2C_RTC")
epicsEnvSet("I2C_IOEXP_PORT",    "AK_I2C_IOEXP")
epicsEnvSet("I2C_LTC2991_PORT",  "AK_I2C_LTC2991")
epicsEnvSet("I2C_IP_PORT",       "AK_I2C_COMM")

epicsEnvSet("TTLIO_PORT",        "AK_TTLIO")
epicsEnvSet("TTLIO_IP_PORT",     "AK_TTLIO_COMM")

# Supported IP port types (see bpmfe.h)
#AK_IP_PORT_INVALID       0
#AK_IP_PORT_RS232         1
#AK_IP_PORT_RS485         2
#AK_IP_PORT_LCD           3
#AK_IP_PORT_I2C           4
#AK_IP_PORT_SPI           5
#AK_IP_PORT_TTLIO         6
#AK_IP_PORT_SDCARD        7
#AK_IP_PORT_DFCARD        8

###
# Serial port
###
# Create the asyn port to talk to the AK-NORD server on command port 1002.
#drvAsynIPPortConfigure($(RS232_IP_PORT),"192.168.100.100:1002")
#asynSetTraceIOMask($(RS232_IP_PORT),0,255)
#asynSetTraceMask($(RS232_IP_PORT),0,255)
# Set the terminators
#asynOctetSetOutputEos($(RS232_IP_PORT), 0, "\r\n")
#asynOctetSetInputEos($(RS232_IP_PORT),  0, "\n")

#BPMFEConfigure($(RS232_PORT), $(RS232_IP_PORT), 1)
#dbLoadRecords("$(BPMFE)/db/bpmfe.template",    "P=$(PREFIX),R=RS232:,PORT=$(RS232_PORT),ADDR=0,TIMEOUT=1,IP_PORT=$(RS232_IP_PORT)")
#asynSetTraceIOMask($(RS232_PORT),0,255)
#asynSetTraceMask($(RS232_PORT),0,255)


###
# LCD Display
###
# Create the asyn port to talk to the AK-NORD server on command port 1003.
#drvAsynIPPortConfigure($(LCD_IP_PORT),"192.168.100.100:1003")
#asynSetTraceIOMask($(LCD_IP_PORT),0,255)
#asynSetTraceMask($(LCD_IP_PORT),0,255)
# Set the terminators
#asynOctetSetOutputEos($(LCD_IP_PORT), 0, "\r\n")
#asynOctetSetInputEos($(LCD_IP_PORT), 0,  "\n")

#BPMFEConfigure($(LCD_PORT), $(LCD_IP_PORT), 3)
#dbLoadRecords("$(BPMFE)/db/bpmfe.template",    "P=$(PREFIX),R=LCD:,PORT=$(LCD_PORT),ADDR=0,TIMEOUT=1,IP_PORT=$(LCD_IP_PORT)")
#asynSetTraceIOMask($(LCD_PORT),0,255)
#asynSetTraceMask($(LCD_PORT),0,255)


###
# I2C
###
# Create the asyn port to talk to the AK-NORD server on command port 1002.
#drvAsynIPPortConfigure($(I2C_IP_PORT),"127.0.0.1:9999")
drvAsynIPPortConfigure($(I2C_IP_PORT),"192.168.100.100:1002")
#asynSetTraceIOMask($(I2C_IP_PORT),0,255)
#asynSetTraceMask($(I2C_IP_PORT),0,255)
# Set the terminators
#asynOctetSetOutputEos($(I2C_IP_PORT), 0, "\003")
#asynOctetSetInputEos($(I2C_IP_PORT), 0,  "\003")

# AKI2CTempConfigure(const char *portName, const char *ipPort,
#        int devCount, const char *devAddrs,
#        int muxAddr, int muxBus,
#        int priority, int stackSize);
#AKI2CTempConfigure($(I2C_TEMP_PORT), $(I2C_IP_PORT), 8, "0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F", 0x70, 0, 1, 0, 0)
#AKI2CTempConfigure($(I2C_TEMP_PORT), $(I2C_IP_PORT), 1, "0x48", 0x70, 0, 1, 0, 0)
#dbLoadRecords("$(BPMFE)/db/AKI2CTemp.template",       "P=$(PREFIX),R=I2C1:Temp1:,PORT=$(I2C_TEMP_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=0,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/AKI2CTemp.template",       "P=$(PREFIX),R=I2C1:Temp2:,PORT=$(I2C_TEMP_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=1,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/AKI2CTemp.template",       "P=$(PREFIX),R=I2C1:Temp3:,PORT=$(I2C_TEMP_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=2,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/AKI2CTemp.template",       "P=$(PREFIX),R=I2C1:Temp4:,PORT=$(I2C_TEMP_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=3,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/AKI2CTemp.template",       "P=$(PREFIX),R=I2C1:Temp5:,PORT=$(I2C_TEMP_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=4,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/AKI2CTemp.template",       "P=$(PREFIX),R=I2C1:Temp6:,PORT=$(I2C_TEMP_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=5,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/AKI2CTemp.template",       "P=$(PREFIX),R=I2C1:Temp7:,PORT=$(I2C_TEMP_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=6,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/AKI2CTemp.template",       "P=$(PREFIX),R=I2C1:Temp8:,PORT=$(I2C_TEMP_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=7,TIMEOUT=1")
#asynSetTraceIOMask($(I2C_TEMP_PORT),0,255)
#asynSetTraceMask($(I2C_TEMP_PORT),0,255)

# AKI2CEepromConfigure(const char *portName, const char *ipPort,
#        int devCount, const char *devAddrs,
#        int muxAddr, int muxBus,
#        int priority, int stackSize);
#AKI2CEepromConfigure($(I2C_EEPROM_PORT), $(I2C_IP_PORT), 1, "0x50", 0x70, 0, 1, 0, 0)
#dbLoadRecords("$(BPMFE)/db/AKI2CEeprom.template",     "P=$(PREFIX),R=I2C1:Eeprom1:,PORT=$(I2C_EEPROM_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=0,TIMEOUT=1,NELM=65536")
#dbLoadRecords("$(BPMFE)/db/AKI2CEeprom.template",     "P=$(PREFIX),R=I2C1:Eeprom2:,PORT=$(I2C_EEPROM_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=1,TIMEOUT=1,NELM=262144")
#asynSetTraceIOMask($(I2C_EEPROM_PORT),0,255)
#asynSetTraceMask($(I2C_EEPROM_PORT),0,255)

# AKI2CIdNumConfigure(const char *portName, const char *ipPort,
#        int devCount, const char *devAddrs,
#        int muxAddr, int muxBus,
#        int priority, int stackSize);
#AKI2CIdNumConfigure($(I2C_IDNUM_PORT), $(I2C_IP_PORT), 1, "0x50", 0x70, 0, 1, 0, 0)
#dbLoadRecords("$(BPMFE)/db/AKI2CIdNum.template",      "P=$(PREFIX),R=I2C1:IdNum1:,PORT=$(I2C_IDNUM_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=0,TIMEOUT=1")
#asynSetTraceIOMask($(I2C_IDNUM_PORT),0,255)
#asynSetTraceMask($(I2C_IDNUM_PORT),0,255)

# AKI2CRTCConfigure(const char *portName, const char *ipPort,
#        int devCount, const char *devAddrs,
#        int muxAddr, int muxBus,
#        int priority, int stackSize);
#AKI2CRTCConfigure($(I2C_RTC_PORT), $(I2C_IP_PORT), 1, "0x51", 0x70, 0, 1, 0, 0)
#dbLoadRecords("$(BPMFE)/db/AKI2CRTC.template",        "P=$(PREFIX),R=I2C1:RTC1:,PORT=$(I2C_RTC_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=0,TIMEOUT=1")
#asynSetTraceIOMask($(I2C_RTC_PORT),0,255)
#asynSetTraceMask($(I2C_RTC_PORT),0,255)

# AKI2CIOExpConfigure(const char *portName, const char *ipPort,
#        int devCount, const char *devAddrs,
#        int muxAddr, int muxBus,
#        int priority, int stackSize);
#AKI2CIOExpConfigure($(I2C_IOEXP_PORT), $(I2C_IP_PORT), 1, "0x20", 0x70, 0, 1, 0, 0)
#dbLoadRecords("$(BPMFE)/db/AKI2CIOExp.template",        "P=$(PREFIX),R=I2C1:IOExp1:,PORT=$(I2C_IOEXP_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=0,TIMEOUT=1")
#asynSetTraceIOMask($(I2C_IOEXP_PORT),0,255)
#asynSetTraceMask($(I2C_IOEXP_PORT),0,255)

# AKI2CLTC2991Configure(const char *portName, const char *ipPort,
#        int devCount, const char *devAddrs,
#        int muxAddr, int muxBus,
#        int priority, int stackSize);
AKI2CLTC2991Configure($(I2C_LTC2991_PORT), $(I2C_IP_PORT), 1, "0x4F", 0x70, 0, 1, 0, 0)
dbLoadRecords("$(BPMFE)/db/AKI2C_LTC2991.template",        "P=$(PREFIX),R=I2C1:VMon1:,PORT=$(I2C_LTC2991_PORT),IP_PORT=$(I2C_IP_PORT),ADDR=0,TIMEOUT=1")
asynSetTraceIOMask($(I2C_LTC2991_PORT),0,255)
asynSetTraceMask($(I2C_LTC2991_PORT),0,255)


###
# TTL-IO
###
# Create the asyn port to talk to the AK-NORD server on command port 1002.
#drvAsynIPPortConfigure($(TTLIO_IP_PORT),"192.168.100.100:1002")
#asynSetTraceIOMask($(TTLIO_IP_PORT),0,255)
#asynSetTraceMask($(TTLIO_IP_PORT),0,255)
# Set the terminators
#asynOctetSetOutputEos($(TTLIO_IP_PORT), 0, "\003")
#asynOctetSetInputEos($(TTLIO_IP_PORT), 0,  "\003")

#BPMFEConfigure($(TTLIO_PORT), $(TTLIO_IP_PORT), 6)
#dbLoadRecords("$(BPMFE)/db/bpmfe.template",          "P=$(PREFIX),R=TTLIO:,PORT=$(TTLIO_PORT),ADDR=0,TIMEOUT=1,IP_PORT=$(TTLIO_IP_PORT)")
#dbLoadRecords("$(BPMFE)/db/bpmfe_ttlio.template",    "P=$(PREFIX),R=TTLIO:,N=1,PORT=$(TTLIO_PORT),ADDR=0,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/bpmfe_ttlio.template",    "P=$(PREFIX),R=TTLIO:,N=2,PORT=$(TTLIO_PORT),ADDR=0,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/bpmfe_ttlio.template",    "P=$(PREFIX),R=TTLIO:,N=3,PORT=$(TTLIO_PORT),ADDR=0,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/bpmfe_ttlio.template",    "P=$(PREFIX),R=TTLIO:,N=4,PORT=$(TTLIO_PORT),ADDR=0,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/bpmfe_ttlio.template",    "P=$(PREFIX),R=TTLIO:,N=5,PORT=$(TTLIO_PORT),ADDR=0,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/bpmfe_ttlio.template",    "P=$(PREFIX),R=TTLIO:,N=6,PORT=$(TTLIO_PORT),ADDR=0,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/bpmfe_ttlio.template",    "P=$(PREFIX),R=TTLIO:,N=7,PORT=$(TTLIO_PORT),ADDR=0,TIMEOUT=1")
#dbLoadRecords("$(BPMFE)/db/bpmfe_ttlio.template",    "P=$(PREFIX),R=TTLIO:,N=8,PORT=$(TTLIO_PORT),ADDR=0,TIMEOUT=1")
#asynSetTraceIOMask($(TTLIO_PORT),0,255)
#asynSetTraceMask($(TTLIO_PORT),0,255)



#set_requestfile_path("./")
#set_requestfile_path("$(BPMFE)/bpmfeApp/Db")
#set_savefile_path("./autosave")
#set_pass0_restoreFile("auto_settings.sav")
#set_pass1_restoreFile("auto_settings.sav")
#save_restoreSet_status_prefix("$(PREFIX)")
#dbLoadRecords("$(AUTOSAVE)/asApp/Db/save_restoreStatus.db", "P=$(PREFIX)")

#cd "${TOP}/iocBoot/${IOC}"
iocInit()

# save things every thirty seconds
#create_monitor_set("${TOP}/iocBoot/${IOC}/auto_settings.req", 30,"P=$(PREFIX)")
