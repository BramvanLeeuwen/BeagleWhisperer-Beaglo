//============================================================================
// Name        : Hellloworldtest.cpp
// Author      :  Bram van Leeuwen
// Version     :
// Copyright   : Bram van Leeuwen
// Description : testing various functions
//============================================================================


#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <pthread.h>

#include "TCPTest.h"

#include "GlobalIncludes.hpp"
#include "leds.hpp"
#include "kbdio.hpp"
#include "GPIO.hpp"
//#include "RotaryEncoder.hpp"
#include "BMA180Accelerometer.hpp"
#include "ADT7420 Tempsensor.hpp"
#include "MyI2C.hpp"
#include "TaskManager.hpp"
#include "Tcp.hpp"

static void toggle(BeagleBone::led *ledio);
static void make_blink(BeagleBone::led *ledio);

#define PORT 		0x0FA0
                             //TCP server is listening  on port 4000 (decimal)
#define DIRSIZE 	8192

#include <pthread.h>
#include <sys/time.h>
#include <iostream>
#include <unistd.h>    ///only? necessary for function usleep
#include <fstream>


using namespace std;
using namespace gpio_namespace;

#define NUM_THREADS     5

struct thread_data{
   int   thread_id;
   char  *message;
   float *floatingpoint;
};

string              controller_name,device_overlay;
RotaryEncoder  		*rotary_encoder=NULL;                   //enables global access of rotary encoder value
Outport				LEDP83(P9,12,OUTPUT_PIN,(char*)"Led op breadbrd");
Outport				Boiler(P9,13,OUTPUT_PIN,(char*)"Boiler");
Outport				O3(P9,14,OUTPUT_PIN,(char*)"Output3");
Inport              Pushbutton(P9,24,(char*)"PushButton",true);    //debounced button

ADT7420_Tempsensor 	temperature_sensor(1, 0x49) ;
Comparators			comparators;
TaskSchedules		schedules;
TaskManager 		taskmanager;
TcpClient			tcpclient;

char 				tcpserver_outgoing_message2[1024];
char                log_string[80];

pthread_mutex_t 	adc_mutex,temperature_sensorADT7420_mutex,rotary_encoder_mutex,key_mutex,clienttcp_mutex;
ADCs   				adcs;

bool 		 		keepThreadsRunning,logging=false;
unsigned int 		nrRunningThreads;
int                 tcpserver_internal_portnr,client_tcpsocket,tcp_ip_port;
sockaddr			pin;
bool				tcpserver_connected;  //true when tcp socket is bound to external socket

//string host_tcpserver= "192.168.2.1";
extern char slotsfilename[];//= SLOTS_FILE;  //  "/sys/devices/bone_capemgr.9/slots";

Outports			outports;//does not own the outports!
Inports             inports;//does not own !

void   PrintHelp();
void   ReadIniFile();

void *PrintHello(void *threadarg)
{
  struct thread_data *my_data;

  my_data = (struct thread_data *) threadarg;
  sprintf(log_string, "Thread ID : %i  Message : %s", my_data->thread_id ,my_data->message);
  log_cl(log_string);
  nrRunningThreads--;
  pthread_exit(NULL);
  return NULL;
}

void *SendAnalogThread(void *threadarg)
{ // sends analog values every 0,5 s to the tcp peer (client) computer
 //struct thread_data *my_data;
  struct timeval tim1,tim2;
  int dt;

  //my_data = (struct thread_data *) threadarg;
  log_cl( "Analog thread is started up.");
  while (keepThreadsRunning) {
	if (tcpserver_connected)
	{	gettimeofday(&tim1, NULL);
	 	tcpclient.SendAnalog();
	 	gettimeofday(&tim2, NULL);
	 	dt=(tim2.tv_sec-tim1.tv_sec)*1000000+tim2.tv_usec-tim1.tv_usec; //time needed to axexute this task
	 	//printf("dt= %i\n\r",dt);
	 	usleep(500000-dt);  // cycle time 0,5 s = 500000 us
	}
  }
  nrRunningThreads--;
  pthread_exit(NULL);
  return NULL;
}


void *RotaryEncoderThread(void *threadarg)
{ struct thread_data *my_data;
  my_data = (struct thread_data *) threadarg;
  RotaryEncoder rotaryencoder(P8,15,17);
  rotary_encoder = &rotaryencoder;
  log_cl( "Rotary encoder thread");
  rotaryencoder.CoupleFloat(my_data->floatingpoint,0.1);
  while (rotaryencoder.value()<20 && keepThreadsRunning)
  {		rotaryencoder.RecordEncoderRotation();
  		sprintf(log_string,"   Value: %i\n\r",rotaryencoder.value());
  		log_cl(log_string);
  }
  printf("\n\r");
  nrRunningThreads--;
  log_cl("Rotaryencoderthread is closed");
  pthread_exit(NULL);
  return NULL;
}

void *TaskManagerThread(void *threadarg)
{ while (keepThreadsRunning)
	  taskmanager.ManageTasks();
  nrRunningThreads--;
  pthread_exit(NULL);
  return NULL;
}

void *PushButtonThread(void *threadarg)
{   bool key_changed,key_ispressed;
    struct timespec t;
    t.tv_nsec=(long)5E6;   //5 ms sleeptime
    t.tv_sec=0;
    while (keepThreadsRunning)
    {   Pushbutton.debounceSwitch(&key_changed,&key_ispressed);
        if (key_changed)
        {  pthread_mutex_lock( &key_mutex);
           Pushbutton.debouncedKeyPress= key_ispressed;
           Pushbutton.action_to_take=((Pushbutton.Keytype()==taskmanager_namespace::pressbutton) && key_ispressed==LOW)?false:key_changed;
           if (Pushbutton.action_to_take)
        	   Pushbutton.ToggleSwitch();
           pthread_mutex_unlock( &key_mutex);
        }
        nanosleep(&t, NULL);//delay_ms(5);  //check every 5 ms
    }
    nrRunningThreads--;
    pthread_exit(NULL);
    return NULL;
}
//ivector<int> ints;

int main (int nrarg,char *arg[])
{   pthread_t threads[NUM_THREADS];
    string strg;
    string overlay;

	Comparator *roomcomparator,*boilercomparator,*comp3;
	unsigned char reg;
	char 	*dt, str[20];//,str2[30];
	int 	rc, i, nr, adc_nr,taskmanager_thread_nr;

	for (i=1;i<nrarg;i++)
	{	 if (strcmp(arg[i],"-l")==0)
		       logging =true;
	}
	adapt_slots_file_name_to_distro();
	ReadIniFile();
	//strg="Kamertemperatuurregeling";
	roomcomparator  = new Comparator(&LEDP83,&temperature_sensor, 20, 20,strg="Kamerthermostaat");
	boilercomparator= new Comparator(&Boiler,&temperature_sensor, 70, 80,strg="Boilerthermostaat");
	comp3= new Comparator(&O3,&temperature_sensor, 20, 20,strg="Thermostaat3");
//	b2=new Comparator(&Boiler,&temperature_sensor, 70, 80,strg="Boilerthermosteet");
	outports.Add(LEDP83,roomcomparator);
	outports.Add(Boiler,boilercomparator);
	outports.Add(O3,comp3);
	inports.Add(&Pushbutton);
//	outports.Add(Boiler,b2);
	boilercomparator->state=low_treshold;
	roomcomparator->state=low_treshold;
	comp3->state=low_treshold;
	tcpserver_internal_portnr=-1;   //notdefined yet serverthread sets portnummer on creating the internal tcp/ip port
	tcpserver_connected=false;
	int     ax,ay,az;  //acceleration values from BMA180
	time_t 	now ;
	timeval now_tv;
	//char device_overlay[]="DM-GPIO-Bram";
	char device_overlay2[]= "BL-GPIO-I2CT";
	TaskSchedule *tsksched;
	string schstr("Nieuwe taak");
	//tsksched = new TaskSchedule(schstr,now, repeat_daily);
	//tsksched->SetName(schstr);
	//schedules.Add(tsksched);
	bool dayss[7]={true,false,true,false,true,false,true};
	tsksched = new TaskSchedule(repeating_weekly,  dayss, comparator_high_tr, roomcomparator,strg="Eerste taak");
	schedules.Add(tsksched);
	dayss[0]=false; dayss[1]=true;
	tsksched = new TaskSchedule(repeating_weekly,  dayss, comparator_low_tr, boilercomparator,strg="Tweede taak");
	schedules.Add(tsksched);

	//printf("%s\n\r",tsksched->Name());
	bool duplo;
    {	string overlay=device_overlay;
        export_overlay(overlay);
    }
    string mess;
    string address;
    //unsigned int port;

    /*   int *i1,*i2,*i3,*i4,*i5;
       //sivector<int> ints(100);

       overlay_was_echoed(device_overlay2);
       i1=new int(123);
       i2=new int(5);
       i3=new int(7);
       i4=new int(1);
       i5=new int(3);
       cout <<"Add";
       ints.Add(i1);
       cout <<"1";
       ints.Add(i2);
       cout <<"2";
       ints.Add(i3);
       ints.Add(i4);
       ints.Add(i5);
       cout <<"5"<<endl;
       ints.Show();
       ints.sort();
       ints.Show();

       outports.Add(&LEDP83);*/

		// current date/time based on current system
		/*time_t now = time(0);  //number of seconds since 1 jan 1970 ??


		   // convert now to string form
		   char* dt = ctime(&now);

		   printf(  "The local date and time is: %s\n\r",dt) ;

		   // convert now to tm struct for UTC
		   tm *gmtm = gmtime(&now);
		   dt = asctime(gmtm);
		   printf(  "The UTC date and time is:%s\n\r",dt) ;


		   const struct timezone *tzp;
		   struct timeval now1;
		   int rc;

		   now1.tv_sec=1066208142;
		   now1.tv_usec=290944;

		    rc=settimeofday(&now1, NULL); //DST_WET //WesternEuropeanTimeZone
		       if(rc==0) {
		           printf("settimeofday() successful.\n");
		       }
		       else {
		           printf("settimeofday() failed, ");
		           return -1;
		       }

		       dt = ctime(&now);

		       printf(  "The local date and time is: %s\n\r",dt) ;

	*/
		   //Planning: On the first message of the client time is send, answer by my name (name of host)
		       	   //timeformat yy/mm/dd hh:mm:ss w Day:0041520  days are  since 1/1/1900 00:00:00
		       	   // convert now to tm struct for UTC

		   			//   now,tzp);

           for (i=0;i<NR_ADCS;i++)    //form array of analog digital converters
        	   adcs.Add(new ADC(i));
		   struct thread_data 	td[NUM_THREADS];
		   BMA180Accelerometer 	accelerometer(1, 0x40);
           myI2C 				tempsensor2(1, 0x49);
           keepThreadsRunning= true;
           nrRunningThreads  = 0;
		   for( i=0; i < NUM_THREADS; i++ )
		   {  //printf("main() : creating thread,%i \n\r " ,i);
		     td[i].thread_id = i;
		     td[i].message   =(char*) "Started up";
		     nrRunningThreads++;
		     switch (i)
		     {	case 0: rc = pthread_create(&threads[i], NULL,	TaskManagerThread,	(void *)&td[i]);
		      	  	  	taskmanager_thread_nr=i;
		        	    break;
		        case 1: td[1].floatingpoint= &(roomcomparator->high_treshold_top);
		        	    rc = pthread_create(&threads[i], NULL,	RotaryEncoderThread,(void *)&td[i]);
		                //wait with coupling of floats until rotarybthread is constucted fully
		        	    break;
		        case 2: rc = pthread_create(&threads[i], NULL,	TcpServerThread,	(void *)&td[i]);
		                break;
		        case 3: rc = pthread_create(&threads[i], NULL,  SendAnalogThread,	(void *)&td[i]);
		        		break;
		        case 4: rc=  pthread_create(&threads[i], NULL,  PushButtonThread,	(void *)&td[i]);
		                break;
		        default:rc = pthread_create(&threads[i], NULL,  PrintHello,			(void *)&td[i]);
		        		break;
		      }
		      if (rc){
		    	 printf("main() : Error:unable to create thread,%i \n\r", rc );
		         exit(-1);
		      }
		   }
      BeagleBone::led *ledio[4];
	  char c;

	  for (int i = 0; i < 4; i++) {
	    ledio[i] = BeagleBone::led::get(i);
	    ledio[i]->off();
	  }
	  //dit werkt op  pin 12 en 14 van P9,
	  //ook op pin 13 en 19 van P8
	  //niet op 3 van P8
	  /*BeagleBone::gpio* Gpio[30];
	  for (i=0;i<1;i++)
	  {   Gpio[i]=BeagleBone::gpio::P9(12+i);
	  	  Gpio[i]->configure(BeagleBone::pin::OUT);
	  }*/
	  //GPIO LEDP83(P9,12,OUTPUT_PIN);
	  // GPIO SCLOCK (P9,BBB_GPIO_SHT1x_SCK,OUTPUT_PIN);   //defined in module SHT75.cpp
	  	  // GPIO DATA   (P9,BBB_GPIO_SHT1x_DATA,OUTPUT_PIN);  //defined in module SHT75.cpp

	  	  //LEDP83->configure(BeagleBone::pin::OUT);
	  //try 			{rotary_encoder->CoupleFloat(&(roomcomparator->high_treshold_top),0.1);}
	  //catch(...) 	{perror("Totary thread not fully constructed");}
	  log_cl("Coupled");
      printf ("     Hit 'h' or '?' to get help. 'q' to quit\n");

	  while ((c = BeagleBone::kbdio::getch()) != 'q') {
	  	 switch (c) {
	  	 case '1':
	     case '2':
	     case '3':
	     case '4':
	  	          toggle(ledio[c - '1']);
	  	          break;
	     case '5':
	  	          LEDP83.high();
	  	          printf("Sets the led on P9 12\n\r");
	  	          break;
	     case '6':
	  	          LEDP83.low();
	  	          printf("Resets the led on P9 12\n\r");
	  	          break;
	  	 case 'B':
	  		   	accelerometer.setRange(PLUSMINUS_1_G);
	  			accelerometer.setBandwidth(BW_150HZ);

	  			printf( "The current bandwidth is: %i . \n\r" ,(int)accelerometer.getBandwidth());
	  			printf( "The current mode is: %i .  \n\r" ,(int)accelerometer.getRange());
	  			printf( "The current temperature is: %f ." ,(float)accelerometer.getTemperature());
	   			//cout << "The current temperature is: " << (float)accelerometer.getTemperature() << endl;
	  			accelerometer.readFullSensorState();
	  			ax = accelerometer.getAccelerationX();
	  			ay = accelerometer.getAccelerationY();
	  			az = accelerometer.getAccelerationZ();
	  			printf("Current Acceleration: ax= %i, ay= %i az= %i \n\r", ax , ay ,az );

	  	          break;

	  	 case '!':
	          make_blink(ledio[0]);
	          break;

	      case '@':
	          make_blink(ledio[1]);
	          break;

	      case '#':
	          make_blink(ledio[2]);
	          break;

	      case '$':
	          make_blink(ledio[3]);
	          break;
	      case 'a':
	    	  printf ("Which adc will we read? (a= all f=full set five times repeated 1x");
	    	  c = BeagleBone::kbdio::getch();
	    	  adc_nr= (int)(c-'0');
	    	  if (adc_nr>=0 && adc_nr<=7)
	    	  {   ADC adc(adc_nr);
	    		  printf ("how many times? (1=1 2=4 3=10)\n\r");
	    		  c = BeagleBone::kbdio::getch();
	    	      nr=0;
	    	      printf("reading: \n\r");
	    	      if (c=='1') nr=1;
	    	      if (c=='2') nr=4;
	    	      if (c=='3') nr=10;
	    	      for (i=0; i<nr; i++)
	    	      { printf("     ADC%i value: %f\n\r",adc_nr,adc.voltage_value());
	    	        usleep(50000);  //sleep 0.05 second == 50,000 microseconds
	    	      }
	    	  }
	    	  if (c=='f')
	    		for (int k=0;k<=1;k++){
	    		  for (i=0;i<=7;i++)  //full test
	    		  {   ADC adc(i);
	    		      printf("\n\r 2 ADC%i values: %f ",i,adc.voltage_value());
	    		  	  for (int j=0; j<5; j++)
	    		  	  {  printf("nr %i voltage=%f ",adc_nr,adc.voltage_value());
	    		  	     usleep(20000);  //sleep 0.02 second == 20,000 microseconds
	    		  	  }
	    			  if (i==7) printf("\n\r");
	    		  }
	    		}
	    	  if (c=='g')
	    	    for (int k=0;k<=5;k++)
	    		  {	  for (i=0;i<=1;i++)
	    		  	  {   ADC adc(i);
	    		  	      printf("\n\r 2 ADC%i values: %f ",i,adc.voltage_value());
	    		  	  	  for (int j=0; j<5; j++)
	    		  	  	  {  printf("adcnr: %i %f ",adc_nr,adc.voltage_value());
	    		  	  	     usleep(300000);  //sleep 0.3 second == 20,000 microseconds
	    		  	  	  }
	    		  		  if (i==1) printf("\n\r");
	    		  	  }
	    		  }
	    	  if (c=='a')
	    	   for (i=0;i<=7;i++)
    		  	 {   ADC adc(i);
    			  	 printf("     ADC%i value: %f\n\r",i,adc.voltage_value());
    		  	 }
	    	  break;
	      case 'A':
	    	    tcpclient.analog_to_send[0]=true;  //send temperature
	    	    tcpclient.AnalogToClientMessage();
	    	    tcpclient.Send();
	    	  break;
	      case 'c':
	    	  // ComparatorToTcpData_buf(tcpserver_outgoing_message2,0);
	    	    //printf(tcpserver_outgoing_message2);
	    	    break;
	      case 'C':
	    	    tcpclient.SendOutportStatus();
	    	  break;

	      case 'd':
	    	  get_distro(strg);
	    	  adapt_slots_file_name_to_distro();
	    	  cout<<" Linux distro:"<< strg<<";-- Slots file:"<< slotsfilename <<endl;

	    	  break;

	      case 'h':
	      case '?':
	    	  PrintHelp();
	    	  break;
	      case 'l':
	    	  logging=!logging;
	    	  if (logging)
	    		  log_cl("Logging to commandline switched on.");
	    	  break;
	      case 'x':
	    	 // tcpclient.Open(host_tcpserver,4016);
	    	 // tcpclient.SendOutportStatus();
	    	 //  tcpclient.Close();
	      	   //mess="Ada is lief";
	      	   //address="192.168.2.1";
	      	   //tcpclient.host_portnr=4016;
	      	   //tcpclient.host_ipaddress="192.168.2.1";
	      	   //port=4016;
	      	   //tcpclient.SendTCPClient(mess,address,port);
               //mess="weer testbericht";
               //tcpclient.Send(mess);
	      	   break;
	      case 'n':
	      	    schedules[0]->NextTask();
	      	    printf("Next task (first taskschedule).\n\r");
	      	    break;
	      case 'o':
	      	    export_overlay(overlay=device_overlay2);
                break;
	      case 'O':
                export_overlay(overlay=device_overlay);
	    	    printf("Overlay is written to Beaglebone.\n\r");
	    	    break;
	      //case 'q'
	      case 'r':
	    	  printf("Rotary encoder value: %i.\n\r",rotary_encoder->value());
	      	  break;


	      case 'T':
	      	  dt=str;
	      	  gettimeofday(&now_tv,NULL);
	      	  now= now_tv.tv_sec;
	      	  dt = ctime(&now);
	      	  printf(  "Local date and time: Day: %i %s",weekday(now),dt) ;
	      	  //time_to_tcpstring(str2, now);
	      	  //printf("Tcp: %s",str2);
	            break;
	      case 't':
	    	  temperature_sensor.readSensorState();
	    	  printf("%f \n\r",temperature_sensor.temperature());
	    	  temperature_sensor.temp_to_string();
	    	  printf(" %s degrees C\n\r",temperature_sensor.tempstring);
	          break;
	      case 'I':
	    	  printf ("Which register will we read? (1 t/m B supported)");
	    	  duplo =true;
	    	  c = BeagleBone::kbdio::getch();
	    	  reg= (int)(c-'0');
	    	  if (c>64)
	    	  { duplo =false;
	    	    reg= 10+(int)(c-'A');
	    	  }
	    	  if (duplo)
	    		  tempsensor2.Read_Multi_Byte(reg,12);
	    	  else
	    		  tempsensor2.Read_I2C_Byte(reg);
	    	  printf( "\n\r");
	    	  for (int i=0;i<12;i++)
	    		  	printf("%2x ",tempsensor2.I2C_RD_Buf[i]);
	    	  printf( "\n\r");
	          break;

	       case 'i':
               //int t=1;
               //printf("Bitscount %i %i %i %i %i %i\n\r",bitsCount(0),bitsCount(1),bitsCount(2),bitsCount(3),bitsCount(4),bitsCount(5));
   	    	  if (Pushbutton.value()==HIGH)
	      	    		  	printf("HIGH\n\r");
	      	    	 else printf( "LOW\n\r");
	      	    break;
	      case 'y':
	    	  if (gpio_exported(67))
	    		  printf("exported\n\r");
	    	  else
	    		  printf("not exported yet");
	    	  break;
	      }
	    }
	  if (tcpserver_connected)
		    tcpclient.Send("Quit");

	  sleep(1);  //wait one second
	  keepThreadsRunning=false;
	  BeagleBone::led::restore_defaults();
	  cout << "Joining threads with main: ";
	  pthread_join(threads[taskmanager_thread_nr],NULL);
	  cout<< "Taskmanager thread is joined.\r"<<endl;
	  return 1;
}

void toggle(BeagleBone::led *ledio)
{
  ledio->set_trigger(BeagleBone::led::NONE);
  if (ledio->is_on())
	  ledio->off();
  else
	  ledio->on();
}

void make_blink(BeagleBone::led *ledio)
{
  if (!ledio->is_on())
	  ledio->on();
  ledio->timer(250, 250);
}

int weekday(time_t& t)
{   return (localtime(&t))->tm_wday;
}

int bitsCount(unsigned x)
 {  int b;
	for (b = 0; x != 0; x >>= 1)
	 if (x & 01)
	     b++;
	return b;
 }

int delay_ms(unsigned int msec)
{
  int ret;
  struct timespec a;
  if (msec>999)
  {
    fprintf(stderr, "delay_ms error: delay value needs to be less than 999\n");
    msec=999;
  }
  a.tv_nsec=((long)(msec))*1E6;
  a.tv_sec=0;
  if ((ret = nanosleep(&a, NULL)) != 0)
  {
    fprintf(stderr, "delay_ms error: %s\n", strerror(errno));
  }
  return(0);
}
void log_cl(char* str)
{  if (logging)
	cout << str << endl;
}

void PrintHelp()
{  printf("**************************************************************\n");
   printf("Hit '1-4' switch LED on board. Hit SHIFT/1-4 to make it blink\n");
   printf("    'a#'  read adc-input number #\n");
   printf("    'B'   read acceleration and temperature  \n");
   printf("		      from I2C bus 1 with BMA180 accelerometer\n");
   printf("    'C/c' set reset clock pin on P9 pin 17\n");
   printf("    'D/d' get the name of the Linux distribution\n");
   printf("    'l'   toggle logging to command line\r\n");
   printf("    'o'   write 12c overlay 'O' for device overlay\n");
   printf("    'n'   next scheduled timepoint (first schedule)\n");
   printf("    'r'   read rotary value\n");
   printf("    't'   temperature from ADT7420 \n");
   printf("    'T'   time \n");
   printf("    'q'   quit.\n");

}

void ReadIniFile()
{	char buffer[MAX_BUF];
    bool found=true;
	snprintf(buffer, sizeof(buffer),"BeagleWhisperer.ini");
	string str2,str3;
	if (file_exists(buffer))
	{  ifstream inputstream(buffer);
	    string key;
	    //ReadIniString(key="IP_PEER",host_tcpserver, inputstream) ;
	   	ReadIniString(key="DEVICE_OVERLAY",device_overlay, inputstream) ;

	    ReadIniString(key="CONTROLLER_NAME",controller_name, inputstream);

	    tcp_ip_port =ReadIniInt(key="LISTEN_PORT",inputstream);
	   // printf("Listen port:%i",listenport);
	}
	else
		cout << "Ini file not found!" << endl;
}


bool ReadIniString(string& key, string& result, ifstream& inputstream)
{  size_t pos,l,eq_pos;
   string line;
   while(inputstream.good())
   {   pos=0;
	   getline(inputstream,line); // get line from file
	   line.erase(line.find_first_of(";"));   //comment, begins with semicolon
	   pos=line.find(key); // search
	   eq_pos=line.find_first_of('=');
	   if ((pos != string::npos ) && (eq_pos!=string::npos) ) // string::npos is returned if string is not found
	   {	l=line.find_first_of('"');
	        if(l==string::npos)  //quotation not found
	        	 line=line.substr(eq_pos+1,string::npos);
	        else
	        { line=line.substr(l+1,string::npos);
	          line.erase(line.find_first_of('"'));   //skip on next quotation
	        }
	        result=line.erase( line.find_last_not_of(' ')+1);  //skip trailing spaces

	        //cout<<"line:"<<line<<"**result:**"<<result<<"**\"" <<endl;
	        return true;
	   }
   }
   cout << "Key \" "<< key<< "\"not found not in ini file."<< endl;
   return false;
}

int  ReadIniInt(string& key, ifstream& inputstream)
{   string result;
    ReadIniString(key, result, inputstream);
    return atoi(result.c_str());
}





