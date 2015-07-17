
class ClientSession: public Session
{
public:
	ClientSession(HINSTANCE hInst):Session(hInst) {};
   void onReceive(VC *connection, const char *transportAddress, BYTE *data, int datalen);
   void onNotify(VC *connection, DWORD notifyKey, bool received);
   void onConnection(VC *connection, ConnectionNotifyType con, BYTE *data, int datalen);
};



