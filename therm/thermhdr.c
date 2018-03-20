/* thermhdr.c - handles vdb headers, flips, etc. */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <std.h>
#include <io.h>
#include <stdio.h>
#include "pctherm.h"
#include "headers\antypes.h"

#define NORAMP 2147483647L

extern VCTRL far *vc;
extern SHEAD far *sh;
extern struct disk_dcmeth  far *dcm;
extern DMETH far *dm;		/* display method */
extern DHEAD far *dh;
extern HCMETH far *hcm;
extern float far *dataptr;

long npoints = 0;
long nords= 0;

float far *thermhdr(void  far *hdbufr)
	{
	char far *temp;

	vc = (VCTRL*)hdbufr;
	swapl(&vc->fileid,1L);
	swapl(&vc->std[0].offset,52L);

	temp = (char *)hdbufr + vc->std[1].offset;
	sh = (SHEAD far *)temp;
	swapl(&sh->datetime,1L);
	swapf(&sh->sweight,7L);

	temp = (char *)hdbufr + vc->std[2].offset;
	dcm = (struct disk_dcmeth *)temp;
	swapl(&dcm->valve1,2L);
	swapl(&dcm->int7pd,27L);	/*includes TRAMP*/
	SWAP_S(dcm->endcon);
	swapl(&dcm->endtemp,2L);
	swapf(&dcm->datapd,1L);

	temp = (char *)hdbufr + vc->std[3].offset;
	dm = (DMETH far *)temp;
	swapf(&dm->yrange,3L);
	swapl(&dm->pwrten,1L);
	swapf(&dm->Ttmin,2L);
	SWAP_S(dm->xscale);
	SWAP_S(dm->yscale);
	swapf(&dm->y2range,3L);

	temp = (char *)hdbufr + vc->std[5].offset;
	dh = (DHEAD far *)temp;
	swapl(&dh->npts,2L);
	swapf(&dh->delt,1L);
	npoints = dh->npts;
	nords = dh->nord;

	temp = (char *)hdbufr + vc->std[7].offset;
	hcm = (HCMETH far *)temp;
	SWAP_S(hcm->anmode);
	SWAP_S(hcm->hirange);
	SWAP_S(hcm->ln2);
	SWAP_S(hcm->fconst);
	swapl(&hcm->gttrate,2L);
/** not used yet, so don't flip for now
	swapl(&hcm->mintemp,6L);
	swapl(&hcm->minpforc,6L);
**/

	}

char *antypes[]=
	{
	"No Analyzer",
	"TGA Low Furnace",
	"TGA High Furnace",
	"TGA error",
	"DSC",
	"??",
	"DMA",
	"TMA",
	NULL
	};

static char *yunits[] = {
	"mW",
	"mcal/sec",
	"mg",
	"% total weight",
	"mm",
	"mils",
	"degree of phase angle shift",
	"loss modulus",
	"degrees C (DTA)"
	};

static char *cramp[3] = {
	"  $$Temperature %d = %ld\n",
	"  $$Time        %d = %ld\n",
	"  $$Rate        %d = %ld\n"
	};

void udate(long,char *);
void pHeader(FILE *infile, FILE *outf)
	{
	int antype;
	char date[24];
	double dtemp;
	int i,j,k;
	long rtemp;
	long *rampval;

	fprintf(outf,"##TITLE=%s\n",&sh->sid[0]);
	fprintf(outf,"##JCAMP-DX=4.24 (Modified)\n");
	fprintf(outf," $$COMMENT= %s\n",&sh->comment[0]);
	antype = hcm->antype - 48;
	if ((antype < 0)||(antype > 7))
		fprintf(outf,"$$antype %c  %d %d\n",hcm->antype,hcm->antype,antype);
	else
		fprintf(outf,"##DATA TYPE= %s\n",antypes[antype]);
	udate(sh->datetime,date);
	fprintf(outf,"##DATE= %s\n",date);
	fprintf(outf,"##SAMPLE DESCRIPTION=\n");
	dtemp = (double)sh->sweight;
	fprintf(outf,"  $$SAMPLE WEIGHT=%10.3f\n",dtemp);
	if (isTMA(hcm->antype) || isDMA(hcm->antype))
		{
		fprintf(outf,"  $$SAMPLE ZERO= %f\n",(double)sh->szero);
		fprintf(outf,"  $$SAMPLE HEIGHT= %f\n",(double)sh->sheight);
		fprintf(outf,"  $$SAMPLE WIDTH= %f\n",(double)sh->swidth);
		fprintf(outf,"  $$SAMPLE LENGTH= %f\n",(double)sh->slength);
		}
	fprintf(outf,"  $$TEMPERATURE SCAN RATE= %ld\n",hcm->gttrate);

	/* Temperature ramp parameters */
	fprintf(outf,"$$TRAMP PARAMETERS=\n");
	rampval = &dcm->tramp[0];
	for (i=0; i < NTRAMPS; i++)
		{
		for (j=0; j < 3; j++)
			{
			rtemp = *rampval++;
			if (rtemp == NORAMP)
				break;
			fprintf(outf,cramp[j],i +1, rtemp);
			}
		 if (rtemp == NORAMP)
		 	break;
		}
	fprintf(outf,"##XUNITS=Degrees C\n");
	i = dm->yscale;
	if ((i >= 0) && (i <= 8))
		fprintf(outf,"##YUNITS= %s\n",yunits[i]);
	else 
		fprintf(outf,"##YUNITS= %d (see definitions)\n",i);
	fprintf(outf,"##XFACTOR=1.0\n");
	fprintf(outf,"##YFACTOR=1.0\n");
	fprintf(outf,"##NORDINATES= %ld\n",nords);
	fprintf(outf,"##NPOINTS= %ld\n",npoints);
	dtemp = (double)dh->delt;
	fprintf(outf,"$$Period of data points= %10.3f\n",dtemp);

/******************************************************
*****	At this tme these are too hard to get and are not used
	fprintf(outf,"##FIRSTX= \n");
	fprintf(outf,"##LASTX= \n");
	fprintf(outf,"##FIRSTY= \n");
******************************************************/
	fprintf(outf,"##XYDATA=(Index X Y1ord..Ynord)\n");

	}

void initData(FILE *);
float gNextFloat(void);

void pData(FILE *infile,FILE *outfil)
	{
	int i,j,k;
/*********************************************************************
	long nread;     /* no chars actually read **
	long nchars;	/* require no. of chars, from vctrl **
	long datapoints;
	long ords[16] = {0};
	void far* dataarea;
	float far *dataptrs[16] = {NULL}; /* pointers to x, y1, y... */

/*	nchars = vc->roff - vc->doff;   /*this should be data area of file */

	fseek(infile,vc->doff,SEEK_SET);

	initData(infile);

	for (i = 0; i < npoints ; i++)
		{
		fprintf(outfil,"%6d",i);
		for ( j = 0; j <= nords; j++)
			fprintf(outfil," %10.7e",gNextFloat());
		fprintf(outfil,"\n");
		}
	}


