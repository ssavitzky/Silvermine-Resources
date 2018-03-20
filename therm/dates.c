/* DateDay - converts yy/mm/dd to serial day         */		
/*           yy/mm/dd is a string with leading zeros */
/*           returns long sday */
#include <stdio.h>

#define ZERO '0'

long DateDay(char *date)
	{
	int year,mo,day;
	char fc;
	long sday;

	if (date[0]=='1' && date[1]=='9')
		date += 2;
	year = (int)(date[0]-ZERO)*10 + (int)(date[1]-ZERO);
	fc = (date[3]==' ') ? ZERO : date[3];
	mo = (int)(fc-ZERO)*10 + (int)(date[4]-ZERO);
	fc = (date[6]==' ') ? ZERO : date[6];
	day = (int)(fc-ZERO)*10 + (int)(date[7]-ZERO);
	
	if (mo > 2)
		mo -=3;
	else
		{
		mo += 9;
		year--;
		}

	sday = (1461L * (long)year)/4L + (long)((153 * mo +2)/5) + day;
	return(sday);
	}

/* wkday - returns day of week given serial day */
/*	        monday = day 1								*/
int wkday(long sday)
	{
	return((sday + 3) % 7);
	}

/* DayDate - fills string with yy/mm/dd given serial day and  */
/*             pointer to string                                */
void DayDate(long sday,
				 char * date)	/* at least 9 chars */
	{
	long year,mo,day,k1,k2;
	year = 4 * sday -1;
	k1 = year % 1461;
	mo = 5 * (int)((k1 + 4)/4) - 3;
	k2 = mo % 153;
	day = (k2 + 5)/5;
	mo /= 153;
	year /= 1461;

	if (mo < 10)
		mo += 3;
	else
		{
		mo -= 9;
		year++;
		}

	sprintf(date,"%02u/%02u/%02u",(int)year,(int)mo,(int)day);
	}

/* udate - converts "universal time" (seconds from 1970/01/01) */
/*          to date, time string                               */

/* define time intervals in seconds */
#define HOUR 3600L
#define DAY (3600L * 24L)
#define MIN 60L
#define BASE 25509L

void udate(long time,
           char *date)		/* yy/mm/dd  hh:mm:ss */
	{
	long today,sday;
	int hrs,mins,secs;

	today = time % DAY;		/* for hrs,mins,secs */
	sday = time/DAY + BASE;	/* serial day */
	DayDate(sday,date);
	date += strlen(date);

	/* since date is set, now fill with time */
	secs = today % MIN;
	mins = (int)(today % HOUR)/MIN;
	hrs = today/HOUR;
	sprintf(date,"  %02u:%02u:%02u",hrs,mins,secs);
	}
