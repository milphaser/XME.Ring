//---------------------------------------------------------------------------

#ifndef UnitDMChannelsH
#define UnitDMChannelsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.Win.ScktComp.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <IniFiles.hpp>

//--CONSTANTS----------------------------------------------------------------
const String PID_UNDEFINED = "UNDEFINED";       // undefined PId

//--MESSAGES-----------------------------------------------------------------
const String MSG_LEADER    = "|";
const String MSG_SEPARATOR = " ";

// RU - ������ ���������
const String MSG_MRK_RING  = MSG_LEADER + "MRK_RING";

// E - ������ ���������
const String MSG_ELECTION  = MSG_LEADER + "MRK_ELECTION";
const String MSG_ELECTED   = MSG_LEADER + "MRK_ELECTED";

// ME - ������ ���������
const String MSG_MRK_ME_CLR   = MSG_LEADER + "MRK_ME_CLR";
const String MSG_MRK_ME_MRK   = MSG_LEADER + "MRK_ME_MRK";

//--TIMERS-------------------------------------------------------------------
const int T_CHN_OUT_MAXWAIT = 3000;
const int CHN_OUT_MAXERR   	= 1000;

const int T_RING_UP_MAXWAIT = 3000;
//---------------------------------------------------------------------------
void __fastcall AddToLog(String str);
void __fastcall AddToLog(TCustomWinSocket* sock, String str);
//---------------------------------------------------------------------------
// Failure - ��������� ���������
enum F_STATE
{
	F_FALSE = 0, F_TRUE
};

// CO - ��������� ���������
enum CO_STATE
{
	CO_INIT = 0, CO_CLOSED, CO_OPENED
};

// RU - ��������� ���������
enum RU_STATE
{
	RU_INIT = 0, RU_DOWN, RU_UP
};

// E - ��������� ���������
enum E_STATE
{
	E_NON_PARTICIPANT = 0, E_PARTICIPANT
};

// ME - ��������� ���������
enum ME_STATE
{
	ME_INIT = 0, ME_RELEASED, ME_WANTED, ME_HELD
};
//---------------------------------------------------------------------------
class TdmChannels : public TDataModule
{
__published:	// IDE-managed Components
	TServerSocket *ssIn;
	TClientSocket *csOut;
	TTimer *timerChannelOut;
	TTimer *timerRingUp;
	void __fastcall ssInClientConnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ssInClientDisconnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ssInClientRead(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall csOutConnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall csOutDisconnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall csOutError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
		  int &ErrorCode);
	void __fastcall timerChannelOutTimer(TObject *Sender);
	void __fastcall timerRingUpTimer(TObject *Sender);
	void __fastcall ssInClientError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
		  int &ErrorCode);

private:
	// ��������������� ���� /////////////////////////////////////////////////
	TIniFile* iniFile;
	String stringIniFileName;

	// ��������� �� ������� /////////////////////////////////////////////////
	String stringPId;               // ������������� �� �������
	String stringElectedPId;        // ������������� �� �������� ������

	// ���������� �� �����������
	F_STATE fState;                 // Failure (F) State
	CO_STATE coState;				// ChannelOut (CO) State
	RU_STATE ruState;				// RingUp (RU) State

	E_STATE eState;					// Election (E) State

	bool boolMrkME;                 // ���� �� �������������� �� ������ME
	TTime timeMrkME;                // ������������� �� ������ ME
    String strClrPending;        	// ��������� ��������� <mrk_me_clr>
	ME_STATE meState;				// Mutual Exclusion (ME) State

	// �������������� ��������� �� ���� /////////////////////////////////////
	int intPortIn;					// ����

	// �������������� ��������� �� ����� ////////////////////////////////////
	int intPortOut;					// ����
	String stringIPAddressOut;		// IP �����
	int intErrorCounter;			// ����� �� �������� �� �������� �����

	int intDelay;					// �������� �� ������������ �� �������, ms
	bool boolStep;                  // ����� ��� �������� ����������� �� �������
	String stringMarker;            // ����� �� ������� � ������� �����

	void __fastcall OnReceiptOfMsg(String stringMsg, TCustomWinSocket *Socket);
	void __fastcall SendMarker(String stringMsg);

	void __fastcall SetTimeMrkME(void);
	void __fastcall SetTimeMrkME(TTime t);
	bool __fastcall IsTimeMrkMENotNull(void);
	TTime __fastcall StrToTime(String stringTime);

public:
	__fastcall TdmChannels(TComponent* Owner);
	void __fastcall ReadIniFile(void);
	void __fastcall WriteIniFile(void);

	F_STATE __fastcall SwitchF_STATE(void);
	ME_STATE __fastcall GetME_STATE(void)	{ return meState; }

	void __fastcall OpenOutChannel(bool boolErrorClear = false);
	void __fastcall CloseOutChannel(void);

	void __fastcall SendMarker(void)        { SendMarker(stringMarker); }

	// RU ///////////////////////////////////////////////////////////////////
	void __fastcall RU_OnReceiptOfMarker(String stringMsg, TCustomWinSocket *Socket);

	// Election /////////////////////////////////////////////////////////////
	void __fastcall E_OnStart(void);
	void __fastcall E_OnReceiptOfElection(String stringMsg, TCustomWinSocket *Socket);
	void __fastcall E_OnReceiptOfElected(String stringMsg, TCustomWinSocket *Socket);

	// MrkME ///////////////////////////////////////////////////////////////////
	void __fastcall MrkME_OnAfterElection(void);
	void __fastcall MrkME_OnReceiptOfClear(String stringMsg);
	void __fastcall MrkME_OnClear(void);
	void __fastcall MrkME_OnReceiptOfMarker(String stringMsg);

	// ME ///////////////////////////////////////////////////////////////////
	void __fastcall ME_OnAfterElection(void);
	void __fastcall ME_OnEnter(void);
	void __fastcall ME_OnReceiptOfMarker(String stringMsg);
	void __fastcall ME_OnRelease(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TdmChannels *dmChannels;
//---------------------------------------------------------------------------
#endif
