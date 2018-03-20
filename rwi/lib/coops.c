/***/static char *moduleID="coops 1.0 (c)1988 S. Savitzky";/***/

/*********************************************************************\
**
**	COOPS -- C Object-Oriented Programming System
**
**		Entities in this file are for the most part global, to
**		facilitate static initialization of class objects.  This
**		may eventually change.
**
**	880423 SS	create
**
\*********************************************************************/

#include <stdio.h>
#include <malloc.h>

#include "coops.h"

#undef  global
#define global

extern int errorPrintf();

/*********************************************************************\
**
** Utilities
**
\*********************************************************************/

global Object objAlloc(cl, extra)
	Class cl;
	ushort extra;
{
	Object o = (Object) malloc(cl -> objClass.isize + extra);
	if (!o) {
		char msg[80];
		sprintf(msg, "Unable to allocate object of class %s.",
				cl -> objClass.name);
		errorSet(msg);
		return (o);
	}
	o -> object.class = cl;
	return (o);
}

global Object objCalloc(cl, extra)
	Class cl;
	ushort extra;
{
	Object o = (Object) calloc(1, cl -> objClass.isize + extra);
	if (!o) {
		char msg[80];
		sprintf(msg, "Unable to allocate object of class %s.",
				cl -> objClass.name);
		errorSet(msg);
		return (o);
	}
	o -> object.class = cl;
	return (o);
}

global void objFree(o)
	Object o;
{
	if (NOTNULL(o))	free(o);
}


/*********************************************************************\
**
** Methods for All Objects
**
\*********************************************************************/

Object objInit (cl)			/* Initialize class */
	Class cl;
{
	
}

Object objNew (cl)			/* Create new object */
	Class cl;
{
	return (objCalloc(cl, 0));
}

Object objClone (o)			/* Clone existing object */
	Object o;
{
	Class cl = fClass(o);
	Object oc = objCalloc(cl, 0);

	memcpy ((char *)oc, (char *)o, fClassIsize(cl));
	return (oc);
}

Object objKill (o)			/* Kill object */
	Object o;
{
	objFree(o);
	return (NIL);
}

Object objDoesNotImplement (o)	/* unimplemented method */
	Object o;
{
	errorPrintf ("object does not implement method");
	return (NIL);
}

Object objRetSelf(o)		/* return self */
	Object o;
{
	return (o);
}

char *objName (o)			/* default name */
	Object o;
{
	return ("UNNAMED");
}

char *className (o)			/* class name */
	Class o;
{
	return (o -> objClass.name);
}

/*********************************************************************\
**
** Object Classes
**
\*********************************************************************/

global ClassRec crClass = {
	&crClass,				/* class */				/* Object Part */
													/* Class Part */
	sizeof (ClassRec),		/* instance size */
	"Class",				/* class name */
	(Class)NIL,				/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	objNew,					/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	objDoesNotImplement,	/* open */
	objDoesNotImplement,	/* close */
	className,				/* name */
};

global ClassRec crObject = {
	&crClass,				/* class */				/* Object Part */
													/* Class Part */
	sizeof (ObjectRec),		/* instance size */
	"Object",				/* class name */
	(Class)NIL,				/* super */
	NIL,					/* method dict */
	objInit,				/* initialize */
	objNew,					/* new object */
	objClone,				/* clone self */
	objKill,				/* kill self */
	objDoesNotImplement,	/* open */
	objDoesNotImplement,	/* close */
	objName,				/* name */
};


Class clObject = &crObject;
Class clClass  = &crClass;


/*********************************************************************\
**
** Class Initialization
**
\*********************************************************************/



