/* Copyright 2000 Kjetil S. Matheussen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */


#ifndef TRACKER_INCLUDE

extern void NewLocalZoom(
	struct LocalZooms **tolocalzoom,
	int line,
	uint_32 counter,
	uint_32 dividor,
	int zoomline,
	int level,
	int realline
);

extern void NewLocalZooms(struct Tracker_Windows *window,struct WBlocks *wblock);
// extern void NewLocalZooms(struct LocalZooms **tolocalzoom,int num_lines);

extern struct LocalZooms *GetNextRealLocalZoom(
	struct WBlocks *wblock,
	struct LocalZooms *localzoom
);

extern int IsOnRealLine(
	struct WBlocks *wblock,
	struct LocalZooms *localzoom,
	Place *placement
);



#endif
