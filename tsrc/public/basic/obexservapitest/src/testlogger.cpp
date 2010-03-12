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

// INCLUDES

#include "testlogger.h"

#include <StifTestModule.h>
#include <StifLogger.h>

#include <e32std.h>
#include <f32file.h>
#include <sysutil.h>

// RDebug
#include <e32debug.h>

const TInt CObexTestLogger::KPrefixLength = 8;
// Timestamp string length
const TInt KTimeStampStrLen = 21;
// Extended Timestamp string length
const TInt KExtTimeStampStrLen = 40;
// Extended time stamp
typedef TBuf<KExtTimeStampStrLen> TExtTimeStamp;
// Logging path
_LIT( KObexServAPITestSystemLogPath, "e:\\logs\\ObexServAPItest\\" );
// Log file
_LIT( KObexServAPITestSystemLogFile, "ObexServAPItest.html" );
// Log file
_LIT( KObexServAPITestSystemLogFullName, "\\logs\\ObexServAPItest\\ObexServAPItest%x.html" );
// Time stamp format string
_LIT( KFormatTimeStamp, "<br/>[%H:%T:%S.%*C4] " );
// Date format string
_LIT( KFormatDate, "%F%D %N %Y" );
// HTML header section
_LIT( KHtmlHeadSection, "<html>\n<head>\n<title>Obex Message Handler Test System results</title>\n<style type=\"text/css\">\n.res, .case { display: inline; }\n</style>\n</head>\n\n<body onLoad=\"FormatResults();\">\n\n" );
// HTML code which will display the statistics
_LIT( KHtmlStatSection, "<p>Logfile created on %S at %S<br />\nS60 version %S</p>\n\n<div id=\"stats\"></div>\n<br />\n\n" );
// HTML identifier for test case start
_LIT( KHtmlTestCaseStart, "<div class=\"case\">" );
// Test case result logging
_LIT( KTestCaseResult, "Test completed; function %S, parameter %S, result <div class=\"res\">%d</div></div>" );
// JavaScript function for calculating test result
#define KJSSECTION _L( "<script type=\"text/javascript\">\nfunction FormatResults() { \
\n\tvar tl = document.getElementsByTagName( \"div\" );\n\tvar passed = 0, failed = 0, testcases = 0; \
\n\tfor( var i = 0; i < tl.length; i++ ) {\n\t\tif( tl[i].className == \"case\" ) {\n\t\t\ttestcases++;\n\t\t\t} \
\n\t\tif( tl[i].className == \"res\" ) {\n\t\t\tif( tl[i].innerHTML == 0 ) {\n\t\t\t\tpassed++;\n\t\t\t\t} \
\n\t\t\telse {\n\t\t\t\tfailed++;\n\t\t\t\ttl[i].parentNode.style.color = \"red\";\n\t\t\t\t} } } \
\n\tvar stat = document.getElementById( \"stats\" );\n\tvar passrate = Math.round( ( passed / testcases ) * 10000 ) / 100; \
\n\tstat.innerHTML = \"Test cases run: \" + testcases + \"<br />Passed: \" + passed \
+\"<br />Failed: \" + failed + \"<br /><b>Passrate: \" + passrate + \"%%</b>\";\n\t}\n</script>" )

// Construction and destruction.

// -----------------------------------------------------------------------------
// CObexTestLogger::NewL
// Static constructor to constructor builder objects.
// -----------------------------------------------------------------------------
//
CObexTestLogger* CObexTestLogger::NewL(const CTestModuleIf* aTestModuleIf)
	{
	CObexTestLogger* self = new (ELeave) CObexTestLogger(aTestModuleIf);

	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop(self);

	return self;
	}

// -----------------------------------------------------------------------------
// CObexTestLogger::CObexTestLogger
// Constructor.
// -----------------------------------------------------------------------------
//
CObexTestLogger::CObexTestLogger(const CTestModuleIf* aTestModuleIf)
	: CBase(),
	  iEnabled(ETrue),
	  iTestModuleIf(aTestModuleIf)
	{
	}

// -----------------------------------------------------------------------------
// CTestLogger::ConstructL
// 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CObexTestLogger::ConstructL()
    {
    TInt err = CheckLogFile();
    iLog = CStifLogger::NewL( KObexServAPITestSystemLogPath, KObexServAPITestSystemLogFile,
                              CStifLogger::ETxt, CStifLogger::EFile, EFalse, EFalse );
    if( err == KErrNotFound )
        {   // Add info & HTML markup to the beginning of the log file.
        LogHeadSectionL();
        }
    }

// -----------------------------------------------------------------------------
// CObexTestLogger::~CObexTestLogger
// Destructor.
// -----------------------------------------------------------------------------
//
CObexTestLogger::~CObexTestLogger()
	{
	delete iLog;
	}

// Member functions.

// -----------------------------------------------------------------------------
// CObexTestLogger::GetLogger
// Returns a pointer to current STIF logger.
// -----------------------------------------------------------------------------
//
CStifLogger* CObexTestLogger::GetLogger() const
	{
	return iLog;
	}

// -----------------------------------------------------------------------------
// CObexTestLogger::GetTestModuleIf
// Returns a pointer to current STIF test module interface.
// -----------------------------------------------------------------------------
//
const CTestModuleIf* CObexTestLogger::GetTestModuleIf() const
	{
	return iTestModuleIf;
	}

// -----------------------------------------------------------------------------
// CObexTestLogger::Log
// Logs 8 bit data to screen and file.
// -----------------------------------------------------------------------------
//
TInt CObexTestLogger::Log(TLogLevel aLevel, TRefByValue<const TDesC8> aLogText, ...) const
    {
    if (!iEnabled)
        {
        return KErrNone;
        }

    const TDesC8& actualData = aLogText;

    // Check that user does not try to output too much data.
    if (actualData.Length() > KMaxLogData - KPrefixLength)
        {
        return KErrTooBig;
        }

    // Buffer to convert 8 bit data to to 16 bit.
    TLogInfo8 converter;

    // Create variable argument list.
    VA_LIST args;
    VA_START(args, aLogText);

    // Format the argument list to the output buffer.
    converter.FormatList( aLogText, args );

    // Free the variable argument list
    VA_END(args);

    TLogInfo buffer;
    buffer.Copy( converter );

    // Append log prefix to the output buffer.
    buffer.Insert( 0, GetPrefix( aLevel ) );

    // Log a timestamp
    TStifLoggerTimeStamp8 time;
    TRAP_IGNORE( LogTimeStampL( time ) );
    buffer.Insert( 0, time );

    // Write log.
    return WriteLog(aLevel, buffer);
    }

// -----------------------------------------------------------------------------
// CObexTestLogger::Log
// Logs 8 bit data to screen and file.
// -----------------------------------------------------------------------------
//
TInt CObexTestLogger::Log(const TDesC& aCategory, TRefByValue<const TDesC8> aLogText, ...) const
    {
    if (!iEnabled)
	    {
        return KErrNone;
        }

    const TDesC8& actualData = aLogText;

    // Check that user does not try to output too much data.
    if (actualData.Length() > KMaxLogData - KPrefixLength)
        {
        return KErrTooBig;
        }

    // Buffer to convert 8 bit data to to 16 bit.
    TLogInfo8 converter;

    // Create variable argument list.
    VA_LIST args;
    VA_START(args, aLogText);

    // Format the argument list to the output buffer.
    converter.FormatList(aLogText, args);

    // Free the variable argument list
    VA_END(args);

    TLogInfo buffer;
    buffer.Copy( converter );

    // Log a timestamp
    TStifLoggerTimeStamp8 time;
    TRAP_IGNORE( LogTimeStampL( time ) );
    buffer.Insert( 0, time );

    // Write log.
    return WriteLog(aCategory, buffer);
    }

// -----------------------------------------------------------------------------
// CObexTestLogger::Log
// Logs 16 bit data to screen and file.
// -----------------------------------------------------------------------------
//
TInt CObexTestLogger::Log(TLogLevel aLevel, TRefByValue<const TDesC16> aLogText, ...) const
	{
	if (!iEnabled)
		{
		return KErrNone;
		}

	const TDesC16& actualData = aLogText;
	// Check that user does not try to output too much data.
	if (actualData.Length() + KPrefixLength > KMaxLogData)
		{
		return KErrTooBig;
		}

	TLogInfo buffer;

	// Log a timestamp
	TRAP_IGNORE( LogTimeStampL( buffer ) );

	// Create variable argument list.
	VA_LIST args;
	VA_START(args, aLogText);

	// Format the argument list to the output buffer.
	buffer.AppendFormatList(aLogText, args);

	// Free the variable argument list
	VA_END(args);

	// Write log.
	return WriteLog(aLevel, buffer);
	}

// -----------------------------------------------------------------------------
// CObexTestLogger::Log
// Logs 16 bit data to screen and file.
// -----------------------------------------------------------------------------
//
TInt CObexTestLogger::Log(const TDesC& aCategory, TRefByValue<const TDesC16> aLogText, ...) const
	{
	if (!iEnabled)
		{
		return KErrNone;
		}

	const TDesC16& actualData = aLogText;
	// Check that user does not try to output too much data.
	if (actualData.Length() + KPrefixLength > KMaxLogData)
		{
		return KErrTooBig;
		}

	TLogInfo buffer;

	// Log a timestamp
	TRAP_IGNORE( LogTimeStampL( buffer ) );

	// Create variable argument list.
	VA_LIST args;
	VA_START(args, aLogText);

	// Format the argument list to the output buffer.
	buffer.AppendFormatList(aLogText, args);

	// Free the variable argument list
	VA_END(args);

	// Write log.
	return WriteLog(aCategory, buffer);
	}

// -----------------------------------------------------------------------------
// CObexTestLogger::LogNewLine
// Logs an HTML  newline and timestamp.
// -----------------------------------------------------------------------------
//
TInt CObexTestLogger::LogResult( const TDesC8& aFunc, const TDesC& aArg, TInt aRes ) const
    {
    TExtTimeStamp func;
    func.Copy( aFunc ); // Copy 8-bit string to 16-bit buffer

    Log( CObexTestLogger::ETLResult, KTestCaseResult, &func, &aArg, aRes );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CObexTestLogger::LogNewLine
// Logs an HTML  newline and timestamp.
// -----------------------------------------------------------------------------
//
TInt CObexTestLogger::LogNewLine() const
    {
    if (!iEnabled)
        {
        return KErrNone;
        }
    TExtTimeStamp time;
    TRAPD( err, LogTimeStampL( time ) );
    if( !err )
        {
        time.Insert( 0, KHtmlTestCaseStart );
        err = iLog->Log( time );
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CObexTestLogger::SetEnabled
// Returns a pointer to current STIF logger.
// -----------------------------------------------------------------------------
//
void CObexTestLogger::SetEnabled(TBool aEnabled)
	{
	iEnabled = aEnabled;
	}

// -----------------------------------------------------------------------------
// CObexTestLogger::SetLogger
// Sets a pointer to current STIF logger.
// -----------------------------------------------------------------------------
//
void CObexTestLogger::SetLogger(CStifLogger* aLog)
	{
	iLog = aLog;
	}

// -----------------------------------------------------------------------------
// CObexTestLogger::SetTestModuleIf
// Sets a pointer to current STIF test module.
// -----------------------------------------------------------------------------
//
void CObexTestLogger::SetTestModuleIf(const CTestModuleIf* aTestModuleIf)
	{
	iTestModuleIf = aTestModuleIf;
	}

// -----------------------------------------------------------------------------
// CObexTestLogger::WriteLog
// Writes log.
// -----------------------------------------------------------------------------
//
TInt CObexTestLogger::WriteLog(TLogLevel aLevel, TDes16& aLog) const
    {
    // Write log to file if file logger is available.
    if (iLog != 0)
        {
        iLog->Log(aLog);
        }
    // Remove the timestamp from the logging data.
    TPtrC16 log( aLog.Mid( KTimeStampStrLen ) );

    // Write log with RDebug.
    RDebug::Print(log);

    // Write log to screen if test module interface is available.
	if (iTestModuleIf != 0)
        {
        CTestModuleIf* nonStaticLogger = const_cast<CTestModuleIf*>(iTestModuleIf);
        nonStaticLogger->Printf(aLevel, _L(""), log);
        }

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CObexTestLogger::WriteLog
// Writes log.
// -----------------------------------------------------------------------------
//
TInt  CObexTestLogger::WriteLog(const TDesC16& aCategory, const TDesC16& aLog) const
    {
    // Write log to file if file logger is available.
    if (iLog != 0)
        {
        iLog->Log(aLog);
        }
    // Remove the timestamp from the logging data.
    TPtrC16 log( aLog.Mid( KTimeStampStrLen ) );

    // Write log with RDebug.
    RDebug::Print(log);

    // Write log to screen if test module interface is available.
    if (iTestModuleIf != 0)
        {
        CTestModuleIf* nonStaticLogger = const_cast<CTestModuleIf*>(iTestModuleIf);
        nonStaticLogger->Printf(0, aCategory, log);
        }

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CObexTestLogger::GetPrefix
// Returns the prefix for a given logging level.
// -----------------------------------------------------------------------------
//
TPtrC CObexTestLogger::GetPrefix(TLogLevel aLevel)
	{
	// Select a prefix for a log level and return a pointer to it.
	switch(aLevel)
		{
		case ETLError:
			{
			TPtrC logPrefix = _L("ERROR : ");
			return logPrefix;
			}
		case ETLInfo:
		case ETLResult: 
			{
			TPtrC logPrefix = _L("INFO  : ");
			return logPrefix;
			}
		case ETLDebug:
			{
			TPtrC logPrefix = _L("DEBUG : ");
			return logPrefix;
			}
		default:
			{
			// Return a null pointer.
			return TPtrC();
			}

		}
	}

// -----------------------------------------------------------------------------
// CObexTestLogger::CheckLogFile
// Check if a log file is already existing, otherwise HTML formatting 
// should be added.
// -----------------------------------------------------------------------------
//
TInt CObexTestLogger::CheckLogFile()
    {
    RThread thread;
    TInt id = thread.Id();
    TBuf<KMaxFileName > file;
    file.Format( KObexServAPITestSystemLogFullName, id );
    RFs fs;
    TInt err = fs.Connect();
    TBool open = EFalse;
    if( !err )
        {
        err = fs.IsFileOpen( file, open );
        }
    fs.Close();
    return err;
    }

// -----------------------------------------------------------------------------
// CObexTestLogger::LogTimeStamp
// Log HTML head section and general information in a newly created logfile.
// -----------------------------------------------------------------------------
//
void CObexTestLogger::LogHeadSectionL()
    {
    iLog->Log( KHtmlHeadSection );
    iLog->Log( KJSSECTION );
    TBuf<KSysUtilVersionTextLength> version( _L( "Version unknown!" ) );
    (void) SysUtil::GetSWVersion( version );
    TStifLoggerTimeStamp8 date;
    LogTimeStampL( date, EFalse );
    TStifLoggerTimeStamp8 time;
    LogTimeStampL( time );
    TPtrC timePtr( time.Mid( 6, 13 ) );
    iLog->Log( KHtmlStatSection, &date, &timePtr, &version );
    }

// -----------------------------------------------------------------------------
// CObexTestLogger::LogTimeStamp
// Format a timestamp for logging.
// If aTime is set to false, the (formatted) date is returned.
// -----------------------------------------------------------------------------
//
void CObexTestLogger::LogTimeStampL(TDes& aBuf, TBool aTime) const
    {
    TTime time;
    time.HomeTime();    // Get time and format it
    if( aTime )
        {
        time.FormatL( aBuf, KFormatTimeStamp );
        }
    else
        {
        time.FormatL( aBuf, KFormatDate );
        }
    }
