extern bool InitializeTAPI( void );
extern bool ShutdownTAPI(void);
extern bool DialCall( char * phone_number );
extern bool OpenTAPILine( DWORD device );
extern bool HangupCall( void );
extern void PostHangupCall( void);


extern void MessageString( char * );
extern void MessageError( long lError, char *);

