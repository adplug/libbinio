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
 * binwrap.cpp - Binary I/O wrapper, using standard iostreams library
 * Copyright (C) 2002 Simon Peter <dn.tlp@gmx.net>
 */

#include "binwrap.h"

#if BINIO_WITH_IOSTREAM

/***** biniwstream *****/

biniwstream::biniwstream(istream *istr)
  : in(istr)
{
}

biniwstream::~biniwstream()
{
}

void biniwstream::seek(unsigned long pos, Offset offs)
{
  switch(offs) {
  case Set: in->seekg(pos, ios::beg); break;
  case Add: in->seekg(pos, ios::cur); break;
  case End: in->seekg(pos, ios::end); break;
  }
}

binio::Byte biniwstream::getByte()
{
  int i = in->get();
  if(i == EOF || in->eof()) err = Eof;
  return (Byte)i;
}

binio::SeekP biniwstream::pos()
{
  return (SeekP)in->tellg();
}

/***** binowstream *****/

binowstream::binowstream(ostream *ostr)
  : out(ostr)
{
}

binowstream::~binowstream()
{
}

void binowstream::seek(unsigned long pos, Offset offs)
{
  switch(offs) {
  case Set: out->seekp(pos, ios::beg); break;
  case Add: out->seekp(pos, ios::cur); break;
  case End: out->seekp(pos, ios::end); break;
  }
}

void binowstream::putByte(binio::Byte b)
{
  out->put((char)b);
}

binio::SeekP binowstream::pos()
{
  return (SeekP)out->tellp();
}

/***** binwstream *****/

binwstream::binwstream(iostream *str)
  : biniwstream(str), binowstream(str), io(str)
{
}

binwstream::~binwstream()
{
}

void binwstream::seek(unsigned long pos, Offset offs)
{
  biniwstream::seek(pos, offs);
  binowstream::seek(pos, offs);
}

binio::SeekP binwstream::pos()
{
  return (SeekP)io->tellg();
}

binio::Byte binwstream::getByte()
{
  Byte in = biniwstream::getByte();
  binowstream::seek(biniwstream::pos(), Set);	// sync stream position
  return in;
}

void binwstream::putByte(binio::Byte b)
{
  binowstream::putByte(b);
  biniwstream::seek(binowstream::pos(), Set);	// sync stream position
}

#endif
