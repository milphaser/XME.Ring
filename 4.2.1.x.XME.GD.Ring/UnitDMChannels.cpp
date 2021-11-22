//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UnitDMChannels.h"
#include "UnitFormMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma classgroup "Vcl.Controls.TControl"
#pragma resource "*.dfm"
TdmChannels *dmChannels;
//---------------------------------------------------------------------------
const TColor clInputStatus[] 	= {clRed, clLime};
const TColor clFailureStatus[] 	= {clLime, clRed};
const TColor clCEHStatus[] 		= {clBtnFace, clRed, clLime};
const TColor clRUPStatus[] 		= {clBtnFace, clRed, clLime};
const TColor clMrkMEStatus[] 	= {clLime, clRed};

const String stringMEStatus[] 	=
{
	L"INIT", L"RELEASED", L"WANTED", L"HELD"
};
//---------------------------------------------------------------------------
__fastcall TdmChannels::TdmChannels(TComponent* Owner)
	: TDataModule(Owner)
{
	ReadIniFile();	// четене на параметрите от конфигурационния файл

	if(boolStep)
	{   // стъпково придвижване на маркера
		formMain->labelStep->Show();
		formMain->buttonSendMarker->Show();
		formMain->buttonSendMarker->Enabled = false;
	}
	else
	{   // автоматично придвижване на маркера
		formMain->labelStep->Hide();
		formMain->buttonSendMarker->Hide();
		formMain->buttonSendMarker->Enabled = false;
	}

	// CEH ////////////////////////////////////////////////////////////////////
	// CEH::OnInit
	cehState = CEH_State::INIT;
	formMain->panelOutput->Color = clCEHStatus[static_cast<int>(cehState)];
	intErrorCounter = 0;
	timerCEH->Interval = MAX_CEH_PERIOD;

	// RUP ////////////////////////////////////////////////////////////////////
	// RUP::OnInit
	rupState = RUP_State::INIT;
	formMain->panelRingUp->Color = clRUPStatus[static_cast<int>(rupState)];
	timerRUP->Interval = MAX_RUP_PERIOD;

	// E //////////////////////////////////////////////////////////////////////
	// E::OnInit
	strElectedPId = PID_NULL;
	formMain->stCoordinator->Caption = strElectedPId;
	eState = E_State::NONPARTICIPANT;

	// MrkME //////////////////////////////////////////////////////////////////
	// MrkME::OnInit
	boolMrkME = false;
	formMain->panelMrkME->Color = clMrkMEStatus[boolMrkME];
	strClrPending = "";
	SetTimeMrkME();

	// ME /////////////////////////////////////////////////////////////////////
	// ME::OnInit - преди да се стартира избора
	meState = ME_State::INIT;
	formMain->stStatus->Caption = stringMEStatus[static_cast<int>(meState)];
	formMain->buttonEnter->Enabled = false;
	formMain->buttonRelease->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ReadIniFile(void)
{
	strIniFileName = Application->ExeName;
	strIniFileName = strIniFileName.SubString(1, strIniFileName.Length() - 3) + "ini";

	iniFile = new TIniFile(strIniFileName);
	if(!FileExists(strIniFileName))
	{
		ShowMessage("Missing configuration file!");
	}

	formMain->Top  = iniFile->ReadInteger(L"FormPos", L"Top", 0);
	formMain->Left = iniFile->ReadInteger(L"FormPos", L"Left", 0);

	formMain->memoLog->Visible = !iniFile->ReadInteger(L"Log", L"Visible", 0);

	intDelay	 = iniFile->ReadInteger(L"Token", L"Delay", 0);
	boolStep	 = iniFile->ReadBool(L"Token", L"Step", 0);

	/////////////////////////////////////////////////////////////////////////
	int id;        // numerical Id part
	String tag;     // textual Id part
	ENDPOINT ep;    // communication end point

	id = iniFile->ReadInteger(L"This", L"ID", 0);
	tag = iniFile->ReadString(L"This", L"Tag", PID_NULL);
	ep.first = iniFile->ReadString(L"This", L"IP", L"127.0.0.1");
	ep.second = iniFile->ReadInteger(L"This", L"Port", 0);

	pidThis = std::unique_ptr<PID<int>>(new PID<int>(id, tag, ep));

	formMain->stPId->Caption = pidThis->GetTag();
	fState = F_State::FAULTLESS;
	formMain->panelFailure->Color = clFailureStatus[static_cast<int>(fState)];

	ssIn->Port = pidThis->GetEndPoint().second;
	ssIn->Active = true;
	formMain->panelInput->Color = clInputStatus[false];

	id = iniFile->ReadInteger(L"Next", L"ID", 0);
	tag = iniFile->ReadString(L"Next", L"Tag", PID_NULL);
	ep.first = iniFile->ReadString(L"Next", L"IP", L"127.0.0.1");
	ep.second = iniFile->ReadInteger(L"Next", L"Port", 0);

	pidNext = std::unique_ptr<PID<int>>(new PID<int>(id, tag, ep));

	csOut->Address = pidNext->GetEndPoint().first;
	csOut->Port = pidNext->GetEndPoint().second;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::WriteIniFile(void)
{
	try
	{
		iniFile->WriteInteger("FormPos", "Top", formMain->Top);
		iniFile->WriteInteger("FormPos", "Left", formMain->Left);

		iniFile->WriteInteger(L"Log", L"Visible", formMain->memoLog->Visible);
	}
	catch(Exception& e)
	{
		ShowMessage(e.Message);
	}

	delete iniFile;
}
//---------------------------------------------------------------------------
F_State __fastcall TdmChannels::SwitchF_State(void)
{
	if(fState == F_State::FAULTLESS)
	{
		fState = F_State::FAULTY;
	}
	else
	{
		fState = F_State::FAULTLESS;
	}
	formMain->panelFailure->Color = clFailureStatus[static_cast<int>(fState)];

	return fState;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ssInClientConnect(TObject *Sender, TCustomWinSocket *Socket)
{
	formMain->panelInput->Color = clInputStatus[true];
	AddToLog(Socket, "CONNECTED");
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ssInClientDisconnect(TObject *Sender, TCustomWinSocket *Socket)
{
	formMain->panelInput->Color = clInputStatus[false];
	AddToLog(Socket, "DISCONNECTED");
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ssInClientError(TObject *Sender, TCustomWinSocket *Socket,
		  TErrorEvent ErrorEvent, int &ErrorCode)
{
	formMain->panelInput->Color = clInputStatus[false];
	AddToLog("Socket Error #" + IntToStr(ErrorCode));

	ErrorCode = 0;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::csOutConnect(TObject *Sender, TCustomWinSocket *Socket)
{
	// CEH::OnConnect
	cehState = CEH_State::OPENED;
	formMain->panelOutput->Color = clCEHStatus[static_cast<int>(cehState)];
	String buf = "Output channel connected to " +  csOut->Address;
	formMain->stbarOutStatus->SimpleText = buf;

	// RUP::OnConnect
	rupState = RUP_State::DOWN;
	formMain->panelRingUp->Color = clRUPStatus[static_cast<int>(rupState)];
	String stringMsgOut = MSG_MRK_RING + MSG_SEPARATOR + pidThis->GetTag();
	csOut->Socket->SendText(stringMsgOut);
	timerRUP->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::csOutDisconnect(TObject *Sender, TCustomWinSocket *Socket)
{
	// CEH::OnDisonnect
	cehState = CEH_State::CLOSED;
	formMain->panelOutput->Color = clCEHStatus[static_cast<int>(cehState)];
	formMain->stbarOutStatus->SimpleText = "Output channel disconnected";

	/* TODO : ДА СЕ АНАЛИЗИРА ЗАЩО ВРЪЗКАТА НЕ МОЖЕ ДА СЕ ОТВОРИ
	ДИРЕКТНО ОТ ОБРАБОТЧИКА НА OnDisconnect */
	timerCEH->Enabled = true;

	// RUP::OnDisconnect
	rupState = RUP_State::DOWN;
	formMain->panelRingUp->Color = clRUPStatus[static_cast<int>(rupState)];
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::csOutError(TObject *Sender, TCustomWinSocket *Socket,
		  TErrorEvent ErrorEvent, int &ErrorCode)
{
	// CEH::OnError
	cehState = CEH_State::CLOSED;
	formMain->panelOutput->Color = clCEHStatus[static_cast<int>(cehState)];

	String buf;
	if(++intErrorCounter >= MAX_CEH_ERR)
	{
		buf = "Достигнат е макс. брой опити #" + IntToStr(intErrorCounter);
	}
	else
	{
		buf = "Неуспешен опит #" + IntToStr(intErrorCounter);
		timerCEH->Enabled = true;
	}
	formMain->stbarOutStatus->SimpleText = buf;

	// RUP::OnError
	rupState = RUP_State::DOWN;
	formMain->panelRingUp->Color = clRUPStatus[static_cast<int>(rupState)];

	ErrorCode = 0;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::OpenOutChannel(bool boolErrorClear)
{
	if(boolErrorClear)
	{
		intErrorCounter = 0;
		formMain->stbarOutStatus->SimpleText = "Wait to connect...";
	}

	csOut->Active = true;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::CloseOutChannel(void)
{
	csOut->Active = false;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ssInClientRead(TObject *Sender, TCustomWinSocket *Socket)
{
	String stringMsgIn = Socket->ReceiveText();     // Receive Message

	for(int intX = MSG_LEADER.Length(); intX > 0;
		stringMsgIn = stringMsgIn.c_str() + MSG_LEADER.Length() + intX - 1)
	{
		String stringMsg;
		String stringBuf = stringMsgIn.c_str() +
						   MSG_LEADER.Length();

		intX = stringBuf.Pos(MSG_LEADER);

		if(intX > 0)
		{
			stringMsg = stringBuf.SubString(1, intX - 1);
		}
		else
		{
			stringMsg = stringBuf;
		}

		OnReceiptOfMsg(MSG_LEADER + stringMsg, Socket);
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::OnReceiptOfMsg(String stringMsg, TCustomWinSocket *Socket)
{
	String stringMsgPId;          // Received PId
	String stringMsgOut;          // Sended Message

	if(stringMsg.Pos(MSG_MRK_RING) == 1)
	{	// RUP::OnReceiptOf <mrk_ring, j>
		RUP_OnReceiptOfMarker(stringMsg, Socket);
	}
	else if(stringMsg.Pos(MSG_ELECTION) == 1)
	{	// E::OnReceiptOf <election, j>
		E_OnReceiptOfElection(stringMsg, Socket);
	}
	else if(stringMsg.Pos(MSG_ELECTED) == 1)
	{	// E::OnReceiptOf <elected, j>
		E_OnReceiptOfElected(stringMsg, Socket);
	}
	else if(stringMsg.Pos(MSG_MRK_ME_CLR) == 1)
	{  // MrkME::OnReceiptOf <mrk_me_clr>
		MrkME_OnReceiptOfClear(stringMsg);
	}
	else if(stringMsg.Pos(MSG_MRK_ME_MRK) == 1)
	{
		if(boolMrkME)
		{  // MrkME::OnReceiptOf <mrk_me, Tj>
			MrkME_OnReceiptOfMarker(stringMsg);
		}
		else if(stringMsg.Pos(MSG_MRK_ME_MRK) == 1)
		{  // ME Marker Message Received
			ME_OnReceiptOfMarker(stringMsg);
		}
	}
	else
	{
		AddToLog(Socket, ">> " + stringMsg + " UNDECODED");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::SendMarker(String stringMsg)
{
	if(!boolStep)
	{
		if(intDelay > 0)
		{
			Sleep(intDelay);		// задръжка за визуализация на маркера
		}

		csOut->Socket->SendText(stringMsg);
	}
	else
	{
		if(!formMain->buttonSendMarker->Enabled)
		{   // стъпка 1: фиксиране стойността и задържане на маркера
			strMarker = stringMsg;
			formMain->buttonSendMarker->Enabled = true;
		}
		else
		{   // стъпка 2: ръчно изпращане на маркера, фиксиран на стъпка 1
			formMain->buttonSendMarker->Enabled = false;
			csOut->Socket->SendText(stringMsg);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::SetTimeMrkME(void)
{
	TTime t0;
	SetTimeMrkME(t0);
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::SetTimeMrkME(TTime t)
{
	timeMrkME = t;

	TTime t0;
	if(timeMrkME == t0)
	{
		formMain->stTimeMrkME->Caption = "NULL";
	}
	else
	{
		formMain->stTimeMrkME->Caption = FormatDateTime("hh:mm:ss.zzz", timeMrkME);
	}
}
//---------------------------------------------------------------------------
bool __fastcall TdmChannels::IsTimeMrkMENotNull(void)
{
	bool boolResult = false;

	TTime t0;
	if(timeMrkME != t0)
	{
        boolResult = true;
    }

	return boolResult;
}
//---------------------------------------------------------------------------
TTime __fastcall TdmChannels::StrToTime(String stringTime)
{
	TTime timeResult;

	try
	{
		String hh  = stringTime.SubString(1, 2);
		String mm  = stringTime.SubString(4, 2);
		String ss  = stringTime.SubString(7, 2);
		String zzz = stringTime.SubString(10, 3);

		Word wHour, wMin, wSec, wMSec;
		wHour = hh.ToInt();
		wMin  = mm.ToInt();
		wSec  = ss.ToInt();
		wMSec = zzz.ToInt();

		timeResult = EncodeTime(wHour, wMin, wSec, wMSec);
	}
	catch(Exception& e)
	{
		AddToLog(e.Message);
	}

	return timeResult;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::RUP_OnReceiptOfMarker(String stringMsg, TCustomWinSocket *Socket)
{
	// RUP::OnReceiptOf <mrk_ring, j>
	AddToLog(Socket, ">> " + stringMsg + " DECODED1");

	String stringMsgPId = stringMsg.c_str() + MSG_MRK_RING.Length() + MSG_SEPARATOR.Length();

	if(stringMsgPId == pidThis->GetTag())
	{
		timerRUP->Enabled = false;
		rupState = RUP_State::UP;
		formMain->panelRingUp->Color = clRUPStatus[static_cast<int>(rupState)];
		AddToLog("RUP::UP");

		// E /////////////////////////////////////////////////////////////////////
		// E::OnStartElection
		E_OnStart();
	}
	else
	{
		String stringMsgOut = stringMsg;
		csOut->Socket->SendText(stringMsgOut);

		AddToLog(csOut->Socket, "<< " + stringMsgOut + " RUP::FORWARD");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::E_OnStart(void)
{
	// E::OnStartElection
	strElectedPId = PID_NULL;
	formMain->stCoordinator->Caption = strElectedPId;
	eState = E_State::PARTICIPANT;

	String stringMsgOut = MSG_ELECTION + MSG_SEPARATOR + pidThis->GetTag();
	csOut->Socket->SendText(stringMsgOut);

	AddToLog(csOut->Socket, "<< " + stringMsgOut + " E::PASS1 START");
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::E_OnReceiptOfElection(String stringMsg, TCustomWinSocket *Socket)
{
	// E::OnReceiptOf <election, j>
	AddToLog(Socket, ">> " + stringMsg + " DECODED2");

	String stringMsgPId = stringMsg.c_str() + MSG_ELECTION.Length() + MSG_SEPARATOR.Length();

	if(stringMsgPId > pidThis->GetTag())
	{   // j > i
		strElectedPId = PID_NULL;
		formMain->stCoordinator->Caption = strElectedPId;
		eState = E_State::PARTICIPANT;

		String stringMsgOut = stringMsg;
		csOut->Socket->SendText(stringMsgOut);

		AddToLog(csOut->Socket, "<< " + stringMsgOut);
	}
	else if(stringMsgPId < pidThis->GetTag())
	{   // j < i
		if(eState != E_State::PARTICIPANT)
		{
			strElectedPId = PID_NULL;
			formMain->stCoordinator->Caption = strElectedPId;
			eState = E_State::PARTICIPANT;

			String stringMsgOut = MSG_ELECTION + MSG_SEPARATOR + pidThis->GetTag();
			csOut->Socket->SendText(stringMsgOut);

			AddToLog(csOut->Socket, "<< " + stringMsgOut);
		}
	}
	else
	{   // j = i    I am the Coordinator
		strElectedPId = pidThis->GetTag();
		formMain->stCoordinator->Caption = strElectedPId;
		eState = E_State::NONPARTICIPANT;

		String stringMsgOut = MSG_ELECTED + MSG_SEPARATOR + pidThis->GetTag();
		csOut->Socket->SendText(stringMsgOut);

		AddToLog(csOut->Socket, "<< " + stringMsgOut + " E::PASS2 START");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::E_OnReceiptOfElected(String stringMsg, TCustomWinSocket *Socket)
{
	// E::OnReceiptOf <elected, j>
	AddToLog(Socket, ">> " + stringMsg + " DECODED3");

	String stringMsgPId = stringMsg.c_str() + MSG_ELECTED.Length() + MSG_SEPARATOR.Length();

	if(stringMsgPId != pidThis->GetTag())
	{	// j != i
		strElectedPId = stringMsgPId;
		formMain->stCoordinator->Caption = strElectedPId;
		eState = E_State::NONPARTICIPANT;

		String stringMsgOut = stringMsg;
		csOut->Socket->SendText(stringMsgOut);

		AddToLog(csOut->Socket, "<< " + stringMsgOut + " E::LOCAL END");

		// ME /////////////////////////////////////////////////////////////////////
		// ME::OnInit - след края на избора
		ME_OnAfterElection();
		MrkME_OnAfterElection();
	}
	else
	{   // j = i
		AddToLog(csOut->Socket, "E::END");

		// ME /////////////////////////////////////////////////////////////////////
		// ME::OnInit - след края на избора
		ME_OnAfterElection();
		MrkME_OnAfterElection();
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::timerCEHTimer(TObject *Sender)
{
	// CEH::OnTimer
	timerCEH->Enabled = false;
	OpenOutChannel(false);
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::timerRUPTimer(TObject *Sender)
{
	// RUP::OnTimer
	String stringMsgOut = MSG_MRK_RING + MSG_SEPARATOR + pidThis->GetTag();
	csOut->Socket->SendText(stringMsgOut);
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::MrkME_OnAfterElection(void)
{
	// MrkME::OnAfterElection
	if(strElectedPId == pidThis->GetTag())
	{
		boolMrkME = true;
		formMain->panelMrkME->Color = clMrkMEStatus[boolMrkME];

		SetTimeMrkME();

		String stringMsgOut = MSG_MRK_ME_CLR;
		SendMarker(stringMsgOut);

		AddToLog(csOut->Socket, "<< " + stringMsgOut + " MrkME::PASS1 START");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::MrkME_OnReceiptOfClear(String stringMsg)
{
	// MrkME::OnReceiptOf <mrk_me_clr>
	if(meState != ME_State::HELD)
	{
		strClrPending = stringMsg;
		MrkME_OnClear();
		strClrPending = "";
	}
	else
	{
		strClrPending = stringMsg;
		AddToLog(csOut->Socket, "<< " + stringMsg + " MrkME::PENDING");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::MrkME_OnClear(void)
{
	// MrkME::OnClear
	if(strElectedPId == pidThis->GetTag())
	{
		AddToLog(csOut->Socket, "<< " + strClrPending + " MrkME::PASS1 END");

		SetTimeMrkME(Time());

		String stringMsgOut = MSG_MRK_ME_MRK + MSG_SEPARATOR + FormatDateTime("hh:mm:ss.zzz", timeMrkME);
		SendMarker(stringMsgOut);

		AddToLog(csOut->Socket, "<< " + stringMsgOut + " MrkME::PASS2 START");
	}
	else
	{
		boolMrkME = true;
		formMain->panelMrkME->Color = clMrkMEStatus[boolMrkME];
		SetTimeMrkME();

		String stringMsgOut = strClrPending;
		csOut->Socket->SendText(stringMsgOut);

		AddToLog(csOut->Socket, "<< " + stringMsgOut + " MrkME::LOCAL START");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::MrkME_OnReceiptOfMarker(String stringMsg)
{
	// MrkME::OnReceiptOf <mrk_me, Tj>
	if(strElectedPId == pidThis->GetTag())
	{
		String strTj = stringMsg.c_str() + MSG_MRK_ME_MRK.Length() + MSG_SEPARATOR.Length();
		String strTimeMrkME = FormatDateTime("hh:mm:ss.zzz", timeMrkME);
		if(strTimeMrkME == strTj)
		{
			if(boolMrkME)
			{
				boolMrkME = false;
				formMain->panelMrkME->Color = clMrkMEStatus[boolMrkME];

				AddToLog(csOut->Socket, "<< " + stringMsg + " MrkME::END");
			}

			SendMarker(stringMsg);
		}
		else
		{   // Диагностическо съобщение
			AddToLog(csOut->Socket, "<< " + stringMsg + " MrkME::OLD MARKER ABSORBED");
		}
	}
	else
	{
		String strTj = stringMsg.c_str() + MSG_MRK_ME_MRK.Length() + MSG_SEPARATOR.Length();
		String strTimeMrkME = FormatDateTime("hh:mm:ss.zzz", timeMrkME);

		TTime t0;
		if(timeMrkME == t0)
		{
			boolMrkME = false;
			formMain->panelMrkME->Color = clMrkMEStatus[boolMrkME];
			SetTimeMrkME(StrToTime(strTj));

			AddToLog(csOut->Socket, "<< " + stringMsg + " MrkME::LOCAL END");

			SendMarker(stringMsg);
		}
		else
		{
			if(strTimeMrkME == strTj)
			{
				String stringMsgOut = stringMsg;
				csOut->Socket->SendText(stringMsgOut);
			}
			else
			{   // Диагностическо съобщение
				AddToLog(csOut->Socket, "<< " + stringMsg + " MrkME::ANCIENT MARKER ABSORBED");
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ME_OnAfterElection(void)
{
    // Първоначалното стартиране на маркер ME е изместено в MrkME::OnAfterElection
	if(meState == ME_State::INIT)
	{
		// Допуска се преход в RELEASED единствено от INIT
		meState = ME_State::RELEASED;
		formMain->stStatus->Caption = stringMEStatus[static_cast<int>(meState)];
		formMain->buttonEnter->Enabled = true;
		formMain->buttonRelease->Enabled = false;
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ME_OnEnter(void)
{
	// ME::OnEnter
	if(meState == ME_State::RELEASED && IsTimeMrkMENotNull())
	{
		meState = ME_State::WANTED;
		formMain->stStatus->Caption = stringMEStatus[static_cast<int>(meState)];
		formMain->buttonEnter->Enabled = false;
		formMain->buttonRelease->Enabled = false;
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ME_OnReceiptOfMarker(String stringMsg)
{
	// ME::OnReceiptOfMarker
	formMain->pbarMarker->StepIt();

	if(meState == ME_State::WANTED && IsTimeMrkMENotNull())
	{
		meState = ME_State::HELD;
		formMain->stStatus->Caption = stringMEStatus[static_cast<int>(meState)];
		formMain->buttonEnter->Enabled = false;
		formMain->buttonRelease->Enabled = true;

		// ВХОД В СЕКЦИЯТА - access()
	}
	else if(meState == ME_State::RELEASED)
	{
		meState = ME_State::RELEASED;
		formMain->stStatus->Caption = stringMEStatus[static_cast<int>(meState)];
		formMain->buttonEnter->Enabled = true;
		formMain->buttonRelease->Enabled = false;

		String stringMsgOut = stringMsg;
		SendMarker(stringMsgOut);
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ME_OnRelease(void)
{
	// ME::OnRelease
	if(meState == ME_State::HELD)
	{
		// отработване на стария маркер
		meState = ME_State::RELEASED;
		formMain->stStatus->Caption = stringMEStatus[static_cast<int>(meState)];
		formMain->buttonEnter->Enabled = true;
		formMain->buttonRelease->Enabled = false;

		String stringMsgOut = MSG_MRK_ME_MRK + MSG_SEPARATOR + FormatDateTime("hh:mm:ss.zzz", timeMrkME);
		SendMarker(stringMsgOut);

		if(!strClrPending.IsEmpty())
		{
			// отработване на задържано съобщение <mrk_me_clr>
			MrkME_OnClear();
			strClrPending = "";
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall AddToLog(String str)
{
	String strNum;
	strNum.printf(L"%04d", formMain->memoLog->Lines->Count + 1);
	String ws = FormatDateTime("hh:mm:ss.zzz", Time()) + " " + str;
	formMain->memoLog->Lines->Add(strNum + " " + ws);
}
//---------------------------------------------------------------------------
void __fastcall AddToLog(TCustomWinSocket* sock, String str)
{
	String strNum;
	strNum.printf(L"%04d", formMain->memoLog->Lines->Count + 1);
	String ws = FormatDateTime("hh:mm:ss.zzz", Time()) +
				" [" + sock->RemoteHost + "::" + sock->RemoteAddress + "] " +
				str;
	formMain->memoLog->Lines->Add(strNum + " " + ws);
}
//---------------------------------------------------------------------------

