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

#include "bttestlogger.h"

#include <StifTestModule.h>
#include <StifLogger.h>

#include <e32std.h>
#include <f32file.h>
#include <sysutil.h>

// RDebug
#include <e32debug.h>

const TInt CBtTestLogger::KPrefixLength = 8;
// Timestamp string length
const TInt KTimeStampStrLen = 21;
// Extended Timestamp string length
const TInt KExtTimeStampStrLen = 80;
// Extended time stamp
typedef TBuf<KExtTimeStampStrLen> TExtTimeStamp;
// Logging path
_LIT( KBtAudioTestSystemLogPath, "e:\\temp\\LC\\BTEng\\" );
//_LIT( KBtAudioTestSystemLogPath, "c:\\temp\\lc_apitest\\" );
// Log file
_LIT( KBtAudioTestSystemLogFile, "BTEngSettingsApiTestLog.html" );
// Log file
_LIT( KBtAudioTestSystemLogFullName, "\\temp\\LC\\BTEng\\BtEngSettingsApiTestLog.html" );
// Time stamp format string
_LIT( KFormatTimeStamp, "<br/>[%H:%T:%S.%*C4] " );
// Date format string
_LIT( KFormatDate, "%F%D %N %Y" );
// HTML header section
_LIT( KHtmlHeadSection, "<html>\n<head>\n<title>BT Audio Test System results</title>\n<style type=\"text/css\">\n.res, .case { display: inline; }\n</style>\n</head>\n\n<body onLoad=\"FormatResults();\">\n\n" );
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
// CBtTestLogger::NewL
// Static constructor to constructor builder objects.
// -----------------------------------------------------------------------------
//
CBtTestLogger* CBtTestLogger::NewL(const CTestModuleIf* aTestModuleIf)
	{
	CBtTestLogger* self = new (ELeave) CBtTestLogger(aTestModuleIf);

	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop(self);

	return self;
	}

// -----------------------------------------------------------------------------
// CBtTestLogger::CBtTestLogger
// Constructor.
// -----------------------------------------------------------------------------
//
CBtTestLogger::CBtTestLogger(const CTestModuleIf* aTestModuleIf)
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
void CBtTestLogger::ConstructL()
    {
    TInt err = CheckLogFile();
    iLog = CStifLogger::NewL( KBtAudioTestSystemLogPath, KBtAudioTestSystemLogFile,
                              CStifLogger::ETxt, CStifLogger::EFile, EFalse, EFalse );
    if( err == KErrNotFound )
        {   // Add info & HTML markup to the beginning of the log file.
        LogHeadSectionL();
        }
    }

// -----------------------------------------------------------------------------
// CBtTestLogger::~CBtTestLogger
// Destructor.
// -----------------------------------------------------------------------------
//
CBtTestLogger::~CBtTestLogger()
	{
	delete iLog;
	}

// Member functions.

// -----------------------------------------------------------------------------
// CBtTestLogger::GetLogger
// Returns a pointer to current STIF logger.
// -----------------------------------------------------------------------------
//
CStifLogger* CBtTestLogger::GetLogger() const
	{
	return iLog;
	}

// -----------------------------------------------------------------------------
// CBtTestLogger::GetTestModuleIf
// Returns a pointer to current STIF test module interface.
// -----------------------------------------------------------------------------
//
const CTestModuleIf* CBtTestLogger::GetTestModuleIf() const
	{
	return iTestModuleIf;
	}

// -----------------------------------------------------------------------------
// CBtTestLogger::Log
// Logs 8 bit data to screen and file.
// -----------------------------------------------------------------------------
//
TInt CBtTestLogger::Log(TLogLevel aLevel, TRefByValue<const TDesC8> aLogText, ...) const
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
// CBtTestLogger::Log
// Logs 8 bit data to screen and file.
// -----------------------------------------------------------------------------
//
TInt CBtTestLogger::Log(const TDesC& aCategory, TRefByValue<const TDesC8> aLogText, ...) const
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
// CBtTestLogger::Log
// Logs 16 bit data to screen and file.
// -----------------------------------------------------------------------------
//
TInt CBtTestLogger::Log(TLogLevel aLevel, TRefByValue<const TDesC16> aLogText, ...) const
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
// CBtTestLogger::Log
// Logs 16 bit data to screen and file.
// -----------------------------------------------------------------------------
//
TInt CBtTestLogger::Log(const TDesC& aCategory, TRefByValue<const TDesC16> aLogText, ...) const
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
// CBtTestLogger::LogNewLine
// Logs an HTML  newline and timestamp.
// -----------------------------------------------------------------------------
//
TInt CBtTestLogger::LogResult( const TDesC8& aFunc, const TDesC& aArg, TInt aRes ) const
    {
    TExtTimeStamp func;
    func.Copy( aFunc ); // Copy 8-bit string to 16-bit buffer

    Log( CBtTestLogger::ETLResult, KTestCaseResult, &func, &aArg, aRes );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtTestLogger::LogNewLine
// Logs an HTML  newline and timestamp.
// -----------------------------------------------------------------------------
//
TInt CBtTestLogger::LogNewLine() const
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
// CBtTestLogger::SetEnabled
// Returns a pointer to current STIF logger.
// -----------------------------------------------------------------------------
//
void CBtTestLogger::SetEnabled(TBool aEnabled)
	{
	iEnabled = aEnabled;
	}

// -----------------------------------------------------------------------------
// CBtTestLogger::SetLogger
// Sets a pointer to current STIF logger.
// -----------------------------------------------------------------------------
//
void CBtTestLogger::SetLogger(CStifLogger* aLog)
	{
	iLog = aLog;
	}

// -----------------------------------------------------------------------------
// CBtTestLogger::SetTestModuleIf
// Sets a pointer to current STIF test module.
// -----------------------------------------------------------------------------
//
void CBtTestLogger::SetTestModuleIf(const CTestModuleIf* aTestModuleIf)
	{
	iTestModuleIf = aTestModuleIf;
	}

// -----------------------------------------------------------------------------
// CBtTestLogger::WriteLog
// Writes log.
// -----------------------------------------------------------------------------
//
TInt CBtTestLogger::WriteLog(TLogLevel aLevel, TDes16& aLog) const
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
// CBtTestLogger::WriteLog
// Writes log.
// -----------------------------------------------------------------------------
//
TInt  CBtTestLogger::WriteLog(const TDesC16& aCategory, const TDesC16& aLog) const
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
// CBtTestLogger::GetPrefix
// Returns the prefix for a given logging level.
// -----------------------------------------------------------------------------
//
TPtrC CBtTestLogger::GetPrefix(TLogLevel aLevel)
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
// CBtTestLogger::CheckLogFile
// Check if a log file is already existing, otherwise HTML formatting 
// should be added.
// -----------------------------------------------------------------------------
//
TInt CBtTestLogger::CheckLogFile()
    {
    RThread thread;
    TInt id = thread.Id();
    TBuf<KMaxFileName > file;
    file.Format( KBtAudioTestSystemLogFullName, id );
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
// CBtTestLogger::LogTimeStamp
// Log HTML head section and general information in a newly created logfile.
// -----------------------------------------------------------------------------
//
void CBtTestLogger::LogHeadSectionL()
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
// CBtTestLogger::LogTimeStamp
// Format a timestamp for logging.
// If aTime is set to false, the (formatted) date is returned.
// -----------------------------------------------------------------------------
//
void CBtTestLogger::LogTimeStampL(TDes& aBuf, TBool aTime) const
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
