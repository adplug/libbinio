/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * binio.cpp - Binary stream I/O classes
 * Copyright (C) 2002 Simon Peter <dn.tlp@gmx.net>
 */

#include <string.h>

#include "binio.h"

/***** Defines *****/

#if BINIO_WITH_STRING
// String buffer size for std::string readString() method
#define STRINGBUFSIZE	256
#endif

/***** binio *****/

binio::Flags binio::system_flags = binio::detect_system_flags();

binio::binio()
  : my_flags(system_flags), err(NoError)
{
}

binio::~binio()
{
}

binio::Flags binio::detect_system_flags()
{
  Flags f = 0;
  union {
    Word word;
    Byte byte;
  } endian_test;

  // Endian test
  endian_test.word = 0x0102;
  switch(endian_test.byte) {
  case 0x01: f |= BigEndian; break;
  case 0x02: f &= !BigEndian; break;
  default: /* Fatal error! Bail out! */ break;
  }

  return f;
}

void binio::set_flag(Flag f, bool set)
{
  if(set)
    my_flags |= f;
  else
    my_flags &= !f;
}

bool binio::get_flag(Flag f)
{
  return (my_flags & f);
}

binio::Error binio::error()
{
  Error e = err;

  err = NoError;
  return e;
}

/***** binistream *****/

binistream::binistream()
{
}

binistream::~binistream()
{
}

binio::Byte binistream::readByte()
{
  return getByte();
}

binio::Word binistream::readWord()
{
  Byte first, last;

  first = getByte(); last = getByte();

  if(get_flag(BigEndian))
    return ((Word)first << 8) | (last & 0xff);
  else
    return ((Word)last << 8) | (first & 0xff);
}

binio::DWord binistream::readDWord()
{
  Word first, last;

  first = readWord(); last = readWord();

  if(get_flag(BigEndian))
    return ((DWord)first << 16) | (last & 0xffff);
  else
    return ((DWord)last << 16) | (first & 0xffff);
}

binio::QWord binistream::readQWord()
{
  DWord first, last;

  first = readDWord(); last = readDWord();

  if(get_flag(BigEndian))
    return ((QWord)first << 32) | (last & 0xffffffffll);
  else
    return ((QWord)last << 32) | (first & 0xffffffffll);
}

binio::Float binistream::readFloat()
{
  DWord dw = readDWord();
  return *(Float *)&dw;
}

binio::Double binistream::readDouble()
{
  QWord qw = readQWord();
  return *(Double *)&qw;
}

unsigned long binistream::readString(char *str, unsigned long maxlen,
				     char delim)
{
  unsigned long i;

  for(i = 0; i < maxlen; i++) {
    str[i] = (char)getByte();
    if(str[i] == delim || error()) {
      str[i] = '\0';
      return i;
    }
  }

  str[maxlen] = '\0';
  return maxlen;
}

#if BINIO_WITH_STRING
std::string binistream::readString(char delim)
{
  char buf[STRINGBUFSIZE + 1];
  std::string tempstr;
  unsigned long read;

  do {
    read = readString(buf, STRINGBUFSIZE, delim);
    tempstr.append(buf, read);
  } while(read == STRINGBUFSIZE);

  return tempstr;
}
#endif

void binistream::ignore(unsigned long amount)
{
  unsigned long i;

  for(i = 0; i < amount; i++)
    getByte();
}

/***** binostream *****/

binostream::binostream()
{
}

binostream::~binostream()
{
}

void binostream::writeByte(Byte b)
{
  putByte(b);
}

void binostream::writeWord(Word w)
{
  if(get_flag(BigEndian)) {
    putByte(w >> 8);
    putByte(w & 0xff);
  } else {
    putByte(w & 0xff);
    putByte(w >> 8);
  }
}

void binostream::writeDWord(DWord dw)
{
  if(get_flag(BigEndian)) {
    writeWord(dw >> 16);
    writeWord(dw & 0xffff);
  } else {
    writeWord(dw & 0xffff);
    writeWord(dw >> 16);
  }
}

void binostream::writeQWord(QWord dw)
{
  if(get_flag(BigEndian)) {
    writeDWord(dw >> 32);
    writeDWord(dw & 0xffffffff);
  } else {
    writeDWord(dw & 0xffffffff);
    writeDWord(dw >> 32);
  }
}

void binostream::writeFloat(Float f)
{
  writeDWord(*(DWord *)&f);
}

void binostream::writeDouble(Double d)
{
  writeQWord(*(QWord *)&d);
}

void binostream::writeString(const char *str)
{
  unsigned int i;

  for(i=0;i<strlen(str);i++)
    putByte(str[i]);
}

#if BINIO_WITH_STRING
void binostream::writeString(const std::string &str)
{
  writeString(str.c_str());
}
#endif
