
/*************************** COLDFIRE SIMULATOR ****************************

File Name: CODECF.CPP
Version: 1.0

The instructions implemented in this file are the instructions new to the
coldfire core (compared to the 68k core)

		BITREV, BYTEREV, EXTB, FF1, MOV3Q, MVS, MVZ, PULSE, REMx,
    SATS, TPF, WDDATA

   Added: Karsten Langeloh
          tematik GmbH
          http://www.tematik.de
***************************************************************************/


#include <stdio.h>
#include "extern.h"         /* contains global "extern" declarations */


//-------------------------------------------------------------------------
//
int BITREV()
{
  int	reg;

  reg = inst & 0x0007;

	source = D[reg] & LONG_MASK;
	dest = D[reg] & LONG_MASK;

  result = 0;
  for (int i=0; i<32; i++)
  {
    result = (result << 1) | ((source >> i) & 0x0001);
  }

  put (&D[reg], result, (long) LONG_MASK);

  inc_cyc(1);
  return SUCCESS;
}

//-------------------------------------------------------------------------
//
int BYTEREV()
{
  int	reg;

  reg = inst & 0x0007;

	source = D[reg] & LONG_MASK;
	dest = D[reg] & LONG_MASK;

  result = 0;
  for (int i=0; i<4; i++)
  {
    result = (result << 8) | ((source >> (i*8)) & 0x00FF);
  }

  put (&D[reg], result, (long) LONG_MASK);

  inc_cyc(1);
  return SUCCESS;
}

//-------------------------------------------------------------------------
//
int EXTB()
{
  long	size;
  int	reg;

  reg = inst & 0x07;

  source = dest = D[reg] & LONG_MASK;

  D[reg] = (D[reg] & BYTE_MASK) | (0xffffff00 * ((D[reg] >> 7) & 0x01));

  cc_update (N_A, GEN, GEN, ZER, ZER, source, dest, D[reg], size, 0);

  inc_cyc (4);

  return SUCCESS;
}

//-------------------------------------------------------------------------
//  Find First One in Register
int FF1()
{
  long runner_mask; //changed to find msb
  int	reg;

  reg = inst & 0x0007;

	source = D[reg] & LONG_MASK;
	dest = D[reg] & LONG_MASK;

  result = 0;
  while (! (source & runner_mask) )
  {
    runner_mask = runner_mask << 1;
    result++;
  }

  put (&D[reg], result, (long) LONG_MASK);
  result = source; //for condition code calculation
  cc_update(N_A, GEN, GEN, ZER, ZER, source, dest, result, (long) LONG_MASK, 0);

  inc_cyc(1);
  return SUCCESS;
}

//-------------------------------------------------------------------------
//
int MOV3Q()
{
  int	reg;

  int error = eff_addr((long)LONG_MASK, ALTERABLE_ADDR, true);
  if (error)              // if address error
    return error;         // return error code

  source = (inst >> 9) & 0x07;
  dest = EV1;

  source = (!source)? 0xFFFFFFFF: source; //0 stands for -1
  put(EA1,source,(long) LONG_MASK);
  result = source;

  cc_update (N_A, GEN, GEN, ZER, ZER, source, dest, result, LONG_MASK, 0);
  inc_cyc (1);
  return SUCCESS;
}

//-------------------------------------------------------------------------
//
int MVS()
{
  int	reg;
  long size_mask;

  size_mask = (inst & 0x0040)?WORD_MASK:BYTE_MASK;

  int error = eff_addr(size_mask, ALL_ADDR, true);
  if (error)              // if address error
    return error;         // return error code
  source = EV1;

  reg = (inst >> 9) & 0x7;
  dest = D[reg];

  /* the source data is sign extended to a longword */
  sign_extend ((int)source, size_mask, &D[reg]);

  cc_update (N_A, GEN, GEN, ZER, ZER, source, dest, D[reg], LONG_MASK, 0);
  inc_cyc (1);

  return SUCCESS;
}

//-------------------------------------------------------------------------
//
int MVZ()
{
  int	reg;
  long size_mask;

  size_mask = (inst & 0x0040)?WORD_MASK:BYTE_MASK;

  int error = eff_addr(size_mask, ALL_ADDR, true);
  if (error)              // if address error
    return error;         // return error code
  source = EV1;

  reg = (inst >> 9) & 0x7;
  dest = D[reg];

  /* the source data is zero-filled to a longword */
  if (size_mask == WORD_MASK)
    result = source & 0x0000FFFF;
  else //BYTE_MASK
    result = source & 0x000000FF;

  put(&D[reg], result, LONG_MASK);

  cc_update (N_A, GEN, GEN, ZER, ZER, source, dest, result, LONG_MASK, 0);
  inc_cyc (1);

  return SUCCESS;
}

//-------------------------------------------------------------------------
// TODO: how to support this in the simulator?
/* from the Coldfire PRM:

Operation:        Set PST = 0x4
Assembler Syntax: PULSE
Attributes:       Unsized
Description:      Performs no operation. The processor state, other than the
                  program counter, is unaffected. However, PULSE generates a
                  special encoding of the Processor Status (PST) output pins,
                  making it very useful for external triggering purposes. The
                  opcode for PULSE is 0x4ACC.
Condition Codes:  Not affected

*/
int PULSE()
{
  return SUCCESS;
  //return BAD_INST;
}

//-------------------------------------------------------------------------
// calculate remainder (singned or unsigned)
int REMX()
{
  return SUCCESS;
}

//-------------------------------------------------------------------------
//
int SATS()
{
  int reg;

  reg = inst & 0x7;

  if (SR & vbit) //overflow bit set
  {
    source = D[reg];
    dest = D[reg];
    if (source & 0x8000000)
      result = 0x7FFFFFFF;
    else
      result = 0x80000000;
    put(&D[reg], result, LONG_MASK);
  }
  cc_update (N_A, GEN, GEN, ZER, ZER, source, dest, result, LONG_MASK, 0);
  inc_cyc(1);
  return SUCCESS;
}

//-------------------------------------------------------------------------
//
int TPF()
{
  if (! (inst & 0x0004) ) //TPF with extension word
  {
    if (inst & 0x0001)    //two extension words
    {
       mem_request (&PC, WORD_SIZE, &source);
    }
    mem_request (&PC, WORD_SIZE, &source);
  }
  return SUCCESS;
}

//-------------------------------------------------------------------------
// TODO: how to support this in the simulator?
/* from the Coldfire PRM:

Operation:        Source . DDATA Signal Pins
Assembler Syntax: WDDATA.sz <ea>y
Attributes:       Size = byte, word, longword
Description:      This instruction fetches the operand defined by the effective
                  address and captures the data in the ColdFire debug module for
                  display on the DDATA output pins. The size of the operand
                  determines the number of nibbles displayed on the DDATA output
                  pins. The value of the debug module configuration/status
                  register (CSR) does not affect the operation of this
                  instruction. The execution of this instruction generates a
                  processor status encoding matching the PULSE instruction (0x4)
                  before the referenced operand is displayed on the DDATA
                  outputs.
Condition Codes:  Not affected
*/
int WDDATA()
{
  return SUCCESS;
}
