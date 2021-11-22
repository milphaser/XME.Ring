//---------------------------------------------------------------------------

#ifndef UnitDMChannelsH
#define UnitDMChannelsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.Win.ScktComp.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <IniFiles.hpp>
//---------------------------------------------------------------------------
#include <deque>
//---------------------------------------------------------------------------
//	CONSTANTS
//
const String PID_NULL		= "NULL";	// undefined PId

const int MAX_CEH_PERIOD 	= 3000;		// period between connection attempts, ms
const int MAX_CEH_ERR		= 5;		// maximum connection attempts
const int MIN_K				= 1;		// min number of faultless processes (max degradation)

const int MAX_RUP_PERIOD	= 3000;		// period to next RUP check, ms
//---------------------------------------------------------------------------
//	MESSAGES
//
const String MSG_LEADER    = "|";
const String MSG_SEPARATOR = " ";

// RU - типове съобщения
const String MSG_MRK_RING  = MSG_LEADER + "MRK_RING";

// E - типове съобщения
const String MSG_ELECTION  = MSG_LEADER + "MRK_ELECTION";
const String MSG_ELECTED   = MSG_LEADER + "MRK_ELECTED";

// ME - типове съобщения
const String MSG_MRK_ME_CLR   = MSG_LEADER + "MRK_ME_CLR";
const String MSG_MRK_ME_MRK   = MSG_LEADER + "MRK_ME_MRK";
//---------------------------------------------------------------------------
//	SET OF STATES
//
// Failure - множество състояния
enum class F_State {FAULTLESS = 0, FAULTY};

// CEH - множество състояния
enum class CEH_State {INIT = 0, CLOSED, OPENED, FAULTY};

// RUP - множество състояния
enum class RUP_State {INIT = 0, DOWN, UP};

// E - множество състояния
enum class E_State {NONPARTICIPANT = 0, PARTICIPANT};

// ME - множество състояния
enum class ME_State {INIT = 0, RELEASED, WANTED, HELD};
//---------------------------------------------------------------------------
//	UTILITIES
//
void __fastcall AddToLog(String str);
void __fastcall AddToLog(TCustomWinSocket* sock, String str);
//---------------------------------------------------------------------------
//	Process Id
//
using ENDPOINT = std::pair<String, int>;

template <class T_ID>
class PID
{
public:
	PID(void)
	{
		id	= static_cast<T_ID>(0);
		tag	= PID_NULL;
		ep.first 	= "";
		ep.second 	= 0;
	};
	PID(T_ID p_id, String p_tag, ENDPOINT p_ep)
	{
		id	= p_id;
		tag	= p_tag;
		ep	= p_ep;
	};
	bool operator==(const PID<T_ID>& pid) const 	{return (id == pid.id);}

	T_ID id;        // numerical Id part
	String tag;     // textual Id part
	ENDPOINT ep;    // communication end point
};
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

private:
	// Configuration File ///////////////////////////////////////////////////
	TIniFile* iniFile;
	String strIniFileName;
	/////////////////////////////////////////////////////////////////////////

	// Internal State Space /////////////////////////////////////////////////
	F_State fState;                 // Failure (F) State
	CEH_State cehState;				// Output Channel Error Handling (CEH) State
	int intErrorCounter;			// CEH Error Counter
	RUP_State rupState;				// Ring Up Check (RUP) State
	//
	E_State eState;					// Election (E) State
	//
	bool boolMrkME;                 // This Process MrkME State
	TTime timeMrkME;                // Token ME Id
	String strClrPending;        	// Hold <mrk_me_clr> Message
	ME_State meState;				// Mutual Exclusion (ME) State
	//
	std::unique_ptr<PID<int>> pidThis;	// This Process Id, i
	String strElectedPId;				// Coordinator Id
	std::unique_ptr<PID<int>> pidNext;	// Next Process Id, j
	std::deque<PID<int>> ListPIds;		// List of System Processes as a chain
	//
	int intDelay;					// Delay for ME Token visualization, ms
	bool boolStep;                  // Trace Mode for ME Token progress by step
	String strMarker;	            // Bufer for ME Token in Trace Mode
	/////////////////////////////////////////////////////////////////////////

	void __fastcall OnReceiptOfMsg(String stringMsg, TCustomWinSocket *Socket);
	void __fastcall SendMarker(String stringMsg);

	void __fastcall SetTimeMrkME(void);
	void __fastcall SetTimeMrkME(TTime t);
	bool __fastcall IsTimeMrkMENotNull(void);
	TTime __fastcall StrToTime(String stringTime);
};
//---------------------------------------------------------------------------
extern PACKAGE TdmChannels *dmChannels;
//---------------------------------------------------------------------------
#endif
