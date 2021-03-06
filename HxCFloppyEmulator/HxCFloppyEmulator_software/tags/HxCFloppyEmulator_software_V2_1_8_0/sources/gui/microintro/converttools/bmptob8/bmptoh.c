#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "stdint.h"

#include <windows.h>
#include "../../packer/pack.h"

typedef struct {
	uint32_t xres;
	uint32_t yres;
	uint32_t size;
	uint32_t csize;
	uint8_t type;
	uint8_t pack;
} bmpinfo;


char * extractbmpdata(char *bmpfile,bmpinfo *info)
{
	FILE * file;
	int p,m,i,j,o,k;
	int32_t s;
	uint32_t taille,taille2;
	uint8_t * dbuffer;
	uint8_t vc;

	BITMAPFILEHEADER bmph;
	BITMAPINFOHEADER bmih;
	uint8_t pallette[256*8];


	file=fopen(bmpfile,"rb");
	if(file!=NULL)
	{
		//Determination taille fichier
		fseek(file,0,SEEK_END);
		taille=ftell(file);
		fseek(file,0,SEEK_SET);

		// lecture entetes
		if(info->type!=0xff)
		{
		fread(&bmph,sizeof(bmph),1,file);
		fread(&bmih,sizeof(bmih),1,file);


		info->xres=bmih.biWidth;
		info->yres=bmih.biHeight;

			if(info->type==9)
			{
				fread(&pallette,256*4,1,file);
			}

		}
		//info->type=bmih.biBitCount;

		s=bmih.biWidth;
		do
		{
			s=s-4;
		}while(s>=4);

		if(s!=0)
		s=4-s;

		taille2=((taille-bmph.bfOffBits)-(s*bmih.biHeight));
		if(info->type==0xff) taille2=taille;

		if(info->type==9) taille2=taille2+(3*256);

		if(info->type==1) 	info->size=taille2/8;
		else	info->size=taille2;

		//mem data
		dbuffer=(char *) malloc(taille2+100);

		p=0;
		j=0;
		if(info->type==9)
		{
			for(i=0;i<256;i++)
			{
				dbuffer[(i*3)]=pallette[(i*4)];
				dbuffer[(i*3)+1]=pallette[(i*4)+1];
				dbuffer[(i*3)+2]=pallette[(i*4)+2];


			}
			p=3*256;
			j=p;
		}


		fseek(file,bmph.bfOffBits,SEEK_SET);



		m=0;

		k=0;
		vc=0;

		for(i=0;i<(int)(taille2-j);i++)
		{

			if(info->type==1)
			{
				if(fgetc(file)) vc=vc|(0x80>>k);
				k++;
				if(k>=8)
				{
					dbuffer[p]=vc;
					p++;
					k=0;
					vc=0;
				}

			}
			else
			{
				dbuffer[p]=fgetc(file);
				p++;
			}

			//sup du padding bmp
			if(info->type!=0xff)
			{
				m++;
				if(m>=bmih.biWidth)
				{
					for(o=0;o<s;o++)fgetc(file);
					m=0;
				}
			}


		}
		fclose(file);
		return dbuffer;
	}
	return NULL;


}

char  buildincludefile(char *includefile,bmpinfo *info,unsigned char * dbuffer)
{
	int l;
	unsigned int i;
	FILE * file2;

	char temp[128];
	char temp2[128];

	sprintf(temp,"%s",includefile);
	for(i=0;i<strlen(temp);i++)
	{
		if(temp[i]=='.') temp[i]='_';
	}
	if(info->type!=0xff) sprintf(temp2,"data_bmp_%s.h",temp);
	else sprintf(temp2,"data_%s.h",temp);
	printf("Create %s :",temp2);


	file2=fopen(temp2,"w");
	if(info->type!=0xff)
	{
		fprintf(file2,"//////////////////////////////\n//\n//\n// Created by Binary2Header V0.6\n// (c) HxC2001\n// (c) PowerOfAsm\n//\n");
		fprintf(file2,"// File: %s  Size: %d  (%d) x:%d y:%d\n//\n//\n",includefile,info->size,info->csize,info->xres,info->yres);
		fprintf(file2,"\n\n");
		fprintf(file2,"#ifndef BMAPTYPEDEF\n#define BMAPTYPEDEF\n\n");
		fprintf(file2,"typedef  struct _bmaptype\n{\n   int type;\n   int Xsize;\n   int Ysize;\n   int size;\n   int csize;\n   unsigned char * data;\n   unsigned char * unpacked_data;\n}bmaptype;\n\n");
		fprintf(file2,"#endif\n");
		fprintf(file2,"\n\n");
		fprintf(file2,"unsigned char data_bmp%s[]={\n",temp);
	}
	else
	{
		fprintf(file2,"//////////////////////////////\n//\n//\n// Created by Binary2Header V0.6\n// (c) HxC2001\n// (c) PowerOfAsm\n//\n");
		fprintf(file2,"// File: %s  Size: %d  (%d) \n//\n//\n",includefile,info->size,info->csize);
		fprintf(file2,"\n\n");
		fprintf(file2,"#ifndef DATATYPEDEF\n#define DATATYPEDEF\n\n");
		fprintf(file2,"typedef  struct _datatype\n{\n   int type;\n   int size;\n   int csize;\n   unsigned char * data;\n   unsigned char * unpacked_data;\n}datatype;\n\n");
		fprintf(file2,"#endif\n");
		fprintf(file2,"\n\n");
		fprintf(file2,"unsigned char data__%s[]={\n",temp);

	}

	l=0;
	for(i=0;i<info->csize;i++)
	{
		{
			fprintf(file2,"0x%.2x",dbuffer[i]);
			if((i+1)<info->csize)fprintf(file2,",");
		}

		l++;
		if(l>=10)
		{
			l=0;
			fprintf(file2,"\n");
		}
	}

	fprintf(file2,"};\n");
	if(info->type!=0xff) fprintf(file2,"\n\nstatic bmaptype bitmap_%s[]=\n{\n\t{ %d, %d, %d, %d, %d, data_bmp%s, 0 }\n};\n",temp,info->type,info->xres,info->yres,info->size,info->csize,temp);
	else fprintf(file2,"\n\nstatic datatype data_%s[]=\n{\n\t{ %d, %d, %d, data__%s, 0 }\n};\n",temp,info->type,info->size,info->csize,temp);
	fclose(file2);
	return 0;
}


unsigned char mi_pack(unsigned char * bufferin, unsigned long sizein,unsigned char * bufferout, int * sizeout)
{
	unsigned char* buffer;
	unsigned char* buffer2;
	unsigned long  newsize;
	unsigned long  newsize_lzw;
	unsigned long  newsize_rle;
	int mode;


	buffer =  (unsigned char*)malloc(sizein * 10);
	buffer2 = (unsigned char*)malloc(sizein * 10);

	if( buffer && buffer2)
	{
		memset(buffer, 0, sizein * 10);
		memset(buffer2, 0, sizein * 10);

		lzw_compress(bufferin,buffer,sizein,&newsize_lzw);
		rlepack(bufferin,sizein,buffer2,&newsize_rle);

		mode=0;

		// Note : only lzw mode for this project.
		//if(newsize_rle<sizein && newsize_rle< newsize_lzw)
		//	mode=1; //rle

		if(newsize_lzw<sizein && newsize_lzw< newsize_rle)
			mode=2; //lzw

		printf("mode : %d\n",mode);

		switch(mode)
		{

			case 0:
				memcpy((buffer2+1),bufferin,sizein);
				buffer2[0]=0x0;
				memcpy(bufferout,buffer2,sizein+1);
				*sizeout=sizein+1;
			break;

			case 1:
				rlepack(bufferin,sizein,buffer+1,(int*)&newsize);
				buffer[0]=0x2;
				memcpy(bufferout,buffer,newsize+1);
				*sizeout=newsize+1;
			break;

			case 2:
				lzw_compress(bufferin,buffer+1,sizein,(int*)&newsize);
				buffer[0]=0x1;
				memcpy(bufferout,buffer,newsize+1);
				*sizeout=newsize+1;
			break;

			case 3:
				rlepack(bufferin,sizein,buffer2,(int*)&newsize);
				lzw_compress(buffer2,buffer+1,newsize,(int*)&newsize_lzw);
				buffer[0]=0x3;
				memcpy(bufferout,buffer,newsize_lzw+1);
				*sizeout=newsize_lzw+1;
			break;
		}

		free(buffer);
		free(buffer2);
	}

	return 0;
};


int main(int argc, char* argv[])
{

	bmpinfo infoo;
	unsigned char * dbuffer;
	unsigned char * cbuffer;
	int size,i;
	printf("Binary2Header V0.6\nHxC2001\n");
	if(argc==1)printf("Usage:\n");
	else
	{
		i=argc-1;
		do{
			if(strcmp("-BMP8",argv[i])==0)  infoo.type=8;
			if(strcmp("-BMP8P",argv[i])==0)  infoo.type=9;
			if(strcmp("-BMP1",argv[i])==0)  infoo.type=1;
			if(strcmp("-DATA",argv[i])==0)  infoo.type=0xff;
			i--;
		}while(i);

		dbuffer=NULL;
		dbuffer=(unsigned char *)extractbmpdata(argv[1],&infoo);

		if(dbuffer!=NULL)
		{
			printf("%d\n",infoo.size+100+1024);
			cbuffer=(unsigned char *)malloc(infoo.size+100+1024);
			if(cbuffer!=NULL)
			{
				printf("Pack...\n");

				mi_pack(dbuffer,infoo.size,cbuffer, &size);
				printf("build include file...\n");
				infoo.csize=size;
				buildincludefile(argv[1],&infoo,cbuffer);

				free(cbuffer);
				free(dbuffer);
			}
			else
			{
				printf("Malloc Error!\n");
			}
		}
	}
	return 0;
}

