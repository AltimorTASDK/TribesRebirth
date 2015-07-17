//
//  MODULE: TapiCode.c
//
//  PURPOSE: Handles all the TAPI routines for TapiComm.
//

#define TAPI_CURRENT_VERSION 0x00010004

#include <windows.h>

#include "tapinet.h"
#include "logfile.h"

#include <string.h>

// All TAPI line functions return 0 for SUCCESS, so define it.
#define SUCCESS 0

// Possible return error for resynchronization functions.
#define WAITERR_WAITABORTED  1
#define WAITERR_WAITTIMEDOUT 2

// Reasons why a line device might not be usable by TapiComm.
#define LINENOTUSEABLE_ERROR            1
#define LINENOTUSEABLE_NOVOICE          2
#define LINENOTUSEABLE_NODATAMODEM      3
#define LINENOTUSEABLE_NOMAKECALL       4
#define LINENOTUSEABLE_ALLOCATED        5
#define LINENOTUSEABLE_INUSE            6
#define LINENOTUSEABLE_NOCOMMDATAMODEM  7

// Constant used in WaitForCallState when any new
// callstate message is acceptable.
#define I_LINECALLSTATE_ANY 0

 // Wait up to 30 seconds for an async completion.
#define WAITTIMEOUT 30000

// TAPI version that this sample is designed to use.
#define SAMPLE_TAPI_VERSION 0x00010004


// Structures needed to handle special non-dialable characters.
#define g_sizeofNonDialable (sizeof(g_sNonDialable)/sizeof(g_sNonDialable[0]))

typedef struct {
    LONG lError;
    DWORD dwDevCapFlag;
    LPSTR szToken;
    LPSTR szMsg;
} NONDIALTOKENS;

NONDIALTOKENS g_sNonDialable[] = {
    {LINEERR_DIALBILLING,  LINEDEVCAPFLAGS_DIALBILLING,  "$",
            "Wait for the credit card bong tone" },
    {LINEERR_DIALDIALTONE, LINEDEVCAPFLAGS_DIALDIALTONE, "W",
            "Wait for the second dial tone" },
    {LINEERR_DIALDIALTONE, LINEDEVCAPFLAGS_DIALDIALTONE, "w",
            "Wait for the second dial tone" },
    {LINEERR_DIALQUIET,    LINEDEVCAPFLAGS_DIALQUIET,    "@",
            "Wait for the remote end to answer" },
    {LINEERR_DIALPROMPT,   0,                            "?",
            "Press OK when you are ready to continue dialing"},
};

extern class LogFile log_file;

namespace DNet
{

HLINEAPP g_hLineApp=NULL; // the usage handle of this application for TAPI
int   g_num_inits = 0;
DWORD g_dwNumDevs = 0;

TAPITransport *myThis[MAX_TAPI_DEVICES] = {0};
DWORD          myThisID[MAX_TAPI_DEVICES] = {0};

TAPITransport *GetMyThis(DWORD id)
{
	int i;

   for(i = 0; i < MAX_TAPI_DEVICES; i++)
   {
   	if( id == myThisID[i] )
	   	return myThis[i];
   }

   return NULL;
}

void TAPITransport::SetMyThis(TAPITransport *my_this, DWORD dev_id, DWORD callback_id)
{
	// this is necessary so the lineCallBackFunc has a pointer to this class
	AssertFatal( dev_id <= MAX_TAPI_DEVICES, "ERROR: Too many TAPI devices" );

//log_file.log("SetMyThis with my_this %lx, dev_id %lx, callback_id %lx",(long)my_this,(long)dev_id,(long)callback_id);
   myThis[dev_id]   = my_this;
   myThisID[dev_id] = callback_id;
}


//
//  FUNCTION: bool InitializeTAPI( DWORD )
//
//  PURPOSE: Initializes TAPI
//
//  PARAMETERS:
//     device - device number to use (MAXDWORD for no device)
//
//  RETURN VALUE:
//    Always returns 0 - command handled.
//
//  COMMENTS:
//
//    This is the API that initializes the app with TAPI.
//
//

bool TAPITransport::InitializeTAPI( void )
{
   long lReturn;

	g_num_inits++; // keep track of inits, we don't want to shut down tapi if still in use

   // If we're already initialized, then initialization succeeds.
   if ( g_hLineApp == NULL )
   {
	  	// Initialize TAPI
		lReturn = lineInitialize(&g_hLineApp, hInstance, lineCallbackFunc, NULL, &g_dwNumDevs);
	  	log_file.log("TAPITransport::InitializeTAPI lineInit with dwNumDevs %ld, hInst %ld",(long)g_dwNumDevs,(long)hInstance);
   	if (!HandleLineErr(lReturn))
	  	{
			log_file.log(LOG_ERROR,"TAPITransport::InitializeTAPI error");
			return false;
	  	}
   }

   return true;
}


//
//  FUNCTION: bool ShutdownTAPI()
//
//  PURPOSE: Shuts down all use of TAPI
//
//  PARAMETERS:
//    None
//
//  RETURN VALUE:
//    True if TAPI successfully shut down.
//
//  COMMENTS:
//
//    If ShutdownTAPI fails, then its likely either a problem
//    with the service provider (and might require a system
//    reboot to correct) or the application ran out of memory.
//
//

bool TAPITransport::ShutdownTAPI()
{
   long lReturn;

//   log_file.log("Shutting down TAPI...");
   // If we aren't initialized, then Shutdown is unnecessary.
	if (g_hLineApp == NULL)
      return true;

   g_num_inits--;
   HangupCall();
   CloseTAPILine();
log_file.log("ShutdownTAPI with g_num_inits %ld",(long)g_num_inits);
   if( !g_num_inits ) // last instance of TAPI, really shut down
   {
	   lReturn = lineShutdown(g_hLineApp);
   	if (!HandleLineErr(lReturn))
	  		log_file.log(LOG_ERROR,"TAPITransport::ShutdownTAPI error");
	   g_hLineApp = NULL;
//   	log_file.log("TAPI uninitialized.");
   }

   g_hCall = NULL;
   g_hLine = NULL;

   return true;
}



//
//  FUNCTION: bool HangupCall()
//
//  PURPOSE: Hangup the call in progress if it exists.
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    TRUE if call hung up successfully.
//
//  COMMENTS:
//
//    If HangupCall fails, then its likely either a problem
//    with the service provider (and might require a system
//    reboot to correct) or the application ran out of memory.
//
//

bool TAPITransport::HangupCall()
{
   LPLINECALLSTATUS pLineCallStatus = NULL;
   long lReturn;

   // Prevent HangupCall re-entrancy problems.
   if (g_bStoppingCall)
      return true;

   g_bCalling   = false;
   g_bConnected = false;

   // If there is not a line currently open, or a call, no need to proceed.
   if (!g_hLine || !g_hCall)
      return true;

   g_bStoppingCall = true;
//   log_file.log("Stopping Call in progress");

   // if there is a living VC, we should terminate it -- this will kill any comm stuff
   if(!VCList.empty())
   {
	   TAPIVC *tapivc = (TAPIVC *)(*(VCList.begin()));
   	if (tapivc)
   		deleteVC(tapivc);
   }
   g_hCommFile = NULL;

   // If there is a call in progress, drop and deallocate it.

	// I_lineGetCallStatus returns a LocalAlloc()d buffer
   pLineCallStatus = I_lineGetCallStatus(pLineCallStatus, g_hCall);
   if (pLineCallStatus == NULL)
   {
   	ShutdownTAPI();
      g_bStoppingCall = false;
      return false;
   }

   // Only drop the call when the line is not IDLE.
   if (!((pLineCallStatus -> dwCallState) & LINECALLSTATE_IDLE))
   {
		do
      {
      	lReturn = WaitForReply(lineDrop(g_hCall, NULL, 0));

         if (lReturn == WAITERR_WAITTIMEDOUT)
         {
            log_file.log(LOG_ERROR,"Call timed out in WaitForReply.");
            break;
         }

         if (lReturn == WAITERR_WAITABORTED)
         {
            log_file.log(LOG_ERROR,"lineDrop: WAITERR_WAITABORTED.");
            break;
         }

         // Was the call already in IDLE?
         if (lReturn == LINEERR_INVALCALLSTATE)
            break;

         if (!HandleLineErr(lReturn))
         {
            log_file.log(LOG_ERROR,"lineDrop unhandled error: %lx",(long)lReturn);
            break;
         }
      }
      while(lReturn != SUCCESS);

      // Wait for the dropped call to go IDLE before continuing.
      lReturn = WaitForCallState(LINECALLSTATE_IDLE);

      if (lReturn == WAITERR_WAITTIMEDOUT)
        	log_file.log(LOG_ERROR,"Call timed out waiting for IDLE state.");

      if (lReturn == WAITERR_WAITABORTED)
       	log_file.log(LOG_ERROR,"WAITERR_WAITABORTED while waiting for IDLE state.");

      log_file.log("Call Dropped.");
   }

	// The call is now idle.  Deallocate it!
	do
	{
		lReturn = lineDeallocateCall(g_hCall);
		if (!HandleLineErr(lReturn))
		{
			log_file.log(LOG_ERROR,"lineDeallocateCall unhandled error: %lx",(long)lReturn);
         break;
      }
	}
	while(lReturn != SUCCESS);

	log_file.log("Call Deallocated.");

	// Call is taken care of.  Finish cleaning up.
   g_hCall = NULL;
	g_bStoppingCall = false; // allow HangupCall to be called again.
	log_file.log("Call stopped");


	// Need to free LocalAlloc()d buffer returned from I_lineGetCallStatus
	if (pLineCallStatus)
		LocalFree(pLineCallStatus);

	return true;
}

//
//  FUNCTION: CloseTAPILine( void )
//
//  PURPOSE: close line if open
//
//  PARAMETERS:
//
//  RETURN VALUE:
//    TRUE if able to close line.
//
//  COMMENTS:
//

bool TAPITransport::CloseTAPILine()
{
   long lReturn;

   HangupCall();

	// if we have a line open, close it.
	if (g_hLine)
	{
		do
		{
			lReturn = lineClose(g_hLine);
			if (!HandleLineErr(lReturn))
			{
				log_file.log(LOG_ERROR,"lineClose unhandled error: %ld",lReturn);
				return false;
			}
		}
		while(lReturn != SUCCESS);

		log_file.log("Line Closed.");
	}
	g_hLine = NULL;
   g_hCall = NULL;

   return true;
}



//
//  FUNCTION: OpenTAPILine( void )
//
//  PURPOSE: open line in preparation of incoming or outgoing calls
//
//  PARAMETERS:
//
//  RETURN VALUE:
//    TRUE if able to open a line.
//
//  COMMENTS:
//

bool TAPITransport::OpenTAPILine( DWORD device )
{
   long lReturn;

   log_file.log("Opening TAPI Line...");

	// should already be initialized but doesn't hurt to test
   if (!g_hLineApp)
	   return false;

   // Check device, Negotiates API version, checks DevCaps, make sure
   //  the line is not in use already, etc
   g_dwDeviceID = device;
	if( !VerifyAndWarnUsableLine() )
   {
      log_file.log(LOG_ERROR,"Bad device");
      return false;
   }

   // Open the Line for monitoring DATAMODEM calls.
   do
   {
	   lReturn = lineOpen(g_hLineApp, g_dwDeviceID, &g_hLine,g_dwAPIVersion, 0, callbackId,
                         LINECALLPRIVILEGE_OWNER, LINEMEDIAMODE_DATAMODEM, 0);
//      log_file.log("OpenTAPILine setting myThis g_hLine %lx",(long)g_hLine);

      if(lReturn == LINEERR_ALLOCATED)
      {
         log_file.log(LOG_ERROR,"Line is already in use by a non-TAPI application, or by another TAPI Service Provider., Unable to Use Line");
         g_hLine = NULL;
         return false;
      }

      if (!HandleLineErr(lReturn))
      {
         log_file.log(LOG_ERROR,"OpenTAPILine::lineOpen error: %lx",(long)lReturn);
         g_hLine = NULL;
         return false;
      }
   }
   while(lReturn != SUCCESS);

   // Tell the service provider that we want all notifications that
   // have anything to do with this line.
   do
   {
      // Set the messages we are interested in.

      // Note that while most applications aren't really interested
      // in dealing with all of the possible messages, its interesting
      // to see which come through the callback for testing purposes.

      lReturn = lineSetStatusMessages(g_hLine,
            LINEDEVSTATE_OTHER          |
            LINEDEVSTATE_RINGING        |
            LINEDEVSTATE_CONNECTED      |  // Important state!
            LINEDEVSTATE_DISCONNECTED   |  // Important state!
            LINEDEVSTATE_MSGWAITON      |
            LINEDEVSTATE_MSGWAITOFF     |
            LINEDEVSTATE_INSERVICE      |
            LINEDEVSTATE_OUTOFSERVICE   |  // Important state!
            LINEDEVSTATE_MAINTENANCE    |  // Important state!
            LINEDEVSTATE_OPEN           |
            LINEDEVSTATE_CLOSE          |
            LINEDEVSTATE_NUMCALLS       |
            LINEDEVSTATE_NUMCOMPLETIONS |
            LINEDEVSTATE_TERMINALS      |
            LINEDEVSTATE_ROAMMODE       |
            LINEDEVSTATE_BATTERY        |
            LINEDEVSTATE_SIGNAL         |
            LINEDEVSTATE_DEVSPECIFIC    |
            LINEDEVSTATE_REINIT         |  // Not allowed to disable this.
            LINEDEVSTATE_LOCK           |
            LINEDEVSTATE_CAPSCHANGE     |
            LINEDEVSTATE_CONFIGCHANGE   |
            LINEDEVSTATE_COMPLCANCEL    ,

            LINEADDRESSSTATE_OTHER      |
            LINEADDRESSSTATE_DEVSPECIFIC|
            LINEADDRESSSTATE_INUSEZERO  |
            LINEADDRESSSTATE_INUSEONE   |
            LINEADDRESSSTATE_INUSEMANY  |
            LINEADDRESSSTATE_NUMCALLS   |
            LINEADDRESSSTATE_FORWARD    |
            LINEADDRESSSTATE_TERMINALS  |
            LINEADDRESSSTATE_CAPSCHANGE);


      if (!HandleLineErr(lReturn))
      {
         // If we do get an unhandled problem, we don't care.
         // We just won't get notifications.
         log_file.log(LOG_ERROR,"lineSetStatusMessages unhandled error: %lx",(long)lReturn);
         break;
      }
   }
   while(lReturn != SUCCESS);


   // if we made it this far, everything worked
//	log_file.log("Line is open and available");
   return true;
}


//
//  FUNCTION: DialCall( char * )
//
//  PURPOSE: Dial the specified number
//
//  PARAMETERS:
//    number - string containing phone number to dial
//
//  RETURN VALUE:
//    TRUE if able to get a number, find a line, and dial successfully.
//
//  COMMENTS:
//
//    This function makes several assumptions:
//    - The number dialed will always explicitly come from the user.
//    - There will only be one outgoing address per line.
//

bool TAPITransport::DialCall( const char * phone_number )
{
	char phoneNumber[32];
   bool call_successful = false;
   LPLINEDEVCAPS lpLineDevCaps = NULL;

   if(!phone_number || phone_number[0] == 0)
   	return false;
   strcpy(phoneNumber,phone_number);
   log_file.log("Dialing %s", phoneNumber);

   if( !g_hLine ) // line hasn't successfully opened
   {
   	log_file.log(LOG_ERROR,"Dial failed, line not open");
      return false;
   }

   // Get a phone number
   // Phone number will be placed in global variables if successful
	if( !TranslatePhoneNumber(phoneNumber) )
   {
      log_file.log(LOG_ERROR,"Bad phone number: TranslatePhoneNumber failed");
      HangupCall();
      goto DeleteBuffers;
   }

    // Need to check the DevCaps to make sure this line is usable.
    lpLineDevCaps = I_lineGetDevCaps(lpLineDevCaps,g_dwDeviceID, g_dwAPIVersion);
    if (lpLineDevCaps == NULL)
    {
        HangupCall();
        log_file.log(LOG_ERROR,"Error on Requested line, Unable to Use Line");
        goto DeleteBuffers;
    }

    // Does this line have the capability to make calls?
    // It is possible that some lines can't make outbound calls.
    if (!(lpLineDevCaps->dwLineFeatures & LINEFEATURE_MAKECALL))
    {
        HangupCall();
        log_file.log(LOG_ERROR,"Error on Requested line, The selected line doesn't support MAKECALL capabilities");
        goto DeleteBuffers;
    }

    // Start dialing the number
    g_bCalling = TRUE;
    if (DialCallInParts(lpLineDevCaps, g_szDialableAddress,
            g_szDisplayableAddress))
    {
        log_file.log("DialCallInParts succeeded.");
    }
    else
    {
        log_file.log(LOG_ERROR,"DialCallInParts failed.");
        HangupCall();
        goto DeleteBuffers;
    }

    call_successful = true;

DeleteBuffers:

    if (lpLineDevCaps)
        LocalFree(lpLineDevCaps);

    return call_successful;
}


//**************************************************
// These APIs are specific to this module 
//**************************************************



//
//  FUNCTION: DialCallInParts(LPLINEDEVCAPS, LPCSTR, LPCSTR)
//
//  PURPOSE: Dials the call, handling special characters.
//
//  PARAMETERS:
//    lpLineDevCaps - LINEDEVCAPS for the line to be used.
//    lpszAddress   - Address to Dial.
//    lpszDisplayableAddress - Displayable Address.
//
//  RETURN VALUE:
//    Returns TRUE if we successfully Dial.
//
//  COMMENTS:
//
//    This function dials the Address and handles any
//    special characters in the address that the service provider
//    can't handle.  It requires input from the user to handle
//    these characters; this can cause problems for fully automated
//    dialing.
//
//    Note that we can return TRUE, even if we don't reach a
//    CONNECTED state.  DIalCallInParts returns as soon as the
//    Address is fully dialed or when an error occurs.
//
//

bool TAPITransport::DialCallInParts(LPLINEDEVCAPS lpLineDevCaps,
    LPCSTR lpszAddress, LPCSTR lpszDisplayableAddress)
{
    LPLINECALLPARAMS  lpCallParams = NULL;
    LPLINEADDRESSCAPS lpAddressCaps = NULL;
    LPLINECALLSTATUS  lpLineCallStatus = NULL;

    long lReturn;
    int i;
    DWORD dwDevCapFlags;
    char szFilter[1+sizeof(g_sNonDialable)] = "";
    bool bFirstDial = TRUE;

    // Variables to handle Dialable Substring dialing.
    LPSTR lpDS; // This is just so we can free lpszDialableSubstring later.
    LPSTR lpszDialableSubstring;
    int nAddressLength;
    int nCurrentAddress = 0;
    char chUnhandledCharacter;

    // Get the capabilities for the line device we're going to use.
    lpAddressCaps = I_lineGetAddressCaps(lpAddressCaps,
        g_dwDeviceID, 0, g_dwAPIVersion, 0);
    if (lpAddressCaps == NULL)
        return FALSE;

    // Setup our CallParams for DATAMODEM settings.
    lpCallParams = CreateCallParams (lpCallParams, lpszDisplayableAddress);
    if (lpCallParams == NULL)
        return FALSE;

    // Determine which special characters the service provider
    // does *not* handle so we can handle them manually.
    // Keep list of unhandled characters in szFilter.

    dwDevCapFlags = lpLineDevCaps -> dwDevCapFlags;  // SP handled characters.
    for (i = 0; i < g_sizeofNonDialable ; i++)
    {
        if ((dwDevCapFlags & g_sNonDialable[i].dwDevCapFlag) == 0)
        {
            strcat(szFilter, g_sNonDialable[i].szToken);
        }
    }

    // szFilter now contains the set of tokens which delimit dialable substrings

    // Setup the strings for substring dialing.

    nAddressLength = strlen(lpszAddress);
    lpDS = lpszDialableSubstring = (LPSTR) LocalAlloc(LPTR, nAddressLength + 1);
    if (lpszDialableSubstring == NULL)
    {
        log_file.log(LOG_ERROR,"LocalAlloc failed: %lx",(long)GetLastError());
        HandleNoMem();
        goto errExit;
    }

//log_file.log("Dialing Call in parts");
    // Lets start dialing substrings!
    while (nCurrentAddress < nAddressLength)
    {
  retryAfterError:

        // Find the next undialable character
        i = strcspn(&lpszAddress[nCurrentAddress], szFilter);

        // Was there one before the end of the Address string?
        if (i + nCurrentAddress < nAddressLength)
        {
            // Make sure this device can handle partial dial.
            if (! (lpAddressCaps -> dwAddrCapFlags &
                   LINEADDRCAPFLAGS_PARTIALDIAL))
            {
                log_file.log(LOG_ERROR,"This line doesn't support partial dialing");
                goto errExit;
            }
            // Remember what the unhandled character is so we can handle it.
            chUnhandledCharacter = lpszAddress[nCurrentAddress+i];

            // Copy the dialable string to the Substring.
            memcpy(lpszDialableSubstring, &lpszAddress[nCurrentAddress], i);

            // Terminate the substring with a ';' to signify the partial dial.
            lpszDialableSubstring[i] = ';';
            lpszDialableSubstring[i+1] = '\0';

            // Increment the address for next iteration.
            nCurrentAddress += i + 1;
        }
        else // No more partial dials.  Dial the rest of the Address.
        {
            lpszDialableSubstring = (LPSTR) &lpszAddress[nCurrentAddress];
            chUnhandledCharacter = 0;
            nCurrentAddress = nAddressLength;
        }

        do
        {
//log_file.log(lpszDialableSubstring);
				if (bFirstDial)
            {
               lReturn = WaitForReply( lineMakeCall(g_hLine, &g_hCall,
                                       lpszDialableSubstring, 0, lpCallParams));
            }
            else
                lReturn = WaitForReply(
                    lineDial(g_hCall, lpszDialableSubstring, 0) );

            switch(lReturn)
            {
                // We should not have received these errors because of the
                // prefiltering strategy, but there may be some ill-behaved
                // service providers which do not correctly set their
                // devcapflags.  Add the character corresponding to the error
                // to the filter set and retry dialing.
                //
                case LINEERR_DIALBILLING:
                case LINEERR_DIALDIALTONE:
                case LINEERR_DIALQUIET:
                case LINEERR_DIALPROMPT:
                {
                    log_file.log(LOG_ERROR,"Service Provider incorrectly sets dwDevCapFlags");

                    for (i = 0; i < g_sizeofNonDialable; i++)
                        if (lReturn == g_sNonDialable[i].lError)
                        {
                            strcat(szFilter, g_sNonDialable[i].szToken);
                        }

                    goto retryAfterError;
                }

                case WAITERR_WAITABORTED:
                    log_file.log(LOG_ERROR,"While Dialing, WaitForReply aborted.");
                    goto errExit;

            }

            if (!HandleLineErr(lReturn))
            {
                if (bFirstDial)
                    log_file.log(LOG_ERROR,"lineMakeCall unhandled error: %lx",(long)lReturn);
                else
                    log_file.log(LOG_ERROR,"lineDial unhandled error: %lx",(long)lReturn);

                goto errExit;
            }

        }
        while (lReturn != SUCCESS);

        bFirstDial = FALSE;

        // The dial was successful; now handle characters the service
        // provider didn't (if any).
        if (chUnhandledCharacter)
        {
            LPSTR lpMsg = "";

            // First, wait until we know we can continue dialing.  While the
            // last string is still pending to be dialed, we can't dial another.

            while(TRUE)
            {

                lpLineCallStatus = I_lineGetCallStatus(lpLineCallStatus, g_hCall);
                if (lpLineCallStatus == NULL)
                    goto errExit;

                // Does CallStatus say we can dial now?
                if ((lpLineCallStatus->dwCallFeatures) & LINECALLFEATURE_DIAL)
                {
                    log_file.log("Ok to continue dialing.");
                    break;
                }

                // We can't dial yet, so wait for a CALLSTATE message
                log_file.log("Waiting for dialing to be enabled.");

                if (WaitForCallState(I_LINECALLSTATE_ANY) != SUCCESS)
                    goto errExit;
            }

            for (i = 0; i < g_sizeofNonDialable; i++)
                if (chUnhandledCharacter == g_sNonDialable[i].szToken[0])
                    lpMsg = g_sNonDialable[i].szMsg;

            log_file.log("Dialing Paused");
        }

    } // continue dialing until we dial all Dialable Substrings.

    LocalFree(lpCallParams);
    LocalFree(lpDS);
    LocalFree(lpAddressCaps);
    if (lpLineCallStatus)
        LocalFree(lpLineCallStatus);

    return TRUE;

  errExit:
        // if lineMakeCall has already been successfully called, there's a call in progress.
        // let the invoking routine shut down the call.
        // if the invoker did not clean up the call, it should be done here.

    if (lpLineCallStatus)
        LocalFree(lpLineCallStatus);
    if (lpDS)
        LocalFree(lpDS);
    if (lpCallParams)
        LocalFree(lpCallParams);
    if (lpAddressCaps)
        LocalFree(lpAddressCaps);

    return FALSE;
}


//
//  FUNCTION: CreateCallParams(LPLINECALLPARAMS, LPCSTR)
//
//  PURPOSE: Allocates and fills a LINECALLPARAMS structure
//
//  PARAMETERS:
//    lpCallParams -
//    lpszDisplayableAddress -
//
//  RETURN VALUE:
//    Returns a LPLINECALLPARAMS ready to use for dialing DATAMODEM calls.
//    Returns NULL if unable to allocate the structure.
//
//  COMMENTS:
//
//    If a non-NULL lpCallParams is passed in, it must have been allocated
//    with LocalAlloc, and can potentially be freed and reallocated.  It must
//    also have the dwTotalSize field correctly set.
//
//

LPLINECALLPARAMS TAPITransport::CreateCallParams (
    LPLINECALLPARAMS lpCallParams, LPCSTR lpszDisplayableAddress)
{
    size_t sizeDisplayableAddress;

    if (lpszDisplayableAddress == NULL)
        lpszDisplayableAddress = "";

    sizeDisplayableAddress = strlen(lpszDisplayableAddress) + 1;

    lpCallParams = (LPLINECALLPARAMS) CheckAndReAllocBuffer(
        (LPVOID) lpCallParams,
        sizeof(LINECALLPARAMS) + sizeDisplayableAddress,
        "CreateCallParams: ");

    if (lpCallParams == NULL)
        return NULL;

    // This is where we configure the line for DATAMODEM usage.
    lpCallParams -> dwBearerMode = LINEBEARERMODE_VOICE;
    lpCallParams -> dwMediaMode  = LINEMEDIAMODE_DATAMODEM;

    // This specifies that we want to use only IDLE calls and
    // don't want to cut into a call that might not be IDLE (ie, in use).
    lpCallParams -> dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;

    // if there are multiple addresses on line, use first anyway.
    // It will take a more complex application than a simple tty app
    // to use multiple addresses on a line anyway.
    lpCallParams -> dwAddressMode = LINEADDRESSMODE_ADDRESSID;
    lpCallParams -> dwAddressID = 0;

    // Since we don't know where we originated, leave these blank.
    lpCallParams -> dwOrigAddressSize = 0;
    lpCallParams -> dwOrigAddressOffset = 0;

    // Unimodem ignores these values.
    (lpCallParams -> DialParams) . dwDialSpeed = 0;
    (lpCallParams -> DialParams) . dwDigitDuration = 0;
    (lpCallParams -> DialParams) . dwDialPause = 0;
    (lpCallParams -> DialParams) . dwWaitForDialtone = 0;

    // Address we are dialing.
    lpCallParams -> dwDisplayableAddressOffset = sizeof(LINECALLPARAMS);
    lpCallParams -> dwDisplayableAddressSize = sizeDisplayableAddress;
    strcpy((LPSTR)lpCallParams + sizeof(LINECALLPARAMS),
           lpszDisplayableAddress);

    return lpCallParams;
}


//
//  FUNCTION: long WaitForReply(long)
//
//  PURPOSE: Resynchronize by waiting for a LINE_REPLY 
//
//  PARAMETERS:
//    lRequestID - The asynchronous request ID that we're
//                 on a LINE_REPLY for.
//
//  RETURN VALUE:
//    - 0 if LINE_REPLY responded with a success.
//    - LINEERR constant if LINE_REPLY responded with a LINEERR
//    - 1 if the line was shut down before LINE_REPLY is received.
//
//  COMMENTS:
//
//    This function allows us to resynchronize an asynchronous
//    TAPI line call by waiting for the LINE_REPLY message.  It
//    waits until a LINE_REPLY is received or the line is shut down.
//
//    Note that this could cause re-entrancy problems as
//    well as mess with any message preprocessing that might
//    occur on this thread (such as TranslateAccelerator).
//
//    This function should to be called from the thread that did
//    lineInitialize, or the PeekMessage is on the wrong thread
//    and the synchronization is not guaranteed to work.  Also note
//    that if another PeekMessage loop is entered while waiting,
//    this could also cause synchronization problems.
//
//    One more note.  This function can potentially be re-entered
//    if the call is dropped for any reason while waiting.  If this
//    happens, just drop out and assume the wait has been canceled.
//    This is signaled by setting bReentered to FALSE when the function
//    is entered and TRUE when it is left.  If bReentered is ever TRUE
//    during the function, then the function was re-entered.
//
//    This function times out and returns WAITERR_WAITTIMEDOUT
//    after WAITTIMEOUT milliseconds have elapsed.
//
//


long TAPITransport::WaitForReply (long lRequestID)
{
    static bool bReentered;
    bReentered = FALSE;

    if (lRequestID > SUCCESS)
    {
        MSG msg;
        DWORD dwTimeStarted;

        g_bReplyReceived = FALSE;
        g_dwRequestedID = (DWORD) lRequestID;

        // Initializing this just in case there is a bug
        // that sets g_bReplyReceived without setting the reply value.
        g_lAsyncReply = LINEERR_OPERATIONFAILED;

        dwTimeStarted = GetTickCount();

        while(!g_bReplyReceived)
        {
            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            // This should only occur if the line is shut down while waiting.
            if (!g_hLine || bReentered)
            {
                bReentered = TRUE;
                return WAITERR_WAITABORTED;
            }

            // Its a really bad idea to timeout a wait for a LINE_REPLY.
            // If we are execting a LINE_REPLY, we should wait till we get
            // it; it might take a long time to dial (for example).

            // If 5 seconds go by without a reply, it might be a good idea
            // to display a dialog box to tell the user that a
            // wait is in progress and to give the user the capability to
            // abort the wait.
        }

        bReentered = TRUE;
        return g_lAsyncReply;
    }

    bReentered = TRUE;
    return lRequestID;
}


//
//  FUNCTION: long WaitForCallState(DWORD)
//
//  PURPOSE: Wait for the line to reach a specific CallState.
//
//  PARAMETERS:
//    dwDesiredCallState - specific CallState to wait for.
//
//  RETURN VALUE:
//    Returns 0 (SUCCESS) when we reach the Desired CallState.
//    Returns WAITERR_WAITTIMEDOUT if timed out.
//    Returns WAITERR_WAITABORTED if call was closed while waiting.
//
//  COMMENTS:
//
//    This function allows us to synchronously wait for a line
//    to reach a specific LINESTATE or until the line is shut down.
//
//    Note that this could cause re-entrancy problems as
//    well as mess with any message preprocessing that might
//    occur on this thread (such as TranslateAccelerator).
//
//    One more note.  This function can potentially be re-entered
//    if the call is dropped for any reason while waiting.  If this
//    happens, just drop out and assume the wait has been canceled.  
//    This is signaled by setting bReentered to FALSE when the function 
//    is entered and TRUE when it is left.  If bReentered is ever TRUE 
//    during the function, then the function was re-entered.
//
//    This function should to be called from the thread that did
//    lineInitialize, or the PeekMessage is on the wrong thread
//    and the synchronization is not guaranteed to work.  Also note
//    that if another PeekMessage loop is entered while waiting,
//    this could also cause synchronization problems.
//
//    If the constant value I_LINECALLSTATE_ANY is used for the 
//    dwDesiredCallState, then WaitForCallState will return SUCCESS
//    upon receiving any CALLSTATE messages.
//    
//
//

long TAPITransport::WaitForCallState(DWORD dwDesiredCallState)
{
    MSG msg;
    DWORD dwTimeStarted;
    static bool bReentered;

    bReentered = FALSE;

    dwTimeStarted = GetTickCount();

    g_bCallStateReceived = FALSE;

    while ((dwDesiredCallState == I_LINECALLSTATE_ANY) || 
           (g_dwCallState != dwDesiredCallState))
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // If we are waiting for any call state and get one, succeed.
        if ((dwDesiredCallState == I_LINECALLSTATE_ANY) && g_bCallStateReceived)
        {
            break;
        }

        // This should only occur if the line is shut down while waiting.
        if (!g_hLine || bReentered)
        {
            bReentered = TRUE;
            log_file.log(LOG_ERROR,"WAITABORTED");
            return WAITERR_WAITABORTED;
        }

        // If we don't get the reply in a reasonable time, we time out.
        if (GetTickCount() - dwTimeStarted > WAITTIMEOUT)
        {
            bReentered = TRUE;
            log_file.log(LOG_ERROR,"WAITTIMEDOUT");
            return WAITERR_WAITTIMEDOUT;
        }

    }

    bReentered = TRUE;
    return SUCCESS;
}

//**************************************************
// lineCallback Function and Handlers.
//**************************************************


//
//  FUNCTION: lineCallbackFunc(..)
//
//  PURPOSE: Receive asynchronous TAPI events
//
//  PARAMETERS:
//    dwDevice  - Device associated with the event, if any
//    dwMsg     - TAPI event that occurred.
//    dwCallbackInstance - User defined data supplied when opening the line.
//    dwParam1  - dwMsg specific information
//    dwParam2  - dwMsg specific information
//    dwParam3  - dwMsg specific information
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    This is the function where all asynchronous events will come.
//    Almost all events will be specific to an open line, but a few
//    will be general TAPI events (such as LINE_REINIT).
//
//    Its important to note that this callback will *ALWAYS* be
//    called in the context of the thread that does the lineInitialize.
//    Even if another thread (such as the COMM threads) calls the API
//    that would result in the callback being called, it will be called
//    in the context of the main thread (since in this sample, the main
//    thread does the lineInitialize).
//
//


void CALLBACK TAPITransport::lineCallbackFunc(
    DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
log_file.log("Callback %lx %lx %lx %lx %lx",(long)dwDevice,(long)dwCallbackInstance,(long)dwParam1,(long)dwParam2,(long)dwParam3);

	TAPITransport *myThis = GetMyThis(dwCallbackInstance);
   if( dwMsg != LINE_CREATE && myThis == NULL )
   {
   	log_file.log(LOG_ERROR,"lineCallbackFunc with NULL myThis");
      return;
   }


    // All we do is dispatch the dwMsg to the correct handler.
    switch(dwMsg)
    {
        case LINE_CALLSTATE:
				AssertFatal(myThis != NULL,"lineCallbackFunc with NULL myThis");
            myThis->DoLineCallState(dwDevice, dwMsg, dwCallbackInstance,
                dwParam1, dwParam2, dwParam3);
            break;

        case LINE_CLOSE:
				AssertFatal(myThis != NULL,"lineCallbackFunc with NULL myThis");
            myThis->DoLineClose(dwDevice, dwMsg, dwCallbackInstance,
                dwParam1, dwParam2, dwParam3);
            break;

        case LINE_LINEDEVSTATE:
				AssertFatal(myThis != NULL,"lineCallbackFunc with NULL myThis");
            myThis->DoLineDevState(dwDevice, dwMsg, dwCallbackInstance,
                dwParam1, dwParam2, dwParam3);
            break;

        case LINE_REPLY:
				AssertFatal(myThis != NULL,"lineCallbackFunc with NULL myThis");
            myThis->DoLineReply(dwDevice, dwMsg, dwCallbackInstance,
                dwParam1, dwParam2, dwParam3);
            break;

        case LINE_CREATE:
				log_file.log(LOG_ERROR,"A Line device has been created; no action taken.");
            //DoLineCreate(dwDevice, dwMsg, dwCallbackInstance,
            //    dwParam1, dwParam2, dwParam3);
            break;

        default:
            log_file.log(LOG_ERROR,"lineCallbackFunc message ignored");
            break;

    }

    return;

}


//
//  FUNCTION: DoLineReply(..)
//
//  PURPOSE: Handle LINE_REPLY asynchronous messages.
//
//  PARAMETERS:
//    dwDevice  - Line Handle associated with this LINE_REPLY.
//    dwMsg     - Should always be LINE_REPLY.
//    dwCallbackInstance - Unused by this sample.
//    dwParam1  - Asynchronous request ID.
//    dwParam2  - success or LINEERR error value.
//    dwParam3  - Unused.
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    All line API calls that return an asynchronous request ID
//    will eventually cause a LINE_REPLY message.  Handle it.
//
//    This sample assumes only one call at time, and that we wait
//    for a LINE_REPLY before making any other line API calls.
//
//    The only exception to the above is that we might shut down
//    the line before receiving a LINE_REPLY.
//
//

void TAPITransport::DoLineReply(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
	if ((long) dwParam2 != SUCCESS)
    	log_file.log(LOG_ERROR,"LINE_REPLY error: %lx",(long)dwParam2);
	else
		log_file.log("LINE_REPLY: successfully replied.");

    // If we are currently waiting for this async Request ID
    // then set the global variables to acknowledge it.
    if (g_dwRequestedID == dwParam1)
    {
        g_bReplyReceived = TRUE;
        g_lAsyncReply = (long) dwParam2;
    }
}


//
//  FUNCTION: DoLineClose(..)
//
//  PURPOSE: Handle LINE_CLOSE asynchronous messages.
//
//  PARAMETERS:
//    dwDevice  - Line Handle that was closed.
//    dwMsg     - Should always be LINE_CLOSE.
//    dwCallbackInstance - Unused by this sample.
//    dwParam1  - Unused.
//    dwParam2  - Unused.
//    dwParam3  - Unused.
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    This message is sent when something outside our app shuts
//    down a line in use.
//
//    The hLine (and any hCall on this line) are no longer valid.
//
//

void TAPITransport::DoLineClose(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    // Line has been shut down.  Clean up our internal variables.
    g_hLine = NULL;
    g_hCall = NULL;
    log_file.log("Call was shut down");
    HangupCall();
}


//
//  FUNCTION: DoLineDevState(..)
//
//  PURPOSE: Handle LINE_LINEDEVSTATE asynchronous messages.
//
//  PARAMETERS:
//    dwDevice  - Line Handle that was closed.
//    dwMsg     - Should always be LINE_LINEDEVSTATE.
//    dwCallbackInstance - Unused by this sample.
//    dwParam1  - LINEDEVSTATE constant.
//    dwParam2  - Depends on dwParam1.
//    dwParam3  - Depends on dwParam1.
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    The LINE_LINEDEVSTATE message is received if the state of the line
//    changes.  Examples are RINGING, MAINTENANCE, MSGWAITON.  Very few of
//    these are relevant to this sample.
//
//    Assuming that any LINEDEVSTATE that removes the line from use by TAPI
//    will also send a LINE_CLOSE message.
//
//

void TAPITransport::DoLineDevState(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    switch(dwParam1)
    {
        case LINEDEVSTATE_RINGING:
            log_file.log("Line Ringing.");
            break;

        case LINEDEVSTATE_REINIT:
        // This is an important case!  Usually means that a service provider
        // has changed in such a way that requires TAPI to REINIT.  
        // Note that there are both 'soft' REINITs and 'hard' REINITs.
        // Soft REINITs don't actually require a full shutdown but is instead
        // just an informational change that historically required a REINIT
        // to force the application to deal with.  TAPI API Version 1.3 apps
        // will still need to do a full REINIT for both hard and soft REINITs.

            switch(dwParam2)
            {
                // This is the hard REINIT.  No reason given, just REINIT.
                // TAPI is waiting for everyone to shutdown.
                // Our response is to immediately shutdown any calls,
                // shutdown our use of TAPI and notify the user.
                case 0:
                    ShutdownTAPI();
                    WarningBox("Tapi line configuration has changed.");
                    break;

                case LINE_CREATE:
                    log_file.log(LOG_ERROR,"Soft REINIT: LINE_CREATE.");
                    DoLineCreate(dwDevice, dwParam2, dwCallbackInstance,
                        dwParam3, 0, 0);
                    break;

                case LINE_LINEDEVSTATE:
                    log_file.log(LOG_ERROR,"Soft REINIT: LINE_LINEDEVSTATE.");
                    DoLineDevState(dwDevice, dwParam2, dwCallbackInstance,
                        dwParam3, 0, 0);
                    break;

                // There might be other reasons to send a soft reinit.
                // No need to to shutdown for these.
                default:
                    log_file.log(LOG_ERROR,"Ignoring soft REINIT");
                    break;
            }
            break;

        case LINEDEVSTATE_OUTOFSERVICE:
            WarningBox("Line selected is now Out of Service.");
            HangupCall();
            break;

        case LINEDEVSTATE_DISCONNECTED:
            WarningBox("Line selected is now disconnected.");
            HangupCall();
            break;

        case LINEDEVSTATE_MAINTENANCE:
            WarningBox("Line selected is now out for maintenance.");
            HangupCall();
            break;

        case LINEDEVSTATE_TRANSLATECHANGE:
//            if (g_hDialog)
//                PostMessage(g_hDialog, WM_COMMAND, IDC_CONFIGURATIONCHANGED, 0);
/*
                    FillLocationInfo(hwndDlg, NULL, NULL, NULL);
                    DisplayPhoneNumber(hwndDlg);

                    MessageBox(hwndDlg,
                        "Location Configuration has been changed.",
                        "Warning",MB_OK);
*/
            break;

        case LINEDEVSTATE_REMOVED:
            log_file.log(LOG_ERROR,"A Line device has been removed;"
                " no action taken.");
            break;

        default:
            log_file.log(LOG_ERROR,"Unhandled LINEDEVSTATE message");
    }
}


//
//  FUNCTION: DoLineCreate(..)
//
//  PURPOSE: Handle LINE_LINECREATE asynchronous messages.
//
//  PARAMETERS:
//    dwDevice  - Unused.
//    dwMsg     - Should always be LINE_CREATE.
//    dwCallbackInstance - Unused.
//    dwParam1  - dwDeviceID of new Line created.
//    dwParam2  - Unused.
//    dwParam3  - Unused.
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    This message is new for Windows 95.  It is sent when a new line is
//    added to the system.  This allows us to handle new lines without having
//    to REINIT.  This allows for much more graceful Plug and Play.
//
//    This sample just changes the number of devices available and can use
//    it next time a call is made.  It also tells the "Dial" dialog.
//
//

void TAPITransport::DoLineCreate(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
    // dwParam1 is the Device ID of the new line.
    // Add one to get the number of total devices.
#if 0
    if (g_dwNumDevs <= dwParam1)
        g_dwNumDevs = dwParam1+1;
    if (g_hDialog)
        PostMessage(g_hDialog, WM_COMMAND, IDC_LINECREATE, 0);
/*
                    FillTAPILine(hwndDlg);
*/
#endif
	log_file.log("A Line device has been created; no action taken.");
}


//
//  FUNCTION: DoLineCallState(..)
//
//  PURPOSE: Handle LINE_CALLSTATE asynchronous messages.
//
//  PARAMETERS:
//    dwDevice  - Handle to Call who's state is changing.
//    dwMsg     - Should always be LINE_CALLSTATE.
//    dwCallbackInstance - Unused by this sample. 
//    dwParam1  - LINECALLSTATE constant specifying state change.
//    dwParam2  - Specific to dwParam1.
//    dwParam3  - LINECALLPRIVILEGE change, if any.
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    This message is received whenever a call changes state.  Lots of
//    things we do, ranging from notifying the user to closing the line
//    to actually connecting to the target of our phone call.
//
//    What we do is usually obvious based on the call state change.
//

void TAPITransport::DoLineCallState(
    DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
	long lError;
log_file.log("Call State %ld %ld %ld",(long)dwParam1,(long)dwParam2,(long)dwParam3);
    // Error if this CALLSTATE doesn't apply to our call in progress.
	if ( g_bCalling && ((HCALL) dwDevice != g_hCall))
	{
		log_file.log(LOG_ERROR,"LINE_CALLSTATE: Unknown device ID: %lx",(long)dwDevice);
		return;
	}

    // This sets the global g_dwCallState variable so if we are waiting
    // for a specific call state change, we will know when it happens.
    g_dwCallState = dwParam1;
    g_bCallStateReceived = TRUE;

    // dwParam3 contains changes to LINECALLPRIVILEGE, if there are any.
    switch (dwParam3)
    {
        case 0:
            break; // no change to call state

         // close line if we are made monitor.  Shouldn't happen!
         case LINECALLPRIVILEGE_MONITOR:
            log_file.log(LOG_ERROR,"line given monitor privilege");
				break;

         // close line if we are made owner.  Shouldn't happen!
        case LINECALLPRIVILEGE_OWNER:
            log_file.log(LOG_ERROR,"line given owner privilege");
            break;

        default: // Shouldn't happen!  All cases handled.
            log_file.log(LOG_ERROR,"Unknown LINECALLPRIVILEGE message: closing");
            HangupCall();
            return;
    }

    // dwParam1 is the specific CALLSTATE change that is occurring.
	switch (dwParam1)
   {
		case LINECALLSTATE_OFFERING:
      	log_file.log(LOG_ERROR,"Offering");
			log_file.log(LOG_ERROR,"Priviledge: %lx",(long)dwParam3);
         // accept incoming call
         if( (lError = lineAnswer((HCALL) dwDevice, NULL, 0)) < SUCCESS)
         {
         	log_file.log(LOG_ERROR,"lineAnswer error: %lx",(long)lError);
         }
        	break;

      case LINECALLSTATE_ACCEPTED:
     		log_file.log("Accepted");
     		break;

      case LINECALLSTATE_DIALTONE:
         log_file.log("Dial Tone");
         break;

      case LINECALLSTATE_DIALING:
         log_file.log("Dialing");
         break;

      case LINECALLSTATE_PROCEEDING:
         log_file.log("Proceeding");
         break;

      case LINECALLSTATE_RINGBACK:
         log_file.log("RingBack");
         break;

      case LINECALLSTATE_BUSY:
         log_file.log("Line busy, shutting down");
         HangupCall();
         break;

      case LINECALLSTATE_IDLE:
         log_file.log("Line idle");
         HangupCall();
         break;

      case LINECALLSTATE_SPECIALINFO:
         log_file.log(
             "Special Info, probably couldn't dial number");
         HangupCall();
         break;

      case LINECALLSTATE_DISCONNECTED:
      {
         LPSTR pszReasonDisconnected;
         switch (dwParam2)
         {
            case LINEDISCONNECTMODE_NORMAL:
                pszReasonDisconnected = "Remote Party Disconnected";
                break;

            case LINEDISCONNECTMODE_UNKNOWN:
                pszReasonDisconnected = "Disconnected: Unknown reason";
                break;

            case LINEDISCONNECTMODE_REJECT:
                pszReasonDisconnected = "Remote Party rejected call";
                break;

            case LINEDISCONNECTMODE_PICKUP:
                pszReasonDisconnected =
                    "Disconnected: Local phone picked up";
                break;

            case LINEDISCONNECTMODE_FORWARDED:
                pszReasonDisconnected = "Disconnected: Forwarded";
                break;

            case LINEDISCONNECTMODE_BUSY:
                pszReasonDisconnected = "Disconnected: Busy";
                break;

            case LINEDISCONNECTMODE_NOANSWER:
                pszReasonDisconnected = "Disconnected: No Answer";
                break;

            case LINEDISCONNECTMODE_BADADDRESS:
                pszReasonDisconnected = "Disconnected: Bad Address";
                break;

            case LINEDISCONNECTMODE_UNREACHABLE:
                pszReasonDisconnected = "Disconnected: Unreachable";
                break;

            case LINEDISCONNECTMODE_CONGESTION:
                pszReasonDisconnected = "Disconnected: Congestion";
                break;

            case LINEDISCONNECTMODE_INCOMPATIBLE:
                pszReasonDisconnected = "Disconnected: Incompatible";
                break;

            case LINEDISCONNECTMODE_UNAVAIL:
                pszReasonDisconnected = "Disconnected: Unavail";
                break;

            case LINEDISCONNECTMODE_NODIALTONE:
                pszReasonDisconnected = "Disconnected: No Dial Tone";
                break;

            default:
                pszReasonDisconnected =
                    "Disconnected: LINECALLSTATE; Bad Reason";
                break;

         }

         log_file.log(pszReasonDisconnected);
         HangupCall();
         break;
      }


      case LINECALLSTATE_CONNECTED:  // CONNECTED!!!
      {
         LPVARSTRING lpVarString = NULL;
         DWORD dwSizeofVarString = sizeof(VARSTRING) + 1024;
         long lReturn;

	 		log_file.log("Connected");
         // Very first, make sure this isn't a duplicated message.
         // A CALLSTATE message can be sent whenever there is a
         // change to the capabilities of a line, meaning that it is
         // possible to receive multiple CONNECTED messages per call.
         // The CONNECTED CALLSTATE message is the only one in TapiComm
         // where it would cause problems if it where sent more
         // than once.

         if (g_bConnected)
         {
				log_file.log("   already connected, leaving...");
            break;
         }
         g_bConnected = true;

         // Get the handle to the comm port from the driver so we can start
         // communicating.  This is returned in a LPVARSTRING structure.
         do
         {
            // Allocate the VARSTRING structure
            lpVarString = (LPVARSTRING)CheckAndReAllocBuffer((LPVOID) lpVarString,
                dwSizeofVarString,"lineGetID: ");

            if (lpVarString == NULL)
            {
					log_file.log(LOG_ERROR,"   can't alloc lpVarString, leaving...");
               goto ErrorConnecting;
            }

            // Fill the VARSTRING structure
         	g_hCall = (HCALL) dwDevice;
            lReturn = lineGetID(0, 0, g_hCall, LINECALLSELECT_CALL,
                lpVarString, "comm/datamodem");

            if (!HandleLineErr(lReturn))
            {
            	log_file.log(LOG_ERROR,"lineGetID unhandled error: %lx",(long)lReturn);
               goto ErrorConnecting;
            }                                                                                             

            // If the VARSTRING wasn't big enough, loop again.
            if ((lpVarString -> dwNeededSize) > (lpVarString -> dwTotalSize))
            {
               dwSizeofVarString = lpVarString -> dwNeededSize;
               lReturn = -1; // Lets loop again.
            }
         }
         while(lReturn != SUCCESS);

         log_file.log("Connected!  Starting communications!");

         // Again, the handle to the comm port is contained in a
         // LPVARSTRING structure.  Thus, the handle is the very first
         // thing after the end of the structure.  Note that the name of
         // the comm port is right after the handle, but I don't want it.
         g_hCommFile = *((LPHANDLE)((LPBYTE)lpVarString + lpVarString -> dwStringOffset));
//      	log_file.log("COM port handle %lx",g_hCommFile);
         LocalFree(lpVarString);
         break;

         // Couldn't start communications.  Clean up instead.
ErrorConnecting:

         // Its very important that we close all Win32 handles.
         // The CommCode module is responsible for closing the commFile
         // handle if it succeeds in starting communications.
			if (g_hCommFile)
         {
         	CloseHandle(g_hCommFile);
            g_hCommFile = NULL;
         }

         HangupCall();
         {
            char szBuff[300];
            wsprintf(szBuff,"Failed to Connect to '%s'", g_szDisplayableAddress);
         }

         if (lpVarString)
            LocalFree(lpVarString);

         break;
      }

      default:
         log_file.log(LOG_ERROR,"Unhandled LINECALLSTATE message");
         break;
   }
}

//**************************************************
// line API Wrapper Functions.
//**************************************************


//
//  FUNCTION: LPVOID CheckAndReAllocBuffer(LPVOID, size_t, LPCSTR)
//
//  PURPOSE: Checks and ReAllocates a buffer if necessary.
//
//  PARAMETERS:
//    lpBuffer          - Pointer to buffer to be checked.  Can be NULL.
//    sizeBufferMinimum - Minimum size that lpBuffer should be.
//    szApiPhrase       - Phrase to print if an error occurs.
//
//  RETURN VALUE:
//    Returns a pointer to a valid buffer that is guarenteed to be
//    at least sizeBufferMinimum size.
//    Returns NULL if an error occured.
//
//  COMMENTS:
//
//    This function is a helper function intended to make all of the 
//    line API Wrapper Functions much simplier.  It allocates (or
//    reallocates) a buffer of the requested size.
//
//    The returned pointer has been allocated with LocalAlloc,
//    so LocalFree has to be called on it when you're finished with it,
//    or there will be a memory leak.
//
//    Similarly, if a pointer is passed in, it *must* have been allocated
//    with LocalAlloc and it could potentially be LocalFree()d.
//
//    If lpBuffer == NULL, then a new buffer is allocated.  It is
//    normal to pass in NULL for this parameter the first time and only
//    pass in a pointer if the buffer needs to be reallocated.
//
//    szApiPhrase is used only for debugging purposes.
//
//    It is assumed that the buffer returned from this function will be used
//    to contain a variable sized structure.  Thus, the dwTotalSize field
//    is always filled in before returning the pointer.
//
//

LPVOID TAPITransport::CheckAndReAllocBuffer(
    LPVOID lpBuffer, size_t sizeBufferMinimum, LPCSTR szApiPhrase)
{
    size_t sizeBuffer;

    if (lpBuffer == NULL)  // Allocate the buffer if necessary.
    {
        sizeBuffer = sizeBufferMinimum;
        lpBuffer = (LPVOID) LocalAlloc(LPTR, sizeBuffer);

        if (lpBuffer == NULL)
        {
				log_file.log(LOG_ERROR,"LocalAlloc: %lx",(long)GetLastError());
            HandleNoMem();
            return NULL;
        }
    }
    else // If the structure already exists, make sure its good.
    {
        sizeBuffer = LocalSize((HLOCAL) lpBuffer);

        if (sizeBuffer == 0) // Bad pointer?
        {
				log_file.log(LOG_ERROR,"LocalSize: %lx",(long)GetLastError());
            return NULL;
        }

        // Was the buffer big enough for the structure?
        if (sizeBuffer < sizeBufferMinimum)
        {
            log_file.log(LOG_ERROR,"Reallocating structure");
            LocalFree(lpBuffer);
            return CheckAndReAllocBuffer(NULL, sizeBufferMinimum, szApiPhrase);
        }

        // Lets zero the buffer out.
        memset(lpBuffer, 0, sizeBuffer);
    }

    ((LPVARSTRING) lpBuffer ) -> dwTotalSize = (DWORD) sizeBuffer;
    return lpBuffer;
}



//
//  FUNCTION: DWORD I_lineNegotiateAPIVersion(DWORD)
//
//  PURPOSE: Negotiate an API Version to use for a specific device.
//
//  PARAMETERS:
//    dwDeviceID - device to negotiate an API Version for.
//
//  RETURN VALUE:
//    Returns the API Version to use for this line if successful.
//    Returns 0 if negotiations fall through.
//
//  COMMENTS:
//
//    This wrapper function not only negotiates the API, but handles
//    LINEERR errors that can occur while negotiating.
//
//

DWORD TAPITransport::I_lineNegotiateAPIVersion(DWORD dwDeviceID)
{
    LINEEXTENSIONID LineExtensionID;
    long lReturn;
    DWORD dwLocalAPIVersion;

    do
    {
        lReturn = lineNegotiateAPIVersion(g_hLineApp, dwDeviceID,
            SAMPLE_TAPI_VERSION, SAMPLE_TAPI_VERSION,
            &dwLocalAPIVersion, &LineExtensionID);

        if (lReturn == LINEERR_INCOMPATIBLEAPIVERSION)
        {
            log_file.log(LOG_ERROR,"lineNegotiateAPIVersion, INCOMPATIBLEAPIVERSION.");
            return 0;
        }

        if (!HandleLineErr(lReturn))
        {
				log_file.log(LOG_ERROR,"lineNegotiateAPIVersion unhandled error: %lx",(long)lReturn);
            return 0;
        }
    }
    while(lReturn != SUCCESS);

    return dwLocalAPIVersion;
}


//
//  FUNCTION: I_lineGetDevCaps(LPLINEDEVCAPS, DWORD , DWORD)
//
//  PURPOSE: Retrieve a LINEDEVCAPS structure for the specified line.
//
//  PARAMETERS:
//    lpLineDevCaps - Pointer to a LINEDEVCAPS structure to use.
//    dwDeviceID    - device to get the DevCaps for.
//    dwAPIVersion  - API Version to use while getting DevCaps.
//
//  RETURN VALUE:
//    Returns a pointer to a LINEDEVCAPS structure if successful.
//    Returns NULL if unsuccessful.
//
//  COMMENTS:
//
//    This function is a wrapper around lineGetDevCaps to make it easy
//    to handle the variable sized structure and any errors received.
//
//    The returned structure has been allocated with LocalAlloc,
//    so LocalFree has to be called on it when you're finished with it,
//    or there will be a memory leak.
//
//    Similarly, if a lpLineDevCaps structure is passed in, it *must*
//    have been allocated with LocalAlloc and it could potentially be
//    LocalFree()d.
//
//    If lpLineDevCaps == NULL, then a new structure is allocated.  It is
//    normal to pass in NULL for this parameter unless you want to use a
//    lpLineDevCaps that has been returned by a previous I_lineGetDevCaps
//    call.
//
//

LPLINEDEVCAPS TAPITransport::I_lineGetDevCaps(
    LPLINEDEVCAPS lpLineDevCaps,
    DWORD dwDeviceID, DWORD dwAPIVersion)
{
    size_t sizeofLineDevCaps = sizeof(LINEDEVCAPS) + 1024;
    long lReturn;

    // Continue this loop until the structure is big enough.
    while(TRUE)
    {
        // Make sure the buffer exists, is valid and big enough.
        lpLineDevCaps =
            (LPLINEDEVCAPS) CheckAndReAllocBuffer(
                (LPVOID) lpLineDevCaps, // Pointer to existing buffer, if any
                sizeofLineDevCaps,      // Minimum size the buffer should be
                "lineGetDevCaps");      // Phrase to tag errors, if any.

        if (lpLineDevCaps == NULL)
            return NULL;

        // Make the call to fill the structure.
        do
        {
            lReturn =
                lineGetDevCaps(g_hLineApp,
                    dwDeviceID, dwAPIVersion, 0, lpLineDevCaps);

            if (!HandleLineErr(lReturn))
            {
					log_file.log(LOG_ERROR,"lineGetDevCaps unhandled error: %lx",(long)lReturn);
               LocalFree(lpLineDevCaps);
               return NULL;
            }
        }
        while (lReturn != SUCCESS);

        // If the buffer was big enough, then succeed.
        if ((lpLineDevCaps -> dwNeededSize) <= (lpLineDevCaps -> dwTotalSize))
            return lpLineDevCaps;

        // Buffer wasn't big enough.  Make it bigger and try again.
        sizeofLineDevCaps = lpLineDevCaps -> dwNeededSize;
    }
}


//
//  FUNCTION: I_lineGetAddressStatus(LPLINEADDRESSSTATUS, HLINE, DWORD)
//
//  PURPOSE: Retrieve a LINEADDRESSSTATUS structure for the specified line.


//
//  PARAMETERS:
//    lpLineAddressStatus - Pointer to a LINEADDRESSSTATUS structure to use.
//    hLine       - Handle of line to get the AddressStatus of.
//    dwAddressID - Address ID on the hLine to be used.
//
//  RETURN VALUE:
//    Returns a pointer to a LINEADDRESSSTATUS structure if successful.
//    Returns NULL if unsuccessful.
//
//  COMMENTS:
//
//    This function is a wrapper around lineGetAddressStatus to make it easy
//    to handle the variable sized structure and any errors received.
//
//    The returned structure has been allocated with LocalAlloc,
//    so LocalFree has to be called on it when you're finished with it,
//    or there will be a memory leak.
//
//    Similarly, if a lpLineAddressStatus structure is passed in, it *must*
//    have been allocated with LocalAlloc and it could potentially be
//    LocalFree()d.
//
//    If lpLineAddressStatus == NULL, then a new structure is allocated.  It
//    is normal to pass in NULL for this parameter unless you want to use a
//    lpLineAddressStatus that has been returned by previous
//    I_lineGetAddressStatus call.
//
//

LPLINEADDRESSSTATUS TAPITransport::I_lineGetAddressStatus(
    LPLINEADDRESSSTATUS lpLineAddressStatus,
    HLINE hLine, DWORD dwAddressID)
{
    size_t sizeofLineAddressStatus = sizeof(LINEADDRESSSTATUS) + 1024;
    long lReturn;
    
    // Continue this loop until the structure is big enough.
    while(TRUE)
    {
        // Make sure the buffer exists, is valid and big enough.
        lpLineAddressStatus = 
            (LPLINEADDRESSSTATUS) CheckAndReAllocBuffer(
                (LPVOID) lpLineAddressStatus,
                sizeofLineAddressStatus,     
                "lineGetAddressStatus");

        if (lpLineAddressStatus == NULL)
            return NULL;

        // Make the call to fill the structure.
        do
        {            
            lReturn = 
                lineGetAddressStatus(hLine, dwAddressID, lpLineAddressStatus);

            if (!HandleLineErr(lReturn))
            {
					log_file.log(LOG_ERROR,"lineGetAddressStatus unhandled error: %lx",(long)lReturn);
                LocalFree(lpLineAddressStatus);
                return NULL;
            }
        }
        while (lReturn != SUCCESS);

        // If the buffer was big enough, then succeed.
        if ((lpLineAddressStatus -> dwNeededSize) <= 
            (lpLineAddressStatus -> dwTotalSize))
        {
            return lpLineAddressStatus;
        }
        
        // Buffer wasn't big enough.  Make it bigger and try again.
        sizeofLineAddressStatus = lpLineAddressStatus -> dwNeededSize;
    }
}


//
//  FUNCTION: I_lineGetCallStatus(LPLINECALLSTATUS, HCALL)
//
//  PURPOSE: Retrieve a LINECALLSTATUS structure for the specified line.
//
//  PARAMETERS:
//    lpLineCallStatus - Pointer to a LINECALLSTATUS structure to use.
//    hCall - Handle of call to get the CallStatus of.
//
//  RETURN VALUE:
//    Returns a pointer to a LINECALLSTATUS structure if successful.
//    Returns NULL if unsuccessful.
//
//  COMMENTS:
//
//    This function is a wrapper around lineGetCallStatus to make it easy
//    to handle the variable sized structure and any errors received.
//
//    The returned structure has been allocated with LocalAlloc,
//    so LocalFree has to be called on it when you're finished with it,
//    or there will be a memory leak.
//
//    Similarly, if a lpLineCallStatus structure is passed in, it *must*
//    have been allocated with LocalAlloc and it could potentially be 
//    LocalFree()d.
//
//    If lpLineCallStatus == NULL, then a new structure is allocated.  It
//    is normal to pass in NULL for this parameter unless you want to use a
//    lpLineCallStatus that has been returned by previous I_lineGetCallStatus
//    call.
//
//

LPLINECALLSTATUS TAPITransport::I_lineGetCallStatus(
    LPLINECALLSTATUS lpLineCallStatus,
    HCALL hCall)
{
    size_t sizeofLineCallStatus = sizeof(LINECALLSTATUS) + 1024;
    long lReturn;
    
    // Continue this loop until the structure is big enough.
    while(TRUE)
    {
        // Make sure the buffer exists, is valid and big enough.
        lpLineCallStatus = 
            (LPLINECALLSTATUS) CheckAndReAllocBuffer(
                (LPVOID) lpLineCallStatus,
                sizeofLineCallStatus,     
                "lineGetCallStatus");     

        if (lpLineCallStatus == NULL)
            return NULL;
            
        // Make the call to fill the structure.
        do
        {
            lReturn = 
                lineGetCallStatus(hCall, lpLineCallStatus);

            if (!HandleLineErr(lReturn))
            {
					log_file.log(LOG_ERROR,"lineGetCallStatus unhandled error: %lx",(long)lReturn);
                LocalFree(lpLineCallStatus);
                return NULL;
            }
        }
        while (lReturn != SUCCESS);

        // If the buffer was big enough, then succeed.
        if ((lpLineCallStatus -> dwNeededSize) <= 
            (lpLineCallStatus -> dwTotalSize))
        {
            return lpLineCallStatus;
        }

        // Buffer wasn't big enough.  Make it bigger and try again.
        sizeofLineCallStatus = lpLineCallStatus -> dwNeededSize;
    }
}


//
//  FUNCTION: I_lineTranslateAddress
//              (LPLINETRANSLATEOUTPUT, DWORD, DWORD, LPCSTR)
//
//  PURPOSE: Retrieve a LINECALLSTATUS structure for the specified line.
//
//  PARAMETERS:
//    lpLineTranslateOutput - Pointer to a LINETRANSLATEOUTPUT structure.
//    dwDeviceID      - Device that we're translating for.
//    dwAPIVersion    - API Version to use.
//    lpszDialAddress - pointer to the DialAddress string to translate.
//
//  RETURN VALUE:
//    Returns a pointer to a LINETRANSLATEOUTPUT structure if successful.
//    Returns NULL if unsuccessful.
//
//  COMMENTS:
//
//    This function is a wrapper around lineGetTranslateOutput to make it
//    easy to handle the variable sized structure and any errors received.
//
//    The returned structure has been allocated with LocalAlloc,
//    so LocalFree has to be called on it when you're finished with it,
//    or there will be a memory leak.
//
//    Similarly, if a lpLineTranslateOutput structure is passed in, it
//    *must* have been allocated with LocalAlloc and it could potentially be 
//    LocalFree()d.
//
//    If lpLineTranslateOutput == NULL, then a new structure is allocated.
//    It is normal to pass in NULL for this parameter unless you want to use
//    a lpLineTranslateOutput that has been returned by previous 
//    I_lineTranslateOutput call.
//
//

LPLINETRANSLATEOUTPUT TAPITransport::I_lineTranslateAddress(
    LPLINETRANSLATEOUTPUT lpLineTranslateOutput,
    DWORD dwDeviceID, DWORD dwAPIVersion,
    LPCSTR lpszDialAddress)
{
    size_t sizeofLineTranslateOutput = sizeof(LINETRANSLATEOUTPUT) + 1024;
    long lReturn;
    
    // Continue this loop until the structure is big enough.
    while(TRUE)
    {
        // Make sure the buffer exists, is valid and big enough.
        lpLineTranslateOutput = 
            (LPLINETRANSLATEOUTPUT) CheckAndReAllocBuffer(
                (LPVOID) lpLineTranslateOutput,
                sizeofLineTranslateOutput,
                "lineTranslateOutput");

        if (lpLineTranslateOutput == NULL)
            return NULL;

        // Make the call to fill the structure.
        do
        {
            // Note that CALLWAITING is disabled 
            // (assuming the service provider can disable it)
            lReturn = 
                lineTranslateAddress(g_hLineApp, dwDeviceID, dwAPIVersion,
                    lpszDialAddress, 0, 
                    LINETRANSLATEOPTION_CANCELCALLWAITING,
                    lpLineTranslateOutput);

            // If the address isn't translatable, notify the user.
            if (lReturn == LINEERR_INVALADDRESS)
                log_file.log(LOG_ERROR,"Unable to translate phone number");

            if (!HandleLineErr(lReturn))
            {
					log_file.log(LOG_ERROR,"lineTranslateOutput unhandled error: %lx",(long)lReturn);
                LocalFree(lpLineTranslateOutput);
                return NULL;
            }
        }
        while (lReturn != SUCCESS);

        // If the buffer was big enough, then succeed.
        if ((lpLineTranslateOutput -> dwNeededSize) <= 
            (lpLineTranslateOutput -> dwTotalSize))
        {
            return lpLineTranslateOutput;
        }

        // Buffer wasn't big enough.  Make it bigger and try again.
        sizeofLineTranslateOutput = lpLineTranslateOutput -> dwNeededSize;
    }
}


//
//  FUNCTION: I_lineGetAddressCaps(LPLINEADDRESSCAPS, ..)
//
//  PURPOSE: Retrieve a LINEADDRESSCAPS structure for the specified line.
//
//  PARAMETERS:
//    lpLineAddressCaps - Pointer to a LINEADDRESSCAPS, or NULL.
//    dwDeviceID        - Device to get the address caps for.
//    dwAddressID       - This sample always assumes the first address.
//    dwAPIVersion      - API version negotiated for the device.
//    dwExtVersion      - Always 0 for this sample.
//
//  RETURN VALUE:
//    Returns a pointer to a LINEADDRESSCAPS structure if successful.
//    Returns NULL if unsuccessful.
//
//  COMMENTS:
//
//    This function is a wrapper around lineGetAddressCaps to make it easy
//    to handle the variable sized structure and any errors received.
//
//    The returned structure has been allocated with LocalAlloc,
//    so LocalFree has to be called on it when you're finished with it,
//    or there will be a memory leak.
//
//    Similarly, if a lpLineAddressCaps structure is passed in, it *must*
//    have been allocated with LocalAlloc and it could potentially be 
//    LocalFree()d.  It also *must* have the dwTotalSize field set.
//
//    If lpLineAddressCaps == NULL, then a new structure is allocated.  It
//    is normal to pass in NULL for this parameter unless you want to use a
//    lpLineCallStatus that has been returned by previous I_lineGetAddressCaps
//    call.
//
//

LPLINEADDRESSCAPS TAPITransport::I_lineGetAddressCaps (
    LPLINEADDRESSCAPS lpLineAddressCaps,
    DWORD dwDeviceID, DWORD dwAddressID,
    DWORD dwAPIVersion, DWORD dwExtVersion)
{
    size_t sizeofLineAddressCaps = sizeof(LINEADDRESSCAPS) + 1024;
    long lReturn;
    
    // Continue this loop until the structure is big enough.
    while(TRUE)
    {
        // Make sure the buffer exists, is valid and big enough.
        lpLineAddressCaps = 
            (LPLINEADDRESSCAPS) CheckAndReAllocBuffer(
                (LPVOID) lpLineAddressCaps,
                sizeofLineAddressCaps,
                "lineGetAddressCaps");

        if (lpLineAddressCaps == NULL)
            return NULL;
            
        // Make the call to fill the structure.
        do
        {
            lReturn = 
                lineGetAddressCaps(g_hLineApp,
                    dwDeviceID, dwAddressID, dwAPIVersion, dwExtVersion,
                    lpLineAddressCaps);

            if (!HandleLineErr(lReturn))
            {
					log_file.log(LOG_ERROR,"lineGetAddressCaps unhandled error: %lx",(long)lReturn);
                LocalFree(lpLineAddressCaps);
                return NULL;
            }
        }
        while (lReturn != SUCCESS);

        // If the buffer was big enough, then succeed.
        if ((lpLineAddressCaps -> dwNeededSize) <= 
            (lpLineAddressCaps -> dwTotalSize))
        {
            return lpLineAddressCaps;
        }

        // Buffer wasn't big enough.  Make it bigger and try again.
        sizeofLineAddressCaps = lpLineAddressCaps -> dwNeededSize;
    }
}



//**************************************************
// LINEERR Error Handlers
//**************************************************


//
//  FUNCTION: HandleLineErr(long)
//
//  PURPOSE: Handle several standard LINEERR errors
//
//  PARAMETERS:
//    lLineErr - Error code to be handled.
//
//  RETURN VALUE:
//    Return TRUE if lLineErr wasn't an error, or if the
//      error was successfully handled and cleared up.
//    Return FALSE if lLineErr was an unhandled error.
//
//  COMMENTS:
//
//    This is the main error handler for all TAPI line APIs.
//    It handles (by correcting or just notifying the user)
//    most of the errors that can occur while using TAPI line APIs.
//
//    Note that many errors still return FALSE (unhandled) even
//    if a dialog is displayed.  Often, the dialog is just notifying
//    the user why the action was canceled.
//    
//
//

bool TAPITransport::HandleLineErr(long lLineErr)
{
    // lLineErr is really an async request ID, not an error.
    if (lLineErr > SUCCESS)
        return FALSE;

    // All we do is dispatch the correct error handler.
    switch(lLineErr)
    {
        case SUCCESS:
            return TRUE;

        case LINEERR_INVALCARD:
        case LINEERR_INVALLOCATION:
        case LINEERR_INIFILECORRUPT:
				log_file.log(LOG_ERROR,"HandleIniFileCorrupt error");
				return FALSE;

        case LINEERR_NODRIVER:
				log_file.log(LOG_ERROR,"One of the components of the Telephony device driver is missing, Use the Control Panel to set up the driver properly.");
				return FALSE;

        case LINEERR_REINIT:
				ShutdownTAPI();
				return FALSE;

        case LINEERR_NOMULTIPLEINSTANCE:
				log_file.log(LOG_ERROR,"You have two copies of the same Telephony driver installed, Use the Control Panel to remove one of the copies.");
				return FALSE;

        case LINEERR_NOMEM:
            return HandleNoMem();

        case LINEERR_OPERATIONFAILED:
				log_file.log(LOG_ERROR,"TAPI Operation Failed for unknown reasons.");
				return FALSE;

        case LINEERR_RESOURCEUNAVAIL:
				log_file.log(LOG_ERROR,	"A Telephony resource is temporarily unavaiable.  "
									"This could mean a short wait is necessary or "
									"that a non-TAPI application is using the line.");
				return FALSE;

        // Unhandled errors fail.
        default:
				log_file.log(LOG_ERROR,"lineInitialize unhandled error: %lx",(long)lLineErr);
            return FALSE;
    }
}





//
//  FUNCTION: HandleNoMem
//
//  PURPOSE: Handle NOMEM error.
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    TRUE  - error was corrected.
//    FALSE - error was not corrected.
//
//  COMMENTS:
//    This is also called if I run out of memory for LocalAlloc()s
//
//

bool TAPITransport::HandleNoMem()
{
    log_file.log(LOG_ERROR,"Out of Memory error, canceling action., Error");
    return FALSE;
}




//
//  FUNCTION: WarningBox(LPCSTR)
//
//  PURPOSE: Prints a warning box when conditions remove a line in use.
//
//  PARAMETERS:
//    lpszMessage - String that specifies why the line was removed form use.
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    If there is a call in progress on the line removed, then display a message
//    specifying why the line was removed and that the call is being canceled.
//
//

void TAPITransport::WarningBox(LPCSTR lpszMessage)
{
    char szBuff[1024];

    strcpy(szBuff, lpszMessage);

    // If there is a call open, tell user we're going to close it.
    if (g_hCall)
        strcat(szBuff, "Closing existing call.");

    log_file.log(LOG_ERROR,szBuff);
    log_file.log(LOG_ERROR,"Warning");

    strcat(szBuff, "\r");
    log_file.log(LOG_ERROR,szBuff);
}


//**************************************************
//
// All the functions from this point on are used solely by the "Dial" dialog.
// This dialog is used to get both the 'phone number' address,
// the line device to be used as well as allow the user to configure
// dialing properties and the line device.
//
//**************************************************

//
//  FUNCTION: DWORD I_lineNegotiateLegacyAPIVersion(DWORD)
//
//  PURPOSE: Negotiate an API Version to use for a specific device.
//
//  PARAMETERS:
//    dwDeviceID - device to negotiate an API Version for.
//
//  RETURN VALUE:
//    Returns the API Version to use for this line if successful.
//    Returns 0 if negotiations fall through.
//
//  COMMENTS:
//
//    This wrapper is slightly different from the I_lineNegotiateAPIVersion.
//    This wrapper allows TapiComm to negotiate an API version between
//    1.3 and SAMPLE_TAPI_VERSION.  Normally, this sample is specific to 
//    API Version SAMPLE_TAPI_VERSION.  However, there are a few times when
//    TapiComm needs to get information from a service provider, but also knows
//    that a lower API Version would be ok.  This allows TapiComm to recognize
//    legacy service providers even though it can't use them.  1.3 is the
//    lowest API Version a legacy service provider should support.
//
//

DWORD TAPITransport::I_lineNegotiateLegacyAPIVersion(DWORD dwDeviceID)
{
    LINEEXTENSIONID LineExtensionID;
    long lReturn;
    DWORD dwLocalAPIVersion;

    do
    {
        lReturn = lineNegotiateAPIVersion(g_hLineApp, dwDeviceID,
            0x00010003, SAMPLE_TAPI_VERSION,
            &dwLocalAPIVersion, &LineExtensionID);

        if (lReturn == LINEERR_INCOMPATIBLEAPIVERSION)
        {
            log_file.log(LOG_ERROR,"INCOMPATIBLEAPIVERSION in Dial Dialog.");
            return 0;
        }

        if (!HandleLineErr(lReturn))
        {
        		log_file.log(LOG_ERROR,"lineNegotiateAPIVersion unhandled error: %lx",(long)lReturn);
            return 0;
        }
    }
    while(lReturn != SUCCESS);

    return dwLocalAPIVersion;
}


//
//  FUNCTION: long VerifyUsableLine(DWORD,LPSTR)
//
//  PURPOSE: Verifies that a specific line device is useable by TapiComm.
//
//  PARAMETERS:
//    dwDeviceID - The ID of the line device to be verified
//    lpszLineName - string to hold name of line, if NULL, do not find name
//
//  RETURN VALUE:
//    Returns SUCCESS if dwDeviceID is a usable line device.
//    Returns a LINENOTUSEABLE_ constant otherwise.
//
//  COMMENTS:
//
//    VerifyUsableLine takes the give device ID and verifies step by step
//    that the device supports all the features that TapiComm requires.
//
//

long TAPITransport::VerifyUsableLine(DWORD dwDeviceID, LPSTR lpszLineName)
{
   char szLineUnnamed[] = "Line Unnamed";
   char szLineNameEmpty[] = "Line Name is Empty";
   LPLINEDEVCAPS lpLineDevCaps = NULL;
   LPLINEADDRESSSTATUS lpLineAddressStatus = NULL;
   LPVARSTRING lpVarString = NULL;
   long lReturn;
   long lUsableLine = SUCCESS;
   HLINE hLine = 0;

//	log_file.log("Testing Line ID: %lx",(long)dwDeviceID);

   // The line device must support an API Version that TapiComm does.
   g_dwAPIVersion = I_lineNegotiateAPIVersion(dwDeviceID);
   if (g_dwAPIVersion == 0)
   {
      log_file.log(LOG_ERROR,"Line Version unsupported by this Sample, Unable to Use Line");
      return LINENOTUSEABLE_ERROR;
   }

   lpLineDevCaps = I_lineGetDevCaps(lpLineDevCaps, dwDeviceID, g_dwAPIVersion);

   if (lpLineDevCaps == NULL)
   {
      log_file.log(LOG_ERROR,"Error on Requested line, Unable to Use Line");
      return LINENOTUSEABLE_ERROR;
   }

   // Must support LINEBEARERMODE_VOICE
   if (!(lpLineDevCaps->dwBearerModes & LINEBEARERMODE_VOICE ))
   {
      lUsableLine = LINENOTUSEABLE_NOVOICE;
      log_file.log(LOG_ERROR,"LINEBEARERMODE_VOICE not supported");
      goto DeleteBuffers;
   }

   // Must support LINEMEDIAMODE_DATAMODEM
   if (!(lpLineDevCaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM))
   {
      lUsableLine = LINENOTUSEABLE_NODATAMODEM;
      log_file.log(LOG_ERROR,"LINEMEDIAMODE_DATAMODEM not supported");
      goto DeleteBuffers;
   }

   // Must be able to make calls
   if (!(lpLineDevCaps->dwLineFeatures & LINEFEATURE_MAKECALL))
   {
      lUsableLine = LINENOTUSEABLE_NOMAKECALL;
      log_file.log(LOG_ERROR,"LINEFEATURE_MAKECALL not supported");
      goto DeleteBuffers;
   }

   // It is necessary to open the line so we can check if
   // there are any call appearances available.  Other TAPI
   // applications could be using all call appearances.
   // Opening the line also checks for other possible problems.
   do
   {
      lReturn = lineOpen(g_hLineApp, dwDeviceID, &hLine,g_dwAPIVersion, 0, 0,
                         LINECALLPRIVILEGE_NONE, LINEMEDIAMODE_DATAMODEM,0);

      if(lReturn == LINEERR_ALLOCATED)
      {
         log_file.log(LOG_ERROR,"Line is already in use by a non-TAPI app or"
                " another Service Provider.");
         lUsableLine = LINENOTUSEABLE_ALLOCATED;
         goto DeleteBuffers;
      }

      if (!HandleLineErr(lReturn))
      {
      	log_file.log(LOG_ERROR,"lineOpen unhandled error: %lx",(long)lReturn);
         lUsableLine = LINENOTUSEABLE_ERROR;
         goto DeleteBuffers;
      }
   }
   while(lReturn != SUCCESS);

   // Get LineAddressStatus to make sure the line isn't already in use.
   lpLineAddressStatus = I_lineGetAddressStatus(lpLineAddressStatus, hLine, 0);

   if (lpLineAddressStatus == NULL)
   {
      lUsableLine = LINENOTUSEABLE_ERROR;
      log_file.log(LOG_ERROR,"Error on Requested line, Unable to Use Line");
      goto DeleteBuffers;
   }

   // Are there any available call appearances (ie: is it in use)?
   if ( !((lpLineAddressStatus -> dwAddressFeatures) & LINEADDRFEATURE_MAKECALL) )
   {
      log_file.log(LOG_ERROR,"LINEADDRFEATURE_MAKECALL not available");
      lUsableLine = LINENOTUSEABLE_INUSE;
      goto DeleteBuffers;
   }

   // Make sure the "comm/datamodem" device class is supported
   // Note that we don't want any of the 'extra' information
   // normally returned in the VARSTRING structure.  All we care
   // about is if lineGetID succeeds.
   do
   {
      lpVarString = (LPVARSTRING)CheckAndReAllocBuffer((LPVOID) lpVarString,
                     sizeof(VARSTRING),"VerifyUsableLine:lineGetID: ");

      if (lpVarString == NULL)
      {
         lUsableLine = LINENOTUSEABLE_ERROR;
         goto DeleteBuffers;
      }

      lReturn = lineGetID(hLine, 0, 0, LINECALLSELECT_LINE, lpVarString, "comm/datamodem");

      if (!HandleLineErr(lReturn))
      {
      	log_file.log(LOG_ERROR,"lineGetID unhandled error: %lx",(long)lReturn);
         lUsableLine = LINENOTUSEABLE_NOCOMMDATAMODEM;
         goto DeleteBuffers;
      }
   }
   while(lReturn != SUCCESS);

// to get the portname here, change lineGetID string to "comm/datamodem/portname" and add '+100' to lpVarString size
//portname = (char *)((char *)lpVarString + lpVarString->dwStringOffset);
//log_file.log("portname %s",portname);

   // get the name of the device
	if( lpszLineName != NULL )
	{
		LPSTR lineName = szLineUnnamed;

      if ((lpLineDevCaps -> dwLineNameSize) &&
          (lpLineDevCaps -> dwLineNameOffset) &&
     	    (lpLineDevCaps -> dwStringFormat == STRINGFORMAT_ASCII))
     	{
        	// This is the name of the device.
         lineName = ((char *) lpLineDevCaps) + lpLineDevCaps -> dwLineNameOffset;

			if (lineName[0] != '\0')
         {
  	  			// Make sure the device name is null terminated.
				if (lineName[lpLineDevCaps->dwLineNameSize -1] != '\0')
				{
	            // If the device name is not null terminated, null
     			   // terminate it.  Yes, this looses the end character.
	          	// Its a bug in the service provider.
        		   lineName[lpLineDevCaps->dwLineNameSize-1] = '\0';
					log_file.log(LOG_ERROR,"Device name for device is not null terminated.: %lx",(long)dwDeviceID);
			   }
     	   }
        	else // Line name started with a NULL.
           	lineName = szLineNameEmpty;
      }

      strcpy( lpszLineName, lineName);
   }

	log_file.log("Line is suitable and available for use.");

DeleteBuffers:

   if (hLine)
      lineClose(hLine);
   if (lpLineAddressStatus)
      LocalFree(lpLineAddressStatus);
   if (lpLineDevCaps)
      LocalFree(lpLineDevCaps);
   if (lpVarString)
      LocalFree(lpVarString);

   return lUsableLine;
}


//
//  FUNCTION: DWORD GetTAPIDevices( DWORD num_ids, DWORD device_ids[])
//
//  PURPOSE: Inits TAPI, gets all available device ids, the shuts down TAPI.
//
//  PARAMETERS:
//    num_ids - size of the device_ids array
//    device_ids  - array to hold available device ids
//
//  RETURN VALUE:
//    number of devices available
//
//  COMMENTS:
//
//    This function enumerates through all the TAPI line devices and
//    queries each for availability.
//
//

DWORD TAPITransport::GetTAPIDevices( DWORD num_ids, DWORD device_ids[] )
{
   long lReturn;
	DWORD dwDeviceID;
   bool  already_inited = (g_hLineApp != NULL); // if TAPI already inited, don't want to shutdown

   // Initialize TAPI
   if( !already_inited )
   {
      		lReturn = lineInitialize(&g_hLineApp, hInstance, lineCallbackFunc, NULL, &g_dwNumDevs);
		log_file.log(LOG_ERROR,"lineInitialize shows dwNumDevs: %lx",(long)g_dwNumDevs);

	   if (!HandleLineErr(lReturn))
   		return 0;

	   if( g_dwNumDevs > num_ids )
   	{
			log_file.log(LOG_ERROR,"too many TAPI devices: %lx",(long)g_dwNumDevs);
	      g_dwNumDevs = num_ids;
   	}
   }


	// enumerate devices for availability

   num_ids = 0;
   for (dwDeviceID = 0; dwDeviceID < g_dwNumDevs; dwDeviceID++ )
   {
	  	if ( VerifyUsableLine(dwDeviceID,NULL) == SUCCESS )
     	{
      	// if line is available and usable, add to list
      	log_file.log("TAPI device is usable: %ld",dwDeviceID);
         device_ids[num_ids++] = dwDeviceID;
      }
   }

   if( !already_inited )
   {
	   lReturn = lineShutdown(g_hLineApp);
      g_hLineApp = NULL;
   }

   return num_ids;
}

//
//  FUNCTION: bool VerifyAndWarnUsableLine()
//
//  PURPOSE: Verifies the line device selected by the user.
//
//  RETURN VALUE:
//    Returns TRUE if the currently selected line device is useable
//      by TapiComm.  Returns FALSE if it isn't.
//
//  COMMENTS:
//
//    This function is very specific to the "Dial" dialog.  It gets
//    the device selected by the user from the 'TAPI Line' control and
//    VerifyUsableLine to make sure this line device is usable.  If the
//    line isn't useable, it notifies the user and disables the 'Dial'
//    button so that the user can't initiate a call with this line.
//
//    This function is also responsible for filling in the line specific
//    icon found on the "Dial" dialog.
//
//

bool TAPITransport::VerifyAndWarnUsableLine( void )
{
    DWORD dwDeviceID;
    long lReturn;

    // Get the selected line device.
    dwDeviceID = g_dwDeviceID;

    // Verify if the device is usable by TapiComm.
    lReturn = VerifyUsableLine(dwDeviceID,NULL);

    switch(lReturn)
    {
        case SUCCESS:
            g_dwDeviceID = dwDeviceID;
            return TRUE;

        case LINENOTUSEABLE_ERROR:
            log_file.log(LOG_ERROR,"The selected line is incompatible with the TapiComm sample");
            break;
        case LINENOTUSEABLE_NOVOICE:
            log_file.log(LOG_ERROR,"The selected line doesn't support VOICE capabilities");
            break;
        case LINENOTUSEABLE_NODATAMODEM:
            log_file.log(LOG_ERROR,"The selected line doesn't support DATAMODEM capabilities");
            break;
        case LINENOTUSEABLE_NOMAKECALL:
            log_file.log(LOG_ERROR,"The selected line doesn't support MAKECALL capabilities");
            break;
        case LINENOTUSEABLE_ALLOCATED:
            log_file.log(LOG_ERROR,"The selected line is already in use by a non-TAPI application");
            break;
        case LINENOTUSEABLE_INUSE:
            log_file.log(LOG_ERROR,"The selected line is already in use by a TAPI application");
            break;

        case LINENOTUSEABLE_NOCOMMDATAMODEM:
            log_file.log(LOG_ERROR,"The selected line doesn't support the COMM/DATAMODEM device class");
            break;
    }

    // g_dwDeviceID == MAXDWORD mean the selected device isn't usable.
    g_dwDeviceID = MAXDWORD;
    return FALSE;
}


//
//  FUNCTION: bool TranslatePhoneNumber( char *)
//
//  PURPOSE: Create, Translate and Display the Phone Number
//
//  PARAMETERS:
//    phone_number - null terminated string containing phone number
//
//  RETURN VALUE:
//    TRUE if translated successfully, FALSE otherwise
//
//  COMMENTS:
//

bool TAPITransport::TranslatePhoneNumber( char * phone_number )
{
    char szPreTranslatedNumber[1024] = "";
    LPLINETRANSLATEOUTPUT lpLineTranslateOutput = NULL;

    strcpy(szPreTranslatedNumber, phone_number);

   // Translate the address!
   lpLineTranslateOutput = I_lineTranslateAddress(
       lpLineTranslateOutput, g_dwDeviceID, SAMPLE_TAPI_VERSION,
       szPreTranslatedNumber);

   // Unable to translate it?
   if (lpLineTranslateOutput == NULL)
   {
		log_file.log(LOG_ERROR,"Invalid Phone Number or Area Code");
   	return FALSE;
   }
   else
	{
      strcpy(g_szDisplayableAddress, (LPSTR) lpLineTranslateOutput +
            lpLineTranslateOutput -> dwDisplayableStringOffset);
      strcpy(g_szDialableAddress,(LPSTR) lpLineTranslateOutput +
            lpLineTranslateOutput -> dwDialableStringOffset);
		LocalFree(lpLineTranslateOutput);
      return TRUE;
	}

}


};
