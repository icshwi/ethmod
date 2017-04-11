#/bin/bash

_pwd=$(pwd)
cd $HOME/epics && source source-me
cd $_pwd

while [ 1 ]
do
    caput ETHMOD:I2C1:VMon1:Trigger 1
    sleep 1
    caput ETHMOD:I2C1:VMon1:Read 1
    sleep 1
    caget ETHMOD:I2C1:VMon1:ValueV7_RBV ETHMOD:I2C1:VMon1:ValueV8_RBV
done
