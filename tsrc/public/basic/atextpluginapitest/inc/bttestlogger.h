/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
*
*/


#ifndef BTTESTLOGGER_H
#define BTTESTLOGGER_H

// INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS
class CStifLogger;
class CTestModuleIf;

// CLASS DECLARATION

NONSHARABLE_CLASS( CBtTestLogger ) : public CBase
{
	public:
	
		static CBtTestLogger* NewL(const CTestModuleIf* aTestModuleIf);
		virtual ~CBtTestLogger();

	public: // Data used by functions

		enum TLogLevel {ETLError, ETLResult, ETLInfo, ETLDebug};

	public: // New functions
	
		TBool GetEnabled() const;
		CStifLogger* GetLogger() const;
		const CTestModuleIf* GetTestModuleIf() const;
		TInt Log(TLogLevel aLevel, TRefByValue<const TDesC8> aLogText, ...) const;
		TInt Log(const TDesC& aCategory, TRefByValue<const TDesC8> aLogText, ...) const;
		TInt Log(TLogLevel aLevel, TRefByValue<const TDesC16> aLogText, ...) const;
		TInt Log(const TDesC& aCategory, TRefByValue<const TDesC16> aLogText, ...) const;
		TInt LogResult( const TDesC8& aFunc, const TDesC& aArg, TInt aRes ) const;
		TInt LogNewLine() const;
		void SetEnabled(TBool aEnabled);
		void SetLogger(CStifLogger* aLog);
		void SetTestModuleIf(const CTestModuleIf* aTestModuleIf);

	public: // Functions from base classes

	private:  // New functions

		TInt WriteLog(TLogLevel aLevel, TDes16& aLog) const;
		TInt WriteLog(const TDesC16& aCategory, const TDesC16& aLog) const;
		static TPtrC GetPrefix(TLogLevel aLevel);
		TInt CheckLogFile();
		void LogHeadSectionL();
		void LogTimeStampL(TDes& aBuf, TBool aTime = ETrue) const;

	private:  // Constructors
	
		CBtTestLogger(const CTestModuleIf* aTestModuleIf);
		void ConstructL();

	public:     // Data
		static const TInt KPrefixLength;

	private:    // Data

		TBool iEnabled;
		CStifLogger* iLog;
		const CTestModuleIf* iTestModuleIf;
	};

#endif // BTTESTLOGGER_H

