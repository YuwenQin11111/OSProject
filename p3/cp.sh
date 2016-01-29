#!/bin/bash

LINUX_HOME="/home/hoover/u2/cli26/QDGL/linux-2.6.26.5"

if [[ $# -ne 1 ]]; then
	echo "USAGE: ./cp.sh to or ./cp.sh from"
	exit -1;
fi

case $1 in
	"to")
		cp doevent.c 		${LINUX_HOME}/cs2456/doevent.c
		cp doevent.h 		${LINUX_HOME}/include/linux/doevent.h
		cp main.c    		${LINUX_HOME}/init/main.c
		cp Makefile  		${LINUX_HOME}/cs2456/Makefile
		cp syscall_table_32.S 	${LINUX_HOME}/arch/x86/kernel/syscall_table_32.S
		cp unistd_32.h 		${LINUX_HOME}/include/asm-x86/unistd_32.h
		;;
	"from")
		cp ${LINUX_HOME}/cs2456/doevent.c ./
		cp ${LINUX_HOME}/include/linux/doevent.h ./
		cp ${LINUX_HOME}/init/main.c ./
		cp ${LINUX_HOME}/cs2456/Makefile ./
		cp ${LINUX_HOME}/arch/x86/kernel/syscall_table_32.S ./
		cp ${LINUX_HOME}/include/asm-x86/unistd_32.h ./
		;;

esac
