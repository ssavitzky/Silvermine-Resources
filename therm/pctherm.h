/* Main header for pctherm functions */
#include "headers\defnvdb.h"
#include "headers\shead.h"
#include "headers\constants.h"
#include "headers\vctrl.h"
#include "headers\d_dcmeth.h"
#include "headers\dmeth.h"
#include "headers\ameth.h"
#include "headers\dhead.h"
#include "headers\rhead.h"
#include "headers\hcmeth.h"

#define HEADERBUF 4096



/*	File contains pointers & sizes-----------------------------
static	int sizes[] = 
	{
	sizeof(struct vctrl),
	sizeof(struct shead),
	sizeof(struct disk_dcmeth),
	sizeof(struct dmeth),
	sizeof(struct ameth),
	sizeof(struct dhead),
	sizeof(struct rhead),
	sizeof(struct hcmeth),
	};
*/

/**** DEFINE SWAPS **********************/
#define SWAP_S(w)   (w = swaps(w))
int swaps(int);
void swapl(LONG *, long );
void swapf(float *, long);


