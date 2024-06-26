/************************************************************************/
/*                                                                      */
/*             CP/M Hardware Emulator Card Support Program              */
/*                       CPM-BIOS.C Ver 1.10                            */
/*                 Copyright (c) By C.J.Chen NTUEE 1988                 */
/*                        All Right Reserved                            */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
//#include <conio.h>

#include "cpmemu.h"
#include "cpmglob.h"

/*----------------------------------------------------------------------*/
void initialbios(void)
{
    return;
}

/*----------------------------------------------------------------------*/
void bios01(void)        /* cold boot */
{
	dmaaddr = 0x0080;
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios02(void)        /* warm boot */
{
	dmaaddr = 0x0080;
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios03(void)        /* console status */
{
//	*rega = (char)_bios_keybrd(_KEYBRD_READY); #dchen to be implemented
	int tmp;
	extern int kbhit(void);

	tmp = kbhit();
	if (tmp  != 0) tmp = 0xff;
	*regl = *rega = (char)tmp;
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios04(void)        /* console input */
{
	static char keybuf = 0;
	unsigned keypress;
	char *ptr1;
#if 0
	if (keybuf == 0x00)
		{
			keypress = _bios_keybrd(_KEYBRD_READ);
			if ((keypress) == 0x2e03) *rega = 0x03;
			else if ((keypress) == 0x0300) *rega = 0x00;
			else
			{
				ptr1 = (char *)&keypress;
				*rega = *ptr1++;
				if (*rega == 0x00) keybuf = *ptr1;
			}
		}
	else
		{
			*rega = keybuf;
			keybuf = 0x00;
		}
#endif
    *rega = getchar(); // #dchen to be modified
    //*rega = getch();
    ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios05(void)            /* console output */
{
    putchar(*rega);        // #dchen to be modified
    return;
}
/*----------------------------------------------------------------------*/
void bios06(void)        /* lister output */
{
//	_bios_printer(_PRINTER_WRITE,0,(UINT16)*rega);
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios07(void)        /* punch output */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios08(void)        /* reader input */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios09(void)        /* home disk */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios10(void)        /* select disk */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios11(void)        /* set track */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios12(void)        /* set sector */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios13(void)        /* set dma */
{
	dmaaddr = *regbc;
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios14(void)        /* read sector */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios15(void)        /* write sector */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bios16(void)        /* list status */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void cpmbios(void)
{
	in_bios = 1;
	switch (*bioscode) {
		case 1:bios01();break;       /* coldboot */
		case 2:bios02();break;       /* warmboot */
		case 3:bios03();break;       /* console status */
		case 4:bios04();break;       /* console input */
		case 5:bios05();break;       /* console output */
		case 6:bios06();break;       /* lister output */
		case 7:bios07();break;       /* punch output */
		case 8:bios08();break;       /* reader input */
		case 9:bios09();break;       /* home disk */
		case 10:bios10();break;      /* select disk */
		case 11:bios11();break;      /* set track */
		case 12:bios12();break;      /* set sector */
		case 13:bios13();break;      /* set dma */
		case 14:bios14();break;      /* read sector */
		case 15:bios15();break;      /* write sector */
		case 16:bios16();break;      /* list status */
		default:ReturnZ80;
	}
    in_bios = 0;
    return;
}

