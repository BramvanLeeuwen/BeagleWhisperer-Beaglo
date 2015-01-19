/*
 * TaskManager.cpp
 *    Created on: May 11, 2014
 * 		Author: Bram van Leeuwen
 */

#include "TaskManager.hpp"
#include "Tcp.hpp"
#include <sys/time.h>
#include <time.h>
#include <string>
//#include <climits>    //for constant LONG_MAX
#include <limits>

#include "GPIO.hpp"

//extern ADT7420_Tempsensor	temperature_sensor;
//extern GPIO 				LEDP83;
extern Comparators			comparators;
extern TcpClient			tcpclient;
extern pthread_mutex_t 	    adc_mutex,temperature_sensorADT7420_mutex,rotary_encoder_mutex,key_mutex;
extern bool					tcpserver_connected;  //true when tcp server socket is bound to external socket
extern ADCs		   			adcs;
extern GPIOs 				outports;
extern Inport               Pushbutton;
extern Inports              inports;
extern TaskSchedules        schedules;
//extern string host_tcpserver;
//extern Tcp					tcpserver;
//extern char*     			tcpserver_outgoing_message;

////**********************************************************************************************////
////***************************************   TASKMANAGER  ***************************************////
////**********************************************************************************************////

TaskManager::TaskManager()
{
 /*  int i;
   max_taskstringlength=98;
   for (i=0;i<nr_ad_converters;i++)     //Initially no Ad-converters readout to serial port
   {  ad[i].ad_nr=i;
      //adreadout[i]=false;
      //adreadout_ready[i]=false;
      //ad[i].amplificationOpAmp=1;
   }
   //float lim,int prt, float hyst,quantity_kind quant,limiter_kind dir
   ad[0].gauge_function=TempGauge_PT1000_4_20mA;  //Gauge for temperature sensor PT1000 Voltage converter
   ad[1].gauge_function=TempGauge_PT1000_4_20mA;
    //ad[4].gauge_function=TempGauge_KTY10_7;
    //ad[5].gauge_function=TempGauge_KTY81_222;

   //for (i=0;i<4;i++)
   //  ad[i].amplificationOpAmp=1;
	nr_taskcycles =0;
   time= new CTime(rtc_time);

   for(i=0;i<nr_outports;i++)
      out.Reset(i);

   OutportstatusToSerialPort();
   partner_alive=true;  //assumed to be connected; partner should give a sign of life within 5 seconds
   task_fulfilled=false;

   for (i=0; i< NR_PUSHBUTTONS; i++)
   {   pushbutton_debounce[i]=0;
   }
*/
}


void TaskManager::ManageTasks(void)
{   //temperature_sensor.readSensorState();
	//temperature_sensor.convertTemperature();
	//if (temperature_sensor.temperature()>22)
	//	LEDP83.high();
	//else
	//    LEDP83.low();

	//char    answer [TCP_MESSBUF_SIZE];
	int i;
	if (comparators.TakeAction() && tcpserver_connected)   //action taken
		tcpclient.SendOutportStatus();
	pthread_mutex_lock( &key_mutex);
	if (Pushbutton.action_to_take)
	{	printf("Button pressed\n\r");
        Pushbutton.action_to_take=false;
        Pushbutton.change_send_to_ftp=false;
    }
	pthread_mutex_unlock( &key_mutex);
	if (tcpserver_connected)
		for(i=0;i<inports.size();i++)
	    if (!(inports[i]->change_send_to_ftp))
	    {	tcpclient.SendInportChanged(i);
	    	inports[i]->change_send_to_ftp=true;
	    }
    schedules.Execute();


	/*
char   answerstr[20]	;
   int	 i;
   int lastsec=0,firsttask= schedules.first_task;
   nr_taskcycles++;

 // *****	 Sense    ***** //
   room_temp		= taskmanager.ad[0].Read();
   outside_temp	= taskmanager.ad[1].Read();
   if(priority_for_rotary_encoder) priority_for_rotary_encoder--;       //every cycle that rotaryencoder not touched deminish priority
      	// Sense pushbuttons
   for (i=0; i< NR_PUSHBUTTONS; i++)
   {  if  (pushbutton_debounce[i]==0 && ButtonPushed(i))   // button responsive
                //buttonpushed        //pushbuttons on header P2 pin 17 18 and 19
         {  RespondPushbutton(i);
         	pushbutton_debounce[i]= PUSHBUTTON_UNRESPONSIVE;   //Now button is unreponsive for PUSHBUTTON_UNRESPONSIVE taskcycles;
            seconds_screen_lighted= LIGHTNING_TIME_SCREEN;
         }
   }

 // *****	 Reason    ***** //
   ReadTaskCycleTime();
   int s1=time->sec1();
   if (s1 !=lastsec )  //time altered
   {   if (seconds_screen_lighted && s1==lastsec+1) seconds_screen_lighted--;
       if (send_clockpulses)
       {   /////
		 }
       lastsec= s1;
   }
   if (partner_alive)
   {
   }
	// *****	   Act()     ***** //
   HeartbeatOnLastPort();      //Heartbeat at port 20
   roomcomparator.state=(state_kind) dn_state;
   if (roomcomparator.TakeAction());  //if true :toggled
     if ( partner_alive && !priority_for_rotary_encoder)
     {   OutportStatusToTcp(answerstr);
         tcpclient.send(answerstr);
     }
   CTime RtcTime;
   if (schedules.size() &&   RtcTime > schedules[firsttask]->time_to_act())
   {   schedules[firsttask]->Execute();   //execute first task on list
       if (!schedules[firsttask]->repeating)
          schedules.Delete(firsttask);
       else
          schedules.SetFirstTask();
   }
   out.Set(OUTPORT_BOILER		,(bool)bl_state);           }
   */
}

void TaskManager::HeartbeatOnLastPort(void)
{ /*static int j=0 ;
  if (j++==10)
  { out.Toggle(nr_outports-1);    //op port 20 (21 op TD40 )
    j=0;
  }*/
}

void TaskManager::ReadTaskCycleTime(void)
{ /*static unsigned int ms_time_0=0, old_nr10kcycles=0,ms;
  unsigned int nr10kcycles;
  int tct;                       //measure one time in 10000 taskcycles
  if (nr_taskcycles%1000==0)
  {   nr10kcycles=nr_taskcycles/1000;
      if (nr10kcycles== ++old_nr10kcycles)   //next10kcycles
         ms= t0_rd();
         duty_cycle_time = (ms-ms_time_0)/1000;	//in ms
         //old_nr10kcycles++;
         ms_time_0=ms;
  }
      											// 0.1 us per counts, time=t+t0_int*65535;
      											 * */
   ;
}

/*int TaskManager::ReadOutportFromTcpBuffer()
{ //char *ts=sock_buf+3;
  //{  //if "Refresh"
   //  if (strncmp(ts,"R",1)==0) // "Refresh"
  //		   return 2;
   //  int portnr;
  //   portnr= (*(ts)-'0');      //max poortnr 32
 //    if (portnr<nr_outports )
//     {  if (*(++ts)=='0')
 //         out.Reset(portnr);
 //              if (*ts=='1')
 //         out.Set(portnr);
 //       return 1;
 //    }
 //    return -1;
 //  }*/
 //  int    dummy=1;
 //  return dummy;
//}*/



int TaskManager::TimeToTcpData_buf()    //returns length
{  /* TIM *timeptr;
    int i;
    timeptr=&rtc_time;
    for(i=0;i<13;i++)
       sock_buf[i]=(unsigned char)(timeptr++);
         //return new line  added by send*/
    return 13;
}


///**********************************************************////
////*******************   NAME AND ORDER    *****************////
////*********************************************************////
void NameAndOrderBase::SetName(string& namestring)
{  	char c;
    unsigned int i;
	for (i=0;i<namestring.length();i++)
	{  c= namestring[i];
	   name[i]=isalnum(c)? c: ' ';
	}
	for (;i<COMPARATOR_NAMELENGTH;i++)
	   name[i]=' ';
	name[COMPARATOR_NAMELENGTH]=0;
}



//// ********************************************************** ////
//// ****************   Comparator              *************** ////
//// ********************************************************** ////
using namespace taskmanager_namespace;

Comparator::Comparator()
{	Init((GPIO*)0,0,0);
	inputclass		    = notdefined_input;
	sensor_connected	= true;
}

Comparator::Comparator(GPIO *outport,float *input, float htrhtp, float ltrhtp, string& namestring)
{   Init(outport,htrhtp,ltrhtp);
	input_float		   	= input;
	inputclass		 	= variable_input;
	sensor_connected	=true;
	SetName(namestring);
}

Comparator::Comparator(GPIO *outport,myI2C *input, float htrhtp, float ltrhtp, string& namestring)
{   Init(outport,htrhtp,ltrhtp);
	input_I2C		   	= input;
	inputclass		   	= i2c_input;
	sensor_connected	= true;
	SetName(namestring);
}

/*Comparator::Comparator(GPIO *outport,ADT7420_Tempsensor *input, float htrhtp, float ltrhtp, char *namestring)
{   Init(outport,htrhtp,ltrhtp);
	input_tempsensor	= input;
	inputclass		   	= ADC7420_temperature_input;
	sensor_connected	= true;
	SetName(namestring);
}*/

Comparator::Comparator(GPIO *outport,string& namestring)
{   Init(outport,0,0);
	inputclass		   	= no_inputdependancy;
	sensor_connected	= false;
	SetName(namestring);
}

Comparator::Comparator(GPIO *outport,ADT7420_Tempsensor *input, float htrhtp, float ltrhtp, string& namestring)
{   Init(outport,htrhtp,ltrhtp);
	input_tempsensor	= input;
	inputclass		   	= ADC7420_temperature_input;
	sensor_connected	= true;
	SetName(namestring);
}


Comparator::Comparator(GPIO *outport,ADC *input,float htrhtp, float ltrhtp, string& namestring)
{   Init(outport,htrhtp,ltrhtp);
	input_Adc		    = input;
	inputclass			= adc_input;
	sensor_connected	= true;
	SetName(namestring);
}

void Comparator::Init(GPIO *outport,float htrhtp, float ltrhtp)
{	name[0] 			= 0;
	outputport			= outport;
	low_treshold_top	= ltrhtp;
	high_treshold_top	= htrhtp;
	state 				= single_treshold;
	hysteresis 			= 0.2;
	input_I2C		   	= 0;
	input_Adc		   	= 0;
	inputvalue		   	= 0;
	input_tempsensor	= 0;

}


/*void Comparator::SetName(string& namestring)
{  	char c;
    unsigned int i;
	for (i=0;i<namestring.length();i++)
	{  c= namestring[i];
	   name[i]=isalnum(c)? c: ' ';
	}
	for (;i<COMPARATOR_NAMELENGTH;i++)
	   name[i]=' ';
	name[COMPARATOR_NAMELENGTH]=0;
}*/

/*void Comparator::SetName(char* namestring)
{  	char c;
    int i;
    bool before_end=true;
	for (i=0;i<COMPARATOR_NAMELENGTH;i++)
	{  c= namestring[i];
	   if (c==0) before_end=false;
	   name[i]=(isalnum(c)&& before_end)? c: ' ';
	}
	name[COMPARATOR_NAMELENGTH]=0;
}*/

Comparator::~Comparator()
{
}

void Comparator::SetInportNr(unsigned int nr)
{	if (inputclass == adc_input || inputclass == adc_gauged_input)
	{	if (nr< (unsigned int)adcs.size())
			 input_Adc= adcs[nr];
		else
			input_Adc=(ADC*)NIL;
	}
}

 void Comparator::SetOutportNr(unsigned int nr)
 {  //unsigned int i=outports.size();
	 if (nr< (unsigned int)outports.size())
	 	 outputport = outports[nr];
 	 else
 		 outputport = (GPIO*)NIL;
 }


void Comparator::Tune(TaskSchedule *schedule) //Tune the comparator with the taskschedule property's
{   switch (schedule->command)     //boiler_high,boiler_low,room_high,room_low
    {  	case comparator_high_tr   :
    	   state=high_treshold;
    	   break;
        case comparator_low_tr:
           state=low_treshold;
           break;
        default:
           break;
    }
}


bool Comparator::TakeAction()     //returns true if outport is altered
{   float tresholdtop;
    //read inputvalue
    switch (inputclass)
	{	case no_inputdependancy:
			return false; //do nothing
		case variable_input:
			inputvalue		= *input_float;
			break;
		case ADC7420_temperature_input:
			if (sensor_connected)
			{	pthread_mutex_lock( &temperature_sensorADT7420_mutex);
				if (input_tempsensor->readSensorState())
					sensor_connected=false;  //failure
				input_tempsensor->convertTemperature();
				inputvalue=input_tempsensor->temperature();
				pthread_mutex_unlock( &temperature_sensorADT7420_mutex);
			}
			break;
		case i2c_input:
			//not implemented!!
			cout<<"comparator action not implemented!"<<endl;
			//	inputvalue	  	= input_I2C->Read();
			break;
		case adc_input:
			pthread_mutex_lock( &adc_mutex);
			inputvalue	= input_Adc->Read();
			pthread_mutex_unlock( &adc_mutex);
			break;
		default:
			cout<<"comparator inputclass error"<<endl;
			return true;
			break;
	}
    //compare and set output
	PIN_VALUE oldpinvalue= outputport->value();
    pthread_mutex_lock( &rotary_encoder_mutex);
    if (state<=high_treshold)
    tresholdtop=  (state==high_treshold || state==single_treshold)
    			    ?high_treshold_top:		low_treshold_top;
    pthread_mutex_unlock( &rotary_encoder_mutex);
    if 	(inputvalue < tresholdtop- hysteresis)
       outputport->value(HIGH);
    else if (inputvalue > tresholdtop)
       outputport->value(LOW);
    return outputport->value()!=oldpinvalue;
}


bool Comparators::TakeAction()   //returns true if one of the outputs is switched
{  	bool acted=false;
	for (int i=0; i<size();i++)
		if ((*this)[i]->TakeAction())
		  acted=true;
	if (acted) log_cl ("Comparator switched output\n\r");
    return acted;
}

//=======================================================//
//             Class Outport                             //
// GPIO dedicated as outport with attached comparator    //
//=======================================================//
Outport::Outport(): GPIO(){}

Outport::Outport(Header_name nm, unsigned char nr,PIN_DIRECTION	dirctn,char* namestring):
		GPIO(nm,nr,OUTPUT_PIN,namestring)
{   comparator =(Comparator*)NIL;
}

void Outport::SetComparator (Comparator* comp)
{	comparator=comp;
}


Outport::~Outport()
{	delete comparator;
   //destuctor GPIO is called
}

void Outports::Add(Outport *outport, Comparator *comparator)
{  ivector<Outport>::Add(outport);
   outport->SetComparator(comparator);
   outport->portnr=currentSize-1;
   comparator->outputport=outport;
   comparators.Add(comparator);
}

void Outports::Add(Outport& outport, Comparator *comparator)
{  ivector<Outport>::Add(&outport);
   outport.SetComparator(comparator);
   outport.portnr=currentSize-1;
   comparator->outputport=&outport;
   comparators.Add(comparator);
}


int  Outports::Nr(Outport* objptr)
{   //Outport *o1,*o2;
	for (unsigned int i=0; i<currentSize;i++)
	{  /* o1=objptr;
	    o2=(*objptrs)[i];
	   	if (o1==o2)
	   		return i;*/
		if (objptr==(*objptrs)[i])
			   		return i;
	}
    return -1;
}

/*Outport* &Outports::operator[](unsigned int index )
{   if (index<currentSize)
	   return((*objptrs)[index]);
    if (index!=99 ||index!=100)
	{    cout <<"Outport operator [] out of range: index= "<< index << " !"<<endl;
		exit(-1);
	}
    return objptrs->at(0) ;
};*/

Outports::~Outports()
{  //printf("flush outports:");
	comparators.Flush();
	//printf("flushed:");
}

//=======================================================//
//             Class Inport                             //
// GPIO dedicated as outport with attached comparator    //
//=======================================================//
//Convert Stability bits to a bit mask, i.e STABILITY_MASK has STABILITY_BITS bits set in its LSB

Inport::Inport(): GPIO(){}

#define MAJORITY_VOTE 5  //MAJORITY_VOTE Number of samples for which the button must remain in pressed state to consider that the button is pressed.
#define STABILITY_BITS 2  //STABILITY_BITS is the number of final samples (last few samples at the end of debounce)after which we consider the input stable

Inport::Inport(Header_name nm, unsigned char nr,char* namestring,bool b,buttontype tp):  //button=true: gebouncing = on
		GPIO(nm,nr,INPUT_PIN,namestring)
{   button=b;
    keytype= tp;
    debouncedKeyPress = false;
    change_send_to_ftp=false;
    coupled_comparator=(Comparator*)NIL;
}


//The Jack G. Ganssle debounce algorithm
#define CHECK_MSEC 5 // Read hardware every 5 msec
#define PRESS_MSEC 10 // Stable time before registering pressed
#define RELEASE_MSEC 50 // Stable time before registering released

// Service routine called every CHECK_MSEC to debounce both edges
void Inport::debounceSwitch(bool *key_changed, bool *key_ispressed)
{	static uint8_t count = RELEASE_MSEC / CHECK_MSEC;   //number of times to chek the key
    bool raw_value;
    *key_changed = false;
	*key_ispressed = debouncedKeyPress;
	raw_value= value();
	if  (raw_value== debouncedKeyPress)
	{	// Set the timer which allows a change from current state.
		if (debouncedKeyPress) count = RELEASE_MSEC / CHECK_MSEC;
		else count = PRESS_MSEC / CHECK_MSEC;
	}
	else // Key has changed - wait for new state to become stable.
	{   if (--count == 0)
		{	// Timer expired - accept the change.
			debouncedKeyPress = raw_value;
			*key_changed=true;
			*key_ispressed=debouncedKeyPress;
			// And reset the timer.
			if (debouncedKeyPress) count = RELEASE_MSEC / CHECK_MSEC;
			else count = PRESS_MSEC / CHECK_MSEC;
		}
	}
}



void Inport::ToggleSwitch()
{  switch_value=(switch_value==HIGH)?LOW:HIGH ;
   if (coupled_comparator && coupled_comparator->state != single_treshold)
	   coupled_comparator->state=LOW ?low_treshold:high_treshold;
   //if (switch_value==HIGH) printf("TogglHe");
   //else printf("ToggleLow");
}

void Inports::Add(Inport *inport)
{  ivector<Inport>::Add(inport);
   inport->portnr=currentSize-1;
}



/*Inport* &Inports::operator[](unsigned int index )
{   if (index<currentSize)
	   return((*objptrs)[index]);
    if (index!=99 ||index!=100)
	{    cout <<"Inport operator [] out of range: index= "<< index << " !"<<endl;
		exit(-1);
	}
    return objptrs->at(0) ;
};*/



//// ********************************************************** ////
//// ****************        TaskSchedule       *************** ////
//// ********************************************************** ////

//using namespace taskmanager_namespace;

TaskSchedule::TaskSchedule()
{   InitClass();

}
// new taskschedule acting on a single outport
/*TaskSchedule::TaskSchedule(string& schedulename, time_t time, repeat_kind rk, const bool *repeat_od, command_kind com, GPIO *port)
	//schedule ment to set an reset a specific  outport
{   //influence = act_on_outport;
	influence = act_on_comparator;
    repeating=rk;
    command=com;
    outport=port;
    comparator=	(Comparator*)NIL;
    timedate_to_act=time;
    SetName(schedulename);
	for (int i=0;i<7;i++) repeat_on_day[i]=repeat_od[i];
}*/

// new taskschedule acting on a single comparator


/*TaskSchedule::TaskSchedule(string& schedulename,time_t tim, repeat_kind rk)
	//schedule ment to set an reset a specific  outport
{   repeating=rk;
    //command=com;
    //comparator=comp;
    timedate_to_act=tim;
    SetName(schedulename);
	//for (int i=0;i<7;i++) repeat_on_day[i]=repeat_od[i];
}*/

/*TaskSchedule::TaskSchedule(string& schedulename,time_t time, repeat_kind rk, const bool *repeat_od, command_kind com, Comparator *comp)
	//schedule ment to set an reset a specific  outport
{   repeating=rk;
    command=com;
    comparator=comp;
    timedate_to_act=time;
    SetName(schedulename);
	for (int i=0;i<7;i++) repeat_on_day[i]=repeat_od[i];
}*/

TaskSchedule::TaskSchedule(repeat_kind rk, const bool *repeat_od, command_kind com, Comparator *comp,string& schedulename)
	//schedule ment to set an reset a specific  outport
{   InitClass();
	repeating  =(rk!= not_repeating);
    repeat_interval=rk;
    command    =com;
    comparator =comp;
    //timedate_to_act=time;
    SetName(schedulename);
	for (int i=0;i<7;i++) repeat_on_day[i]=repeat_od[i];
}

void TaskSchedule::InitClass()
{   timeval timenow;
	gettimeofday(&timenow,NULL);
	timedate_to_act=timenow.tv_sec;
	for (int i=0;i<7;i++)
		repeat_on_day[i]=false;
	repeating=false;
    command 	= comparator_disabld;
    command2	= comparator_disabld;     // H high Low D disabled
   	enabled		= false;
    comparator	= (Comparator*)NIL;
    comparator2	= (Comparator*)NIL;
    name[0] 	= 0;
    nr_secminticks=1;
}
/*TaskSchedule::TaskSchedule(char* schedulename, time_t time, repeat_kind rk, const bool *repeat_od, command_kind com, Comparator *comp)
	//schedule ment to set an reset a specific  outport
{   repeating=rk;
    command=com;
    comparator=comp;
    timedate_to_act=time;
    SetName(schedulename);
	for (int i=0;i<7;i++) repeat_on_day[i]=repeat_od[i];
}*/

/*void TaskSchedule::SetName(string& namestring)
{  	char c;
	unsigned int i;
	for (i=0;i<namestring.length();i++)
	{  c= namestring[i];
		   name[i]=isalnum(c)? c: ' ';
	}
	for (;i<SCHEDULE_NAMELENGTH;i++)
		   name[i]=' ';
	name[SCHEDULE_NAMELENGTH]=0;
}*/

/*void TaskSchedule::SetName(char* namestring)
{  	char c;
    int i;
    bool beforeend=true;
	for (i=0;i<SCHEDULE_NAMELENGTH;i++)
	{  c= namestring[i];
	   if (c==0) beforeend=false;
	   name[i]=(isalnum(c)&& beforeend)? c: ' ';
	}
	name[SCHEDULE_NAMELENGTH]=0;
}*/

/*
TaskSchedule::TaskSchedule(char *name, time_t time, repeat_kind rk, const bool *repeat_od,comparator_inputkind input_nr,
								comparatorstate_kind hl,  command_kind com):
								influence (infl), timedate_to_act (time), command (com)
{
	SetName(newname);
	for (int i=0;i<7;i++)
		repeat_on_day[i]= repeat_od[i];
	enabled=true;
}
*/

/*void TaskSchedule::SetName(char* namestring)
{  	char c;
	for (int i=0;i<SCHEDULE_NAMELENGTH;i++)
	{  c= namestring[i];
	   name[i]=isalnum(c)? c: ' ';
	}
	name[SCHEDULE_NAMELENGTH]=0;
}*/

void TaskSchedule::Execute()
{    //make changes in comparator or outport
	ExecuteCommand(command,comparator);
	log_cl("Task executed\n\r");
	if (comparator2)
	    ExecuteCommand(command2,comparator2);
    if (repeating)//on repeating==true set new date  else delete the task in Taskschedules
    	NextTask();

}

void TaskSchedule::ExecuteCommand(command_kind command,Comparator * comparator)
{   switch (command)
	{  case comparator_high_tr: comparator->state= high_treshold;
	     break;
       case comparator_low_tr :  comparator->state= low_treshold	;
         break;
       case comparator_toggle :
		 if (comparator->state == high_treshold) comparator->state = low_treshold ;
		 if (comparator->state == low_treshold ) comparator->state = high_treshold;
		 break;
       default: break;
	}
	if (comparator->inputclass ==no_inputdependancy)
	{  if ((command==comparator_high_tr && comparator->state== low_treshold) ||
			   (command==comparator_low_tr && comparator->state== high_treshold)  ||
			    command==comparator_toggle)
		   comparator->outputport->toggle();
	}
}

void TaskSchedule::NextTask()
{  struct tm tim1s,tim2s;
   struct tm *tim1,*tim2;
   tim1=&tim1s,tim2=&tim2s;
   int dlst1;   //dayligtsavingtime current task and next task
   if (repeating)
   switch (repeat_interval)
   {case repeating_weekly:
    case repeating_daily:
	   for (int i=0; i<7; i++)
	   {   tim1= localtime(&timedate_to_act);
	       timedate_to_act +=  24*3600;  //next day   xxxx  klopt niet bij zomer winter
	   	   time_t secs=timedate_to_act;
	   	   tim2= localtime(&secs);  //24 hours later
	   	   if (repeat_on_day[tim2->tm_wday] ||repeat_interval == repeating_daily)
	   	   {   dlst1=tim1->tm_isdst;
	   		   if (dlst1>=0)   //daylightsaving time correction
	   			   timedate_to_act+= (dlst1-tim2->tm_isdst)*3600;
                  	  	  	  	  	  //summer-> winter:+1 hour extra sleep
	   	   	   	 	 	 	 	 	  //winter->summer:-1
	   		   break;
	   	   }
	   }
	   break;
     case not_repeating:
    	 enabled=false;
    	 break;
     case repeating_yearly:
    	 tim1= localtime(&timedate_to_act);
    	 tim1->tm_year+=1;
    	 timedate_to_act=mktime(tim1);
    	 break;
     case repeating_sectick:
    	 timedate_to_act +=nr_secminticks;
    	 break;
     case repeating_mintick:
    	 timedate_to_act =+60*nr_secminticks;
    	 break;
    break;
   }
   return;
}

/*int weekday(time_t t)
{  struct tm *tim, time;
   tim=&time;
   tim= localtime(&t);
   int wd=tim->tm_wday;
   wd=(localtime(&t))->tm_wday;
   return tim->tm_wday;
}

int weekday1(time_t& t)
{  struct tm *tim, time;
   tim=&time;
   tim= localtime(&t);
   int wd=(&time)->tm_wday;
   wd=(localtime(&t))->tm_wday;
   return tim->tm_wday;
}*/



void TaskSchedule::NextTask(time_t time)
{  while (time > timedate_to_act)
    NextTask();
}


TaskSchedule::~TaskSchedule()
{

}
//// ********************************************************** ////
//// **************** Array of taskschedules    *************** ////
//// ********************************************************** ////

TaskSchedules::TaskSchedules()
{ 	objptrs= new TaskSchedule *[20];
  	currentSize=0;
    capac=20;
}
TaskSchedules::~TaskSchedules()
{  for(unsigned int i=0;i<currentSize;i++)
       delete objptrs[i];
  	delete objptrs;
  	currentSize=0;
}

void TaskSchedules::resize (int newCapacity)
{  TaskSchedule  	**newobjptrs,**oldobjptrs;
   newobjptrs= new TaskSchedule *[newCapacity];
  	for(unsigned int i=0;i<currentSize;i++)
   		newobjptrs[i]=objptrs[i];
   oldobjptrs =objptrs;
   objptrs=newobjptrs;
   delete oldobjptrs;
   capac=newCapacity;
}

bool TaskSchedules::Add(TaskSchedule* element) //not implemented:  //Adds only if name unique   if name == any of the old names: delete element and return false;
{  if (currentSize==capacity())
     resize(capacity()+10);
   objptrs[currentSize++]=element;
   //element->NextTask(rtc_time);
   SetFirstTask();
   return true;
}

void TaskSchedules::Delete(unsigned int i)
{   delete objptrs[i];
    Detach(i);

}

void TaskSchedules::Update(unsigned int i,TaskSchedule* updated_element)
{   TaskSchedule *element_to_delete;
    element_to_delete= objptrs[i];
    objptrs[i]=updated_element;
    SetFirstTask();
    delete element_to_delete;
}

void TaskSchedules::Detach(unsigned int i)
{   for (;i<currentSize;i++)
       objptrs[i]=objptrs[i+1];
    objptrs[currentSize--]=0;
    SetFirstTask();
}

void TaskSchedules::Interchange(unsigned int i,unsigned int j)
{   TaskSchedule *element;
    element=objptrs[i];
    objptrs[i]=objptrs[j];
    objptrs[j]=element;
    //printf("interchanged %i with %i\r\n",i,j);
    SetFirstTask();
}


void TaskSchedules::Flush()
{   for (int i=currentSize;i>0;)
      Delete(--i);
    first_task= -1;
}

//void TaskSchedules::NextTask(int i)					//Set schedule number i on next moment
//{   (*this)[i]->NextTask();
//}

void TaskSchedules::SetFirstTask()
{  //time_t 	first_exe_time= LONG_MAX;  //in other version of gcc defined in limits.h
   time_t   first_exe_time= std::numeric_limits<time_t>::max();
   first_task= -1;      //Number of first task -1 if no tasks
   for (unsigned int i=0; i<currentSize;i++)
     if  ( first_exe_time > (*this)[i]->timedate_to_act )
       	first_exe_time=  (*this)[first_task=i]->timedate_to_act;
}

void TaskSchedules::Execute()                   //Executes first task
{  TaskSchedule *schedule_to_execute;
	time_t  now, firsttimetoexecute=(*this)[first_task]->timedate_to_act ;
	time(&now);
   if (firsttimetoexecute < now)
   {
	   schedule_to_execute=(*this)[first_task];
	   schedule_to_execute->Execute();
       if (!schedule_to_execute->repeating)
    	   Delete(first_task);
   	   SetFirstTask();
   }
}
