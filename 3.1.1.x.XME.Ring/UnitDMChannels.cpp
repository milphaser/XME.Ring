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
const TColor clInputStatus[] =
{
	clRed, clLime
};

const TColor clFailureStatus[] =
{
	clLime, clRed
};

const TColor clCOStatus[] =
{
	clBtnFace, clRed, clLime
};

const TColor clRUStatus[] =
{
	clBtnFace, clRed, clLime
};

const TColor clMrkMEStatus[] =
{
	clLime, clRed
};

const String stringMEStatus[] =
{
	L"ME_INIT", L"ME_RELEASED", L"ME_WANTED", L"ME_HELD"
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

	formMain->stMyPId->Caption = stringPId;
	fState = F_FALSE;
	formMain->panelFailure->Color = clFailureStatus[fState];

	ssIn->Port = intPortIn;
	ssIn->Active = true;
	formMain->panelInput->Color = clInputStatus[false];

	csOut->Port = intPortOut;
	csOut->Address = stringIPAddressOut;

	// CO /////////////////////////////////////////////////////////////////////
	// CO::OnInit
	coState = CO_INIT;
	formMain->panelOutput->Color = clCOStatus[coState];
	intErrorCounter = 0;
	timerChannelOut->Interval = T_CHN_OUT_MAXWAIT;

	// RU /////////////////////////////////////////////////////////////////////
	// RU::OnInit
	ruState = RU_INIT;
	formMain->panelRingUp->Color = clRUStatus[ruState];
	timerRingUp->Interval = T_RING_UP_MAXWAIT;

	// E //////////////////////////////////////////////////////////////////////
	// E::OnInit
	stringElectedPId = PID_UNDEFINED;
	formMain->stCoordinator->Caption = stringElectedPId;
	eState = E_NON_PARTICIPANT;

	// MrkME //////////////////////////////////////////////////////////////////
	// MrkME::OnInit
	boolMrkME = false;
	formMain->panelMrkME->Color = clMrkMEStatus[boolMrkME];
	strClrPending = "";
	SetTimeMrkME();

	// ME /////////////////////////////////////////////////////////////////////
	// ME::OnInit - преди да се стартира избора
	meState = ME_INIT;
	formMain->stStatus->Caption = stringMEStatus[meState];
	formMain->buttonEnter->Enabled = false;
	formMain->buttonRelease->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ReadIniFile(void)
{
	stringIniFileName = Application->ExeName;
	stringIniFileName = stringIniFileName.SubString(1, stringIniFileName.Length() - 3) + "ini";

	iniFile = new TIniFile(stringIniFileName);
	if(!FileExists(stringIniFileName))
		ShowMessage("Missing configuration file!");

	formMain->Top  = iniFile->ReadInteger(L"FormPos", L"Top", 0);
	formMain->Left = iniFile->ReadInteger(L"FormPos", L"Left", 0);

	formMain->memoLog->Visible = !iniFile->ReadInteger(L"Log", L"Visible", 0);

	stringPId  = iniFile->ReadString(L"My", L"ID", L"PID???");

	intDelay	 = iniFile->ReadInteger(L"Marker", L"Delay", 0);
	boolStep	 = iniFile->ReadBool(L"Marker", L"Step", 0);

	intPortIn  = iniFile->ReadInteger(L"In", L"Port", 0);
	intPortOut = iniFile->ReadInteger(L"Out", L"Port", 0);
	stringIPAddressOut = iniFile->ReadString(L"Out", L"IPAddress", L"127.0.0.1");
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
F_STATE __fastcall TdmChannels::SwitchF_STATE(void)
{
	if(fState == F_FALSE)
	{
		fState = F_TRUE;
	}
	else
	{
		fState = F_FALSE;
	}
	formMain->panelFailure->Color = clFailureStatus[fState];

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
	// CO::OnOutputConnect
	coState = CO_OPENED;
	formMain->panelOutput->Color = clCOStatus[coState];
	String buf = "Output channel connected to " +  csOut->Address;
	formMain->stbarOutStatus->SimpleText = buf;

	// RU::OnOutputConnect
	ruState = RU_DOWN;
	formMain->panelRingUp->Color = clRUStatus[ruState];
	String stringMsgOut = MSG_MRK_RING + MSG_SEPARATOR + stringPId;
	csOut->Socket->SendText(stringMsgOut);
	timerRingUp->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::csOutDisconnect(TObject *Sender, TCustomWinSocket *Socket)
{
	// CO::OnOutputDisonnect
	coState = CO_CLOSED;
	formMain->panelOutput->Color = clCOStatus[coState];
	formMain->stbarOutStatus->SimpleText = "Output channel disconnected";

	/* TODO : ДА СЕ АНАЛИЗИРА ЗАЩО ВРЪЗКАТА НЕ МОЖЕ ДА СЕ ОТВОРИ
	ОТ ОБРАБОТЧИКА НА OnDisconnect */
	timerChannelOut->Enabled = true;

	// RU::OnOutputDisconnect
	ruState = RU_DOWN;
	formMain->panelRingUp->Color = clRUStatus[ruState];
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::csOutError(TObject *Sender, TCustomWinSocket *Socket,
		  TErrorEvent ErrorEvent, int &ErrorCode)
{
	// CO::OnOutputError
	coState = CO_CLOSED;
	formMain->panelOutput->Color = clCOStatus[coState];

	String buf;
	if(++intErrorCounter >= CHN_OUT_MAXERR)
	{
		buf = "Достигнат е макс. брой опити #" + IntToStr(intErrorCounter);
	}
	else
	{
		buf = "Неуспешен опит #" + IntToStr(intErrorCounter);
		timerChannelOut->Enabled = true;
	}
	formMain->stbarOutStatus->SimpleText = buf;

	// RU::OnOutputError
	ruState = RU_DOWN;
	formMain->panelRingUp->Color = clRUStatus[ruState];

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
	{	// RU::OnReceiptOf <mrk_ring, j>
		RU_OnReceiptOfMarker(stringMsg, Socket);
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
			stringMarker = stringMsg;
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
void __fastcall TdmChannels::RU_OnReceiptOfMarker(String stringMsg, TCustomWinSocket *Socket)
{
	// RU::OnReceiptOf <mrk_ring, j>
	AddToLog(Socket, ">> " + stringMsg + " DECODED1");

	String stringMsgPId = stringMsg.c_str() + MSG_MRK_RING.Length() + MSG_SEPARATOR.Length();

	if(stringMsgPId == stringPId)
	{
		timerRingUp->Enabled = false;
		ruState = RU_UP;
		formMain->panelRingUp->Color = clRUStatus[ruState];
		AddToLog("RU::UP");

		// E /////////////////////////////////////////////////////////////////////
		// E::OnStartElection
		E_OnStart();
	}
	else
	{
		String stringMsgOut = stringMsg;
		csOut->Socket->SendText(stringMsgOut);

		AddToLog(csOut->Socket, "<< " + stringMsgOut + " RU::FORWARD");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::E_OnStart(void)
{
	// E::OnStartElection
	stringElectedPId = PID_UNDEFINED;
	formMain->stCoordinator->Caption = stringElectedPId;
	eState = E_PARTICIPANT;

	String stringMsgOut = MSG_ELECTION + MSG_SEPARATOR + stringPId;
	csOut->Socket->SendText(stringMsgOut);

	AddToLog(csOut->Socket, "<< " + stringMsgOut + " E::PASS1 START");
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::E_OnReceiptOfElection(String stringMsg, TCustomWinSocket *Socket)
{
	// E::OnReceiptOf <election, j>
	AddToLog(Socket, ">> " + stringMsg + " DECODED2");

	String stringMsgPId = stringMsg.c_str() + MSG_ELECTION.Length() + MSG_SEPARATOR.Length();

	if(stringMsgPId > stringPId)
	{   // j > i
		stringElectedPId = PID_UNDEFINED;
		formMain->stCoordinator->Caption = stringElectedPId;
		eState = E_PARTICIPANT;

		String stringMsgOut = stringMsg;
		csOut->Socket->SendText(stringMsgOut);

		AddToLog(csOut->Socket, "<< " + stringMsgOut);
	}
	else if(stringMsgPId < stringPId)
	{   // j < i
		if(eState != E_PARTICIPANT)
		{
			stringElectedPId = PID_UNDEFINED;
			formMain->stCoordinator->Caption = stringElectedPId;
			eState = E_PARTICIPANT;

			String stringMsgOut = MSG_ELECTION + MSG_SEPARATOR + stringPId;
			csOut->Socket->SendText(stringMsgOut);

			AddToLog(csOut->Socket, "<< " + stringMsgOut);
		}
	}
	else
	{   // j = i    I am the Coordinator
		stringElectedPId = stringPId;
		formMain->stCoordinator->Caption = stringElectedPId;
		eState = E_NON_PARTICIPANT;

		String stringMsgOut = MSG_ELECTED + MSG_SEPARATOR + stringPId;
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

	if(stringMsgPId != stringPId)
	{	// j != i
		stringElectedPId = stringMsgPId;
		formMain->stCoordinator->Caption = stringElectedPId;
		eState = E_NON_PARTICIPANT;

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
void __fastcall TdmChannels::timerChannelOutTimer(TObject *Sender)
{
	// CO::OnTimer
	timerChannelOut->Enabled = false;
	OpenOutChannel(false);
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::timerRingUpTimer(TObject *Sender)
{
	// RU::OnTimer
	String stringMsgOut = MSG_MRK_RING + MSG_SEPARATOR + stringPId;
	csOut->Socket->SendText(stringMsgOut);
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::MrkME_OnAfterElection(void)
{
	// MrkME::OnAfterElection
	if(stringElectedPId == stringPId)
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
	if(meState != ME_HELD)
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
	if(stringElectedPId == stringPId)
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
	if(stringElectedPId == stringPId)
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
	if(meState == ME_INIT)
	{
		// Допуска се преход в ME_RELEASED единствено от ME_INIT
		meState = ME_RELEASED;
		formMain->stStatus->Caption = stringMEStatus[meState];
		formMain->buttonEnter->Enabled = true;
		formMain->buttonRelease->Enabled = false;
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ME_OnEnter(void)
{
	// ME::OnEnter
	if(meState == ME_RELEASED && IsTimeMrkMENotNull())
	{
		meState = ME_WANTED;
		formMain->stStatus->Caption = stringMEStatus[meState];
		formMain->buttonEnter->Enabled = false;
		formMain->buttonRelease->Enabled = false;
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ME_OnReceiptOfMarker(String stringMsg)
{
	// ME::OnReceiptOfMarker
	formMain->pbarMarker->StepIt();

	if(meState == ME_WANTED && IsTimeMrkMENotNull())
	{
		meState = ME_HELD;
		formMain->stStatus->Caption = stringMEStatus[meState];
		formMain->buttonEnter->Enabled = false;
		formMain->buttonRelease->Enabled = true;

		// ВХОД В СЕКЦИЯТА - access()
	}
	else if(meState == ME_RELEASED)
	{
		meState = ME_RELEASED;
		formMain->stStatus->Caption = stringMEStatus[meState];
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
	if(meState == ME_HELD)
	{
        // отработване на стария маркер
		meState = ME_RELEASED;
		formMain->stStatus->Caption = stringMEStatus[meState];
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

