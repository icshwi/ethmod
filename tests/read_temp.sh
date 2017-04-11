#/bin/bash

_pwd=$(pwd)
cd $HOME/epics && source source-me
cd $_pwd

while [ 1 ]
do
	for i in $(seq 1 7)
	do
		caput ETHMOD:I2C1:Temp${i}:Read 1
		caget -a ETHMOD:I2C1:Temp${i}:Value_RBV
		caget -aS ETHMOD:I2C1:Temp${i}:StatusMessage_RBV
	
	done
	sleep 1
done
