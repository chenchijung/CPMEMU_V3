/* Header file for the instruction set simulator.
   Copyright (C) 1995  Frank D. Cringle.

   Module MEM_MMU.[hc] Copyright (C) 1998/1999 by Andreas Gerlich (agl)

This file is part of yaze - yet another Z80 emulator.

Yaze is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include <limits.h>

#if UCHAR_MAX == 255
typedef unsigned char	BYTE;
#else
#error Need to find an 8-bit type for BYTE
#endif

#if USHRT_MAX == 65535
typedef unsigned short	WORD;
#else
#error Need to find an 16-bit type for WORD
#endif

/* FASTREG needs to be at least 16 bits wide and efficient for the
   host architecture */

#if UINT_MAX >= 65535
typedef unsigned short	FASTREG;
#else
typedef unsigned long	FASTREG;
#endif

/* FASTWORK needs to be wider than 16 bits and efficient for the host
   architecture */
#if UINT_MAX > 65535
typedef unsigned short	FASTWORK;
#else
typedef unsigned long	FASTWORK;
#endif


/*-------------------------------- definitions for memory space --------*/

#define Z80MEMSIZE 64		/* logical Addressspace of the Z80 */

#ifndef MEMSIZE			/* if MEMSIZE are not given */
 #ifdef MMU
  #define MEMSIZE 8*Z80MEMSIZE	/* default with MMU: MEMSIZE = 8*Z80MEMSIZE */
 #else
  #define MEMSIZE Z80MEMSIZE	/* default without MMU: MEMSIZE = Z80MEMSIZE */
 #endif
#endif

extern BYTE ram[];	/* RAM which is present */

/*---------------------------------- definitions for MMU tables --------*/

#ifdef MMU

 #ifndef MMUTABLES
  #define MMUTABLES 8			  /* default: 8 MMU page tables */
 #endif

 #ifndef PAGESIZE
  #define PAGESIZE 4			/* Pagesize 4 KByte */
 #endif			     /* if you want to modify then it is also	*/
			     /* nessasary to modify the section		*/
			     /* "central definitions for memory access"	*/

 #define RAMPAGES MEMSIZE/PAGESIZE		/* No. of pages of RAM  */

 #define MMUPAGEPOINTERS Z80MEMSIZE/PAGESIZE	/* No. of Page-Pointers */

 typedef struct {
	BYTE *page[MMUPAGEPOINTERS];	/* page table for Z80 (64KB)	*/
 } pagetab_struct;

 extern pagetab_struct MMUtable[MMUTABLES];/* MMU page tables		    */
 extern pagetab_struct *mmu;		  /* Pointers to one MMU-pagetable */
 extern pagetab_struct *dmmu;		 /* ^ to destination MMU-pagetbl  */
 extern pagetab_struct *mmuget,*mmuput; /* Pointer for get/put		 */
 extern int mmutab;		       /* selected MMU-pagetable	*/

							  /* choose	*/
 #define ChooseMMUtab(mmut)  mmu=&MMUtable[mmutab=mmut]  /* mmutable	*/

 #define PP(cp) (ram + ((cp)<<12))	/* calculate page pointer	*/

#endif


/*----------------------- central definitions for memory access -----------*/

#ifdef MMU
 /* The following macros are nessasary for Access to Memory through a
    MMU-table : */
					/* normale access to Memory: */
 #define RAM(a)		 *( (mmu->page[((a)&0xffff)>>12]) + ((a)&0x0fff) )

				/* normale access to Memory with MMUpointer: */
 #define MRAM(xmmu,a)	 *((xmmu->page[((a)&0xffff)>>12]) + ((a)&0x0fff) )


					/* after access make a++ */
 #define RAM_pp(a)	 *( (mmu->page[((a)&0xffff)>>12]) + ((a++)&0x0fff) )
				/* with MMUpointer; after access make a++ */
 #define MRAM_pp(xmmu,a) *((xmmu->page[((a)&0xffff)>>12]) + ((a++)&0x0fff) )


					/* after access make a-- */
 #define RAM_mm(a)	 *( (mmu->page[((a)&0xffff)>>12]) + ((a--)&0x0fff) )
				/* with MMUpointer; after access make a-- */
 #define MRAM_mm(xmmu,a) *((xmmu->page[((a)&0xffff)>>12]) + ((a--)&0x0fff) )


					/* bevor access make --a */
 #define mm_RAM(a)	 *( (mmu->page[((--a)&0xffff)>>12]) + ((a)&0x0fff) )
					/* bevor access make --a */
 #define mm_MRAM(xmmu,a) *((xmmu->page[((--a)&0xffff)>>12]) + ((a)&0x0fff) )

#else
 /* ram access without MMU, like Version 1.05/1.06/1.10 */
 #define RAM(a)		 ram[(a)&0xffff]
 #define MRAM(xmmu,a)	 ram[(a)&0xffff]

 #define RAM_pp(a)	 ram[(a++)&0xffff]
 #define MRAM_pp(xmmu,a) ram[(a++)&0xffff]

 #define RAM_mm(a)	 ram[(a--)&0xffff]
 #define MRAM_mm(xmmu,a) ram[(a--)&0xffff]

 #define mm_RAM(a)	 ram[(--a)&0xffff]
 #define mm_MRAM(xmmu,a) ram[(--a)&0xffff]
#endif

/* Some important macros. They are the interface between an access from
   the simz80-/yaze-Modules and the method of the memory access: */

#define GetBYTE(a)	RAM(a)
#define GetBYTE_pp(a)	RAM_pp(a)
#define GetBYTE_mm(a)	RAM_mm(a)
#define mm_GetBYTE(a)	mm_RAM(a)
#define PutBYTE(a, v)	RAM(a) = v
#define PutBYTE_pp(a,v)	RAM_pp(a) = v
#define PutBYTE_mm(a,v)	RAM_mm(a) = v
#define GetWORD(a)	(RAM(a) | (RAM((a)+1) << 8))

/* don't work: #define GetWORD_pppp(a)	(RAM_pp(a) + (RAM_pp(a) << 8)) */
/* make once more a try at 18.10.1999/21:45 ... with the following macro:  */
/* works also not #define GetWORD_pppp(a) (RAM_pp(a) | (RAM_pp(a) << 8))  */
/* I dont know what the optimizer do with such macro.
   If someone knows about it - I'am very interessed to that knowledge.
 */

#define PutWORD(a, v)							\
    do { RAM(a) = (BYTE)(v);						\
	 RAM((a)+1) = (v) >> 8;						\
     } while (0)


/*------------------- Some macros for manipulating Z80-memory : -------*/

#define memcpy_get_z(d,s,n)						\
    do { size_t len = n;						\
	 size_t source = s;						\
         BYTE *p1 = d;							\
         while (len--) *p1++ = GetBYTE_pp(source);			\
    } while (0)


#define memcpy_M_get_z(xm,d,s,n)					\
    do { size_t len = n;						\
	 size_t source = s;						\
	 BYTE *p1 = d;							\
	 while (len--) *p1++ = MRAM_pp(xm,source);			\
    } while (0)


#define memcpy_put_z(d,s,n)						\
    do { size_t len = n;						\
	 size_t dest = d;						\
         BYTE *p1 = s;							\
         while (len--) PutBYTE_pp(dest,*p1++);				\
    } while (0)


#define memcpy_M_put_z(xm,d,s,n)					\
    do { size_t len = n;						\
	 size_t dest = d;						\
         BYTE *p1 = s;							\
	 while (len--) MRAM_pp(xm,dest) = *p1++;			\
    } while (0)


#define memset_M_z(xm,p,v,n)						\
    do { size_t len = n;						\
         while (len--) MRAM_pp(xm,p) = v;				\
    } while (0)


#define memset_z(p,v,n)							\
    do { size_t len = n;						\
         while (len--) PutBYTE_pp(p,v);					\
    } while (0)


/*-------------------- Some macros for unix-memory operations : -------*/

#ifdef BSD
#if defined(sun)
#include <memory.h>
#include <string.h>
#endif
#ifndef strchr
#define strchr index
#endif
#ifndef strrchr
#define strrchr rindex
#endif
#define memclr(p,n)     bzero(p,n)
#define memcpy(t,f,n)   bcopy(f,t,n)
#define memcmp(p1,p2,n) bcmp(p1,p2,n)
#define memset(p,v,n)                                                   \
    do { size_t len = n;                                                \
         char *p1 = p;                                                  \
         while (len--) *p1++ = v;                                       \
    } while (0)
#else
#include <string.h>
#define memclr(p,n)     (void) memset(p,0,n)
#endif


/*-------------------------------------------- prototyping -------------*/
extern void initMEM();
extern void initMMU();
extern void loadMMU();
extern void printMMU();
