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

#ifndef __FRAMING_H_
#define __FRAMING_H_

int init_serial(char *device);
void close_serial(int serialfd);
unsigned char communicate(int serialfd,unsigned char data[],int length);
void synchronise(int serialfd, unsigned char frame_header, int frame_size);

#endif  /* __FRAMING_H_ */
