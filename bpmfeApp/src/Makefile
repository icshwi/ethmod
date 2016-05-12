TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += bpmfe

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install bpmfe.dbd into <top>/dbd
DBD += bpmfe.dbd

# specify all source files to be compiled and added to the library
#bpmfe_SRCS += xxx

bpmfe_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

