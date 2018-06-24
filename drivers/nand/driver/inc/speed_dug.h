#ifndef _SPEED_DUG_H_
#define _SPEED_DUG_H_

#include <pagelist.h>
#include <nand_debug.h>

#define NDD_READ	1
#define NDD_WRITE	0
#define DEBUG_TIME_BYTES (10 * 1024 *1024) //10MB

void __speed_dug_begin(int mode, PageList *pl);
void __speed_dug_end(int mode);

#ifdef DEBUG_SPEED
#define speed_dug_begin(mode, pl) __speed_dug_begin(mode, pl)
#define speed_dug_end(mode) __speed_dug_end(mode)
#else
#define speed_dug_begin(mode, pl)
#define speed_dug_end(mode)
#endif

#endif /* _SPEED_DUG_H_ */
