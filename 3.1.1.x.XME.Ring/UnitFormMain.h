//---------------------------------------------------------------------------

#ifndef UnitFormMainH
#define UnitFormMainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Menus.hpp>
//---------------------------------------------------------------------------
//#define CLOSE_CONDITIONAL
//---------------------------------------------------------------------------
const String stringLogFileName              = "Log.txt";

const String stringShowHideLogCaptionShow 	= "+";
const String stringShowHideLogCaptionHide 	= "-";
const String stringShowHideLogHintShow 		= "Show Log";
const String stringShowHideLogHintHide 		= "Hide Log";
//---------------------------------------------------------------------------
class TformMain : public TForm
{
__published:	// IDE-managed Components
	TStatusBar *stbarOutStatus;
	TProgressBar *pbarMarker;
	TMemo *memoLog;
	TGroupBox *GroupBox1;
	TGroupBox *gbE;
	TLabel *labelMyPId;
	TLabel *labelCoordinator;
	TStaticText *stCoordinator;
	TStaticText *stMyPId;
	TGroupBox *gbME;
	TLabel *labelStatus;
	TStaticText *stStatus;
	TButton *buttonEnter;
	TButton *buttonRelease;
	TGroupBox *gbReliabilityStatus;
	TPanel *panelInput;
	TPanel *panelOutput;
	TPanel *panelRingUp;
	TPanel *panelFailure;
	TLabel *labelInput;
	TLabel *labelOutput;
	TLabel *labelRing;
	TLabel *labelFailure;
	TPanel *panelShowHideProtocol;
	TLabel *labelShowHideLog;
	TBevel *bevelShowHideProtokol;
	TPanel *panelMrkME;
	TPopupMenu *pupLog;
	TMenuItem *miSave;
	TButton *buttonSendMarker;
	TLabel *labelStep;
	TStaticText *stTimeMrkME;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall buttonEnterClick(TObject *Sender);
	void __fastcall buttonReleaseClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall panelFailureDblClick(TObject *Sender);
	void __fastcall labelShowHideLogClick(TObject *Sender);
	void __fastcall miSaveClick(TObject *Sender);
	void __fastcall buttonSendMarkerClick(TObject *Sender);

private:
	String stringVersion;

public:
	__fastcall TformMain(TComponent* Owner);
	String __fastcall GetVersion(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TformMain *formMain;
//---------------------------------------------------------------------------
#endif
