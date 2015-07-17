#ifndef _H_TAPINET
#define _H_TAPINET


#define MAX_PHONE_NUMBER_SIZE 32
#define MAX_TAPI_DEVICES      100

#include <tapi.h>
#include "dnet.h"
#include "vcprotocol.h"
#include "commcode.h"

namespace DNet
{

class TAPIVC;

class TAPITransport : public Transport
{
private:
   DWORD deviceId;
   DWORD callbackId; // used to get transport pointer in the callback function
	HINSTANCE hInstance;

   bool getAddressFromString(const char *addr_string, LPVOID addr);
   void getStringFromAddress(const LPVOID addr, char *nameBuf);

// this is the TAPI specific data and internal functions
   DWORD g_dwDeviceID;           // the line device ID
   DWORD g_dwRequestedID;
   long  g_lAsyncReply;
   HCALL    g_hCall;             // handle to the call
   HLINE    g_hLine;             // handle to the open line
   HANDLE   g_hCommFile;         // handle to the comm file after connecting
   DWORD    g_dwAPIVersion;      // the API version
   DWORD 	g_dwCallState;
	char     g_szDisplayableAddress[256];
	char     g_szDialableAddress[256];

   // BOOLEANS to handle reentrancy.
   bool g_bConnected;
   bool g_bStoppingCall;
   bool g_bCalling;
   bool g_bReplyReceived;
   bool g_bCallStateReceived;

	bool HandleLineErr(long lLineErr);
	LPVOID CheckAndReAllocBuffer( LPVOID lpBuffer, size_t sizeBufferMinimum, LPCSTR szApiPhrase);
	bool DialCallInParts (LPLINEDEVCAPS lpLineDevCaps, LPCSTR lpszAddress, LPCSTR lpszDisplayableAddress);
	LPLINECALLPARAMS CreateCallParams (LPLINECALLPARAMS lpCallParams,
                                      LPCSTR lpszDisplayableAddress);
	DWORD I_lineNegotiateAPIVersion ( DWORD dwDeviceID);
	LPLINEDEVCAPS I_lineGetDevCaps (LPLINEDEVCAPS lpLineDevCaps,
   										  DWORD dwDeviceID,DWORD dwAPIVersion);
	LPLINEADDRESSSTATUS I_lineGetAddressStatus (LPLINEADDRESSSTATUS lpLineAddressStatus,
														     HLINE hLine, DWORD dwAddressID);
	LPLINETRANSLATEOUTPUT I_lineTranslateAddress (LPLINETRANSLATEOUTPUT lpLineTranslateOutput,
                                                 DWORD dwDeviceID, DWORD dwAPIVersion, LPCSTR lpszDialAddress);
	LPLINECALLSTATUS I_lineGetCallStatus ( LPLINECALLSTATUS lpLineCallStatus, HCALL hCall);
	LPLINEADDRESSCAPS I_lineGetAddressCaps ( LPLINEADDRESSCAPS lpLineAddressCaps,
														  DWORD dwDeviceID, DWORD dwAddressID,
														  DWORD dwAPIVersion, DWORD dwExtVersion);
	long WaitForCallState (DWORD dwNewCallState);
   long WaitForReply (long lRequestID);
	void DoLineReply( DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance,
                     DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
	void DoLineClose( DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance,
					      DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
	void DoLineDevState( DWORD dwDevice, DWORD dwsg, DWORD dwCallbackInstance,
							   DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
	void DoLineCallState( DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance,
							    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
	void DoLineCreate( DWORD dwDevice, DWORD dwMessage, DWORD dwCallbackInstance,
						    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
	bool HandleNoMem();
   void WarningBox(LPCSTR lpszMessage);
   DWORD I_lineNegotiateLegacyAPIVersion(DWORD dwDeviceID);
	long VerifyUsableLine(DWORD dwDeviceID,LPSTR lpszLineName);
	bool VerifyAndWarnUsableLine( void );
	bool TranslatePhoneNumber( char * );

public:

   TAPITransport(Session *ssn, int deviceId, HINSTANCE hInst);
   ~TAPITransport();

   bool activate(int port);
   VC *connect(const char *addrString);
   void deactivate( void );

   Error send(const char *address, BYTE *data, int datalen);
   void process();
   Session *getSession() { return mySession; }

	bool InitializeTAPI( void );
	bool ShutdownTAPI(void);
	bool DialCall( const char * phone_number );
	bool OpenTAPILine( DWORD device );
	bool CloseTAPILine( void );
	bool HangupCall( void );
   DWORD GetTAPIDevices(DWORD, DWORD []);
   void SetDeviceId(DWORD id) { deviceId = id; };
   void SetMyThis( TAPITransport *my_this, DWORD deviceId, DWORD callbackId );

	static void CALLBACK lineCallbackFunc(
        DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance,
        DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
};



class TAPIVC : public VC
{
   friend class TAPITransport;

   BYTE packetBuffer[MaxPacketSize + MaxHeaderSize];
   BYTE *wait_data;
   int  wait_data_size;

   int deviceNo;

   VCProtocol *protocol;  // responsible for allocating and deallocating
   Comm *comm_port;       // responsible for allocating and deallocating

   DWORD DataStartTime, DataRecvTotal, DataPacketsTotal;

public:
   TAPIVC(const char *number,Session *ssn, Transport *trans);
   ~TAPIVC();

   void accept(BYTE *data, int datalen);
   void reject(BYTE *data, int datalen);
   void disconnect() { disconnect(NULL,0);}
   void disconnect(BYTE *data, int datalen);

   BYTE *getDataBuffer() { return packetBuffer + MaxHeaderSize; }
   BYTE *getHeaderBuffer() { return packetBuffer; }

   void receivePacket(BYTE *data, int len, DWORD recvTime);
   Error send(BYTE *data, int len, DWORD notifyKey, int packetType = DataPacket);
   bool initiateConnection(const char *remoteAddress);
   void initiateConnectionFrom(struct sockaddr *remAddr, int fromlen);
   bool windowFull();

   static void  receive_data_from_comm_port( BYTE *, int , void *myThis);

};


};

#endif
