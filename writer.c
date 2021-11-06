#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define SET 0x03
#define UA 0x07
#define FLAG 0x7E
#define A 0x03
int Alarm = FALSE;
int alarmes =0;
volatile int STOP = FALSE;
bool leucontrolo = FALSE;
struct termios oldtio, newtio;

void alarmHandler()
{
  printf("Alarm=%d\n", alarmes + 1);
  Alarm = TRUE;
  alarmes++;
}


int main(int argc, char **argv)
{
  int fd, res;
  char buf[255];

  if ((argc < 2) ||
      ((strcmp("/dev/ttyS10", argv[1]) != 0) &&
       (strcmp("/dev/ttyS11", argv[1]) != 0)))
  {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0)
  {
    perror(argv[1]);
    exit(-1);
  }
(void)signal(SIGALRM, alarmHandler);
 llopen(fd);
 close(fd);
}
void sendControl(int fd, unsigned char control)
{
unsigned char packet[5];
packet[0]=FLAG;
packet[1]=A;
packet[2]=control;
packet[3]= A^control;
packet[4]=FLAG;
write(fd,packet,5);
}

int readControl(int state, unsigned char leitura, unsigned char control)
{
switch (state)
{

case 0:
if(leitura==FLAG)
{
state=1;
}
break;

case 1:
if(leitura==A)
{state=2;}
else
{
if( leitura == FLAG)
{state=1;}
else
{state=0;}
}
break;


case 2:
if (leitura == control)
{state=3;}
else
{
if( leitura == FLAG)
{state=1;}
else
{state=0;}
}
break;

case 3:
if(leitura == (A^control))
{state=4;}
else
{
if( leitura == FLAG)
{state=1;}
else
{state=0;}
}
break;

case 4:
if (leitura == FLAG)
{leucontrolo = TRUE;}
else
{state=0;}
break;
}

return 1;
}

int llopen(int fd)
{
if (tcgetattr(fd, &oldtio) == -1)
  { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure asdasdsadsadsadsadsadsadset\n");
  unsigned char leitura;
  int state =0;
  printf("entrar no envio");
  do
  {
  printf("a tentar ler");
  sendControl(fd,SET);
  alarm(3);
  Alarm = FALSE;
  while(!leucontrolo && !Alarm)
  {
  read(fd,&leitura,1);
  readControl(&state,&leitura,UA);
  }
 }while(alarmes<3 && !leucontrolo);
 if(alarmes ==3)
 {return FALSE;}
 
 return TRUE; 
}
