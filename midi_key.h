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

#ifndef __MIDI_KEY_H__
#define __MIDI_KEY_H__

typedef struct{
    unsigned char note;
    unsigned char keying;
    unsigned char velocity;
    unsigned int timestamp;
}midi_key_event;

#endif /* __MIDI_KEY_H__ */
