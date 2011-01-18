/***************************************************************************

 TheBar.mcc - Next Generation Toolbar MUI Custom Class
 Copyright (C) 2003-2005 Alfonso Ranieri
 Copyright (C) 2005-2009 by TheBar.mcc Open Source Team

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 TheBar class Support Site:  http://www.sf.net/projects/thebar

 $Id$

***************************************************************************/

// uncompressed ARGB data
extern const unsigned long icon32[];
#define ICON32_WIDTH 24
#define ICON32_HEIGHT 20
#define ICON32_DEPTH 32


#ifdef USE_ICON8_COLORS
static const ULONG icon8_colors[24] =
{
  0x96969696,0x96969696,0x96969696,
  0xffffffff,0xffffffff,0xffffffff,
  0x7a7a7a7a,0x7a7a7a7a,0x7a7a7a7a,
  0x39393939,0x66666666,0xa2a2a2a2,
  0xafafafaf,0xafafafaf,0xafafafaf,
  0x00000000,0x00000000,0x00000000,
  0xd9d9d9d9,0x6d6d6d6d,0x2b2b2b2b,
  0x6d6d6d6d,0x5d5d5d5d,0x2b2b2b2b,
};
#endif

#define ICON8_WIDTH        24
#define ICON8_HEIGHT       16
#define ICON8_DEPTH         5
#define ICON8_COMPRESSION   0
#define ICON8_MASKING       2

#ifdef USE_ICON8_BODY
static const UBYTE icon8_body[320] =
{
  0xdf,0xff,0xdf,0xe0,0x20,0x00,0x20,0xf8,0x00,0x00,0x00,0x98,0x00,0x00,0x00,
  0x38,0x00,0x00,0x00,0x38,0xf0,0x00,0x70,0xe0,0x60,0x00,0x00,0xf8,0x0f,0xff,
  0xa7,0x98,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x38,0xf0,0x00,0x70,0xe0,0x60,
  0x00,0x00,0xf8,0x0f,0xff,0xa7,0x98,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x38,
  0xf0,0x00,0x70,0xe0,0x60,0x00,0x00,0xf8,0x0f,0xff,0xa7,0x98,0x00,0x00,0x00,
  0x38,0x00,0x00,0x00,0x38,0xf0,0x00,0x70,0xe0,0x60,0x00,0x00,0xf8,0x0f,0xff,
  0xa7,0x98,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x38,0xf0,0x00,0x70,0xe0,0x60,
  0x00,0x00,0xf8,0x0f,0xff,0xa7,0x98,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x38,
  0xf0,0x00,0x70,0xe0,0x60,0x00,0x00,0xf8,0x0f,0xff,0xa7,0x98,0x00,0x00,0x00,
  0x38,0x00,0x00,0x00,0x38,0xf0,0x00,0x70,0xe0,0x60,0x00,0x00,0xf8,0x0f,0xff,
  0xa7,0x98,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x38,0xf0,0x00,0x70,0xe0,0x60,
  0x00,0x00,0xf8,0x0f,0xff,0xa7,0x98,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x38,
  0xf0,0x00,0x70,0xe0,0x60,0x00,0x00,0xf8,0x0f,0xff,0xa7,0x98,0x00,0x00,0x00,
  0x38,0x00,0x00,0x00,0x38,0xf0,0x00,0x70,0xe0,0x60,0x00,0x00,0xf8,0x0f,0xff,
  0xa7,0x98,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x38,0xf0,0x00,0x70,0xe0,0x60,
  0x00,0x00,0xf8,0x0f,0xff,0xa7,0x98,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x38,
  0xf0,0x00,0x70,0xe0,0x60,0x00,0x00,0xf8,0x0f,0xff,0xa7,0x98,0x00,0x00,0x00,
  0x38,0x00,0x00,0x00,0x38,0xf0,0x00,0x70,0xe0,0x60,0x00,0x00,0xf8,0x0f,0xff,
  0xa7,0x98,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x38,0xf0,0x00,0x70,0xe0,0x60,
  0x00,0x00,0xf8,0x0f,0xff,0xa7,0x98,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x38,
  0x6f,0xff,0xef,0xe0,0x90,0x00,0x10,0xf8,0x6f,0xff,0xef,0x98,0x00,0x00,0x00,
  0x38,0x00,0x00,0x00,0x38
};
#endif

#ifdef USE_ICON8_BITMAP
static const struct BitMap icon8_bitmap =
{
  4, 14, 0, ICON8_DEPTH, 0,
  { (UBYTE *)icon8_body,
    (UBYTE *)icon8_body+(1*14*4),
    (UBYTE *)icon8_body+(2*14*4),
    0,0,0,0,0 }
};
#endif
