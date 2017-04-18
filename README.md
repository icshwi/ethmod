# ESS Ethernet Module (EthMod)

This repository holds EPICS support for ESS Ethernet module, based on
AK-NORD XT-PICO-SXL board.

ESS repository is at https://bitbucket.org/europeanspallationsource/m-epics-ethmod

See repository _docs_ folder for I2C chip PDF manuals and board schematics.

See also vendor information:

* http://www.ak-nord.de/
* http://www.ak-nord.de/en/daten/manual_xxl_ts.pdf
* http://www.ak-nord.de/de/daten/handbuch_xxl_ts.pdf


## Features

* Several on-board and break-out I2C chips connected to a single I2C bus
* Monitoring of board power supply voltage
* Two user controlled LEDs
* 14 user controlled GPIOs
* 256kB of EEPROM storage (with read only identification page)
* 6 voltage monitors
* +5 .. +25 V input power supply


# EthMod I2C chips

Here is a list of on-board I2C chips found on EthMod PCB:

* Linear LTC2991 volatage monitor (0x48)
* TI TMP100 temperature sensor (0x49)
* TI TCA9555 port expander (0x21)
* ST M24M02 EEPROM (0x50)

Other I2C devices can be added to the I2C bus via expander connector P1. These
are avalable as break-out boards:

* 6x TI TMP100 temperature sensor (0x4A .. 0x4F) 

# XT-PICO-SXL configuration

The first port is configured for I2C and the second one is left for
driving EVA KIT board on-board LCD (default).

Here are some screen dumps from web interface. 

## Main menu

Landing page when opening *http://192.168.1.100* in web browser.

	=========================== MAIN MENU =======================================
	
	  A = ADMIN MENU
	  G = GENERAL MENU
	
	  N = NETWORK MENU
	  D = DISK DRIVES MENU
	  I = INTERFACE MENU
	
	  F = Factory Settings(and restart Interface)
	  R = Restart Interface
	  Q = EXIT TELNET(Restart if any value changed)
	
	
	
	
	
	
	
	
	
	
	  For example:'A'[ENTER]
	-----------------------------------------------------------------------------
	[Q = QUIT] Please enter your choice:


## General menu

Some general XT-PICO-SXL information.

	=========================== GENERAL MENU ====================================
	
	      Time since start  = 00:000:00:03:15 (yy:ddd:hh:mm:ss)
	      Hardware          = XT-PICO-SXL-EF-02
	      Software          = PIC32MZ2048EF064P-AK-STACK-XXL/SXL-EF
	      Version           = 1.7.8
	      Date              = 07.08.2016
	      Processor clock   = 200.000.000
	      Periphery clock   = 100.000.000
	
	
	  M = Memory summary
	
	
	
	
	
	
	
	
	
	
	
	-----------------------------------------------------------------------------
	[Q = QUIT] Please enter your choice:


## Interface menu

Use *1* to access I2C port.
	
	=========================== INTERFACE MENU ==================================
	
	  1 = I2C1 Menu
	  2 = LCD2 Menu
	  E = ETHERNET Menu
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	  For example:'A'[ENTER]
	-----------------------------------------------------------------------------
	[Q = QUIT] Please enter your choice:
	
## I2C port menu

	=========================== I2C MENU ========================================
	
	  1 = I2C Config Menu
	  2 = I2C Destination Menu
	  3 = I2C Alert Menu
	  4 = I2C Dump Menu
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	  For example:'1'[ENTER]
	-----------------------------------------------------------------------------
	[Q = QUIT] Please enter your choice:


## I2C port configuration

Most settings are defaults.

One important setting to change is *d* field. Make sure it is > 0, otherwise
communication to certain chips might fail since the chip does not have time to
process input request and we get no response.

	=========================== I2C CONFIG MENU =================================
	
	  1 = Slave Addr        = 0
	  2 = Baudrate          = 100000
	  3 = Data Control      = P
	  4 = Data ready timeout= 10(*10ms)
	
	  5 = Flow Control      = N
	  6 = RTS Protocol      = 0
	
	  a = Emulation         = TCPSERVER
	  b = EmuCode           = 0000
	  c = BUS               = I2C
	  d = InputTimeOut*10ms = 10
	  e = Local Port        = 1002
	  f = With SSL/TLS      = N
	
	
	  STATE=HW ONLINE
	
	  RTS = LOW   CTS = LOW 
	
	  For example:'1=2'
	-----------------------------------------------------------------------------
	[Q = QUIT] Please enter your choice:



# Sample OPI

![Screenshot_2016-05-26_15-07-09.png](https://bitbucket.org/repo/zkaz5r/images/461379777-Screenshot_2016-05-26_15-07-09.png)