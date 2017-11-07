

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<linux/i2c-dev.h>
#include<pthread.h>
#include<curl/curl.h>


#define D0 11 //Connect to IO0
#define D1 12 //Connect to IO1
#define D2 13 //Connect to IO2
#define D3 14 //Connect to IO3
#define STROBE 6 //Connect to IO3
#define RESET 0x0
#define PING 0x1
#define GET 0x2

//GLOBAL VARIABLES
int globalvar1;
int analogvalue;
char *timestamp;
int globalinput;


//Methods to communicate with the PIC
int pin(int a);
void writepin(int a, int value);
void jwritepin(int a, int value);
int readpin(int a);
int jreadpin(int a);
void rreset(int a);
void init(int gpio1, int gpio2);
void deinit(int gpio1, int gpio2);

void reset();
void ping();
void getdata();
void reserve();
void ack();

//Method to communicate with the I2C
void I2CINIT();
void I2CDEINIT();
unsigned char timecal(char val[2]);
void RTCWRITE(int file);
void RTCREAD(int file);

//Methods that are implemented by the Threads
void *user(void *var1);
void *com(void *var1);
void *HTTP();



void main()
{
printf("Enter global input\n");
scanf("%d",&globalinput);
//I2C
I2CINIT();
int file;
char buffer[256];
sprintf(buffer,"/dev/i2c-0");
file = open(buffer, O_RDWR);
int addr = 0x68;
if (ioctl(file, I2C_SLAVE, addr) < 0)
{
printf("Error - no such slave");
}
//End of I2C intialization

//thread 1 implementataion
pthread_t t1;
globalvar1 = 1;
while(1)
{
if(globalvar1 == 1)
{
globalvar1 = 0;
RTCREAD(file);
pthread_create(&t1, NULL, user, (void *)&file );
printf("Thread 1 created\n");

usleep(10000);
}
}
}

//Method for Thread 1
void *user(void *var1)
{
printf("-----USER-----\n");
int file = *(int *)var1;
char str;
pthread_t t2;
init(11,32);
init(12,28);
init(13,34);
init(14,16);
init(6,36);
printf("enter 0-reset, 1-ping, 2-get\n");
scanf("%s",&str);
int d;
if(str=='0')
{
int d = RESET;
printf("THREAD 2 created\n");
pthread_create(&t2, NULL, com,(void *)&d );

}
if(str=='1')
{
int d = PING;
printf("THREAD 2 created\n");
pthread_create(&t2, NULL, com,(void *)&d );

}
if(str=='2')
{	
int d = GET;
printf("THREAD 2 created\n");
pthread_create(&t2, NULL, com,(void *)&d );


}
printf("-----END OF THREAD 1-----\n");
pthread_exit(NULL);
}

//Method for Thread 2
//METHOD TO START THE TYPE OF COMMUNICATION
void *com(void *var1)
{
printf("#####COM#####\n");
int a = *(int *)var1;
pthread_t t3;
switch(a)
{
case 0x0:
printf("RESET\n");
//reserve();
reset();
ack();
printf("#####END OF THREAD 2#####\n");
globalvar1 = 1;
pthread_exit(NULL);
break;
case 0x1:
printf("PING\n");
//reserve();
ping();
ack();
printf("#####END OF THREAD 2#####\n");
globalvar1 = 1;
pthread_exit(NULL);
break;
case 0x2:
printf("GET\n");
//reserve();
getdata();
ack();
pthread_create(&t3, NULL, HTTP, NULL);
printf("Thread 3 created\n");
printf("#####END OF THREAD 2#####\n");
pthread_exit(NULL);
break;

}

}

//METHOD IMPLEMENTED BY THREAD 3
//METHOD TO COMMUNICATE WITH THE SERVER
void *HTTP()
{
printf("*****HTTP*****\n");
char buf[1024];
char *host = "ec2-54-152-121-129.compute-1.amazonaws.com";
int port = 8080;
int id = 11;
char *pass = "9nIFDLyRcd";
char *name = "THE_TEAM";
char *status = "OK";
sprintf(buf, "http://%s:%d/update?id=%d&password=%s&name=%s&data=%d&status=%s&timestamp=%s",host,port,id,pass,name,analogvalue,status,timestamp);

CURL *curl;
curl = curl_easy_init();
if(curl)
{
curl_easy_setopt(curl, CURLOPT_URL, buf);
curl_easy_perform(curl);
curl_easy_cleanup(curl);
}
globalvar1 = 1;
printf("*****END OF THREAD 3*****\n");
pthread_exit(NULL);
}



//METHOD TO FIND THE GPIO

int pin(int a)
{
if(a==11)
{
return(32);
}
if(a==12)
{
return(28);
}
if(a==13)
{
return(34);
}
if(a==14)
{
return(16);
}
if(a==6)
{
return(36);
}
}

//METHOD TO WRITE OUTPUT TO THE PIN

void writepin(int a, int value)
{
int gpio1 = a;
int gpio2 = pin(a);
//init(gpio1, gpio2);
int file;
char buffer[256];
sprintf(buffer,"/sys/class/gpio/gpio%d/direction",gpio1 );
file = open(buffer, O_WRONLY);
write(file, "out", 3);
close(file);
sprintf(buffer,"/sys/class/gpio/gpio%d/direction",gpio2 );
file = open(buffer, O_WRONLY);
write(file, "out", 3);
close(file);
sprintf(buffer,"/sys/class/gpio/gpio%d/value",gpio1);
file = open(buffer, O_WRONLY);
if(value == 0)
{
write(file, "0", 1);
}
else
{
write(file, "1", 1);
}
close(file);
//deinit(gpio1, gpio2);
}

//METHOD TO WRITE OUTPUT TO THE PIN
void jwritepin(int a, int value)
{
int gpio1 = a;
int file;
char buffer[256];
sprintf(buffer,"/sys/class/gpio/gpio%d/value",gpio1);
file = open(buffer, O_WRONLY);
if(value == 0)
{
write(file, "0", 1);
}
else
{
write(file, "1", 1);
}
close(file);
}

//METHOD TO READ THE INPUT ON THE PIN
int readpin(int a)
{
int gpio1 = a;
int gpio2 = pin(a);
//init(gpio1, gpio2);
int file;
char buffer[256];
sprintf(buffer,"/sys/class/gpio/gpio%d/direction",gpio1 );
file = open(buffer, O_WRONLY);
write(file, "in", 2);
close(file);
sprintf(buffer,"/sys/class/gpio/gpio%d/direction",gpio2 );
file = open(buffer, O_WRONLY);
write(file, "in", 2);
close(file);
unsigned char b;
sprintf(buffer, "/sys/class/gpio/gpio%d/value",gpio1);
file = open(buffer, O_RDONLY);
read(file, &b, 1);
close(file);
//deinit(gpio1, gpio2);
if(b==48)
{
return(0);
}
else
{
return(1);
}
}

//METHOD TO READ THE INPUT ON THE PIN
int jreadpin(int a)
{
int gpio1 = a;
int file;
char buffer[256];
unsigned char b;
sprintf(buffer, "/sys/class/gpio/gpio%d/value",gpio1);
file = open(buffer, O_RDONLY);
read(file, &b, 1);
close(file);
if(b==48)
{
return(0);
}
else
{
return(1);
}
}
//METHOD TO RESET THE PINS
void rreset(int a)
{
int gpio1 = a;
int gpio2 = pin(a);
int file;
char buffer[256];
unsigned char b;
sprintf(buffer,"/sys/class/gpio/gpio%d/direction",gpio1 );
file = open(buffer, O_WRONLY);
write(file, "out", 3);
close(file);
sprintf(buffer,"/sys/class/gpio/gpio%d/direction",gpio2 );
file = open(buffer, O_WRONLY);
write(file, "out", 3);
close(file);

sprintf(buffer,"/sys/class/gpio/gpio%d/direction",gpio1 );
file = open(buffer, O_WRONLY);
write(file, "in", 2);
close(file);
sprintf(buffer,"/sys/class/gpio/gpio%d/direction",gpio2 );
file = open(buffer, O_WRONLY);
write(file, "in", 2);
close(file);

}


//METHOD TO EXPORT THE GPIOS
void init(int gpio1, int gpio2)
{
int file;
char buffer[256];
file = open("/sys/class/gpio/export", O_WRONLY);
sprintf(buffer,"%d",gpio1);
write(file, buffer, strlen(buffer));
close(file);
file = open("/sys/class/gpio/export", O_WRONLY);
sprintf(buffer,"%d",gpio2);
write(file, buffer, strlen(buffer));
close(file);
}

//METHOD TO UNEXPORT THE GPIOS
void deinit(int gpio1, int gpio2)
{
int file;
char buffer[256];
file = open("/sys/class/gpio/unexport", O_WRONLY);
sprintf(buffer,"%d",gpio1);
write(file, buffer, strlen(buffer));
close(file);
file = open("/sys/class/gpio/unexport", O_WRONLY);
sprintf(buffer,"%d",gpio2);
write(file, buffer, strlen(buffer));
close(file);
}



//METHOD TO RESET THE PIC MICROCONTROLLER
void reset()
{
//printf("data to be written d0=d1=d2=d3=0\n");
writepin(STROBE,0);
writepin(D0,0);
writepin(D1,0);
writepin(D2,0);
writepin(D3,0);
//printf("Wait for reset\n");
//printf("data present %d,%d,%d,%d \n", jreadpin(D0),jreadpin(D1),jreadpin(D2),jreadpin(D3));
while(jreadpin(STROBE)==1);
//printf("strobe low reset");
while(jreadpin(D0)==1);//just changed
while(jreadpin(D1)==1);
while(jreadpin(D2)==1);
while(jreadpin(D3)==1);
//printf("reset is init\n");
jwritepin(STROBE,1);
while(jreadpin(STROBE)==0);
//printf("strobe high reset\n");
usleep(1000000);
jwritepin(STROBE,0);
while(jreadpin(STROBE)==1);
//printf("strobe low reset\n");
return;
}

//METHOD TO PING THE PIC MICROCONTROLLER
void ping()
{
writepin(STROBE,0);
//printf("strobe low ping\n");
writepin(D0,1);
writepin(D1,0);
writepin(D2,0);
writepin(D3,0);
//printf("Wait for ping\n");
//printf("data present ping %d,%d,%d,%d \n", jreadpin(D0),jreadpin(D1),jreadpin(D2),jreadpin(D3));
while(jreadpin(STROBE)==1);
while(jreadpin(D0)==0);//just changed
while(jreadpin(D1)==1);
while(jreadpin(D2)==1);
while(jreadpin(D3)==1);
//printf("ping is init\n");

jwritepin(STROBE,1);
while(jreadpin(STROBE)==0);
//printf("strobe high ping\n");
usleep(1000000);
jwritepin(STROBE,0);
while(jreadpin(STROBE)==1);
//printf("strobe low ping\n");
return;
}

//METHOD TO GET THE DATA FROM PIC MICROCONTROLLER
void getdata()
{
//printf("in get data\n");
writepin(STROBE,0);
writepin(D0,0);
writepin(D1,1);
writepin(D2,0);
writepin(D3,0);
//printf("Wait for data\n");
//printf("data present data %d,%d,%d,%d \n", jreadpin(D0),jreadpin(D1),jreadpin(D2),jreadpin(D3));
while(jreadpin(STROBE)==1);
//printf("strobe low getdata\n");
while(jreadpin(D0)==1);//just changed
while(jreadpin(D1)==0);
while(jreadpin(D2)==1);
while(jreadpin(D3)==1);
//printf("getdata is init\n");

jwritepin(STROBE,1);
while(jreadpin(STROBE)==0);
//printf("strobe high getdata\n");

usleep(1000000);
jwritepin(STROBE,0);
while(jreadpin(STROBE)==1);
//printf("strobe low GETDATA\n");
usleep(10000);
int d0,d1,d2,d3;
unsigned int value = 0;
int i;
d0 = readpin(D0);
d1 = readpin(D1);
d2 = readpin(D2);
d3 = readpin(D3);


//int just = readpin(STROBE);
for(i=0;i<3;i++)
{

value = value<<4;

jwritepin(STROBE, 1);
while(jreadpin(STROBE)==0);

//while(jreadpin(STROBE)^1);
usleep(1000000);
//while(jreadpin(STROBE))//just changed
int k;
d0=d1=d2=d3=0;
for(k= 0; k<globalinput;k++)
{
d0 = jreadpin(D0)+d0;
d1 = jreadpin(D1)+d1;
d2 = jreadpin(D2)+d2;
d3 = jreadpin(D3)+d3;
}
float store;
store = (float)d0/k;
if(store>=0.5)
{
d0 = 1;
}
else
{
d0 = 0;
}
store = (float)d1/k;
if(store>=0.5)
{
d1 = 1;
}
else
{
d1 = 0;
}
store = (float)d2/k;
if(store>=0.5)
{
d2 = 1;
}
else
{
d2 = 0;
}

store = (float)d3/k;
if(store>=0.5)
{
d3 = 1;
}
else
{
d3 = 0;
}

jwritepin(STROBE, 0);
while(jreadpin(STROBE)==1);
usleep(1000);
//printf("Data pins read\n");
if(d3==1)
{
value = value|(d3<<3);
//printf("D3 is HIGH\n");
}
if(d2==1)
{
value = value|(d2<<2);
//printf("D2 is HIGH\n");
}
if(d1==1)
{
value = value|(d1<<1);
//printf("D1 is HIGH\n");
}
if(d0==1)
{
value = value|(d0);
//printf("D0 is HIGH\n");
}
}
if(value>1023)
{
value = 1023;
}
analogvalue = value;
printf("ADC value is %d\n",value);
printf("TIME: %s\n",timestamp);
return;
}

//METHOD TO READ THE ACKNOWLEDGEMENT RECIEVED FROM THE PIC MICROCONTROLLER
void ack()
{
int d0=0,d1=0,d2=0,d3=0;
d0 = readpin(D0);
d1 = readpin(D1);
d2 = readpin(D2);
d3 = readpin(D3);
int just = readpin(STROBE);
//printf("waiting for strobe to become high in ack\n");
/*writepin(D0,0);
writepin(D1,0);
writepin(D2,0);
writepin(D3,0);
printf("data suppose to be 0b0000");
while(jreadpin(D0)==1);
while(jreadpin(D1)==1);
while(jreadpin(D2)==1);
while(jreadpin(D3)==1);*/
//printf("data present ack %d,%d,%d,%d \n", jreadpin(D0),jreadpin(D1),jreadpin(D2),jreadpin(D3));
while(jreadpin(STROBE)^1);
usleep(200000);
int i=0;
while(jreadpin(STROBE))//just changed
{
d0 = jreadpin(D0);
d1 = jreadpin(D1);
d2 = jreadpin(D2);
d3 = jreadpin(D3);
i++;
}

//printf("d0 i ack %d\n",d0);
//printf("d1 i ack %d\n",d1);
//printf("d2 i ack %d\n",d2);
//printf("d3 i ack %d\n",d3);

if(d0==0 && d1==1 && d2==1 &&  d3==1)
{
printf("MSG_ACK Recieved\n");
}
else
{
printf("MSG_ACK CORRUPTED\n");
}
return;
}

//METHOD TO CHECK WHETHER THE PIC MICROCONTROLLER IS RESERVED OR NOT
void reserve()
{
printf("Reserved\n");
while(readpin(D0)&&readpin(D1)&&readpin(D2)&&readpin(D3));

}

//Methods to communicate with I2C
//Method to initiate I2C
void I2CINIT()
{
int file;
char buffer[256];
file = open("/sys/class/gpio/export", O_WRONLY);
sprintf(buffer, "%d",57);
write(file, buffer, strlen(buffer));
close(file);
file = open("/sys/class/gpio/export", O_WRONLY);
sprintf(buffer, "%d",59);
write(file, buffer, strlen(buffer));
close(file);
file = open("/sys/class/gpio/export", O_WRONLY);
sprintf(buffer, "%d",60);
write(file, buffer, strlen(buffer));
close(file);
sprintf(buffer,"/sys/class/gpio/gpio%d/direction", 60 );
file = open(buffer, O_WRONLY);
write(file, "out", 3);
close(file);
sprintf(buffer,"/sys/class/gpio/gpio%d/direction", 57 );
file = open(buffer, O_WRONLY);
write(file, "out", 3);
close(file);
sprintf(buffer,"/sys/class/gpio/gpio%d/direction", 59 );
file = open(buffer, O_WRONLY);
write(file, "out", 3);
close(file);
sprintf(buffer,"/sys/class/gpio/gpio%d/value",60);
file = open(buffer, O_WRONLY);
write(file, "0", 1);
close(file);
sprintf(buffer,"/sys/class/gpio/gpio%d/value",57);
file = open(buffer, O_WRONLY);
write(file, "1", 1);
close(file);
sprintf(buffer,"/sys/class/gpio/gpio%d/value",58);
file = open(buffer, O_WRONLY);
write(file, "1", 1);
close(file);
}

//Method to deinitialize I2C
void I2CDEINIT()
{
int file;
char buffer[256];
file = open("/sys/class/gpio/unexport", O_WRONLY);
sprintf(buffer, "%d",57);
write(file, buffer, strlen(buffer));
close(file);
file = open("/sys/class/gpio/unexport", O_WRONLY);
sprintf(buffer, "%d",59);
write(file, buffer, strlen(buffer));
close(file);
file = open("/sys/class/gpio/unexport", O_WRONLY);
sprintf(buffer, "%d",60);
write(file, buffer, strlen(buffer));
close(file);
}

//Method for conversion of integer to BCD
unsigned char timecal(char val[2])
{
unsigned int temp;
unsigned int offset=48;
unsigned char b =0;
temp = val[0];
b = b|((temp-offset)<<4);
temp = val[1];
b = b|(temp-offset);
return b;
}

//Method to write to the RTC
void RTCWRITE(int file)
{
int value;
int temp;
char val[2];
unsigned char buf[9];
buf[0] =0x00;//register
buf[1] =0;//seconds
buf[2] =0;//minutes
buf[3] =0;//hours
buf[4] =0;//day
buf[5] =0;//date
buf[6] =0;//month
buf[7] =0;//year
buf[8] =0x00;//square wave
//Enter the year
printf("Enter years between 0-99 in 00 format\n");
scanf("%s",&val);
buf[7] = timecal(val);
printf("%d\n",buf[7]);
//Enter the month
printf("Enter month between 0-12 in 00 format\n");
scanf("%s",&val);
buf[6] = timecal(val);
printf("%d\n",buf[6]);
//Enter the date
printf("Enter date between 1-30 in 00 format\n");
scanf("%s",&val);
buf[5] = timecal(val);
printf("%d\n",buf[5]);
//Enter the day
printf("Enter days between 1-Sunday to 7-Monday in 00 format\n");
scanf("%s",&val);
buf[4] = timecal(val);
printf("%d\n",buf[4]);
//Enter the hours
printf("Enter hours between 0-23 in 00 format\n");
scanf("%s",&val);
buf[3] = timecal(val);
printf("%d\n",buf[3]);
//Enter the minutes
printf("Enter minutes between 0-59 in 00 format\n");
scanf("%s",&val);
buf[2] = timecal(val);
printf("%d\n",buf[2]);
//Enter the seconds
printf("Enter seconds between 0-59 in 00 format\n");
scanf("%s",&val);
buf[1] = timecal(val);
buf[1] = buf[1]&0x7F;
printf("%d\n",buf[1]);
//Write the registers
if(write(file, buf, 9) != 9)
{
printf("Error writing the bytes");
}
}

//Method to read from the RTC
void RTCREAD(int file)
{
char reg =0x00;
char data = 0;
char buf[1024];
unsigned char val[2];
unsigned char val1[2];
unsigned char val2[2];
unsigned char val3[2];
unsigned char val4[2];
unsigned char val5[2];
unsigned char val6[2];
int temp;
//Read the Year
reg =0x06;
write(file, &reg, 1);
read(file, &data, 1);
val1[0] = (0xF0&data)>>4;
val1[1] = (0x0F&data);
//printf("Year:%d%d\n",val1[0],val1[1]);
//Read the Month
reg =0x05;
write(file, &reg, 1);
read(file, &data, 1);
val3[0] = (0xF0&data)>>4;
val3[1] = (0x0F&data);
//printf("Month:%d%d\n",val3[0],val3[1]);
//Read the Date
reg =0x04;
write(file, &reg, 1);
read(file, &data, 1);
val2[0] = (0xF0&data)>>4;
val2[1] = (0x0F&data);
//printf("Date:%d%d\n",val2[0],val2[1]);
//Read the Day
reg =0x03;
write(file, &reg, 1);
read(file, &data, 1);
val[0] = (0xF0&data)>>4;
val[1] = (0x0F&data);
//printf("Day:%d%d\n",val[0],val[1]);
//Read the Hour
reg =0x02;
write(file, &reg, 1);
read(file, &data, 1);
val4[0] = (0xF0&data)>>4;
val4[1] = (0x0F&data);
//printf("Hour:%d%d\n",val4[0],val4[1]);
//Read the Minute
reg =0x01;
write(file, &reg, 1);
read(file, &data, 1);
val5[0] = (0xF0&data)>>4;
val5[1] = (0x0F&data);
//printf("Minute:%d%d\n",val5[0],val5[1]);
//Read the Second
reg =0x00;
write(file, &reg, 1);
read(file, &data, 1);
val6[0] = (0xF0&data)>>4;
val6[1] = (0x0F&data);
//printf("Second:%d%d\n",val6[0],val6[1]);
sprintf(buf,"%d%d:%d%d:%d%d-%d%d/%d%d/%d%d",val4[0],val4[1],val5[0],val5[1],val6[0],val6[1],val3[0],val3[1],val2[0],val2[1],val1[0],val1[1]);
timestamp = buf;
}


