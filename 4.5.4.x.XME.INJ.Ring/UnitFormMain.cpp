//---------------------------------------------------------------------------

#include <vcl.h>
#include <iphlpapi.h>
#include <tlhelp32.h>
#pragma hdrstop
//---------------------------------------------------------------------------
#include "UnitFormMain.h"
#include "UnitDMChannels.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TformMain *formMain;
//---------------------------------------------------------------------------
__fastcall TformMain::TformMain(TComponent* Owner)
	: TForm(Owner)
{
	// Get Application Version
	strVersion = GetVersion();
	Caption = Caption + " [ver." + strVersion + "]";
}
//---------------------------------------------------------------------------
void __fastcall TformMain::FormShow(TObject *Sender)
{
	labelShowHideLogClick(Sender);

	// CEH::OnShow
	dmChannels->OpenOutChannel(true);
}
//---------------------------------------------------------------------------
void __fastcall TformMain::FormClose(TObject *Sender, TCloseAction &Action)
{
#if defined(CLOSE_CONDITIONAL)
	if(dmChannels->GetME_State() == ME_State::ME_INIT ||
	   dmChannels->GetME_State() == ME_State::ME_RELEASED)
	{
		Action = caFree;
	}
	else
	{
		Action = caNone;		// забранен изход
		return;
	}
#endif

	miSaveClick(Sender);
	dmChannels->WriteIniFile();
}
//---------------------------------------------------------------------------
void __fastcall TformMain::buttonSendMarkerClick(TObject *Sender)
{
	dmChannels->SendMarkerMETrace();
}
//---------------------------------------------------------------------------
void __fastcall TformMain::buttonEnterClick(TObject *Sender)
{
	dmChannels->ME_OnEnter();
}
//---------------------------------------------------------------------------
void __fastcall TformMain::buttonReleaseClick(TObject *Sender)
{
	dmChannels->ME_OnRelease();
}
//---------------------------------------------------------------------------
void __fastcall TformMain::panelFailureDblClick(TObject *Sender)
{
	dmChannels->SwitchF_State();
}
//---------------------------------------------------------------------------
void __fastcall TformMain::labelShowHideLogClick(TObject *Sender)
{
	if(memoLog->Visible)
	{	// Hide
		labelShowHideLog->Caption = strShowHideLogCaptionShow;
		labelShowHideLog->Hint = strShowHideLogHintShow;

		memoLog->Hide();
		ClientHeight = ClientHeight - memoLog->Height;
	}
	else
	{	// Show
		labelShowHideLog->Caption = strShowHideLogCaptionHide;
		labelShowHideLog->Hint = strShowHideLogHintHide;

		ClientHeight = ClientHeight + memoLog->Height;
		memoLog->Show();
	}
}
//---------------------------------------------------------------------------
void __fastcall TformMain::miSaveClick(TObject *Sender)
{
	String strLogFileName = strLogFilePrefix + "." +
                            dmChannels->GetTag() + "." +
							FormatDateTime("yyyymmdd.hhmmss", Date() + Time()) +
							strLogFileExpension;
	try
	{
		memoLog->Lines->SaveToFile(strLogFileName);
		MessageDlg(strLogFileName + " saved", mtInformation, TMsgDlgButtons() << mbOK, 0);
	}
	catch(Exception& e)
	{
		MessageDlg(e.Message, mtError, TMsgDlgButtons() << mbOK, 0);
	}
}
//---------------------------------------------------------------------------
String __fastcall TformMain::GetVersion(void)
{
	String strVersion = "0.0.0.0000";
	String strKeyFileInfo = "";

	String FileName = Application->ExeName;

	DWORD dwDummy;
	DWORD dwSize = GetFileVersionInfoSize(FileName.w_str(),
										&dwDummy);
	if(dwSize > 0)
	{
		TCHAR* lpFileVersionInfo = new TCHAR[dwSize];
		GetFileVersionInfo(FileName.w_str(),
						   0,
						   dwSize,
						   lpFileVersionInfo);

		// Extract Language and Character Set
		struct
		{
			unsigned short language;
			unsigned short character_set;
		} *translation;

		UINT wVVBuferLength;
		VerQueryValue(lpFileVersionInfo,
					  TEXT("\\VarFileInfo\\Translation"),
					  (void **)&translation,
					  &wVVBuferLength) ;

		strKeyFileInfo = "\\StringFileInfo\\"
						 + String::IntToHex(translation[0].language, 4)
						 + String::IntToHex(translation[0].character_set, 4);

		// Extract File Version
		TCHAR* lpFileVersion;
		if(VerQueryValue(lpFileVersionInfo,
						 (strKeyFileInfo + "\\FileVersion").w_str(),
						 (void**)&lpFileVersion,
						 &wVVBuferLength))
		{
			strVersion = lpFileVersion;
		}

		delete [] lpFileVersionInfo;
	}

	return strVersion;
}
//---------------------------------------------------------------------------

