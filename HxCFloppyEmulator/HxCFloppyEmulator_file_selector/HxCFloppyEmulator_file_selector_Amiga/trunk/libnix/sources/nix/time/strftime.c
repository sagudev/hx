#include <time.h>
#include <locale.h>
#include <libraries/locale.h>
#include <proto/locale.h>

#define ADDS(st)  tmp=strftime(s,maxsize-size,(st),timeptr);break;

#define ADDN(a,b) tmp=strfnumb(s,maxsize-size,(a),(b));break;

#define STOR(c)   if(++size<=maxsize)*s++=(c);

#define STR(a) \
(__localevec[LC_TIME-1]==NULL?strings[(a)-1]:GetLocaleStr(__localevec[LC_TIME-1],(a)))

extern struct Locale *__localevec[];

/* All calendar strings */
static const unsigned char *const strings[]=
{ "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday",
  "Sun","Mon","Tue","Wed","Thu","Fri","Sat",
  "January","February","March","April","May","June",
  "July","August","September","October","November","December",
  "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec",
  "","","AM","PM" };

static size_t strfnumb(char *s,size_t maxsize,signed int places,size_t value)
{ size_t size=0;
  if(places>1)
    size=strfnumb(s,maxsize,places-1,value/10);
  else if(value>=10)
    size=strfnumb(s,maxsize,places+1,value/10);
  else
    while ((places++<-1) && (++size<=maxsize)) s[size-1]=' ';
  if(++size<=maxsize)
    s[size-1]=(value%10+'0');
  return size;
}

size_t strftime(char *s,size_t maxsize,const char *format,const struct tm *timeptr)
{ size_t size=0,tmp;
  while(*format)
  { if(*format=='%')
    { tmp=0;
      switch(*++format)
      { case 'a':
          ADDS(STR(ABDAY_1+timeptr->tm_wday));
        case 'b':
        case 'h':
          ADDS(STR(ABMON_1+timeptr->tm_mon));
        case 'c':
          ADDS("%m/%d/%y %H:%M:%S");
        case 'd':
          ADDN(2,timeptr->tm_mday);
        case 'e':
          ADDN(-2,timeptr->tm_mday);
        case 'j':
          ADDN(3,timeptr->tm_yday+1);
        case 'k':
          ADDN(-2,timeptr->tm_hour);
        case 'l':
          ADDN(-2,timeptr->tm_hour%12+(timeptr->tm_hour%12==0)*12);
        case 'm':
          ADDN(2,timeptr->tm_mon+1);
        case 'p':
          ADDS(STR(AM_STR+(timeptr->tm_hour>=12)));
        case 'r':
          ADDS("%I:%M:%S %p");
        case 'w':
          ADDN(1,timeptr->tm_wday);
        case 'x':
          ADDS("%m/%d/%y");
        case 'y':
          ADDN(2,timeptr->tm_year%100);
        case 'A':
          ADDS(STR(DAY_1+timeptr->tm_wday));
        case 'B':
          ADDS(STR(MON_1+timeptr->tm_mon));
        case 'C':
          ADDS("%a %b %e %H:%M:%S %Y");
        case 'D':
          ADDS("%m/%d/%y");
        case 'H':
          ADDN(2,timeptr->tm_hour);
        case 'I':
          ADDN(2,timeptr->tm_hour%12+(timeptr->tm_hour%12==0)*12);
        case 'M':
          ADDN(2,timeptr->tm_min);
        case 'R':
          ADDS("%H:%M");
        case 'S':
          ADDN(2,timeptr->tm_sec);
        case 'T':
        case 'X':
          ADDS("%H:%M:%S");
        case 'U':
          ADDN(2,(timeptr->tm_yday+7-timeptr->tm_wday)/7);
        case 'W':
          ADDN(2,(timeptr->tm_yday+7-(6+timeptr->tm_wday)%7)/7);
        case 'Y':
          ADDN(4,timeptr->tm_year+1900);
        case 't':
          STOR('\t');
          break;
        case 'n':
          STOR('\n');
          break;
        case '%':
          STOR('%');
          break;
        case '\0':
          format--;
          break;
      }
      size+=tmp;
      s+=tmp;
    }
    else
      STOR(*format);
    format++;
  }
  STOR('\0');
  if(size>maxsize)
  { s-=size;
    if(maxsize) /* Don't know if this is necessary, therefore it's here ;-) */
      s[maxsize-1]='\0';
    size=1;
  }
  return size-1;
}
