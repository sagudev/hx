#!/bin/sh

cat >myctest.c <<END

#include<stdio.h>

union u{
    long l;
    char c[4];
    };

int main(int argc, char *argv[])
{
    union u val;

    printf("%d ",sizeof(long));
    printf("%d ",sizeof(short));
    printf("%d ",sizeof(int));
    
    val.l=1L;
    if (val.c[3]==1)
        puts("BIG");
    else
        puts("LITTLE");
}

END

gcc myctest.c -o myctest
rm myctest.c

if [ `./myctest |cut -d' ' -f 1` != 4 ]
    then echo "Error : sizeof(long)!=4"
fi
if [ `./myctest |cut -d' ' -f 2` != 2 ]
    then echo "Error : sizeof(short)!=2"
fi
if [ `./myctest |cut -d' ' -f 3` != 4 ]
    then echo "Error  :sizeof(int)!=4"
fi

if [ `./myctest |cut -d' ' -f 4` = LITTLE ]
    then 
    echo "#ifndef LITT_ENDIAN" >defendian.h
    echo "#define LITT_ENDIAN 1" >>defendian.h 
    echo "#endif /* LITT_ENDIAN */" >>defendian.h
    echo Little Endian machine detected 
else
    echo "#ifndef LITT_ENDIAN" >defendian.h
    echo "#endif /* LITT_ENDIAN */" >>defendian.h
    echo Big Endian machine detected
fi

rm myctest.exe myctest
