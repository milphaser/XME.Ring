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

const String strMEStatus[] 	=
{
	L"INIT", L"RELEASED", L"WANTED", L"HELD"
};
//---------------------------------------------------------------------------
__fastcall TdmChannels::TdmChannels(TComponent* Owner)
	: TDataModule(Owner)
{
	ReadIniFile();	// read parameters from the configuration file

	if(boolStep)
	{   // step movement of ME token
		formMain->labelStep->Show();
		formMain->buttonSendMarker->Show();
		formMain->buttonSendMarker->Enabled = false;
	}
	else
	{   // automatic movement of ME token
		formMain->labelStep->Hide();
		formMain->buttonSendMarker->Hide();
		formMain->buttonSendMarker->Enabled = false;
	}

	// CEH ////////////////////////////////////////////////////////////////////
	// CEH::OnInit
	intErrorCounter = 0;
	cehState = CEH_State::INIT;
	formMain->panelOutput->Color = clCEHStatus[static_cast<int>(cehState)];
	timerCEH->Interval = MAX_CEH_PERIOD;

	// RUP ////////////////////////////////////////////////////////////////////
	// RUP::OnInit
	rupState = RUP_State::INIT;
	formMain->panelRingUp->Color = clRUPStatus[static_cast<int>(rupState)];
	timerRUP->Interval = MAX_RUP_PERIOD;

	// INJ ////////////////////////////////////////////////////////////////////
	// INJ::OnInit
	strInjMsg = "";

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
	// ME::OnInit - before election
	meState = ME_State::INIT;
	formMain->stStatus->Caption = strMEStatus[static_cast<int>(meState)];
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
	boolInjection = iniFile->ReadBool(L"This", L"Injection", false);
	if(boolInjection)
	{
		formMain->labelInjection->Show();
	}
	else
	{
		formMain->labelInjection->Hide();
	}

	PID<int> pidItem;

	pidItem.id = iniFile->ReadInteger(L"This", L"ID", 0);
	pidItem.tag = iniFile->ReadString(L"This", L"Tag", PID_NULL);
	pidItem.ep.first = iniFile->ReadString(L"This", L"IP", L"127.0.0.1");
	pidItem.ep.second = iniFile->ReadInteger(L"This", L"Port", 0);

	pidThis = pidItem;

	formMain->stPId->Caption = pidThis.id;
	fState = F_State::FAULTLESS;
	formMain->panelFailure->Color = clFailureStatus[static_cast<int>(fState)];

	ssIn->Port = pidThis.ep.second;
	ssIn->Open();
	formMain->panelInput->Color = clInputStatus[false];

	for(int i = 1; true; i++)
	{
		String strSection = "Next" + IntToStr(i);
		if(!iniFile->SectionExists(strSection)) break;

		pidItem.id = iniFile->ReadInteger(strSection, L"ID", 0);
		pidItem.tag = iniFile->ReadString(strSection, L"Tag", PID_NULL);
		pidItem.ep.first = iniFile->ReadString(strSection, L"IP", L"127.0.0.1");
		pidItem.ep.second = iniFile->ReadInteger(strSection, L"Port", 0);

		ListPIds.push_back(pidItem);

		if(pidItem == pidThis)
		{
			// pidThis is the bottom of ListPIds
			break;
		}
	}

	if(ListPIds.empty())
	{
		// pidThis is the bottom of ListPIds
		// and ListPIds must contain at least pidThis
		ListPIds.push_back(pidThis);
	}

	formMain->stPIdNext->Caption = ListPIds.front().id;
	csOut->Address = ListPIds.front().ep.first;
	csOut->Port = ListPIds.front().ep.second;

	for(int i = 1; true; i++)
	{
		String strSection = "Backup" + IntToStr(i);
		if(!iniFile->SectionExists(strSection)) break;

		pidItem.id = iniFile->ReadInteger(strSection, L"ID", 0);
		pidItem.tag = iniFile->ReadString(strSection, L"Tag", PID_NULL);
		pidItem.ep.first = iniFile->ReadString(strSection, L"IP", L"127.0.0.1");
		pidItem.ep.second = iniFile->ReadInteger(strSection, L"Port", 0);

		ListBackups.push_back(pidItem);
	}

	// Diagnostic print of ListPIds
	String strListPIds = "ListPIds:";
	for(auto& x : ListPIds)
	{
		strListPIds += " " + IntToStr(x.id);
	}
	AddToLog(strListPIds);
	formMain->labelCurrentScale->Caption = ListPIds.size();
	formMain->labelBackupScale->Caption = ListBackups.size();
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
	AddToLog(Socket, "[IN CNC]");
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ssInClientDisconnect(TObject *Sender, TCustomWinSocket *Socket)
{
	if(ssIn->Socket->ActiveConnections == 0)
	{
		formMain->panelInput->Color = clInputStatus[false];
		AddToLog(Socket, "[IN DSC]");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ssInClientError(TObject *Sender, TCustomWinSocket *Socket,
		  TErrorEvent ErrorEvent, int &ErrorCode)
{
	formMain->panelInput->Color = clInputStatus[false];
	AddToLog("[IN ERR #" + IntToStr(ErrorCode) + "]");

	ErrorCode = 0;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::csOutConnect(TObject *Sender, TCustomWinSocket *Socket)
{
	AddToLog("[OUT " + ListPIds.front().tag + " CNC]");

	// CEH::OnConnect
	intErrorCounter = 0;
	cehState = CEH_State::OPENED;
	formMain->panelOutput->Color = clCEHStatus[static_cast<int>(cehState)];
	String str = "Output channel connected to " +  csOut->Address;
	formMain->stbarOutStatus->SimpleText = str;

	if(boolInjection)
	{
		// INJ::OnConnect
		String strMsgOut = MSG_MRK_INJ + MSG_SEPARATOR + pidThis.id + MSG_SEPARATOR + ListPIds.front().id;
		csOut->Socket->SendText(strMsgOut);

		AddToLog(Socket, "<< " + strMsgOut + " [INJ::START]");

		rupState = RUP_State::DOWN;
		formMain->panelRingUp->Color = clRUPStatus[static_cast<int>(rupState)];
	}
	else
	{
		if(!strInjMsg.IsEmpty())
		{
			// INJ::OnReceiptOf <mrk_inj, j, k>
			// Last Phase after Reconnection to Injected process id <j>
			// Forward  <mrk_inj, j, k> after Pi reconection from Pk to Pj
			String strMsgOut = strInjMsg;
			csOut->Socket->SendText(strMsgOut);
			AddToLog(Socket, "<< " + strMsgOut);
			strInjMsg = "";
		}

		// RUP::OnConnect
		rupState = RUP_State::DOWN;
		formMain->panelRingUp->Color = clRUPStatus[static_cast<int>(rupState)];
		String strMsgOut = MSG_MRK_RUP + MSG_SEPARATOR + pidThis.id;
		csOut->Socket->SendText(strMsgOut);
		AddToLog(Socket, "<< " + strMsgOut + " [RUP::START]");
		timerRUP->Enabled = true;
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::csOutDisconnect(TObject *Sender, TCustomWinSocket *Socket)
{
	AddToLog("[OUT " + ListPIds.front().tag + " DSC]");

	// INJ::OnError
	if(boolInjection)
	{
		formMain->stbarOutStatus->SimpleText = "Injection Failed";
		return;
	}

	// CEH::OnDisonnect
	cehState = CEH_State::CLOSED;
	formMain->panelOutput->Color = clCEHStatus[static_cast<int>(cehState)];
	formMain->stbarOutStatus->SimpleText = "Output channel disconnected";

	// RUP::OnDisconnect
	rupState = RUP_State::DOWN;
	formMain->panelRingUp->Color = clRUPStatus[static_cast<int>(rupState)];

	/* TODO : ÄÀ ÑÅ ÀÍÀËÈÇÈÐÀ ÇÀÙÎ ÂÐÚÇÊÀÒÀ ÍÅ ÌÎÆÅ ÄÀ ÑÅ ÎÒÂÎÐÈ
	ÄÈÐÅÊÒÍÎ ÎÒ ÎÁÐÀÁÎÒ×ÈÊÀ ÍÀ OnDisconnect */
	OpenOutChannel(true);
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::csOutError(TObject *Sender, TCustomWinSocket *Socket,
		  TErrorEvent ErrorEvent, int &ErrorCode)
{
	AddToLog("[OUT " + ListPIds.front().tag + " ERR #" + IntToStr(ErrorCode) + "]");

	// INJ::OnError
	if(boolInjection)
	{
		formMain->stbarOutStatus->SimpleText = "Injection Failed";
		ErrorCode = 0;
		return;
	}

	// CEH::OnError
	cehState = CEH_State::CLOSED;
	formMain->panelOutput->Color = clCEHStatus[static_cast<int>(cehState)];

	String str;
	if(++intErrorCounter >= MAX_CEH_ERR)
	{
		if(ListPIds.size() >= MIN_K)
		{
			// Remove faulty neighbor
			ListPIds.pop_front();

			// Diagnostic print of ListPIds
			String strListPIds = "ListPIds:";
			for(auto& x : ListPIds)
			{
				strListPIds += " " + IntToStr(x.id);
			}
			AddToLog(strListPIds);
			formMain->labelCurrentScale->Caption = ListPIds.size();

			formMain->stPIdNext->Caption = ListPIds.front().id;
			csOut->Address = ListPIds.front().ep.first;
			csOut->Port = ListPIds.front().ep.second;

			OpenOutChannel(true);
		}
		else
		{
			str = "UNRECOVERABLE FAILURE at attempt #" + IntToStr(intErrorCounter);
		}
	}
	else
	{
		str = "Unsuccessful attempt #" + IntToStr(intErrorCounter);
		OpenOutChannel(false);
	}
	formMain->stbarOutStatus->SimpleText = str;

	// RUP::OnError
	rupState = RUP_State::DOWN;
	formMain->panelRingUp->Color = clRUPStatus[static_cast<int>(rupState)];

	ErrorCode = 0;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::OpenOutChannel(bool boolClearErrorCounter)
{
	if(boolClearErrorCounter)
	{
		intErrorCounter = 0;
		formMain->stbarOutStatus->SimpleText = "Wait to connect...";
	}

	csOut->Close();             // Clear previous comm. errors if any (!)
	timerCEH->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::CloseOutChannel(void)
{
	csOut->Close();
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ssInClientRead(TObject *Sender, TCustomWinSocket *Socket)
{
	String strMsgIn = Socket->ReceiveText();     // Receive Message

	for(int intX = MSG_LEADER.Length(); intX > 0;
		strMsgIn = strMsgIn.c_str() + MSG_LEADER.Length() + intX - 1)
	{
		String strMsg;
		String strBuf = strMsgIn.c_str() + MSG_LEADER.Length();

		intX = strBuf.Pos(MSG_LEADER);

		if(intX > 0)
		{
			strMsg = strBuf.SubString(1, intX - 1);
		}
		else
		{
			strMsg = strBuf;
		}

		OnReceiptOfMsg(MSG_LEADER + strMsg, Socket);
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::OnReceiptOfMsg(String strMsg, TCustomWinSocket *Socket)
{
	String strMsgPId;          // Received PId
	String strMsgOut;          // Sended Message

	if(strMsg.Pos(MSG_MRK_INJ) == 1)
	{	// INJ::OnReceiptOf <mrk_inj, j, k>
		INJ_OnReceiptOfMarker(strMsg, Socket);
	}
	else if(strMsg.Pos(MSG_MRK_RUP) == 1)
	{	// RUP::OnReceiptOf <mrk_ring, j>
		RUP_OnReceiptOfMarker(strMsg, Socket);
	}
	else if(strMsg.Pos(MSG_ELECTION) == 1)
	{	// E::OnReceiptOf <election, j>
		E_OnReceiptOfElection(strMsg, Socket);
	}
	else if(strMsg.Pos(MSG_ELECTED) == 1)
	{	// E::OnReceiptOf <elected, j>
		E_OnReceiptOfElected(strMsg, Socket);
	}
	else if(strMsg.Pos(MSG_MRK_ME_CLR) == 1)
	{  // MrkME::OnReceiptOf <mrk_me_clr>
		MrkME_OnReceiptOfClear(strMsg);
	}
	else if(strMsg.Pos(MSG_MRK_ME_MRK) == 1)
	{
		if(boolMrkME)
		{  // MrkME::OnReceiptOf <mrk_me, Tj>
			MrkME_OnReceiptOfMarker(strMsg);
		}
		else if(strMsg.Pos(MSG_MRK_ME_MRK) == 1)
		{  // ME Marker Message Received
			ME_OnReceiptOfMarker(strMsg);
		}
	}
	else
	{
		AddToLog(Socket, ">> " + strMsg + " [UND]");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::SendMarkerME(String strMsg)
{
	if(!boolStep)
	{
		if(intDelay > 0)
		{
			Sleep(intDelay);		// token preview delay
		}

		csOut->Socket->SendText(strMsg);
	}
	else
	{
		if(!formMain->buttonSendMarker->Enabled)
		{   // Step 1: fixing of the value and hold ME token
			strMarkerTrace = strMsg;
			formMain->buttonSendMarker->Enabled = true;
		}
		else
		{   // Step 2: manual sending of ME token, hold at Step 1
			formMain->buttonSendMarker->Enabled = false;
			csOut->Socket->SendText(strMsg);
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
TTime __fastcall TdmChannels::StrToTime(String strTime)
{
	TTime timeResult;

	try
	{
		String hh  = strTime.SubString(1, 2);
		String mm  = strTime.SubString(4, 2);
		String ss  = strTime.SubString(7, 2);
		String zzz = strTime.SubString(10, 3);

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
void __fastcall TdmChannels::INJ_OnReceiptOfMarker(String strMsg, TCustomWinSocket *Socket)
{
	// INJ::OnReceiptOf <mrk_inj, j, k>
	//   <j> - id of the injected process
	//   <k> - id of the point of injection
	AddToLog(Socket, ">> " + strMsg + " [INJ::DEC]");

	// Extract <j> and <k> parameters from the received message
	String strMsgPars = strMsg.c_str() + MSG_MRK_INJ.Length() + MSG_SEPARATOR.Length();
	int intX = strMsgPars.Pos(MSG_SEPARATOR);

	int intPar1;					// Get <j> id
	int intPar2;					// Get <k> id
	{
		// Get <j> id
		String strMsgPar1 = strMsgPars.SubString(1, intX - 1);
		// Get <k> id
		String strMsgPar2 = strMsgPars.c_str() + (intX - 1) + MSG_SEPARATOR.Length();

		intPar1 = strMsgPar1.ToInt();
		intPar2 = strMsgPar2.ToInt();
	}

	if(pidThis.id == intPar1)
	{
		// i == j
		boolInjection = false;
		formMain->labelInjection->Hide();

		rupState = RUP_State::UP;
		formMain->panelRingUp->Color = clRUPStatus[static_cast<int>(rupState)];
		AddToLog("INJ::END [RUP::UP]");

		// E /////////////////////////////////////////////////////////////////////
		// E::OnStartElection
		E_OnStart();
		return;
	}
	else
	{
		// i != j
		bool boolInjIsPlaced = false;
		for(auto& item : ListPIds)
		{
			if(item.id == intPar1)
			{
				// the id <j> of the injected process already hev been placed in the ListedPIds
				boolInjIsPlaced = true;
				break;
			}
		}

		// Get the injected process parameters
		PID<int> pidInj;
		for(auto& item : ListBackups)
		{
			if(item.id == intPar1)
			{
				pidInj = item;
				break;
			}
		}

		PID<int> pidNext = ListPIds.front();

		//..................................................................
		// Refresh ListPIds inserting <j> before <k>
		//..................................................................
		if(!boolInjIsPlaced)
		{
			std::deque<PID<int>> ListPIdsHead;
			while(!ListPIds.empty())
			{
				auto& item = ListPIds.front();
				if(item.id != intPar2)
				{
					// remove the head until <k>
					ListPIds.pop_front();
					ListPIdsHead.push_back(item);
				}
				else
				{
					// insert <j> before <k>
					ListPIds.push_front(pidInj);
					break;
				}
			}

			// insert back the head before <j>
			while(!ListPIdsHead.empty())
			{
				auto& item = ListPIdsHead.back();
				ListPIdsHead.pop_back();
				ListPIds.push_front(item);
			}

			// Diagnostic print of ListPIds
			String strListPIds = "ListPIds:";
			for(auto& x : ListPIds)
			{
				strListPIds += " " + IntToStr(x.id);
			}
			AddToLog(strListPIds);
			formMain->labelCurrentScale->Caption = ListPIds.size();
		}
		//..................................................................

		//..................................................................
		// If <k> equals the current next process id of Pi
		// than disconnect Pi from Pk and reconnect Pi to Pj
		//..................................................................
		if(pidNext.id == intPar2)
		{
			// Save <mrk_inj, j, k> to forward after reconnection
			strInjMsg = strMsg;

			// Disconnect Pi from Pk
			CloseOutChannel();

			// Diagnostic print of ListPIds
			String strListPIds = "ListPIds:";
			for(auto& x : ListPIds)
			{
				strListPIds += " " + IntToStr(x.id);
			}
			AddToLog(strListPIds);
			formMain->labelCurrentScale->Caption = ListPIds.size();

			// Connect Pi to Pj
			formMain->stPIdNext->Caption = ListPIds.front().id;
			csOut->Address = ListPIds.front().ep.first;
			csOut->Port = ListPIds.front().ep.second;
			OpenOutChannel(true);
		}
		else
		{
			// Forward  <mrk_inj, j, k>
			String strMsgOut = strMsg;
			csOut->Socket->SendText(strMsgOut);
			AddToLog(Socket, "<< " + strMsgOut);

			// RUP::OnConnect
			rupState = RUP_State::DOWN;
			formMain->panelRingUp->Color = clRUPStatus[static_cast<int>(rupState)];
			strMsgOut = MSG_MRK_RUP + MSG_SEPARATOR + pidThis.id;
			csOut->Socket->SendText(strMsgOut);
			AddToLog(Socket, "<< " + strMsgOut + " [RUP::START]");
			timerRUP->Enabled = true;
		}
		//..................................................................
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::RUP_OnReceiptOfMarker(String strMsg, TCustomWinSocket *Socket)
{
	// RUP::OnReceiptOf <mrk_ring, j>
	AddToLog(Socket, ">> " + strMsg + " [DEC1]");

	String strMsgPId = strMsg.c_str() + MSG_MRK_RUP.Length() + MSG_SEPARATOR.Length();

	if(strMsgPId == pidThis.id)
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
		String strMsgOut = strMsg;
		csOut->Socket->SendText(strMsgOut);

		AddToLog(csOut->Socket, "<< " + strMsgOut + " [RUP::FORWARD]");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::E_OnStart(void)
{
	// E::OnStartElection
	strElectedPId = PID_NULL;
	formMain->stCoordinator->Caption = strElectedPId;
	eState = E_State::PARTICIPANT;

	String strMsgOut = MSG_ELECTION + MSG_SEPARATOR + pidThis.id;
	csOut->Socket->SendText(strMsgOut);

	AddToLog(csOut->Socket, "<< " + strMsgOut + " [E::PASS1 START]");
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::E_OnReceiptOfElection(String strMsg, TCustomWinSocket *Socket)
{
	// E::OnReceiptOf <election, j>
	AddToLog(Socket, ">> " + strMsg + " [DEC2]");

	String strMsgPId = strMsg.c_str() + MSG_ELECTION.Length() + MSG_SEPARATOR.Length();

	if(strMsgPId > pidThis.id)
	{   // j > i
		strElectedPId = PID_NULL;
		formMain->stCoordinator->Caption = strElectedPId;
		eState = E_State::PARTICIPANT;

		String strMsgOut = strMsg;
		csOut->Socket->SendText(strMsgOut);

		AddToLog(csOut->Socket, "<< " + strMsgOut);
	}
	else if(strMsgPId < pidThis.id)
	{   // j < i
		if(eState != E_State::PARTICIPANT)
		{
			strElectedPId = PID_NULL;
			formMain->stCoordinator->Caption = strElectedPId;
			eState = E_State::PARTICIPANT;

			String strMsgOut = MSG_ELECTION + MSG_SEPARATOR + pidThis.id;
			csOut->Socket->SendText(strMsgOut);

			AddToLog(csOut->Socket, "<< " + strMsgOut);
		}
	}
	else
	{   // j = i    I am the Coordinator
		strElectedPId = pidThis.id;
		formMain->stCoordinator->Caption = strElectedPId;
		eState = E_State::NONPARTICIPANT;

		String strMsgOut = MSG_ELECTED + MSG_SEPARATOR + pidThis.id;
		csOut->Socket->SendText(strMsgOut);

		AddToLog(csOut->Socket, "<< " + strMsgOut + " [E::PASS2 START]");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::E_OnReceiptOfElected(String strMsg, TCustomWinSocket *Socket)
{
	// E::OnReceiptOf <elected, j>
	AddToLog(Socket, ">> " + strMsg + " [DEC3]");

	String strMsgPId = strMsg.c_str() + MSG_ELECTED.Length() + MSG_SEPARATOR.Length();

	if(strMsgPId != pidThis.id)
	{	// j != i
		strElectedPId = strMsgPId;
		formMain->stCoordinator->Caption = strElectedPId;
		eState = E_State::NONPARTICIPANT;

		String strMsgOut = strMsg;
		csOut->Socket->SendText(strMsgOut);

		AddToLog(csOut->Socket, "<< " + strMsgOut + " [E::LOCAL END]");

		// ME /////////////////////////////////////////////////////////////////////
		// ME::OnInit - after election
		ME_OnAfterElection();
		MrkME_OnAfterElection();
	}
	else
	{   // j = i
		AddToLog(csOut->Socket, "E::END");

		// ME /////////////////////////////////////////////////////////////////////
		// ME::OnInit - after election
		ME_OnAfterElection();
		MrkME_OnAfterElection();
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::timerCEHTimer(TObject *Sender)
{
	// CEH::OnTimer
	timerCEH->Enabled = false;
	csOut->Open();
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::timerRUPTimer(TObject *Sender)
{
	// RUP::OnTimer
	String strMsgOut = MSG_MRK_RUP + MSG_SEPARATOR + pidThis.id;
	csOut->Socket->SendText(strMsgOut);
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::MrkME_OnAfterElection(void)
{
	// MrkME::OnAfterElection
	if(strElectedPId == pidThis.id)
	{
		boolMrkME = true;
		formMain->panelMrkME->Color = clMrkMEStatus[boolMrkME];

		SetTimeMrkME();

		String strMsgOut = MSG_MRK_ME_CLR;
		SendMarkerME(strMsgOut);

		AddToLog(csOut->Socket, "<< " + strMsgOut + " [MrkME::PASS1 START]");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::MrkME_OnReceiptOfClear(String strMsg)
{
	// MrkME::OnReceiptOf <mrk_me_clr>
	if(meState != ME_State::HELD)
	{
		strClrPending = strMsg;
		MrkME_OnClear();
		strClrPending = "";
	}
	else
	{
		strClrPending = strMsg;
		AddToLog(csOut->Socket, "<< " + strMsg + " [MrkME::PENDING]");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::MrkME_OnClear(void)
{
	// MrkME::OnClear
	if(strElectedPId == pidThis.id)
	{
		AddToLog(csOut->Socket, "<< " + strClrPending + " [MrkME::PASS1 END]");

		SetTimeMrkME(Time());

		String strMsgOut = MSG_MRK_ME_MRK + MSG_SEPARATOR + FormatDateTime("hh:mm:ss.zzz", timeMrkME);
		SendMarkerME(strMsgOut);

		AddToLog(csOut->Socket, "<< " + strMsgOut + " [MrkME::PASS2 START]");
	}
	else
	{
		boolMrkME = true;
		formMain->panelMrkME->Color = clMrkMEStatus[boolMrkME];
		SetTimeMrkME();

		String strMsgOut = strClrPending;
		csOut->Socket->SendText(strMsgOut);

		AddToLog(csOut->Socket, "<< " + strMsgOut + " [MrkME::LOCAL START]");
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::MrkME_OnReceiptOfMarker(String strMsg)
{
	// MrkME::OnReceiptOf <mrk_me, Tj>
	if(strElectedPId == pidThis.id)
	{
		String strTj = strMsg.c_str() + MSG_MRK_ME_MRK.Length() + MSG_SEPARATOR.Length();
		String strTimeMrkME = FormatDateTime("hh:mm:ss.zzz", timeMrkME);
		if(strTimeMrkME == strTj)
		{
			if(boolMrkME)
			{
				boolMrkME = false;
				formMain->panelMrkME->Color = clMrkMEStatus[boolMrkME];

				AddToLog(csOut->Socket, "<< " + strMsg + " [MrkME::END]");
			}

			SendMarkerME(strMsg);
		}
		else
		{   // Diagnostic log record
			AddToLog(csOut->Socket, "<< " + strMsg + " [MrkME::OLD MARKER ABSORBED]");
		}
	}
	else
	{
		String strTj = strMsg.c_str() + MSG_MRK_ME_MRK.Length() + MSG_SEPARATOR.Length();
		String strTimeMrkME = FormatDateTime("hh:mm:ss.zzz", timeMrkME);

		TTime t0;
		if(timeMrkME == t0)
		{
			boolMrkME = false;
			formMain->panelMrkME->Color = clMrkMEStatus[boolMrkME];
			SetTimeMrkME(StrToTime(strTj));

			AddToLog(csOut->Socket, "<< " + strMsg + " [MrkME::LOCAL END]");

			SendMarkerME(strMsg);
		}
		else
		{
			if(strTimeMrkME == strTj)
			{
				String strMsgOut = strMsg;
				csOut->Socket->SendText(strMsgOut);
			}
			else
			{   // Diagnostic log record
				AddToLog(csOut->Socket, "<< " + strMsg + " [MrkME::ANCIENT MARKER ABSORBED]");
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ME_OnAfterElection(void)
{
	// The initial start of ME token is moved into MrkME::OnAfterElection
	if(meState == ME_State::INIT)
	{
		// It is allowed transition into RELEASED only from INIT
		meState = ME_State::RELEASED;
		formMain->stStatus->Caption = strMEStatus[static_cast<int>(meState)];
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
		formMain->stStatus->Caption = strMEStatus[static_cast<int>(meState)];
		formMain->buttonEnter->Enabled = false;
		formMain->buttonRelease->Enabled = false;
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ME_OnReceiptOfMarker(String strMsg)
{
	// ME::OnReceiptOfMarker
	formMain->pbarMarker->StepIt();

	if(meState == ME_State::WANTED && IsTimeMrkMENotNull())
	{
		meState = ME_State::HELD;
		formMain->stStatus->Caption = strMEStatus[static_cast<int>(meState)];
		formMain->buttonEnter->Enabled = false;
		formMain->buttonRelease->Enabled = true;

		// ENTRANCE INTO THE CRITICAL SECTION
		// access()
	}
	else if(meState == ME_State::RELEASED)
	{
		meState = ME_State::RELEASED;
		formMain->stStatus->Caption = strMEStatus[static_cast<int>(meState)];
		formMain->buttonEnter->Enabled = true;
		formMain->buttonRelease->Enabled = false;

		String strMsgOut = strMsg;
		SendMarkerME(strMsgOut);
	}
}
//---------------------------------------------------------------------------
void __fastcall TdmChannels::ME_OnRelease(void)
{
	// ME::OnRelease
	if(meState == ME_State::HELD)
	{
		// working out of the old ME token
		meState = ME_State::RELEASED;
		formMain->stStatus->Caption = strMEStatus[static_cast<int>(meState)];
		formMain->buttonEnter->Enabled = true;
		formMain->buttonRelease->Enabled = false;

		String strMsgOut = MSG_MRK_ME_MRK + MSG_SEPARATOR + FormatDateTime("hh:mm:ss.zzz", timeMrkME);
		SendMarkerME(strMsgOut);

		if(!strClrPending.IsEmpty())
		{
			// working out of the hold <mrk_me_clr> message
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

