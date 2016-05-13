< envPaths
errlogInit(20000)

#cd "${TOP}"

## Register all support components
dbLoadDatabase("$(TOP)/dbd/bpmfeDemoApp.dbd")
bpmfeDemoApp_registerRecordDeviceDriver(pdbbase)
epicsEnvSet("EPICS_CA_ADDR_LIST", "192.168.100.100 192.168.100.1")
epicsEnvSet("EPICS_CA_AUTO_ADDR_LIST", "NO")

# Prefix for all records
epicsEnvSet("PREFIX", "BPMFE1:")
#epicsEnvSet("PORT",             "BPMFE")

epicsEnvSet("RS232_PORT",        "BPMFE_RS232")
epicsEnvSet("RS232_IP_PORT",     "BPMFE_RS232_COMM")
epicsEnvSet("LCD_PORT",          "BPMFE_LCD")
epicsEnvSet("LCD_IP_PORT",       "BPMFE_LCD_COMM")
epicsEnvSet("I2C_PORT",          "BPMFE_I2C")
epicsEnvSet("I2C_IP_PORT",       "BPMFE_I2C_COMM")

# Supported IP port types (see bpmfe.h)
#BPMFE_IP_PORT_INVALID       0
#BPMFE_IP_PORT_RS232         1
#BPMFE_IP_PORT_RS485         2
#BPMFE_IP_PORT_LCD           3
#BPMFE_IP_PORT_I2C           4
#BPMFE_IP_PORT_SPI           5
#BPMFE_IP_PORT_TTLIO         6
#BPMFE_IP_PORT_SDCARD        7
#BPMFE_IP_PORT_DFCARD        8

###
# Serial port
###
# Create the asyn port to talk to the AK-NORD server on command port 1002.
drvAsynIPPortConfigure($(RS232_IP_PORT),"192.168.100.100:1002")
#asynSetTraceIOMask($(RS232_IP_PORT),0,255)
#asynSetTraceMask($(RS232_IP_PORT),0,255)
# Set the terminators
asynOctetSetOutputEos($(RS232_IP_PORT), 0, "\r\n")
asynOctetSetInputEos($(RS232_IP_PORT),  0, "\n")

BPMFEConfigure($(RS232_PORT), $(RS232_IP_PORT), 1)
dbLoadRecords("$(BPMFE)/db/bpmfe.template",    "P=$(PREFIX),R=RS232:,PORT=$(RS232_PORT),ADDR=0,TIMEOUT=1,IP_PORT=$(RS232_IP_PORT)")
#asynSetTraceIOMask($(RS232_PORT),0,255)
#asynSetTraceMask($(RS232_PORT),0,255)


###
# LCD Display
###
# Create the asyn port to talk to the AK-NORD server on command port 1003.
drvAsynIPPortConfigure($(LCD_IP_PORT),"192.168.100.100:1003")
#asynSetTraceIOMask($(LCD_IP_PORT),0,255)
#asynSetTraceMask($(LCD_IP_PORT),0,255)
# Set the terminators
asynOctetSetOutputEos($(LCD_IP_PORT), 0, "\r\n")
#asynOctetSetInputEos($(LCD_IP_PORT), 0,  "\n")

BPMFEConfigure($(LCD_PORT), $(LCD_IP_PORT), 3)
dbLoadRecords("$(BPMFE)/db/bpmfe.template",    "P=$(PREFIX),R=LCD:,PORT=$(LCD_PORT),ADDR=0,TIMEOUT=1,IP_PORT=$(LCD_IP_PORT)")
#asynSetTraceIOMask($(LCD_PORT),0,255)
#asynSetTraceMask($(LCD_PORT),0,255)


###
# I2C
###
# Create the asyn port to talk to the AK-NORD server on command port 1004.
drvAsynIPPortConfigure($(I2C_IP_PORT),"192.168.100.100:1004")
#asynSetTraceIOMask($(I2C_IP_PORT),0,255)
#asynSetTraceMask($(I2C_IP_PORT),0,255)
# Set the terminators
asynOctetSetOutputEos($(I2C_IP_PORT), 0, "\r\n")
#asynOctetSetInputEos($(I2C_IP_PORT), 0,  "\n")

BPMFEConfigure($(I2C_PORT), $(I2C_IP_PORT), 4)
dbLoadRecords("$(BPMFE)/db/bpmfe.template",    "P=$(PREFIX),R=I2C:,PORT=$(I2C_PORT),ADDR=0,TIMEOUT=1,IP_PORT=$(I2C_IP_PORT)")
#asynSetTraceIOMask($(I2C_PORT),0,255)
#asynSetTraceMask($(I2C_PORT),0,255)


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
