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

#include <iostream.h>

#include "binwrap.h"

/***** biniwstream *****/

biniwstream::biniwstream(istream &istr)
  : in(&istr)
{
}

biniwstream::~biniwstream()
{
}

void biniwstream::seek(unsigned long pos, Offset offs)
{
  switch(offs) {
  case Start: in->seekg(pos, ios::beg); break;
  case Add: in->seekg(pos, ios::cur); break;
  case End: in->seekg(pos, ios::end); break;
  }
}

binio::Byte biniwstream::getByte()
{
  return in->get();
}

/***** binowstream *****/

binowstream::binowstream(ostream &ostr)
  : out(&ostr)
{
}

binowstream::~binowstream()
{
}

void binowstream::seek(unsigned long pos, Offset offs)
{
  switch(offs) {
  case Start: out->seekp(pos, ios::beg); break;
  case Add: out->seekp(pos, ios::cur); break;
  case End: out->seekp(pos, ios::end); break;
  }
}

void binowstream::putByte(binio::Byte b)
{
  out->put((char)b);
}
