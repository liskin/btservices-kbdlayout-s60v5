/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Contains internal classes.
*
*/


#ifndef ATCODECDEFS_H
#define ATCODECDEFS_H

//  INCLUDES
#include <e32base.h>

class SATNameDef
    {
    public:
        const TATId KId;
        const TText8* KName;
    };

const SATNameDef KATNameTable[] = 
    {
    {EATA,    _S8("A"),},
    {EATBINP, _S8("+BINP")},
    {EATBLDN, _S8("+BLDN")},
    {EATBRSF, _S8("+BRSF")},
    {EATBVRA, _S8("+BVRA")},
    {EATCCWA, _S8("+CCWA")},
    {EATCHLD, _S8("+CHLD")},
    {EATCHUP, _S8("+CHUP")},
    {EATCIND, _S8("+CIND")},
    {EATCKPD, _S8("+CKPD")},
    {EATCLCC, _S8("+CLCC")},
    {EATCLIP, _S8("+CLIP")},
    {EATCMEE, _S8("+CMEE")},
    {EATCMER, _S8("+CMER")},
    {EATCNUM, _S8("+CNUM")},
    {EATCOPS, _S8("+COPS")},
    {EATD2,   _S8("D>")},
    {EATD1,   _S8("D")},
    {EATNREC, _S8("+NREC")},
    {EATVGM,  _S8("+VGM")},
    {EATVGS,  _S8("+VGS")},
    {EATVTS,  _S8("+VTS")},
    {EATCIEV,   _S8("+CIEV")},
    {EATERROR,  _S8("ERROR")},
    {EATOK,     _S8("OK")},
    {EATRING,   _S8("RING")},
    {EATBIA,	_S8("+BIA")},
    {EATCREG,	_S8("+CREG")},
    {EATCGSN,	_S8("+CGSN")},
  	{EATCSQ,	_S8("+CSQ")},
  	{EATCIMI,	_S8("+CIMI")},
  	{EATCGMI,	_S8("+CGMI")},
  	{EATCGMM,	_S8("+CGMM")},
  	{EATCGMR,	_S8("+CGMR")},
  	{EATCOLP,	_S8("+COLP")},
    };

const TInt KATNameTableSize = (sizeof(KATNameTable) / sizeof(SATNameDef));

class SCommandParamDef
    {
    public:
        const TATId KId;
        const TATType KType;
        const TInt KParamNum;
        const TInt* KParamTypes;
    };

const TInt KIntList1[] =         {EATIntParam};
const TInt KIntList2[] =         {EATIntParam, EATIntParam};
const TInt KIntList3[] =         {EATIntParam, EATIntParam, EATIntParam};
const TInt KIntList4[] =         {EATIntParam, EATIntParam, EATIntParam, EATIntParam};
const TInt KIntList5[] =         {EATIntParam, EATIntParam, EATIntParam, EATIntParam, EATIntParam};
const TInt KIntList8[] =         {EATIntParam, EATIntParam, EATIntParam, EATIntParam, EATIntParam, EATIntParam, EATIntParam, EATIntParam};
const TInt KStrList1[] =         {EATStringParam};
const TInt KDQStrIntList2[] =    {EATDQStringParam, EATIntParam};
const TInt KDQStrIntList3[] =    {EATDQStringParam, EATIntParam, EATIntParam};
const TInt KIntDQStrList3[] =    {EATIntParam, EATIntParam, EATDQStringParam};
const TInt KNilDQStrIntList5[] = {EATNullParam, EATDQStringParam, EATIntParam, EATNullParam, EATIntParam};

const SCommandParamDef KCommandParamTable[] = 
    {
    {EATBINP, EATWriteCmd,  1, KIntList1},
    {EATBRSF, EATWriteCmd,  1, KIntList1},
    {EATBVRA, EATWriteCmd,  1, KIntList1},
    {EATCCWA, EATWriteCmd,  1, KIntList1},
    {EATCHLD, EATWriteCmd,  1, KIntList1},
    {EATCKPD, EATWriteCmd,  1, KIntList1},
    {EATCLIP, EATWriteCmd,  1, KIntList1},
    {EATCMEE, EATWriteCmd,  1, KIntList1},
    {EATCMER, EATWriteCmd,  4, KIntList4},
    {EATCOPS, EATWriteCmd,  2, KIntList2},
    {EATD1,   EATActionCmd, 1, KStrList1},
    {EATD2,   EATActionCmd, 1, KIntList1},
    {EATNREC, EATWriteCmd,  1, KIntList1},
    {EATVGM,  EATWriteCmd,  1, KIntList1},
    {EATVGS,  EATWriteCmd,  1, KIntList1},
    {EATVTS,  EATWriteCmd,  1, KStrList1},
    {EATBIA,  EATWriteCmd,  8, KIntList8},
    {EATCREG, EATWriteCmd,  1, KIntList1},
    {EATCOLP, EATWriteCmd,	1, KIntList1},
    };

const TInt KCommandParamTableSize = (sizeof(KCommandParamTable) / sizeof(SCommandParamDef));


class SResultCodeParamDef
    {
    public:
        const TATId KId;
        const TATType KType;
        const TInt KParamNum;
        const TInt* KParamTypes;
    };

const SResultCodeParamDef KResultCodeParamTable[] = 
    {
    {EATBRSF, EATWriteResult,         1, KIntList1},
    {EATBVRA, EATUnsolicitedResult,   1, KIntList1},
    {EATCCWA, EATUnsolicitedResult,   3, KDQStrIntList3},
    {EATCIND, EATReadResult,          3, KIntList3},
    {EATCLCC, EATActionResult,        5, KIntList5},
    {EATCLIP, EATReadResult,          2, KIntList2},
    {EATCLIP, EATUnsolicitedResult,   2, KDQStrIntList2},
    {EATCNUM, EATActionResult,        5, KNilDQStrIntList5},
    {EATCOPS, EATReadResult,          1, KIntDQStrList3},
    {EATVGS,  EATUnsolicitedResult,   1, KIntList1},
    {EATCIEV,   EATUnsolicitedResult, 2, KIntList2},
    {EATCREG, EATReadResult,          2, KIntList2},
    {EATCREG, EATUnsolicitedResult,   2, KIntList1},
    {EATCGSN, EATActionResult,        1, KStrList1},
    {EATCSQ,  EATActionResult,	      2, KIntList2},
    {EATCIMI, EATActionResult,        1, KStrList1},
    {EATCGMI, EATActionResult,        1, KStrList1},
    {EATCGMM, EATActionResult,        1, KStrList1},
    {EATCGMR, EATActionResult,        1, KStrList1},
    {EATCOLP, EATReadResult,          1, KIntList1},
    {EATCOLP, EATUnsolicitedResult,   2, KDQStrIntList2},    
    };

const TInt KResultCodeParamTableSize = (sizeof(KResultCodeParamTable) / sizeof(SResultCodeParamDef));


#endif  // ATCODECDEFS_H

// End of File
