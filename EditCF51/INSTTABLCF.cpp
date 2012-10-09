/***********************************************************************
 *
 *		INSTTABLECF.C
 *		Instruction Table for Coldfire Assembler
 *
 * Description: This file contains two kinds of data structure declara-
 *		tions: "flavor lists" and the instruction table. First
 *		in the file are "flavor lists," one for each different
 *		instruction. Then comes the instruction table, which
 *		contains the mnemonics of the various instructions, a
 *		pointer to the flavor list for each instruction, and
 *		other data. Finally, the variable tableSize is
 *		initialized to contain the number of instructions in
 *		the table.
 *
 *      Author: Paul McKee
 *		ECE492    North Carolina State University
 *
 *        Date:	12/13/86
 *
 *    Modified: Chuck Kelly
 *              Monroe County Community College
 *              http://www.monroeccc.edu/ckelly
 *
 *    05/2012   Karsten Langeloh - Coldfire version (core V1)
 *              tematik GmbH
 *              http://www.tematik.de
 *
 ************************************************************************/

/*********************************************************************

          HOW THE INSTRUCTION TABLE AND FLAVOR LISTS ARE USED

     The procedure which instLookup() and assemble() use to look up
and verify an instruction (or directive) is as follows. Once the
mnemonic of the instruction has been parsed and stripped of its size
code and trailing spaces, the instLookup() does a binary search on the
instruction table to determine if the mnemonic is present. If it is
not found, then the INV_OPCODE error results. If the mnemonic is
found, then assemble() examines the field parseFlag for that entry.
This flag is true if the mnemonic represents a normal instruction that
can be parsed by assemble(); it is false if the instruction's operands
have an unusual format (as is the case for MOVEM and DC).

      If the parseFlag is true, then assemble will parse the
instruction's operands, check them for validity, and then pass the
data to the proper routine which will build the instruction. To do
this it uses the pointer in the instruction table to the instruction's
"flavor list" and scans through the list until it finds a particular
"flavor" of the instruction which matches the addressing mode(s)
specified.  If it finds such a flavor, it checks the instruction's
size code and passes the instruction mask for the appropriate size
(there are three masks for each flavor) to the building routine
through a pointer in the flavor list for that flavor.

*********************************************************************/


#include <stdio.h>
#include "asm.h"

/* Definitions of addressing mode masks for various classes of references */

#define Data	(DnDirect | AnInd | AnIndPost | AnIndPre | AnIndDisp \
		 | AnIndIndex | AbsShort | AbsLong | PCDisp | PCIndex \
		 | IMMEDIATE)

#define Memory	(AnInd | AnIndPost | AnIndPre | AnIndDisp | AnIndIndex \
		 | AbsShort | AbsLong | PCDisp | PCIndex | IMMEDIATE)

/* (kl) */
#define AddrIndirect (AnInd | AnIndPost | AnIndPre | AnIndDisp )

#define Control	(AnInd | AnIndDisp | AnIndIndex | AbsShort | AbsLong | PCDisp \
		 | PCIndex)

#define ControlAlt (AnInd | AnIndDisp | AnIndIndex | AbsShort | AbsLong )

/* (kl) move source adressing modes for which all dest addressing modes are possible */
#define MoveSrcAll (DnDirect | AnDirect | AnInd | AnIndPost | AnIndPre)

/* (kl) move destination addressing modes for displacement source adressing modes  */
#define MoveDestDisp (Alter ^ (AnIndIndex | AbsShort | AbsLong))

/* (kl) move source modes for index and absolute / immediate */
#define MoveIndexAbs (AnIndIndex | PCIndex | AbsShort | AbsLong | IMMEDIATE)

#define MoveDestIndexAbs (MoveDestDisp ^ AnIndDisp)

/*#define Alter	(DnDirect | AnDirect | AnInd | AnIndPost | AnIndPre \
		 | AnIndDisp | AnIndIndex | AbsShort | AbsLong)
*/
#define Alter	(DnDirect | AnInd | AnIndPost | AnIndPre \
		 | AnIndDisp | AnIndIndex | AbsShort | AbsLong)

#define All      (Data | AnDirect)

// Dn  [An] [An]+ -[An] d[An] d[An,Xi] Abs.W  Abs.L
#define DataAlt	(Data & Alter)

// [An] [An]+ -[An] d[An] d[An,Xi] Abs.W Abs.L
#define MemAlt	(Memory & Alter)

#define Absolute (AbsLong | AbsShort)
#define GenReg	(DnDirect | AnDirect)

// Dn (An) (d,An) (d,An,Xn)  Abs.W  Abs.L  d(PC)  d(PC,Xn)
#define DnControl (DnDirect | Control)

// Dn (An) (d,An) (d,An,Xn) Abs.W Abs.L
#define DnControlAlt (DnDirect | ControlAlt)

/* Define size code masks for instructions that allow more than one size */

#define BW	(BYTE_SIZE | WORD_SIZE)
#define WL	(WORD_SIZE | LONG_SIZE)
#define BWL	(BYTE_SIZE | WORD_SIZE | LONG_SIZE)
#define BL	(BYTE_SIZE | LONG_SIZE)


//---------------------------------------------------------
// Define the "flavor lists" for each different instruction
// The order of the "flavors" is important

flavor addfl[] = {
	{ All, AnDirect, LONG_SIZE, arithReg, 0, 0, 0xD1C0 },             // ADDA <ea>,An
	{ IMMEDIATE, DataAlt, LONG_SIZE, immedInst, 0x0600, 0x0640, 0x0680 }, // ADDI or ADDQ #d,<ea>
     	{ IMMEDIATE, AnDirect, LONG_SIZE, quickMath, 0, 0x5040, 0x5080 },      // ADDQ #d,An
	{ Data, DnDirect, LONG_SIZE, arithReg, 0xD000, 0xD040, 0xD080 },      // ADD <ea>,Dn
	{ AnDirect, DnDirect, LONG_SIZE, arithReg, 0xD000, 0xD040, 0xD080 },   // ADD An,Dn
	{ DnDirect, MemAlt, LONG_SIZE, arithAddr, 0xD100, 0xD140, 0xD180 },   // ADD Dn,<ea>
       };

flavor addafl[] = {
	{ All, AnDirect, LONG_SIZE, arithReg, 0, 0xD0C0, 0xD1C0 }
       };

flavor addifl[] = {
	{ IMMEDIATE, DnDirect, LONG_SIZE, immedInst, 0x0600, 0x0640, 0x0680 }
       };

flavor addqfl[] = {
	{ IMMEDIATE, DataAlt, LONG_SIZE, quickMath, 0x5000, 0x5040, 0x5080 },
	{ IMMEDIATE, AnDirect, LONG_SIZE, quickMath, 0, 0x5040, 0x5080 }
       };

flavor addxfl[] = {
	{ DnDirect, DnDirect, LONG_SIZE, twoReg, 0xD100, 0xD140, 0xD180 }
       };

flavor andfl[] = {
	{ Data, DnDirect, LONG_SIZE, arithReg, 0xC000, 0xC040, 0xC080 },
	{ DnDirect, MemAlt, LONG_SIZE, arithAddr, 0xC100, 0xC140, 0xC180 },
	{ IMMEDIATE, DataAlt, LONG_SIZE, immedInst, 0x0200, 0x0240, 0x0280 }
       };

flavor andifl[] = {
	{ IMMEDIATE, DnDirect, LONG_SIZE, immedInst, 0x0200, 0x0240, 0x0280 }
       };

flavor aslfl[] = {
	{ DnDirect, DnDirect, LONG_SIZE, shiftReg, 0xE120, 0xE160, 0xE1A0 },
	{ IMMEDIATE, DnDirect, LONG_SIZE, shiftReg, 0xE100, 0xE140, 0xE180 }
       };

flavor asrfl[] = {
	{ DnDirect, DnDirect, LONG_SIZE, shiftReg, 0xE020, 0xE060, 0xE0A0 },
	{ IMMEDIATE, DnDirect, LONG_SIZE, shiftReg, 0xE000, 0xE040, 0xE080 }
       };

flavor bccfl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6400, 0x6400, 0x6400 }
       };

flavor bchgfl[] = {
	{ DnDirect, MemAlt, BYTE_SIZE, arithAddr, 0x0140, 0x0140, 0 },
	{ DnDirect, DnDirect, LONG_SIZE, arithAddr, 0, 0x0140, 0x0140 },
	{ IMMEDIATE, MemAlt, BYTE_SIZE, staticBit, 0x0840, 0x0840, 0 },
	{ IMMEDIATE, DnDirect, LONG_SIZE, staticBit, 0, 0x0840, 0x0840 }
       };

flavor bclrfl[] = {
	{ DnDirect, MemAlt, BYTE_SIZE, arithAddr, 0x0180, 0x0180, 0 },
	{ DnDirect, DnDirect, LONG_SIZE, arithAddr, 0, 0x0180, 0x0180 },
	{ IMMEDIATE, MemAlt, BYTE_SIZE, staticBit, 0x0880, 0x0880, 0 },
	{ IMMEDIATE, DnDirect, LONG_SIZE, staticBit, 0, 0x0880, 0x0880 }
       };

flavor bcsfl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6500, 0x6500, 0x6500 }
       };

flavor beqfl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6700, 0x6700, 0x6700 }
       };

flavor bgefl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6C00, 0x6C00, 0x6C00 }
       };

flavor bgtfl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6E00, 0x6E00, 0x6E00 }
       };

flavor bhifl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6200, 0x6200, 0x6200 }
       };

flavor bhsfl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6400, 0x6400, 0x6400 }
       };

flavor bitrevfl[] = {
	{ DnDirect, 0, LONG_SIZE, oneReg, 0, 0x00C0, 0x00C0 }
       };

flavor blefl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6f00, 0x6F00, 0x6F00 }
       };

flavor blofl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6500, 0x6500, 0x6500 }
       };

flavor blsfl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6300, 0x6300, 0x6300 }
       };

flavor bltfl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6d00, 0x6D00, 0x6D00 }
       };

flavor bmifl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6b00, 0x6B00, 0x6B00 }
       };

flavor bnefl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6600, 0x6600, 0x6600 }
       };

flavor bplfl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6a00, 0x6A00, 0x6A00 }
       };

flavor brafl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6000, 0x6000, 0x6000 }
       };

flavor bsetfl[] = {
	{ DnDirect, MemAlt, BYTE_SIZE, arithAddr, 0x01C0, 0x01C0, 0 },
	{ DnDirect, DnDirect, LONG_SIZE, arithAddr, 0, 0x01C0, 0x01C0 },
	{ IMMEDIATE, MemAlt, BYTE_SIZE, staticBit, 0x08C0, 0x08C0, 0 },
	{ IMMEDIATE, DnDirect, LONG_SIZE, staticBit, 0, 0x08C0, 0x08C0 }
       };

flavor bsrfl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6100, 0x6100, 0x6100 }
       };

flavor btstfl[] = {
	{ DnDirect, Memory, BYTE_SIZE, arithAddr, 0x0100, 0x0100, 0 },
	{ DnDirect, DnDirect, LONG_SIZE, arithAddr, 0, 0x0100, 0x0100 },
	{ IMMEDIATE, Memory, BYTE_SIZE, staticBit, 0x0800, 0x0800, 0 },
	{ IMMEDIATE, DnDirect, LONG_SIZE, staticBit, 0, 0x0800, 0x0800 }
       };

flavor bvcfl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6800, 0x6800, 0x6800 }
       };

flavor bvsfl[] = {
	{ Absolute, 0, SHORT_SIZE | LONG_SIZE | BYTE_SIZE | WORD_SIZE, branch, 0x6900, 0x6900, 0x6900 }
       };

flavor byterevfl[] = {
	{ DnDirect, 0, LONG_SIZE, oneReg, 0, 0x02C0, 0x02C0 }
	   };

flavor clrfl[] = {
	{ DataAlt, 0, BWL, oneOp, 0x4200, 0x4240, 0x4280 }
       };

flavor cmpfl[] = {
	{ Data, DnDirect, BWL, arithReg, 0xB000, 0xB040, 0xB080 },
	{ AnDirect, DnDirect, BWL, arithReg, 0xB000, 0xB040, 0xB080 },
	{ All, AnDirect, BWL, arithReg, 0, 0xB0C0, 0xB1C0 },          // CMPA
	{ IMMEDIATE, DnDirect, BWL, immedInst, 0x0C00, 0x0C40, 0x0C80 },  // CMPI
       };

flavor cmpafl[] = {
	{ All, AnDirect, LONG_SIZE, arithReg, 0, 0xB0C0, 0xB1C0 }
       };

flavor cmpifl[] = {
	{ IMMEDIATE, DnDirect, LONG_SIZE, immedInst, 0x0C00, 0x0C40, 0x0C80 }
       };

flavor divsfl[] = {
	{ Data, DnDirect, WL, arithLongSigned, 0, 0x81C0, 0x4C40 }
       };

flavor divufl[] = {
	{ Data, DnDirect, WL, arithLong, 0, 0x80C0, 0x4C40 }
       };

flavor eorfl[] = {
	{ DnDirect, DataAlt, LONG_SIZE, arithAddr, 0xB100, 0xB140, 0xB180 },
	{ IMMEDIATE, DnDirect, LONG_SIZE, immedInst, 0x0A00, 0x0A40, 0x0A80 } //EORI
       };

flavor eorifl[] = {
	{ IMMEDIATE, DnDirect, LONG_SIZE, immedInst, 0x0A00, 0x0A40, 0x0A80 }
       };

flavor extfl[] = {
	{ DnDirect, 0, WL, oneReg, 0, 0x4880, 0x48C0 }
       };

flavor extbfl[] = {
	{ DnDirect, 0, LONG_SIZE, oneReg, 0, 0x49C0, 0x49C0 }
       };

flavor ff1fl[] = {
	{ DnDirect, 0, LONG_SIZE, oneReg, 0, 0x04C0, 0x04C0 }
	   };

flavor illegalfl[] = {
	{ 0, 0, 0, zeroOp, 0, 0x4AFC, 0 }
       };

flavor jmpfl[] = {
	{ Control, 0, 0, oneOp, 0, 0x4EC0, 0 }
       };

flavor jsrfl[] = {
	{ Control, 0, 0, oneOp, 0, 0x4E80, 0 }
       };

flavor leafl[] = {
	{ Control, AnDirect, LONG_SIZE, arithReg, 0, 0x41C0, 0x41C0 }
       };

flavor linkfl[] = {
	{ AnDirect, IMMEDIATE, 0, link, 0, 0x4E50, 0 }
       };

flavor lslfl[] = {
	{ DnDirect, DnDirect, LONG_SIZE, shiftReg, 0xE128, 0xE168, 0xE1A8 },
	{ IMMEDIATE, DnDirect, LONG_SIZE, shiftReg, 0xE108, 0xE148, 0xE188 }
       };

flavor lsrfl[] = {
	{ DnDirect, DnDirect, LONG_SIZE, shiftReg, 0xE028, 0xE068, 0xE0A8 },
	{ IMMEDIATE, DnDirect, LONG_SIZE, shiftReg, 0xE008, 0xE048, 0xE088 }
       };

flavor mov3qfl[] = {
	{ IMMEDIATE, Alter | AnDirect, LONG_SIZE, mov3q, 0, 0xA140, 0xA140 }
       };

/* (kl) changed flavors according to Coldfire PRM  */
flavor movefl[] = {
	{ MoveSrcAll, Alter, BWL, move, 0x1000, 0x3000, 0x2000 },
	{ AnIndDisp | PCDisp, MoveDestDisp, BWL, move, 0x1000, 0x3000, 0x2000 },
	{ MoveIndexAbs, MoveDestIndexAbs, BWL, move, 0x1000, 0x3000, 0x2000 },
	{ IMMEDIATE, AnIndDisp, BW, move, 0x1000, 0x3000, 0x2000 },
	{ All, AnDirect, WL, move, 0, 0x3000, 0x2000 },                      // MOVEA
	{ DnDirect | IMMEDIATE, CCRDirect, WORD_SIZE, oneOp, 0, 0x44C0, 0 }, //Move to CCR
	{ Data, SRDirect, WORD_SIZE, oneOp, 0, 0x46C0, 0 },                  // supervisor
  	{ CCRDirect, DnDirect, WORD_SIZE, moveReg, 0, 0x42C0, 0 },           //Move from CCR
	{ SRDirect, DataAlt, WORD_SIZE, moveReg, 0, 0x40C0, 0 },             // supervisor
	{ AnDirect, USPDirect, LONG_SIZE, moveUSP, 0, 0x4E60, 0x4E60 },      // supervisor
	{ USPDirect, AnDirect, LONG_SIZE, moveUSP, 0, 0x4E68, 0x4E68 }       // supervisor
       };

flavor moveafl[] = {
	{ All, AnDirect, WL, move, 0, 0x3000, 0x2000 }
       };

flavor moveqfl[] = {
	{ IMMEDIATE, DnDirect, LONG_SIZE, moveq, 0, 0x7000, 0x7000 }
       };

flavor mulsfl[] = {
	{ Data, DnDirect, WL, arithLongSigned, 0, 0xC1C0, 0x4C00 }
       };

flavor mulufl[] = {
	{ Data, DnDirect, WL, arithLong, 0, 0xC0C0, 0x4C00 }
       };

flavor mvsfl[] = {
	{ All, DnDirect, BW, move, 0x7100, 0x7140, 0 }
	   };

flavor mvzfl[] = {
	{ All, DnDirect, BW, move, 0x7180, 0x71C0, 0 }
       };

flavor negfl[] = {
	{ DnDirect, 0, LONG_SIZE, oneOp, 0x4400, 0x4440, 0x4480 }
       };

flavor negxfl[] = {
	{ DnDirect, 0, LONG_SIZE, oneOp, 0x4000, 0x4040, 0x4080 }
       };

flavor nopfl[] = {
	{ 0, 0, 0, zeroOp, 0, 0x4E71, 0 }
       };

flavor notfl[] = {
	{ DnDirect, 0, LONG_SIZE, oneOp, 0x4600, 0x4640, 0x4680 }
       };

flavor orfl[] = {
	{ Data, DnDirect, LONG_SIZE, arithReg, 0, 0x8040, 0x8080 },
	{ DnDirect, MemAlt, LONG_SIZE, arithAddr, 0, 0x8140, 0x8180 },
	{ IMMEDIATE, DnDirect, LONG_SIZE, immedInst, 0, 0x0040, 0x0080 } //ORI
       };

flavor orifl[] = {
	{ IMMEDIATE, DnDirect, LONG_SIZE, immedInst, 0, 0x0080, 0x0080 }
       };

flavor peafl[] = {
	{ Control, 0, LONG_SIZE, oneOp, 0, 0x4840, 0x4840 }
       };

flavor pulsefl[] = {
	{ 0, 0, WORD_SIZE, zeroOp, 0, 0x4ACC, 0 }
	   };

flavor remsfl[] = {
	{ AddrIndirect | DnDirect, DnDouble, LONG_SIZE, arithLongSigned, 0, 0x4C40, 0x4C40 }
	   };

flavor remufl[] = {
	{ AddrIndirect | DnDirect, DnDouble, LONG_SIZE, arithLong, 0, 0x4C40, 0x4C40 }
	   };

flavor rtefl[] = {
	{ 0, 0, 0, zeroOp, 0, 0x4E73, 0 }
       };

flavor rtsfl[] = {
	{ 0, 0, 0, zeroOp, 0, 0x4E75, 0 }
       };

flavor satsfl[] = {
	{ DnDirect, 0, LONG_SIZE, oneReg, 0, 0x4C80, 0x4C80 }
	   };

flavor sccfl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x54C0, 0x54C0, 0 }
       };

flavor scsfl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x55C0, 0x55C0, 0 }
       };

flavor seqfl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x57C0, 0x57C0, 0 }
       };

flavor sffl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x51C0, 0x51C0, 0 }
       };

flavor sgefl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x5CC0, 0x5CC0, 0 }
       };

flavor sgtfl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x5EC0, 0x5EC0, 0 }
       };

flavor shifl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x52C0, 0x52C0, 0 }
       };

flavor shsfl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x54C0, 0x54C0, 0 }
       };

flavor slefl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x5FC0, 0x5FC0, 0 }
       };

flavor slofl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x55C0, 0x55C0, 0 }
       };

flavor slsfl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x53C0, 0x53C0, 0 }
       };

flavor sltfl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x5DC0, 0x5DC0, 0 }
       };

flavor smifl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x5BC0, 0x5BC0, 0 }
       };

flavor snefl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x56C0, 0x56C0, 0 }
       };

flavor splfl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x5AC0, 0x5AC0, 0 }
       };

flavor stfl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x50C0, 0x50C0, 0 }
       };

flavor stopfl[] = {
	{ IMMEDIATE, 0, 0, immedWord, 0, 0x4E72, 0 }
       };

flavor subfl[] = {
	{ All, AnDirect, LONG_SIZE, arithReg, 0, 0, 0x91C0 },                // changed order to match addfl CK 5/22/07
	{ IMMEDIATE, DataAlt, LONG_SIZE, immedInst, 0, 0, 0x0480 },
	{ IMMEDIATE, AnDirect, LONG_SIZE, quickMath, 0, 0, 0x5180 },
	{ Data, DnDirect, LONG_SIZE, arithReg, 0, 0, 0x9080 },
	{ AnDirect, DnDirect, LONG_SIZE, arithReg, 0, 0, 0x9080 },
	{ DnDirect, MemAlt, LONG_SIZE, arithAddr, 0, 0, 0x9180 },
       };

flavor subafl[] = {
	{ All, AnDirect, LONG_SIZE, arithReg, 0, 0, 0x91C0 }
       };

flavor subifl[] = {
	{ IMMEDIATE, DnDirect, LONG_SIZE, immedInst, 0, 0, 0x0480 }
       };

flavor subqfl[] = {
	{ IMMEDIATE, DataAlt, LONG_SIZE, quickMath, 0, 0, 0x5180 },
	{ IMMEDIATE, AnDirect, LONG_SIZE, quickMath, 0, 0, 0x5180 }
       };

flavor subxfl[] = {
	{ DnDirect, DnDirect, LONG_SIZE, twoReg, 0x9100, 0x9140, 0x9180 }
       };

flavor svcfl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x58C0, 0x58C0, 0 }
       };

flavor svsfl[] = {
	{ DnDirect, 0, BYTE_SIZE, scc, 0x59C0, 0x59C0, 0 }
       };

flavor swapfl[] = {
	{ DnDirect, 0, WORD_SIZE, oneReg, 0, 0x4840, 0 }
       };

flavor tasfl[] = {
	{ DataAlt, 0, BYTE_SIZE, oneOp, 0x4AC0, 0x4AC0, 0 }
       };

flavor tpffl[] = {
 	{ 0, 0, 0, zeroOp, 0, 0x51FC, 0 },
	{ 0, 0, WL, tpf, 0, 0x51FA, 0x51FB },
	{ IMMEDIATE, 0, WL, tpf, 0, 0x51FA, 0x51FB }
	   };

flavor trapfl[] = {
	{ IMMEDIATE, 0, 0, trap, 0, 0x4E40, 0 }
       };

flavor tstfl[] = {
	{ DataAlt, 0, BWL, oneOp, 0x4A00, 0x4A40, 0x4A80 }
       };

flavor unlkfl[] = {
	{ AnDirect, 0, 0, oneReg, 0, 0x4E58, 0 }
       };

flavor wddatafl[] = {
	{(Alter ^ DnDirect), 0, BWL, oneOp, 0xFB00, 0xFB40, 0xFB80 }
	   };

/* Define a macro to compute the length of a flavor list */

#define flavorCount(flavorArray) (sizeof(flavorArray)/sizeof(flavor))


/* The instruction table itself... */
// Instructions MUST BE IN ALPHABETICAL ORDER

instruction instTable[] = {
	{ "ADD", addfl, flavorCount(addfl), true, NULL },
	{ "ADDA", addafl, flavorCount(addafl), true, NULL },
	{ "ADDI", addifl, flavorCount(addifl), true, NULL },
	{ "ADDQ", addqfl, flavorCount(addqfl), true, NULL },
	{ "ADDX", addxfl, flavorCount(addxfl), true, NULL },
	{ "AND", andfl, flavorCount(andfl), true, NULL },
	{ "ANDI", andifl, flavorCount(andifl), true, NULL },
	{ "ASL", aslfl, flavorCount(aslfl), true, NULL },
	{ "ASR", asrfl, flavorCount(asrfl), true, NULL },
	{ "BCC", bccfl, flavorCount(bccfl), true, NULL },
	{ "BCHG", bchgfl, flavorCount(bchgfl), true, NULL },
	{ "BCLR", bclrfl, flavorCount(bclrfl), true, NULL },
	{ "BCS", bcsfl, flavorCount(bcsfl), true, NULL },
	{ "BEQ", beqfl, flavorCount(beqfl), true, NULL },
	{ "BGE", bgefl, flavorCount(bgefl), true, NULL },
	{ "BGT", bgtfl, flavorCount(bgtfl), true, NULL },
	{ "BHI", bhifl, flavorCount(bhifl), true, NULL },
	{ "BHS", bccfl, flavorCount(bccfl), true, NULL },
  { "BITREV", bitrevfl, flavorCount(bitrevfl), true, NULL }, //(kl)
	{ "BLE", blefl, flavorCount(blefl), true, NULL },
	{ "BLO", bcsfl, flavorCount(bcsfl), true, NULL },
	{ "BLS", blsfl, flavorCount(blsfl), true, NULL },
	{ "BLT", bltfl, flavorCount(bltfl), true, NULL },
	{ "BMI", bmifl, flavorCount(bmifl), true, NULL },
	{ "BNE", bnefl, flavorCount(bnefl), true, NULL },
	{ "BPL", bplfl, flavorCount(bplfl), true, NULL },
	{ "BRA", brafl, flavorCount(brafl), true, NULL },
	{ "BSET", bsetfl, flavorCount(bsetfl), true, NULL },
	{ "BSR", bsrfl, flavorCount(bsrfl), true, NULL },
	{ "BTST", btstfl, flavorCount(btstfl), true, NULL },
	{ "BVC", bvcfl, flavorCount(bvcfl), true, NULL },
	{ "BVS", bvsfl, flavorCount(bvsfl), true, NULL },
  { "BYTEREV", byterevfl, flavorCount(byterevfl), true, NULL }, //(kl)
	{ "CLR", clrfl, flavorCount(clrfl), true, NULL },
	{ "CMP", cmpfl, flavorCount(cmpfl), true, NULL },
	{ "CMPA", cmpafl, flavorCount(cmpafl), true, NULL },
	{ "CMPI", cmpifl, flavorCount(cmpifl), true, NULL },
        { "DBLOOP", NULL, 0, false, asmStructure },
	{ "DC", NULL, 0, false, dc },
	{ "DCB", NULL, 0, false, dcb },
	{ "DIVS", divsfl, flavorCount(divsfl), true, NULL },
	{ "DIVU", divufl, flavorCount(divufl), true, NULL },
	{ "DS", NULL, 0, false, ds },
        { "ELSE", NULL, 0, false, asmStructure },
	{ "END", NULL, 0, false, funct_end },
        { "ENDF", NULL, 0, false, asmStructure },
        { "ENDI", NULL, 0, false, asmStructure },
        { "ENDW", NULL, 0, false, asmStructure },
	{ "EOR", eorfl, flavorCount(eorfl), true, NULL },
	{ "EORI", eorifl, flavorCount(eorifl), true, NULL },
	{ "EQU", NULL, 0, false, equ },
	{ "EXT", extfl, flavorCount(extfl), true, NULL },
  { "EXTB", extbfl, flavorCount(extbfl), true, NULL }, //(kl)
        { "FAIL", NULL, 0, false, failError },
  { "FF1", ff1fl, flavorCount(ff1fl), true, NULL }, //(kl)
        { "FOR", NULL, 0, false, asmStructure },
        { "IF", NULL, 0, false, asmStructure },
	{ "ILLEGAL", illegalfl, flavorCount(illegalfl), true, NULL },
        { "INCBIN", NULL, 0, false, incbin },
        { "INCLUDE", NULL, 0, false, include },
	{ "JMP", jmpfl, flavorCount(jmpfl), true, NULL },
	{ "JSR", jsrfl, flavorCount(jsrfl), true, NULL },
	{ "LEA", leafl, flavorCount(leafl), true, NULL },
	{ "LINK", linkfl, flavorCount(linkfl), true, NULL },
        { "LIST", NULL, 0, false, listOn },
	{ "LSL", lslfl, flavorCount(lslfl), true, NULL },
	{ "LSR", lsrfl, flavorCount(lsrfl), true, NULL },
        { "MACRO", NULL, 0, false, macro },
        { "MEMORY", NULL, 0, false, memory },
  { "MOV3Q", mov3qfl, flavorCount(mov3qfl), true, NULL }, //(kl)
	{ "MOVE", movefl, flavorCount(movefl), true, NULL },
	{ "MOVEA", moveafl, flavorCount(moveafl), true, NULL },
	{ "MOVEM", movefl, 0, false, movem },   // movefl is only used for syntax highlighting
	{ "MOVEQ", moveqfl, flavorCount(moveqfl), true, NULL },
	{ "MULS", mulsfl, flavorCount(mulsfl), true, NULL },
	{ "MULU", mulufl, flavorCount(mulufl), true, NULL },
  { "MVS", mvsfl, flavorCount(mvsfl), true, NULL },
  { "MVZ", mvzfl, flavorCount(mvzfl), true, NULL },
	{ "NEG", negfl, flavorCount(negfl), true, NULL },
	{ "NEGX", negxfl, flavorCount(negxfl), true, NULL },
        { "NOLIST", NULL, 0, false, listOff },
	{ "NOP", nopfl, flavorCount(nopfl), true, NULL },
	{ "NOT", notfl, flavorCount(notfl), true, NULL },
	{ "OFFSET", NULL, 0, false, offset },
	{ "OPT", NULL, 0, false, opt },
	{ "OR", orfl, flavorCount(orfl), true, NULL },
	{ "ORG", NULL, 0, false, org },
	{ "ORI", orifl, flavorCount(orifl), true, NULL },
	{ "PEA", peafl, flavorCount(peafl), true, NULL },
  { "PULSE", pulsefl, flavorCount(pulsefl), true, NULL }, //(kl)
	{ "REG", NULL, 0, false, reg },
	{ "REMS", remsfl, flavorCount(remsfl), true, NULL }, //(kl)
  { "REMU", remufl, flavorCount(remufl), true, NULL }, //(kl)
        { "REPEAT", NULL, 0, false, asmStructure },
	{ "RTE", rtefl, flavorCount(rtefl), true, NULL },
	{ "RTS", rtsfl, flavorCount(rtsfl), true, NULL },
  { "SATS", satsfl, flavorCount(satsfl), true, NULL }, //(kl)
	{ "SCC", sccfl, flavorCount(sccfl), true, NULL },
	{ "SCS", scsfl, flavorCount(scsfl), true, NULL },
	{ "SECTION", NULL, 0, false, section },
	{ "SEQ", seqfl, flavorCount(seqfl), true, NULL },
	{ "SET", NULL, 0, false, set },
	{ "SF", sffl, flavorCount(sffl), true, NULL },
	{ "SGE", sgefl, flavorCount(sgefl), true, NULL },
	{ "SGT", sgtfl, flavorCount(sgtfl), true, NULL },
	{ "SHI", shifl, flavorCount(shifl), true, NULL },
	{ "SHS", sccfl, flavorCount(sccfl), true, NULL },
	{ "SLE", slefl, flavorCount(slefl), true, NULL },
	{ "SLO", scsfl, flavorCount(scsfl), true, NULL },
	{ "SLS", slsfl, flavorCount(slsfl), true, NULL },
	{ "SLT", sltfl, flavorCount(sltfl), true, NULL },
	{ "SMI", smifl, flavorCount(smifl), true, NULL },
	{ "SNE", snefl, flavorCount(snefl), true, NULL },
	{ "SPL", splfl, flavorCount(splfl), true, NULL },
	{ "ST", stfl, flavorCount(stfl), true, NULL },
	{ "STOP", stopfl, flavorCount(stopfl), true, NULL },
	{ "SUB", subfl, flavorCount(subfl), true, NULL },
	{ "SUBA", subafl, flavorCount(subafl), true, NULL },
	{ "SUBI", subifl, flavorCount(subifl), true, NULL },
	{ "SUBQ", subqfl, flavorCount(subqfl), true, NULL },
	{ "SUBX", subxfl, flavorCount(subxfl), true, NULL },
	{ "SVC", svcfl, flavorCount(svcfl), true, NULL },
	{ "SVS", svsfl, flavorCount(svsfl), true, NULL },
	{ "SWAP", swapfl, flavorCount(swapfl), true, NULL },
	{ "TAS", tasfl, flavorCount(tasfl), true, NULL },
  { "TPF", tpffl, flavorCount(tpffl), true, NULL }, //(kl)
	{ "TRAP", trapfl, flavorCount(trapfl), true, NULL },
	{ "TST", tstfl, flavorCount(tstfl), true, NULL },
        { "UNLESS", NULL, 0, false, asmStructure },
	{ "UNLK", unlkfl, flavorCount(unlkfl), true, NULL },
        { "UNTIL", NULL, 0, false, asmStructure },
  { "WDDATA", wddatafl, flavorCount(wddatafl), true, NULL },
        { "WHILE", NULL, 0, false, asmStructure }
};


/* Declare a global variable containing the size of the instruction table */

int tableSize = sizeof(instTable)/sizeof(instruction);


