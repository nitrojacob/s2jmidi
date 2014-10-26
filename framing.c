/*
  This file is part of s2jmidi project
  
  Copyright (C) 2014 Jacob Mathew
  
  s2jmidi is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  
*/

#include <sys/time.h>
#include <sys/select.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

/** Maximum number of non frame-header characters possible in a frame.
    eg: if frame_size if 4, SYNC_ATTEMPTS can be 4 or anything greater */
#define SYNC_ATTEMPTS    10

/**
 * Initialises the serial port
 * @arg device: The serial port to be opened eg: /dev/ttyS0
 * @return The filedescriptior for the serial port. To be used with subsequent
 *           calls to functions in this file/library
 */
int init_serial(char *device)
{
  int serialfd;
  struct termios config;

  serialfd=open(device,O_RDWR|O_NONBLOCK);
  config.c_iflag=IGNPAR;  //Ignore input parity errors
  config.c_oflag=0;
  config.c_cflag=CS8|CREAD|CLOCAL|B115200;     //Baudrate=115200, 8 bits, no parity, single stop bit, reception enabled, handshaking lines(DTR,DSR etc..) are not considered.
  config.c_lflag=0;
  if(tcsetattr(serialfd,TCSANOW,&config)==0)
    printf("Serial Port Opened\n");
  return serialfd;
}

/**
 * Closes a serial port opened using init_serial()
 */
void close_serial(int serialfd)
{
  close(serialfd);
}

/**
 * Reads specifies number of characters to data[]
 * User must ensure that the frame synchronisation is already done before
 * calling this funciton.
 */
unsigned char communicate(int serialfd,unsigned char data[],int length)
{
  struct timeval timeout;
  fd_set rfds;
  int i;

  FD_ZERO(&rfds);
  FD_SET(serialfd,&rfds);
  timeout.tv_sec=100;
  timeout.tv_usec=0;

  for(i=0;i<length;i++){
    do{
      select(serialfd+1,&rfds,NULL,NULL,&timeout);
    }while(read(serialfd,&(data[i]),1)!=1);
  }
  return 0;
}

/**
 * Synchronise the frame so that the next communicate() funciton will
 * read a full frame
 */
void synchronise(int serialfd, unsigned char frame_header, int frame_size)
{
  struct timeval timeout;
  unsigned char data = 0,attempt=0;
  fd_set rfds;
  int read_count;
  int i;

  do{
    attempt++;
    printf("Connection attempt %d\n",attempt);
    timeout.tv_sec=2;
    timeout.tv_usec=0;
    FD_ZERO(&rfds);
    FD_SET(serialfd,&rfds);
    select(serialfd+1,&rfds,NULL,NULL,&timeout);
    read_count = read(serialfd,&data,1);
  }while(attempt<SYNC_ATTEMPTS && (read_count<=0 || data != frame_header));

  if(attempt>=SYNC_ATTEMPTS)
    printf("Not Responding\n");
  else if(data==frame_header)
    printf("Synchronised\n");
  else
    printf("Unexpected response\n");

  /*Read frame_size-1 more characters to align frame */
  for(i=0; i<(frame_size-1); i++)
    while(read(serialfd,&data,1)!=1);
}
