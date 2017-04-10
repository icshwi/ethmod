#Makefile at top of application tree
TOP = .
include $(TOP)/configure/CONFIG
DIRS += configure
DIRS += ethmodApp

ifeq ($(BUILD_IOCS), YES)
DIRS += ethmodDemoApp
ethmodDemoApp_DEPEND_DIRS += ethmodApp
iocBoot_DEPEND_DIRS += ethmodDemoApp
DIRS += iocBoot
endif

include $(TOP)/configure/RULES_TOP
