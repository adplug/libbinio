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

#include <stdio.h>

#include <string.h>

#include "binio.h"

/***** Defines *****/

#if BINIO_WITH_STRING
// String buffer size for std::string readString() method
#define STRINGBUFSIZE	256
#endif

/***** binio *****/

const binio::Flags binio::system_flags = binio::detect_system_flags();

const binio::Flags binio::detect_system_flags()
{
  Flags f = 0;

  // Endian test
  union {
    int word;
    Byte byte;
  } endian_test;

  endian_test.word = 1;
  if(endian_test.byte != 1) f |= BigEndian;

  // IEEE-754 floating-point test
  float fl = 6.5;
  Byte	*dat = (Byte *)&fl;

  if(sizeof(float) == 4 && sizeof(double) == 8)
    if(f & BigEndian) {
      if(dat[0] == 0x40 && dat[1] == 0xD0 && !dat[2] && !dat[3])
	f |= FloatIEEE;
    } else
      if(dat[3] == 0x40 && dat[2] == 0xD0 && !dat[1] && !dat[0])
      f |= FloatIEEE;

  return f;
}

binio::binio()
  : my_flags(system_flags), err(NoError)
{
}

binio::~binio()
{
}

void binio::setFlag(Flag f, bool set)
{
  if(set)
    my_flags |= f;
  else
    my_flags &= !f;
}

bool binio::getFlag(Flag f)
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

binio::Int binistream::readInt(unsigned int size)
{
  unsigned int	i;
  Int		val = 0, in;

  // Check if 'size' doesn't exceed our system's biggest type.
  if(size > sizeof(Int)) {
    err = Unsupported;
    return 0;
  }

  for(i = 0; i < size; i++) {
    in = getByte();
    if(getFlag(BigEndian))
      val <<= 8;
    else
      in <<= i * 8;
    val |= in;
  }

  return val;
}

binio::Float binistream::readFloat(FType ft)
{
  if(getFlag(FloatIEEE)) {	// Read IEEE-754 floating-point value
    unsigned int	i, size;
    Byte		in[8];
    bool		swap;

    // Determine appropriate size for given type.
    switch(ft) {
    case Single: size = 4; break;
    case Double: size = 8; break;
    }

    // Determine byte ordering, depending on what we do next
    if(system_flags & FloatIEEE)
      swap = getFlag(BigEndian) ^ (system_flags & BigEndian);
    else
      swap = !getFlag(BigEndian);

    // Read the float byte by byte, converting endianess
    for(i = 0; i < size; i++)
      if(swap)
	in[size - i - 1] = getByte();
      else
	in[i] = getByte();

    if(system_flags & FloatIEEE) {
      // Compatible system, let the hardware do the conversion
      switch(ft) {
      case Single: return *(float *)in; break;
      case Double: return *(double *)in; break;
      }
    } else {	// Incompatible system, convert manually
      switch(ft) {
      case Single: return ieee_single2float(in);
      case Double: return ieee_double2float(in);
      }
    }
  }

  // User tried to read a (yet) unsupported floating-point type. Bail out.
  err = Unsupported; return 0.0;
}

binio::Float binistream::ieee_single2float(Byte *data)
{
  signed int	sign = data[0] >> 7 ? -1 : 1;
  unsigned int	exp = ((data[0] << 1) & 0xff) | ((data[1] >> 7) & 1),
    fracthi7 = data[1] & 0x7f;
  Float		fract = fracthi7 * 65536.0 + data[2] * 256.0 + data[3];

  // Signed and unsigned zero
  if(!exp && !fracthi7 && !data[2] && !data[3]) return sign * 0.0;

  // Signed and unsigned infinity (unsupported on non-IEEE systems)
  if(exp == 255)
    if(!fracthi7 && !data[2] && !data[3]) {
      err = Unsupported;
      if(sign == -1) return -1.0; else return 1.0;
    } else {	  // Not a number (unsupported on non-IEEE systems)
      err = Unsupported; return 0.0;
    }

  if(!exp)	// Unnormalized float values
    return sign * pow(2, -126) * fract * pow(2, -23);
  else		// Normalized float values
    return sign * pow(2, exp - 127) * (fract * pow(2, -23) + 1);

  err = Fatal; return 0.0;
}

binio::Float binistream::ieee_double2float(Byte *data)
{
  signed int	sign = data[0] >> 7 ? -1 : 1;
  unsigned int	exp = ((data[0] << 1) & 0xff) | ((data[1] >> 7) & 1),
    fracthi7 = data[1] & 0x7f;
  Float		fract = fracthi7 * 65536.0 + data[2] * 256.0 + data[3];

  // Signed and unsigned zero
  if(!exp && !fracthi7 && !data[2] && !data[3]) return sign * 0.0;

  // Signed and unsigned infinity
  if(exp == 255)
    if(!fracthi7 && !data[2] && !data[3]) {
      err = Unsupported;
      if(sign == -1) return -1.0; else return 1.0;
    } else {	  // Not a number
      err = Unsupported; return 0.0;
    }

  if(!exp)	// Unnormalized float values
    return (Float)sign * pow(2, -126) * pow(fract, -23);
  else		// Normalized float values
    return (Float)sign * pow(2, exp - 127) * (pow(fract, -23) + 1);

  err = Fatal; return 0.0;
}

binio::Float binio::pow(Float base, signed int exp)
/* Our own, stripped-down version of pow() for not having to depend on 'math.h'.
 * This one handles float values for the base and an integer exponent, both
 * positive and negative.
 */
{
  int	i;
  Float	val = base;

  if(!exp) return 1.0;

  for(i = 1; i < (exp < 0 ? -exp : exp); i++)
    val *= base;

  if(exp < 0) val = 1.0 / val;

  return val;
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

void binostream::writeInt(Int val, unsigned int size)
{
  unsigned int	i;

  // Check if 'size' doesn't exceed our system's biggest type.
  if(size > sizeof(Int)) { err = Unsupported; return; }

  for(i = 0; i < size; i++) {
    if(getFlag(BigEndian))
      putByte((val >> (size - i - 1) * 8) & 0xff);
    else {
      putByte(val & 0xff);
      val >>= 8;
    }
  }
}

void binostream::writeFloat(Float f, FType ft)
{
  if(getFlag(FloatIEEE)) {	// Write IEEE-754 floating-point value
    if(system_flags & FloatIEEE) {
      // compatible system, let the hardware do the conversion
      float		outf = f;
      double	       	outd = f;
      unsigned int	i, size;
      Byte		*out;
      bool		swap = getFlag(BigEndian) ^ (system_flags & BigEndian);

      // Determine appropriate size for given type.
      switch(ft) {
      case Single: size = 4; out = (Byte *)&outf; break;
      case Double: size = 8; out = (Byte *)&outd; break;
      }

      // Write the float byte by byte, converting endianess
      if(swap) out += size - 1;
      for(i = 0; i < size; i++) {
	putByte(*out);
	if(swap) out--; else out++;
      }
    }
  }

  // User tried to write a (yet) unsupported floating-point type. Bail out.
  err = Unsupported;
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
