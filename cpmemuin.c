/************************************************************************/
/*                                                                      */
/*             CP/M Hardware Emulator Card Support Program              */
/*                       CPMEMUINT.C Ver 1.45 (QC 2.00 only)            */
/*                 Copyright (c) By C.J.Chen NTUEE 1988                 */
/*                         All Right Reserved                           */
/*                                                                      */
/************************************************************************/

//#include <dos.h>
#include <setjmp.h>
#include <stdio.h>
#include <signal.h>

#include "cpmemu.h"
#include "cpmglob.h"

void handle_ctrl_c(sig_t s)
{
//	_enable();
//	if (in_bios == 1) return;
//	else {
//#ifdef MSC
//		_asm    mov     ax,word ptr stack_segment
//		_asm    mov     ss,ax
//#endif
		longjmp(ctrl_c,0);
//	}
}

