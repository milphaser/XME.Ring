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
const int MAX_CEH_PERIOD 	= 3000;
const int MAX_CEH_ERR		= 1000;

const int MAX_RUP_PERIOD	= 3000;
//---------------------------------------------------------------------------
void __fastcall AddToLog(String str);
void __fastcall AddToLog(TCustomWinSocket* sock, String str);
//---------------------------------------------------------------------------
// Failure - ��������� ���������
enum class F_State {FAULTLESS = 0, FAULTY};

// CEH - ��������� ���������
enum class CEH_State {INIT = 0, CLOSED, OPENED, FAULTY};

// RUP - ��������� ���������
enum class RUP_State {INIT = 0, DOWN, UP};

// E - ��������� ���������
enum class E_State {NONPARTICIPANT = 0, PARTICIPANT};

// ME - ��������� ���������
enum class ME_State {INIT = 0, RELEASED, WANTED, HELD};
//---------------------------------------------------------------------------
class TdmChannels : public TDataModule
{
__published:	// IDE-managed Components
	TServerSocket *ssIn;
	TClientSocket *csOut;
	TTimer *timerCEH;
	TTimer *timerRUP;
	void __fastcall ssInClientConnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ssInClientDisconnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ssInClientRead(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall csOutConnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall csOutDisconnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall csOutError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
		  int &ErrorCode);
	void __fastcall timerCEHTimer(TObject *Sender);
	void __fastcall timerRUPTimer(TObject *Sender);
	void __fastcall ssInClientError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
		  int &ErrorCode);

private:
	// ��������������� ���� /////////////////////////////////////////////////
	TIniFile* iniFile;
	String strIniFileName;

	// ��������� �� ������� /////////////////////////////////////////////////
	String strPId;               	// ������������� �� �������
	String strElectedPId;	        // ������������� �� �������� ������

	// ���������� �� �����������
	F_State fState;                 // Failure (F) State
	CEH_State cehState;				// Output Channel Error Handling (CEH) State
	RUP_State rupState;				// Ring Up Check (RUP) State

	E_State eState;					// Election (E) State

	bool boolMrkME;                 // ���� �� �������������� �� ������ME
	TTime timeMrkME;                // ������������� �� ������ ME
	String strClrPending;        	// ��������� ��������� <mrk_me_clr>
	ME_State meState;				// Mutual Exclusion (ME) State

	// �������������� ��������� �� ���� /////////////////////////////////////
	int intPortIn;					// ����

	// �������������� ��������� �� ����� ////////////////////////////////////
	int intPortOut;					// ����
	String strIPAddressOut;			// IP �����
	int intErrorCounter;			// ����� �� �������� �� �������� �����

	int intDelay;					// �������� �� ������������ �� �������, ms
	bool boolStep;                  // ����� ��� �������� ����������� �� �������
	String strMarker;	            // ����� �� ������� � ������� �����

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

	F_State __fastcall SwitchF_State(void);
	ME_State __fastcall GetME_State(void)	{return meState;}

	void __fastcall OpenOutChannel(bool boolErrorClear = false);
	void __fastcall CloseOutChannel(void);

	void __fastcall SendMarker(void)        {SendMarker(strMarker);}

	// RUP //////////////////////////////////////////////////////////////////
	void __fastcall RUP_OnReceiptOfMarker(String stringMsg, TCustomWinSocket *Socket);

	// E ////////////////////////////////////////////////////////////////////
	void __fastcall E_OnStart(void);
	void __fastcall E_OnReceiptOfElection(String stringMsg, TCustomWinSocket *Socket);
	void __fastcall E_OnReceiptOfElected(String stringMsg, TCustomWinSocket *Socket);

	// MrkME ////////////////////////////////////////////////////////////////
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
