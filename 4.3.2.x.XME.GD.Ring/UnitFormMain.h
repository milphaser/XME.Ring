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
const String strLogFileName             = "Log.txt";

const String strShowHideLogCaptionShow 	= "+";
const String strShowHideLogCaptionHide 	= "-";
const String strShowHideLogHintShow		= "Show Log";
const String strShowHideLogHintHide		= "Hide Log";
//---------------------------------------------------------------------------
class TformMain : public TForm
{
__published:	// IDE-managed Components
	TStatusBar *stbarOutStatus;
	TProgressBar *pbarMarker;
	TMemo *memoLog;
	TGroupBox *GroupBox1;
	TGroupBox *gbE;
	TLabel *labelCoordinator;
	TStaticText *stCoordinator;
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
	TStaticText *stPId;
	TLabel *labelPId;
	TLabel *labelCurrentScale;
	TStaticText *stPIdNext;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall buttonEnterClick(TObject *Sender);
	void __fastcall buttonReleaseClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall panelFailureDblClick(TObject *Sender);
	void __fastcall labelShowHideLogClick(TObject *Sender);
	void __fastcall miSaveClick(TObject *Sender);
	void __fastcall buttonSendMarkerClick(TObject *Sender);

public:
	__fastcall TformMain(TComponent* Owner);
	String __fastcall GetVersion(void);

private:
	String strVersion;
};
//---------------------------------------------------------------------------
extern PACKAGE TformMain *formMain;
//---------------------------------------------------------------------------
#endif
