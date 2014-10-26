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

/*
  To learn more about the internals of the Jack MIDI interface, refer
  https://github.com/jackaudio/jack2/blob/master/example-clients/midiseq.c
 */

#include <jack/jack.h>
#include <jack/midiport.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <mqueue.h>

#include "midi_key.h"
#include "framing.h"

#define SERIAL_DEVICE   "/dev/ttyAMA0"
#define MQUEUE_NAME     "/Mozartqueue"
#define FRAME_HEADER    0xFA
#define FRAME_SIZE      4

static void play_test_pattern(mqd_t p_queue);
static void play_from_serial(int serialfd, mqd_t mq_fd);

midi_key_event C4_press = {60,0x90,64,0};
midi_key_event D4_press = {62,0x90,64,0};
midi_key_event C5_press = {72,0x90,64,0};
midi_key_event C4_release = {60,0x80,64,0};
midi_key_event D4_release = {62,0x80,64,0};
midi_key_event C5_release = {72,0x80,64,0};

char client_name[] = "mozart2";

int serialfd;
jack_client_t* pclient;
jack_port_t* pport;
mqd_t write_end;

/**
 * The Jack callback. Checks the interprocess message queue for data. If data available, push all the available data
 * to Jack.
 */
int process(jack_nframes_t nframes, void* arg)
{
  midi_key_event midi_event;
  int length;
  mqd_t read_end;
  void* port_buf;
  unsigned char* buffer;

  port_buf =  jack_port_get_buffer(pport, nframes);
  jack_midi_clear_buffer(port_buf);
  read_end = mq_open(MQUEUE_NAME,O_RDONLY|O_NONBLOCK);

  while((length = mq_receive(read_end,(char*)&midi_event,sizeof(midi_key_event),NULL))==sizeof(midi_key_event))
  {
    buffer = jack_midi_event_reserve(port_buf, midi_event.timestamp, 3);
    {
      buffer[2] = midi_event.velocity;    //velocity
      buffer[1] = midi_event.note;        //eg: C4
      buffer[0] = midi_event.keying;      //Keying: key down/up
    }
  }
  mq_close(read_end);
  return 0;
}

/**
 * Release all system resources.
 */
void cleanup(void)
{
  jack_deactivate(pclient);
  jack_client_close(pclient);
  close_serial(serialfd);
  mq_close(write_end);
  mq_unlink(MQUEUE_NAME);
}

void signal_handler(int sig)
{
  cleanup();
  printf("Caught signal...Exiting...\n");
  exit(0);
}

int main(void)
{
  struct mq_attr my_mq_attr =
  {
    0,                      /* flag */
    10,                     /* max # of messgaes in queue */
    sizeof(midi_key_event), /* message size */
    0                       /* # of messages currently in queue */
  };

  /* Initialise the message queue to send data to Jack process */
  write_end = mq_open(MQUEUE_NAME,O_WRONLY|O_CREAT, S_IRWXU|S_IRWXG|S_IROTH, &my_mq_attr);

  /* Open the serial port for reading */
  serialfd = init_serial(SERIAL_DEVICE);
  synchronise(serialfd, FRAME_HEADER, FRAME_SIZE);

  /* Create Jack ports and register callback */
  pclient = jack_client_open(client_name,JackNullOption,NULL);
  jack_set_process_callback(pclient,process,NULL);
  pport = jack_port_register(pclient,"MIDI",JACK_DEFAULT_MIDI_TYPE,JackPortIsOutput,0);

  jack_activate(pclient);

  /* install a signal handler to properly quit jack client */
#ifndef WIN32
  signal(SIGQUIT, signal_handler);
  signal(SIGHUP, signal_handler);
#endif
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);

  while(1){
    //play_test_pattern(write_end);
    play_from_serial(serialfd, write_end);
  }

  cleanup();
  return 0;
}

/**
 * Plays a predefined sound pattern.
 * Useful for testing the Jack connection.
 */
static void play_test_pattern(mqd_t mq_fd)
{
  printf("Playing test pattern...\n");
  mq_send(mq_fd, (char*)&C4_press, sizeof(midi_key_event), 1);
  sleep(2);
  mq_send(mq_fd, (char*)&D4_press, sizeof(midi_key_event), 1);
  sleep(1);
  mq_send(mq_fd, (char*)&C4_release, sizeof(midi_key_event), 1);
  sleep(3);
  mq_send(mq_fd, (char*)&D4_release, sizeof(midi_key_event), 1);
  mq_send(mq_fd, (char*)&C5_press, sizeof(midi_key_event), 1);
  sleep(5);
  mq_send(mq_fd, (char*)&C5_release, sizeof(midi_key_event), 1);
}

/**
 * Reads one frame from serial port and pushes it to the inter-process queue to Jack callback.
 */
static void play_from_serial(int serialfd, mqd_t mq_fd)
{
  midi_key_event event;
  unsigned char data[FRAME_SIZE];

  communicate(serialfd,data, FRAME_SIZE);
  if(data[0] == FRAME_HEADER && (data[1] ==0x80 || data[1] == 0x90)){
    printf("0x%x,0x%x,0x%x,0x%x\n",data[0],data[1],data[2],data[3]);
    event.keying = data[1];
    event.note = data[2];
    event.velocity = data[3];
    event.timestamp = 0;
    mq_send(mq_fd, (char*)&event, sizeof(midi_key_event), 1);
  }
  if(data[0] != FRAME_HEADER){
    printf("Lost Synchronisation. Resynchronising...\n");
    synchronise(serialfd, FRAME_HEADER, FRAME_SIZE);
  }
}
