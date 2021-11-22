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

// RU - типове съобщения
const String MSG_MRK_RING  = MSG_LEADER + "MRK_RING";

// E - типове съобщения
const String MSG_ELECTION  = MSG_LEADER + "MRK_ELECTION";
const String MSG_ELECTED   = MSG_LEADER + "MRK_ELECTED";

// ME - типове съобщения
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
	// Конфигурационен файл /////////////////////////////////////////////////
	TIniFile* iniFile;
	String strIniFileName;

	// Параметри на процеса /////////////////////////////////////////////////
	String strPId;               	// Идентификатор на процеса
	String strElectedPId;	        // Идентификатор на избрания процес

	// Променливи на състоянието
	F_State fState;                 // Failure (F) State
	CEH_State cehState;				// Output Channel Error Handling (CEH) State
	RUP_State rupState;				// Ring Up Check (RUP) State

	E_State eState;					// Election (E) State

	bool boolMrkME;                 // флаг за възстановяване на маркерME
	TTime timeMrkME;                // идентификатор на маркер ME
	String strClrPending;        	// задържано съобщение <mrk_me_clr>
	ME_State meState;				// Mutual Exclusion (ME) State

	// Комуникационни параметри по вход /////////////////////////////////////
	int intPortIn;					// порт

	// Комуникационни параметри по изход ////////////////////////////////////
	int intPortOut;					// порт
	String strIPAddressOut;			// IP адрес
	int intErrorCounter;			// брояч на грешките по изходния канал

	int intDelay;					// задръжка за визуализация на маркера, ms
	bool boolStep;                  // режим със стъпково придвижване на маркера
	String strMarker;	            // буфер за маркера в стъпков режим

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
