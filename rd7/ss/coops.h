/*********************************************************************\
**
**	COOPS -- C Object-Oriented Programming System
**
**		COOPS is an attempt at a simple object-oriented programming
**		system based on C.  The idea is NOT to require a pre-processor,
**		but to allow programming directly in C.
**
**		The subclassing technique herein was derived from the X
**		Window System toolkit intrinsics.
**
**		The following prefixes are used:
**
**			o			pointers to objects
**			c			pointers to classes
**			r			object records
**			cr			class records
**			cf			class function
**			f			field access macros (internal)
**			g			generic operations
**
**			obj			object operations
**			class		class operations
**
**	880428	SS	create
**
\*********************************************************************/

#define COOPS_VERSION	"Coops 1.0"
#define COOPS_COPYRIGHT	"Copyright (c) 1988 Stephen Savitzky."


#ifndef TRUE
#define TRUE 1
#define FALSE 0
typedef unsigned char uchar, 	Card8,  Bool;
typedef unsigned short ushort, 	Card16;
typedef unsigned long ulong, 	Card32, Cardinal, (*CardFunc)(), OpaqueC;
typedef char					Int8, (*CharFunc)();
typedef short					Int16;
typedef long					Int32, Integer, (*IntFunc)(), OpaqueI;
typedef double					Real64, Real, (*RealFunc)();
typedef char *				    String;
typedef String					(*StringFunc)();
typedef void					(*VoidFunc)();
typedef void *					Opaque;
typedef char *					OpaqueP;
#endif

#ifndef global
#define global extern
#endif


/*********************************************************************\
**
** Generic Object
**
**	Note the way subclassing is done.  For each superclass Mumble of an
**	object we have a typedef MumblePart, and a structure component 
**	called just mumble.  This technique is taken from Xtk.  This makes
**	it possible to assign a fixed name to each field no matter how many
**	levels of subclassing are involved.
**
**	To be precise:
**
**		MumbleRec	typedef		the structure of a Mumble object
**		Mumble		typedef		pointer to a MumbleRec
**		MumblePart	typedef		the part of a Mumble that is unique
**								to the class, not a superclass
**
**		MumbleClassRec etc...	the class of a Mumble
**		clMumble	variable	-> crMumble (a MumbleClassRec).
**
**		mumble		field		the name of the MumblePart of a MumbleRec
**		mumble		prefix		procedures on class Mumble.
**
**	Note that Nil could a real object, not zero.  This means that you
**	can't use "if (foo)" to test for null, but it also means that
**	null pointers are less likely, AND that you can get the class
**	of anything without worrying about dereferencing a null pointer.
**
\*********************************************************************/

typedef struct _ObjectRec	*Object;
typedef struct _ClassRec	*Class;

typedef struct _ObjectPart {
	Class			 class;
} ObjectPart;

typedef struct _ObjectRec {
	ObjectPart	object;
} ObjectRec;

typedef Object		(*ObjFunc)();

#define fClass(o) 	((o) -> object.class)
#define fSuper(o) 	((o) -> object.class -> super)

#define NIL			((Object)0L)
#define ISNULL(o)	((Object)(o) == NIL)
#define NOTNULL(o)	((Object)(o) != NIL)

/*
** NOTE:  in METH, the function name passed must include the
**		  component of the class structure that contains it,
**  e.g.	METH(foo, TreeClass, treeClass.opname)
*/
#define METH(o,c,f) 		(* ((c)fClass(o)) -> f)
#define CMETH(o,c,f)		(* ((c)(o)) -> f)

#define METH0(o,c,f)		METH(o, c, f)(o)
#define METH1(o,c,f,a1)		METH(o, c, f)(o, a1)
#define METH2(o,c,f,a1,a2)	METH(o, c, f)(o, a1, a2)


/*********************************************************************\
**
** Classes
**
**		The idea is for certain generic operations to be done
**		quickly by indirection through fixed fields in the class.
**		Anything else (e.g. properties) is done by hashing a symbol
**		into the class's method dictionary.  
**
\*********************************************************************/

typedef struct _ObjectClassPart {
	ushort		isize;		/* =  instance size */
	String		name;		/* -> name string */
	Class		super;		/* -> superclass */
	Object		mdict;		/* -> method dictionary */

	ObjFunc		m_init;		/* (class)		initialize object */
	ObjFunc		m_new;		/* (...)		make a new object */
	ObjFunc		m_clone;	/* (self)		clone an object */
	ObjFunc		m_kill;		/* (self)		destroy an object */
				
	ObjFunc		m_open;		/* (self)		open subtree */
	ObjFunc		m_close;	/* (self)		close subtree */
	StringFunc	m_name;		/* (self)		object name */
} ObjectClassPart;


typedef struct _ClassRec {
	ObjectPart		obj;
	ObjectClassPart	objClass;
} ClassRec;


global Class 	cObject;	/* -> object's class */
global Class 	cClass;		/* -> class's class */

global ClassRec crObject;
global ClassRec crClass;

#define fClassIsize(c)	((c) -> objClass.isize)
#define fClassName(c)	((c) -> objClass.name)
#define fClassSuper(c)	((c) -> objClass.super)
#define fClassMdict(c)	((c) -> objClass.mdict)

/*********************************************************************\
**
** Utility Operations
**
\*********************************************************************/

global Object objAlloc();		/* (class, extra)	allocate an object */
global Object objCalloc();		/* (class, extra)	allocate and zero */
global void	  objFree();		/* (obj)	free an object */


/*********************************************************************\
**
** Generic Object Operations
**
**		These are defined as macros that go direct to fields
**		in the class structure.
**
\*********************************************************************/

#define cfNew(o)		CMETH((o), Class, objClass.m_new)

#define gInit(o) 		METH0((o), Class, objClass.m_init)
#define gClone(o)		METH0((o), Class, objClass.m_clone)
#define gOpen(o) 		METH0((o), Class, objClass.m_open)
#define gClose(o)		METH0((o), Class, objClass.m_close)
#define gName(o)		METH0((o), Class, objClass.m_name)

#define gKill(o)		(ISNULL(o)? NIL : METH0((o), Class, objClass.m_kill))




