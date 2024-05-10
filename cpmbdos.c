/************************************************************************/
/*																		*/
/*	  		   CP/M Hardware Emulator Card Support Program				*/
/*			  				   BDOS Part								*/
/*						 CPM-BDOS.C Ver 1.20							*/
/*	      		 Copyright (c) By Chen Chi-Jung NTUEE 1988				*/
/*						All Right Reserved								*/
/*																		*/
/************************************************************************/
/*
** Include File Defination
*/
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
//#include <conio.h>
//#include <bios.h>
//#include <dos.h>
//#include <io.h>
#include <string.h>
#include <signal.h>

#include "cpmemu.h"
#include "cpmglob.h"

/*
**  Grobol Variable Used in This File only
*/
static UINT8 user_code = 0;
//static struct find_t search_file;	   /* structure for Findfirst & Findnext */
static char tmp1[128];		      /* R/W File Data Buffer  */
static char filename[15];		   /* Filename Storage space */
static UINT32 offset = 0L;		      /* File Seek Offset for File R/W  */
static ldiv_t result;
/*----------------------------------------------------------------------*/
/* filloffset: Return Offset Value indicates by FCB pointer by HL       */
/*     offset = (fcbrl * 256 + fcbcr) * 128				*/
/*------------------ztr----------------------------------------------------*/
void filloffset(UINT32 *offset)
{
	UINT8 *ptr1;
	UINT8 *ptr2;
	char *ptr3;

	*offset = 0L;
	ptr1 = fcbrl + *regde;
	ptr2 = fcbcr + *regde;
	ptr3 = (char *)offset;
	ptr3++;
	*ptr3++ = *ptr2;
	*ptr3 = *ptr1;
	*offset >>= 1;
	return;
}
/*----------------------------------------------------------------------*/
/* fcbtofilename:							*/
/*	 Input: ptr1: pointer indicate the filename in a FCB		*/
/*		ptr2: pointer to a buffer				*/
/*	Output: Filename in the buffer pointer by ptr2			*/
/*----------------------------------------------------------------------*/
void fcbtofilename(char *ptr1,char *ptr2)
{
	char i;

	/* if (*ptr1 != 0x00) { *ptr2++ = (*ptr1++)+64; *ptr2++ = ':'; }
	else*/
	ptr1++;
	for (i = 0 ; i < 8 ; i++)
		if(*ptr1 != ' ') *ptr2++ = *ptr1++;
		else ptr1++;
	*ptr2++ = '.';
	for (i = 0 ; i < 3 ; i++)
		if(*ptr1 != ' ') *ptr2++ = *ptr1++;
		else ptr1++;
	*ptr2 = 0x00;
        return;
}
/*----------------------------------------------------------------------*/
/*  opensuccess: If we open a File required by CP/M Successfully,       */
/*	       Then we call this function to fill the table.	  */
/*----------------------------------------------------------------------*/
void opensuccess(UINT8 fcbptr)
{
	UINT8 *ptr1;

	ptr1 = fcbcr + (*regde);
	*ptr1 = 0x00;				/* Fill cr  =  0 */
	ptr1 = fcbrl + (*regde);
	*ptr1 = 0x00;				/* Fill rl  =  0 */
	fseek(fcbfile[fcbptr],0L,SEEK_END);	/* Move to EOF */
	fcbfilelen[fcbptr] = ftell(fcbfile[fcbptr]); /* Fill File Length */
	rewind(fcbfile[fcbptr]);		   /* Restore file */
	strcpy((char *)fcbused[fcbptr],filename);	  /* Fill Filename to table */
        return;
}
/*----------------------------------------------------------------------*/
void PrintDebug(void)
{
	char name[20],newname[20];
	if (lastcall == *regc) repeats++;
	else {
		if ( *regc >= 15 ) {
			fcbtofilename((char *)fcbdn + *regde, name);
			fcbtofilename((char *)fcbrc + *regde + 1, newname);
		}
		if (lastcall < 39) {
			fprintf(lpt,"% 5d  %s%s\n",
				repeats, debugmess1, debugmess2);

                    printf("%5d  %s%s\n", repeats, debugmess1, debugmess2);
                    fflush(stdout);
                }
		sprintf(debugmess1,"%04XH    %04XH     %04XH   %02XH "
				,*regip,(*regde & 0xFFFF),(dmaaddr & 0xFFFF),*regc);
		switch ( *regc ) {
			case 0: sprintf(debugmess2,"System reset ");
				break;
			case 1: sprintf(debugmess2,"Console input");
				break;
			case 2: sprintf(debugmess2,"Console output");
				break;
			case 3: sprintf(debugmess2,"Reader input");
				break;
			case 4: sprintf(debugmess2,"Punch output");
				break;
			case 5: sprintf(debugmess2,"List output");
				break;
			case 6: sprintf(debugmess2,"Direct console i/o");
				break;
			case 7: sprintf(debugmess2,"Get iobyte");
				break;
			case 8: sprintf(debugmess2,"Set iobyte");
				break;
			case 9: sprintf(debugmess2,"Print string");
				break;
			case 10: sprintf(debugmess2,"Read console buffer");
				 break;
			case 11: sprintf(debugmess2,"Get console status");
				 break;
			case 12: sprintf(debugmess2,"Get version number");
				 break;
			case 13: sprintf(debugmess2,"Reset disk system");
				 break;
			case 14: sprintf(debugmess2,"Select disk");
				 break;
			case 15: sprintf(debugmess2,"Open file %s",name);
				 break;
			case 16: sprintf(debugmess2,"Close file %s",name);
				 break;
			case 17: sprintf(debugmess2,"Search for first");
				 break;
			case 18: sprintf(debugmess2,"Search for next");
				 break;
			case 19: sprintf(debugmess2,"Delete file %s",name);
				 break;
			case 20: sprintf(debugmess2,"Read sequential %s"
					,name);
				 break;
			case 21: sprintf(debugmess2,"Write sequential %s"
				 	,name);
				 break;
			case 22: sprintf(debugmess2,"Make file %s",name);
				 break;
			case 23: sprintf(debugmess2,"Rename file %s to %s"
					,name,newname);
				 break;
			case 24: sprintf(debugmess2,"Return login vector");
				 break;
			case 25: sprintf(debugmess2,"Return current disk");
				 break;
			case 26: sprintf(debugmess2,"Set DMA address");
				 break;
			case 27: sprintf(debugmess2,"Get alloc address");
				 break;
			case 28: sprintf(debugmess2,"Write protect disk");
				 break;
			case 29: sprintf(debugmess2,"Get r/o vector");
				 break;
			case 30: sprintf(debugmess2,"Set file attributes");
				 break;
			case 31: sprintf(debugmess2,"Get disk parms");
				 break;
			case 32: sprintf(debugmess2,"Get & set user code");
				 break;
			case 33: sprintf(debugmess2,"Read random %s",name);
				 break;
			case 34: sprintf(debugmess2,"Write random %s",name);
				 break;
			case 35: sprintf(debugmess2,"Compute file size");
				 break;
			case 36: sprintf(debugmess2,"Set random record");
				 break;
			case 37: sprintf(debugmess2,"Reset drive");
				 break;
			case 38: sprintf(debugmess2,"Write random (zero)");
				 break;
                        default: sprintf(debugmess2,"Unknown bdos call %2XH", *regc);
                                 break;
		}
		repeats = 1;
		lastcall = *regc;
	}
        return;
}
/*----------------------------------------------------------------------*/
void dumpfcb(UINT16 address)
{
    UINT16 addr;
    UINT16 i;

    for (addr = (address) ; addr < address + 0x40; addr += 0x10)
    {
    	printf("%04X: %02X %02X %02X %02X %02X %02X %02X %02X-%02X %02X %02X %02X %02X %02X %02X %02X ",
 	   addr, ram[addr], ram[addr+1], ram[addr+2], ram[addr+3], ram[addr+4], ram[addr+5], ram[addr+6], ram[addr+7],
   	   ram[addr+8], ram[addr+9], ram[addr+10], ram[addr+11], ram[addr+12], ram[addr+13], ram[addr+14], ram[addr+15]);
   	for (i = 0; i <= 15; i++)
   	{
   	    if (ram[addr+i] >= 0x20) printf("%c", ram[addr+i]);
   	    else printf(".");
   	}
   	printf("\n");
    	fflush(stdout);
    }
    printf("\n"); fflush(stdout);
    return;
}

/*----------------------------------------------------------------------*/
/* Hexdump API for debug purpose                                        */
/*----------------------------------------------------------------------*/
#define HD_COLUMN_MASK          0xff
#define HD_DELIM_MASK           0xff00
#define HD_OMIT_COUNT           (1 << 16)
#define HD_OMIT_HEX             (1 << 17)
#define HD_OMIT_CHARS           (1 << 18)
void
hexdump(const void *ptr, int length, const char *hdr, int flags)
{
        int i, j, k;
        int cols;
        const unsigned char *cp;
        char delim;

        if ((flags & HD_DELIM_MASK) != 0)
                delim = (flags & HD_DELIM_MASK) >> 8;
        else
                delim = ' ';

        if ((flags & HD_COLUMN_MASK) != 0)
                cols = flags & HD_COLUMN_MASK;
        else
                cols = 16;

        cp = ptr;
        for (i = 0; i < length; i+= cols) {
                if (hdr != NULL)
                        printf("%s", hdr);

                if ((flags & HD_OMIT_COUNT) == 0)
                        printf("%04x  ", i);

                if ((flags & HD_OMIT_HEX) == 0) {
                        for (j = 0; j < cols; j++) {
                                k = i + j;
                                if (k < length)
                                        printf("%c%02x", delim, cp[k]);
                                else
                                        printf("   ");
                        }
                }

                if ((flags & HD_OMIT_CHARS) == 0) {
                        printf("  |");
                        for (j = 0; j < cols; j++) {
                                k = i + j;
                                if (k >= length)
                                        printf(" ");
                                else if (cp[k] >= ' ' && cp[k] <= '~')
                                        printf("%c", cp[k]);
                                else
                                        printf(".");
                        }
                        printf("|");
                }
                printf("\n");
        }
}
/*----------------------------------------------------------------------*/
/* CPM BDOS API Services                                                */
/*----------------------------------------------------------------------*/
void initialbdos(void)
{
    return;
}
/*----------------------------------------------------------------------*/
void bdos00(void)	/* system reset -not ready */
{
	dmaaddr = 0x0080;		 	/* Set DMA address to 80h */
	*eop = 1;			 		/* End of execution */
    return;
}
/*----------------------------------------------------------------------*/
void bdos01(void)	/* console input */
{

// #dchen need to find how unix handle keyboard io
    char ch;
    ch=getchar();
 	*regl = *rega = ch;

 	ReturnZ80;
 	return;

#if 0 // #dos style
	UINT16 keypress;
	UINT8 ch;

	keypress = _bios_keybrd(_KEYBRD_READ);
	ch = (char)(keypress & 0x00ff);
	*regl = *rega = ch;
	if (*rega == 0x03) {puts("^C");longjmp(ctrl_c,0);}
	ReturnZ80;
	switch (ch) {
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0d: putchar(ch); break;
		  default: if (ch < 32) {putchar('^');putchar(ch+64);}
				  else putchar(ch);
	}
	fflush(stdout);
        return;
#endif

}
/*----------------------------------------------------------------------*/
void bdos02(void)	    /* character output */
{

	putchar(*rege);
	ReturnZ80;
	return;
}
/*----------------------------------------------------------------------*/
void bdos03(void)		/* reader input */
{
	ReturnZ80;
        return;
}
/*----------------------------------------------------------------------*/
void bdos04(void)	    /* punch output */
{
	ReturnZ80;
        return;
}
/*----------------------------------------------------------------------*/
void bdos05(void)	    /* list output */
{
	UINT16 ch;

	ch = (UINT16)*rege;
	ReturnZ80;
//	_bios_printer(_PRINTER_WRITE,0,ch);
        return;
}
/*----------------------------------------------------------------------*/
void bdos06(void)	    /* direct console i/o -not check!*/
{
#if 0
	union REGS regs;

	regs.h.dl = *rege;
	regs.h.ah = 0x06;
	int86(0x21,&regs,&regs);
	*regl = *rega = regs.h.al;
#endif
	ReturnZ80;

        return;
/*
	if ( (*regde & 0x00ff) == 0xff ) {
		tmp = _bios_keybrd(_KEYBRD_READY);
		if (tmp != 0) tmp = _bios_keybrd(_KEYBRD_READ);
		*ch = (char)tmp;
	}
	else putch( *regde & 0x00ff);
*/
}
/*----------------------------------------------------------------------*/
void bdos07(void)	    /* get i/o byte */
{
	char *ptr1;

	ptr1 = (char *)(&ram[0x0003]);
	*rega = *ptr1;
	ReturnZ80;
        return;
}
/*----------------------------------------------------------------------*/
void bdos08(void)	    /* set i/o byte */
{
	char *ptr1;

	ptr1 = (char *)(&ram[0x0003]);
	*ptr1 = *rege;
	ReturnZ80;
        return;
}
/*----------------------------------------------------------------------*/
void bdos09(void)	    /* print string */
{

	char *addr;

	addr = (char *)&ram[(*regde)];
	do {
		putchar(*addr++);
	} while (*addr != '$');
	ReturnZ80;
        return;
}
/*----------------------------------------------------------------------*/

void bdos10(void)	    /* read console buffer */
{
	char *ptr;
	char *ptr2;
	UINT8 strln = 0;
	char tmpbuffer[256];

    if(gear_fgets(tmpbuffer,sizeof(tmpbuffer), stdin, 1))
    {
        ptr2 = tmpbuffer;
        ptr = (char *)(ram + *regde+2);
        do {
            *ptr = *ptr2++;
            strln++;
        }while ( *ptr++ != 0x00);
        *(--ptr) = 0X0D;
        *(++ptr) = 0x00;

        ptr = (char *)(ram + *regde + 1);
        *ptr = strln - 1;
	}
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/


#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}
void bdos11(void)   /* read console status */
{
	int tmp;

	tmp = kbhit();
	if (tmp  != 0) tmp = 0xff;
	*regl = *rega = (char)tmp;

	ReturnZ80;
    return;

}
/*----------------------------------------------------------------------*/
void bdos12(void)	    /* get version number */
{
	*reghl = 0X0022;
    ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos13(void)	     /* reset disk system */
{
	dmaaddr = 0x0080;
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos14(void)	     /* select disk */
{
/*
	UINT16 disknumber;
	UINT16 *drives;

	disknumber = (*regde & 0x00ff)+1;
	_dos_setdrive(disknumber,drives);
*/

	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos15(void)	/* open file by fcb filename */
{
	UINT8 fcbptr = 0;

/*	dumpfcb(*regde);  */ /* djgpp */
	fcbtofilename((char *)&ram[*regde] ,filename);
	//printf("bdos15() open filename=%s\n", filename);

	while (strcasecmp((char *)fcbused[fcbptr],filename) != 0 &&
		fcbused[fcbptr][0] != 0	&& fcbptr <= MAXFILE) fcbptr++;
        //printf("bdos15() fcbptr=%d\n",fcbptr);
	if ( fcbptr >MAXFILE ) *regl = *rega = 0xFF;
	else if ( *filename == 0x00 ) *regl = *rega = 0xff; /*Null Filename*/
	else if ( strcasecmp((char *)fcbused[fcbptr],filename) == 0 )
		{opensuccess(fcbptr);*regl = *rega = 0x00;}
	else if ( (fcbfile[fcbptr] = fopen(filename,"r+b")) == NULL )
		*regl = *rega = 0XFF;
	else {
		*regl = *rega = 0x00;
		opensuccess(fcbptr);
	}
        //printf("bdos15() Return REGA = %02x\n",*rega);
	ReturnZ80;
        return;
}
/*----------------------------------------------------------------------*/
void bdos16(void)      /* close file */
{
	UINT8 fcbptr = 0;

	fcbtofilename((char *)fcbdn + *regde,filename);

  	//printf("bdos16() close filename=%s\n", filename);
	while (strcasecmp((char *)fcbused[fcbptr],filename) != 0 &&
		fcbptr <= MAXFILE) fcbptr++;
        //printf("bdos16() fcbptr=%d\n",fcbptr);
	if (fcbptr>MAXFILE) *rega  = 0xff;
	else if (fcbfile == NULL) *rega = 0xff;
	else {
		if (fclose(fcbfile[fcbptr]) == 0) {
			*rega = 0x00;
			fcbused[fcbptr][0] = 0;
			fcbfile[fcbptr] = NULL;
		}
		else *regaf |= 0xff00;		/* *rega = 0xff */
	}
        //printf("bdos16() Return REGA = %02x\n",*rega);
        //printf("-------------------------------------------\n");
        //printf("Dump Close File %s\n", filename);
        //char cmd[80];
        //sprintf(cmd,"cat '%s'",filename);
        //system(cmd);
        //printf("-------------------------------------------\n");
	ReturnZ80;
        return;
}
/*----------------------------------------------------------------------*/
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>

static DIR *dir;
static struct dirent *dp;

void bdos17(void)	 /* search for first */
{

	fcbtofilename((char *)fcbdn + *regde,filename);

    if ((dir = opendir(".")) == NULL) {
        *rega = 0xff;
    } else {
        *rega = 0x00;
        fillfcb((char *)(ram+dmaaddr),dp->d_name);
    }

	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos18(void)	 /* search for next */
{

    if ((dp = readdir(dir)) == NULL) {
        *regaf |= 0xff00;
        closedir(dir);
    } else {
        *rega = 0x00;
        fillfcb((char *)(ram+dmaaddr),dp->d_name);
    }
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos19(void)		 /* delete file */
{
	UINT8 fcbptr = 0;

	fcbtofilename((char *)fcbdn + *regde,filename);

        //printf("bdos19() delete filename=%s\n", filename);

        // Find if user want to delete an already opened file,
        // We must close it before delete. Otherwise further
        // make file won't take effect. (It seems MSDOS will do it for us
        // but Linux don't)
        // Found in Linux Porting with HITECH C, 2014/3/27

	while (strcasecmp((char *)fcbused[fcbptr],filename) != 0 &&
		fcbptr <= MAXFILE) fcbptr++;
        //printf("bdos19() fcbptr=%d\n",fcbptr);

        // already opened? close it before delete.
	if (fcbptr<=MAXFILE)
        {
             //printf("bdos19() delete an opened file. close it before delete!\n");
             if (fclose(fcbfile[fcbptr]) != 0) printf("Oops! can't close file before delete!\n");
             fcbused[fcbptr][0] = 0;
             fcbfile[fcbptr] = NULL;
        }

        // delete file
        *rega = (char)remove(filename);

        //printf("bdos19() Return REGA = %02x\n",*rega);
 	ReturnZ80;
        return;
}
/*----------------------------------------------------------------------*/
void bdos20(void)				/* squential read */
{
	char *ptr1;
	char *ptr2;
	UINT8 i;
	UINT32 l;
	UINT8 fcbptr = 0;

	fcbtofilename((char *)fcbdn + *regde,filename);
	while (strcasecmp((char *)fcbused[fcbptr],filename) != 0 &&
		fcbptr <= MAXFILE) fcbptr++;
	filloffset(&offset);
        //printf("OFFSET = %ld, FCBFileLen=%ld\n", offset, fcbfilelen[fcbptr]);
	ptr1 = (char *)( ram + dmaaddr);
	if ( offset  >= fcbfilelen[fcbptr] ) *rega = 0x01;
	else if ( fcbptr >MAXFILE) *rega = 0xff;
	else if ( fseek(fcbfile[fcbptr],offset,SEEK_SET) != 0 ) *rega = 0xff;
							      /* read fail */
	else {
		fread(tmp1,1,128,fcbfile[fcbptr]);
		l =  fcbfilelen[fcbptr] - offset;
		if ( l < 128L )
		tmp1[ (int)l ] = 0x1a;	/* add CP/M EOF char 0x1a */
			for (i = 0;i<128;i++) { *ptr1++ = *(tmp1+i); }
			*regl = *rega = 0x00;
			ptr1 = (char *)fcbcr + *regde;
			ptr2 = (char *)fcbrl + *regde;
			(*ptr1)++;
			if (*ptr1 == 0) (*ptr2)++;
               //hexdump((char *)ram + dmaaddr, 128, NULL, 0);
	}
        //printf("Return REGA = %02x\n",*rega);
	ReturnZ80;
        return;
}
/*----------------------------------------------------------------------*/
void bdos21(void)		/* write sequential */
{
	char *ptr1;
	char *ptr2;
	UINT8 i;
	UINT8 fcbptr = 0;

	fcbtofilename((char *)fcbdn + *regde,filename);

	while (strcasecmp((char *)fcbused[fcbptr],filename) != 0 &&
		fcbptr <= MAXFILE) fcbptr++;

	filloffset(&offset);
	ptr1 = (char *)(ram + dmaaddr);
	if ( fcbptr >MAXFILE) { *rega = 0xff; ReturnZ80; }
	else if ( fseek(fcbfile[fcbptr],offset,SEEK_SET)  != 0 ) {
		*rega = 0xff;ReturnZ80;  /* write fail */
	}
	else {
		for (i = 0;i<128;i++) *(tmp1+i) = *ptr1++;
		*rega = 0x00;
		ptr1 = (char *)fcbcr + *regde;
		ptr2 = (char *)fcbrl + *regde;
		(*ptr1)++;
		if (*ptr1 == 0) (*ptr2)++;
		ReturnZ80;
		if (offset >= fcbfilelen[fcbptr])
			fcbfilelen[fcbptr] = offset+128;
		fwrite(tmp1,1,128,fcbfile[fcbptr]);
	}
     	ReturnZ80;
        return;
}
/*----------------------------------------------------------------------*/
void bdos22(void)		/* make file by fcb filename */
{
	UINT8 fcbptr = 0;\

	fcbtofilename((char *)fcbdn + *regde,filename);

	//printf("bdos22() make filename=%s\n", filename);

	while (strcasecmp((char *)fcbused[fcbptr],filename) != 0 &&
		fcbused[fcbptr][0] != 0 && fcbptr <= MAXFILE) fcbptr++;
    //printf("bdos22() fcbptr=%d\n",fcbptr);

    if ( fcbptr > MAXFILE )  *rega = 0xFF;
	else if ( strcasecmp((char *)fcbused[fcbptr],filename) == 0 ) {
		opensuccess(fcbptr);*rega = 0x00;
	}
	else if ( (fcbfile[fcbptr] = fopen(filename,"w+b")) == NULL )
		*rega = 0XFF;
	else {
		*rega = 0x00;
		opensuccess(fcbptr);
	}
    //printf("bdos22() Return REGA = %02x\n",*rega);
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos23(void)		/*  rename file  */
{
	char oldname[15];
	char newname[15];

	fcbtofilename((char *)fcbdn + *regde,oldname);
	fcbtofilename((char *)fcbrc + *regde+1,newname);
	*rega = (char)rename(oldname,newname);
	if (*rega != 0x00) *rega = 0xff;
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos24(void)		/* return login vector ---not checked */
{
	*reghl = 0x0007;
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos25(void)		/* return currect disk -- not checked! */
{
#if 0
	union REGS regs;

	regs.h.ah = 0x19;
	int86(0x21,&regs,&regs);
	*rega = regs.h.al;
#endif
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos26(void)		/* set dma address */
{
	dmaaddr = *regde;
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos27(void)		/* get alloc address */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos28(void)		/* write protect disk */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos29(void)		/* get r/o vector */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos30(void)		/* set file attributes */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos31(void)		/* get disk parms */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos32(void)		/* get and set user code --not ready */
{
/*	printf("bdos32() *rege= %02x, user_code=%02x\n", *rege, user_code); */
	if (*rege == (UINT8)0xff) *rega = user_code;
	else user_code = *rege;
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos33(void)		/* read random */
{
	char  *ptr1;
	UINT16 *ptr2;
	UINT8 i;
	UINT32 l;
	UINT8 fcbptr = 0;

	fcbtofilename((char *)fcbdn + *regde,filename);

	while (strcasecmp((char *)fcbused[fcbptr],filename) != 0 &&
		fcbptr <= MAXFILE) fcbptr++;

	ptr1 = (char *)( ram + dmaaddr);
	ptr2 = (UINT16 *)(fcbln + *regde);
	offset = (UINT32)(*ptr2) * 128L;
	if ( (char)*(fcbln + *regde+2) != 0x00) *rega = 0x06;
	else if ( offset > fcbfilelen[fcbptr] ) *rega = 0x04;
	else if ( fcbptr >MAXFILE) *rega = 0x05;
	else if ( fseek(fcbfile[fcbptr],offset,SEEK_SET) != 0 ) *rega = 0x03;
							      /* seek fail */
	else {
		fread(tmp1,1,128,fcbfile[fcbptr]);
		l =  fcbfilelen[fcbptr] - offset;
		if ( l < 128L )
                {
                  for (i = (UINT8)l; i < 128; i++)
                  {
                      tmp1[i] = 0x1a;	/* add CP/M EOF chars 0x1a */
                  }
                }
		for (i = 0 ; i < 128 ; i++) *ptr1++ = *(tmp1+i);
		if ( offset == fcbfilelen[fcbptr] ) *rega = 0x01;
		else *rega = 0x00;
	}
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos34(void)		/* write random */
{
	char *ptr1;
	UINT16 *ptr2;
	UINT8 i;
	UINT8 fcbptr = 0;

	fcbtofilename((char *)fcbdn + *regde,filename);
	while (strcasecmp((char *)fcbused[fcbptr],filename) != 0 && fcbptr <= MAXFILE)
		fcbptr++;
	ptr1 = (char *)( ram + dmaaddr);
	ptr2 = (UINT16 *)(fcbln + *regde);
	offset = (UINT32)( *ptr2 ) * 128L;
	if ( (char)*(fcbln + *regde+2) != 0x00) { *rega = 0x06; ReturnZ80;}
	else if ( fcbptr >MAXFILE)  { *rega = 0x05; ReturnZ80; }
	else if ( fseek(fcbfile[fcbptr],offset,SEEK_SET) != 0 )
		{ *rega = 0x03;ReturnZ80; } /* seek fail */
	else {
		for (i = 0 ; i < 128 ; i++) *(tmp1+i) = *ptr1++;
		*rega = 0x00;
		ReturnZ80;
		fwrite(tmp1,1,128,fcbfile[fcbptr]);
		if (offset >= fcbfilelen[fcbptr])
			fcbfilelen[fcbptr] = offset + 128;
	}
    return;
}
/*----------------------------------------------------------------------*/
void bdos35(void)		/* compute file size -- not checked*/
{
	char  *ptr1;
	UINT16 *ptr3;
	UINT8 fcbptr = 0;

	fcbtofilename((char *)fcbdn + *regde,filename);
	while (strcasecmp((char *)fcbused[fcbptr],filename) != 0 &&
		fcbused[fcbptr][0] != 0	&& fcbptr <= MAXFILE) fcbptr++;
	if ( (fcbfile[fcbptr] = fopen(filename,"r+b")) != NULL ) {
		opensuccess(fcbptr);
		result = ldiv((long)(fcbfilelen[fcbptr]),128L);
		if (result.rem != 0) result.quot++;
		if (result.quot >65535L ) {
			ptr1 = (char *)fcbln + *regde;
			*ptr1++ = 0x00;
			*ptr1++ = 0x00;
			*ptr1++ = 0x01;
		}
		else {
			ptr3 = (UINT16 *)(fcbln + *regde);
			*ptr3++ = (UINT16)result.quot;
			*ptr3 &=  0xff00;
		}
	}
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos36(void)		/* set random record --not checked--have error!*/
{
	char *ptr1;
	char *ptr2;

	ptr1 = (char *)fcbln + *regde;
	ptr2 = (char *)fcbcr + *regde;
	*ptr1++ = *ptr2;
	ptr2 = (char *)fcbrl + *regde;
	*ptr1 = *ptr2;
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos37(void)		/* reset drive */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
void bdos38(void)		/* write random (zero) */
{
	ReturnZ80;
    return;
}
/*----------------------------------------------------------------------*/
/* Process CPM BDOS Services API                                        */
/*----------------------------------------------------------------------*/
void cpmbdos(void)
{
	if (DebugFlag == 1 && lpt != NULL) PrintDebug();
	switch ( *regc ) {
		case 0: bdos00(); break;       /* system reset */
		case 1: bdos01(); break;       /* console input */
		case 2: bdos02(); break;       /* console output */
		case 3: bdos03(); break;       /* reader input */
		case 4: bdos04(); break;       /* punch output */
		case 5: bdos05(); break;       /* list output */
		case 6: bdos06(); break;       /* direct console i/o */
		case 7: bdos07(); break;       /* get iobyte */
		case 8: bdos08(); break;       /* set iobyte */
		case 9: bdos09(); break;       /* print string */
		case 10: bdos10(); break;      /* read console buffer */
		case 11: bdos11(); break;      /* get console status */
		case 12: bdos12(); break;      /* get version number */
		case 13: bdos13(); break;      /* reset disk system */
		case 14: bdos14(); break;      /* select disk */
		case 15: bdos15(); break;      /* open file */
		case 16: bdos16(); break;      /* close file */
		case 17: bdos17(); break;      /* search for first */
		case 18: bdos18(); break;      /* search for next */
		case 19: bdos19(); break;      /* delete file */
		case 20: bdos20(); break;      /* read sequential */
		case 21: bdos21(); break;      /* write sequential */
		case 22: bdos22(); break;      /* make file */
		case 23: bdos23(); break;      /* rename file */
		case 24: bdos24(); break;      /* return login vector */
		case 25: bdos25(); break;      /* return current disk */
		case 26: bdos26(); break;      /* set dma address */
		case 27: bdos27(); break;      /* get alloc address */
		case 28: bdos28(); break;      /* write protect disk */
		case 29: bdos29(); break;      /* get r/o vector */
		case 30: bdos30(); break;      /* set file attributes */
		case 31: bdos31(); break;      /* get disk parms */
		case 32: bdos32(); break;      /* get & set user code */
		case 33: bdos33(); break;      /* read random */
		case 34: bdos34(); break;      /* write random */
		case 35: bdos35(); break;      /* compute file size */
		case 36: bdos36(); break;      /* set random record */
		case 37: bdos37(); break;      /* reset drive */
		case 38: bdos38(); break;      /* write random (zero) */
		default: *reghl = 0; ReturnZ80;
	}
}

