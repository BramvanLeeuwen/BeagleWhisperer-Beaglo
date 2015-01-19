/*
 *  GlobalIncludes.hpp
 *
 *  Created on: Aug 30, 2013
 *      Author: Bram van Leeuwen
 */

#ifndef GLOBALINCLUDES_HPP_
#define GLOBALINCLUDES_HPP_



#define COMPARATOR_NAMELENGTH	20
#define SCHEDULE_NAMELENGTH 	20
#define NR_OUTPORTS				 4
#define NR_ADCS					 7

#define NIL						(void*)0

#include <cstdlib>
#include <iostream>


#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>

#include "Vectorclasses.hpp"
#include "ADT7420 Tempsensor.hpp"

using namespace std;

//#define CONTROLLER_NAME  		"BeagleBone Black\r\n"
                            //now in ini file
#define	DEVICE_OVERLAY	 	 	"DM-GPIO-Bram"   //now in ini file

int weekday(time_t& t);
int bitsCount(unsigned x);
int delay_ms(unsigned int msec);
void log_cl(char* str);               //log to commandline
extern char log_string[];             //log to commandline

bool ReadIniString(string& key,string& rest, ifstream& inputstream);
int  ReadIniInt(string& key, ifstream& inputstream);
# define CONTROLLER_NAME="Beglebone Black/n/r"


#endif /* GLOBALINCLUDES_HPP_ */
