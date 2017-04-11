TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += ethmod
PROD_NAME = ethmod

# install ethmod.dbd into <top>/dbd
DBD += $(PROD_NAME).dbd
$(PROD_NAME)_DBD += $(PROD_NAME)Support.dbd

# specify all source files to be compiled and added to the library
$(PROD_NAME)_SRCS += AKBase.cpp
$(PROD_NAME)_SRCS += AKI2C.cpp
$(PROD_NAME)_SRCS += AKI2C_TMP100.cpp
$(PROD_NAME)_SRCS += AKI2C_DS28CM00.cpp
$(PROD_NAME)_SRCS += AKI2C_PCF85063TP.cpp
$(PROD_NAME)_SRCS += AKI2C_TCA9555.cpp
$(PROD_NAME)_SRCS += AKI2C_LTC2991.cpp
$(PROD_NAME)_SRCS += AKI2C_M24M02.cpp

PROD_LIBS += asyn

$(PROD_NAME)_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

