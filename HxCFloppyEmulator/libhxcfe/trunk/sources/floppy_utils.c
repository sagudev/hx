/*
//
// Copyright (C) 2006-2014 Jean-Fran�ois DEL NERO
//
// This file is part of the HxCFloppyEmulator library
//
// HxCFloppyEmulator may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/
///////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-----------H----H--X----X-----CCCCC----22222----0000-----0000------11----------//
//----------H----H----X-X-----C--------------2---0----0---0----0--1--1-----------//
//---------HHHHHH-----X------C----------22222---0----0---0----0-----1------------//
//--------H----H----X--X----C----------2-------0----0---0----0-----1-------------//
//-------H----H---X-----X---CCCCC-----222222----0000-----0000----1111------------//
//-------------------------------------------------------------------------------//
//----------------------------------------------------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////////
// File : floppy_utils.c
// Contains: utils functions
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "libhxcfe.h"
#include "floppy_loader.h"

#include "libhxcadaptor.h"

unsigned long us2index(unsigned long startindex,SIDE * track,unsigned long us,unsigned char fill,char fillorder)
{
	uint32_t time,freq;

	if(!fillorder)
	{
		if(track->bitrate==VARIABLEBITRATE)
		{
			time=0;
			do
			{
				if(fill)track->indexbuffer[startindex>>3]=0xFF;
				freq=track->timingbuffer[startindex>>3];
				startindex++;
				if(startindex>=track->tracklen) startindex=0;
				if(freq)
					time=time+(((1000000000/2)/freq)*1);
			}while(us>(time/1000));

			return startindex;
		}
		else
		{
			freq=track->bitrate;
			time=0;

			if ( freq )
			{
				do
				{
					if(fill)track->indexbuffer[startindex>>3]=0xFF;
					startindex++;
					if(startindex>=track->tracklen) startindex=0;
					time=time+(((1000000000/2)/freq)*1);
				}while(us>(time/1000));
			}
			return startindex;
		}
	}
	else
	{
		if(track->bitrate==VARIABLEBITRATE)
		{
			time=0;
			do
			{
				if(fill)track->indexbuffer[startindex>>3]=0xFF;
				freq=track->timingbuffer[startindex>>3];

				if(startindex)
					startindex--;
				else
					startindex=track->tracklen-1;

				if(freq)
					time=time+(((1000000000/2)/freq)*1);

			}while(us>(time/1000));

			return startindex;
		}
		else
		{
			freq=track->bitrate;
			time=0;
			if( freq )
			{
				do
				{
					if(fill)track->indexbuffer[startindex>>3]=0xFF;

					if(startindex)
						startindex--;
					else
						startindex=track->tracklen-1;

					time=time+(((1000000000/2)/freq)*1);
				}while(us>(time/1000));
			}
			return startindex;
		}
	}
};

unsigned long fillindex(int startindex,SIDE * track,unsigned long us,unsigned char fill,char fillorder)
{
	int start_index;

	if(startindex>=0)
	{
		start_index=us2index(0,track,startindex,0,0);
	}
	else
	{
		start_index=us2index(0,track,-startindex,0,1);
	}


	return us2index(start_index,track,us&0xFFFFFF,fill,fillorder);
}

CYLINDER* allocCylinderEntry(unsigned short rpm,unsigned char number_of_side)
{
	CYLINDER* cyl;

	cyl=(CYLINDER*)malloc(sizeof(CYLINDER));
	cyl->floppyRPM=rpm;
	cyl->number_of_side=number_of_side;
	cyl->sides=(SIDE**)malloc(sizeof(SIDE*)*number_of_side);
	memset(cyl->sides,0,sizeof(SIDE*)*number_of_side);
	return cyl;
}

void savebuffer(char * name,unsigned char * buffer, int size)
{
	FILE * f;

	f=hxc_fopen(name,"w+b");
	if(f)
	{
		fwrite(buffer,size,1,f);
		hxc_fclose(f);
	}
}

double GetTrackPeriod(HXCFLOPPYEMULATOR* floppycontext,SIDE * curside)
{
	int tracklen,i;
	double total_period;

	tracklen = curside->tracklen /8;
	if(curside->tracklen & 7)
		tracklen++;

	total_period = 0;
	for(i=0;i<tracklen;i++)
	{
		total_period = total_period + (double)((double)(1*4)/(double)curside->timingbuffer[i]);
	}

	return total_period;
}

int tracktypelisttoscan[]=
{
	ISOIBM_MFM_ENCODING,
	ISOIBM_FM_ENCODING,
	AMIGA_MFM_ENCODING,
	EMU_FM_ENCODING,
	ARBURGSYS_ENCODING,
	ARBURGDAT_ENCODING,
	UNKNOWN_ENCODING
};

int floppyTrackTypeIdentification(HXCFLOPPYEMULATOR* floppycontext,FLOPPY *fp)
{
	int i,j,t;
	int sectnum;
	unsigned char first_track_encoding;
	int nb_sectorfound;
	SECTORSEARCH* ss;
	SECTORCONFIG** scl;
	
	i = 0;
	
	first_track_encoding = UNKNOWN_ENCODING;

	ss = hxcfe_initSectorSearch(floppycontext,fp);
	if(ss)
	{
		for(i=0;i<fp->floppyNumberOfTrack;i++)
		{
			for(j=0;j<fp->floppyNumberOfSide;j++)
			{

				nb_sectorfound = 0;
				t=0;
				do{
					first_track_encoding = tracktypelisttoscan[t];
					scl = hxcfe_getAllTrackSectors(ss,i,j,first_track_encoding,&nb_sectorfound);
					ss->old_bitoffset = 0;
					ss->bitoffset = 0;
					t++;
				}while(!nb_sectorfound && (tracktypelisttoscan[t] != UNKNOWN_ENCODING));

				if(nb_sectorfound)
				{
					fp->tracks[i]->sides[j]->track_encoding = first_track_encoding ;
				}

				if(scl)
				{
					for(sectnum=0;sectnum<nb_sectorfound;sectnum++)
					{
						if(first_track_encoding == UNKNOWN_ENCODING)
							first_track_encoding = tracktypelisttoscan[i];

						hxcfe_freeSectorConfig  (ss,scl[sectnum]);
					}
					
					free(scl);
				}
			}
		}

		hxcfe_deinitSectorSearch(ss);

	}

	return 0;
}
