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
 * binfile.h - Binary file I/O
 * Copyright (C) 2002 Simon Peter <dn.tlp@gmx.net>
 */

#include <stdio.h>
#include <errno.h>

#include "binfile.h"

/***** binfbase *****/

binfbase::binfbase()
  : f(0)
{
}

binfbase::~binfbase()
{
  if(f) close();
}

void binfbase::close()
{
  if(f) {
    if(fclose(f) == EOF) err = Fatal; else f = 0;
  } else
    err = NotOpen;
}

void binfbase::seek(SeekP pos, Offset offs)
{
  switch(offs) {
  case Set: fseek(f, pos, SEEK_SET); break;
  case Add: fseek(f, pos, SEEK_CUR); break;
  case End: fseek(f, pos, SEEK_END); break;
  }
}

binio::SeekP binfbase::pos()
{
  long pos = ftell(f);

  if(pos == -1) {
    err = Fatal;
    return 0;
  } else
    return pos;
}

bool binfbase::eof()
{
  return feof(f);
}

/***** binifstream *****/

binifstream::binifstream()
{
}

binifstream::binifstream(const char *filename, const Mode mode)
{
  open(filename, mode);
}

#if BINIO_WITH_STRING
binifstream::binifstream(const std::string &filename, const Mode mode)
{
  open(filename, mode);
}
#endif

binifstream::~binifstream()
{
}

void binifstream::open(const char *filename, const Mode mode)
{
  f = fopen(filename, "rb");

  if(!f)
    switch(errno) {
    case ENOENT:
    case ENOTDIR:
      err = NotOpen;
      break;
    case EACCES: err = Denied; break;
    default: err = Fatal; break;
    }
}

#if BINIO_WITH_STRING
void binifstream::open(const std::string &filename, const Mode mode)
{
  open(filename.c_str(), mode);
}
#endif

binio::Byte binifstream::getByte()
{
  int read;

  if(f) {
    read = fgetc(f);
    if(read == EOF) err = Eof;
    return (Byte)read;
  } else {
    err = NotOpen;
    return (Byte)0;
  }
}

/***** binofstream *****/

binofstream::binofstream()
{
}

binofstream::binofstream(const char *filename, const Mode mode)
{
  open(filename, mode);
}

#if BINIO_WITH_STRING
binofstream::binofstream(const std::string &filename, const Mode mode)
{
  open(filename, mode);
}
#endif

binofstream::~binofstream()
{
}

void binofstream::open(const char *filename, const Mode mode)
{
  f = fopen(filename, "wb");

  if(!f)
    switch(errno) {
    case EEXIST:
    case EACCES:
    case EROFS:
    case ETXTBSY:
      err = Denied;
      break;
    case EISDIR:
    case ENOTDIR:
    case ENOENT:
      err = NotOpen;
      break;
    default: err = Fatal; break;
    }
}

#if BINIO_WITH_STRING
void binofstream::open(const std::string &filename, const Mode mode)
{
  open(filename.c_str(), mode);
}
#endif

void binofstream::putByte(Byte b)
{
  if(fputc(b, f) == EOF)
    err = Fatal;
}

/***** binfstream *****/

// TODO: Still needs to be implemented!
