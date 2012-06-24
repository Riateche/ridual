#!/bin/bash
rm gmon.out
./ridual
FILENAME=report_`date +%Y-%m-%d--%H-%M-%S`.txt
#echo filename: $FILENAME
gprof ridual > $FILENAME
gedit $FILENAME &


