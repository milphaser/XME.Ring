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
	stringVersion = GetVersion();
	Caption = Caption + " [ver." + stringVersion + "]";
}
//---------------------------------------------------------------------------
void __fastcall TformMain::FormShow(TObject *Sender)
{
	labelShowHideLogClick(Sender);

	// CO::OnShow
	dmChannels->OpenOutChannel(true);
}
//---------------------------------------------------------------------------
void __fastcall TformMain::FormClose(TObject *Sender, TCloseAction &Action)
{
#if defined(CLOSE_CONDITIONAL)
	if(dmChannels->GetME_STATE() == ME_INIT ||
	   dmChannels->GetME_STATE() == ME_RELEASED)
	{
		Action = caFree;
	}
	else
	{
		Action = caNone;		// �������� �����
		return;
	}
#endif

	dmChannels->WriteIniFile();
}
//---------------------------------------------------------------------------
void __fastcall TformMain::buttonSendMarkerClick(TObject *Sender)
{
    dmChannels->SendMarker();
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
	dmChannels->SwitchF_STATE();
}
//---------------------------------------------------------------------------
void __fastcall TformMain::labelShowHideLogClick(TObject *Sender)
{
	if(memoLog->Visible)
	{	// Hide
		labelShowHideLog->Caption = stringShowHideLogCaptionShow;
		labelShowHideLog->Hint = stringShowHideLogHintShow;

		memoLog->Hide();
		ClientHeight = ClientHeight - memoLog->Height;
	}
	else
	{	// Show
		labelShowHideLog->Caption = stringShowHideLogCaptionHide;
		labelShowHideLog->Hint = stringShowHideLogHintHide;

		ClientHeight = ClientHeight + memoLog->Height;
		memoLog->Show();
	}
}
//---------------------------------------------------------------------------
void __fastcall TformMain::miSaveClick(TObject *Sender)
{
	String stringLogFileNamePath = ExtractFilePath(Application->ExeName);
	try
	{
		memoLog->Lines->SaveToFile(stringLogFileNamePath + stMyPId->Caption + "_" + stringLogFileName);
		MessageDlg(stMyPId->Caption + "_" + stringLogFileName + " saved", mtInformation, TMsgDlgButtons() << mbOK, 0);
	}
	catch(Exception& e)
	{
		MessageDlg(e.Message, mtError, TMsgDlgButtons() << mbOK, 0);
    }
}
//---------------------------------------------------------------------------
String __fastcall TformMain::GetVersion(void)
{
	String stringVersion = "0.0.0.0000";
	String stringKeyFileInfo = "";

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

		stringKeyFileInfo = "\\StringFileInfo\\"
							+ String::IntToHex(translation[0].language, 4)
							+ String::IntToHex(translation[0].character_set, 4);

		// Extract File Version
		TCHAR* lpFileVersion;
		if(VerQueryValue(lpFileVersionInfo,
						 (stringKeyFileInfo + "\\FileVersion").w_str(),
						 (void**)&lpFileVersion,
						 &wVVBuferLength))
		{
			stringVersion = lpFileVersion;
		}

		delete [] lpFileVersionInfo;
	}

	return stringVersion;
}
//---------------------------------------------------------------------------

