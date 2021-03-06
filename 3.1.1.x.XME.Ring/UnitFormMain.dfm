object formMain: TformMain
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Ringbased Distributed ME'
  ClientHeight = 371
  ClientWidth = 694
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -13
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 120
  TextHeight = 16
  object stbarOutStatus: TStatusBar
    Left = 0
    Top = 352
    Width = 694
    Height = 19
    Panels = <>
    SimplePanel = True
  end
  object pbarMarker: TProgressBar
    Left = 0
    Top = 330
    Width = 694
    Height = 12
    Align = alBottom
    Step = 1
    TabOrder = 1
  end
  object memoLog: TMemo
    Left = 0
    Top = 12
    Width = 694
    Height = 203
    Align = alClient
    PopupMenu = pupLog
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 2
  end
  object GroupBox1: TGroupBox
    Left = 0
    Top = 262
    Width = 694
    Height = 68
    Align = alBottom
    TabOrder = 3
    object gbE: TGroupBox
      Left = 0
      Top = 0
      Width = 261
      Height = 66
      Align = alCustom
      Caption = ' E Control '
      TabOrder = 0
      object labelMyPId: TLabel
        Left = 42
        Top = 20
        Width = 38
        Height = 16
        Caption = 'My PId'
      end
      object labelCoordinator: TLabel
        Left = 12
        Top = 40
        Width = 67
        Height = 16
        Caption = 'Coordinator'
      end
      object stCoordinator: TStaticText
        Left = 86
        Top = 38
        Width = 174
        Height = 21
        Hint = 'Doubleclick to start election'
        AutoSize = False
        BevelInner = bvLowered
        BevelOuter = bvRaised
        BorderStyle = sbsSunken
        ParentShowHint = False
        ShowHint = True
        TabOrder = 0
      end
      object stMyPId: TStaticText
        Left = 86
        Top = 18
        Width = 174
        Height = 20
        AutoSize = False
        BevelInner = bvLowered
        BevelOuter = bvRaised
        BorderStyle = sbsSunken
        TabOrder = 1
      end
    end
    object gbME: TGroupBox
      Left = 264
      Top = 0
      Width = 429
      Height = 66
      Align = alCustom
      Caption = ' ME Control '
      TabOrder = 1
      object labelStatus: TLabel
        Left = 16
        Top = 41
        Width = 36
        Height = 16
        Caption = 'Status'
      end
      object labelStep: TLabel
        Left = 125
        Top = 17
        Width = 105
        Height = 16
        Alignment = taRightJustify
        Caption = 'Marker Step Mode'
      end
      object stStatus: TStaticText
        Left = 55
        Top = 38
        Width = 175
        Height = 21
        AutoSize = False
        BevelInner = bvLowered
        BevelOuter = bvRaised
        BorderStyle = sbsSunken
        TabOrder = 0
      end
      object buttonEnter: TButton
        Left = 238
        Top = 36
        Width = 93
        Height = 23
        Caption = 'ENTER'
        TabOrder = 1
        OnClick = buttonEnterClick
      end
      object buttonRelease: TButton
        Left = 329
        Top = 36
        Width = 93
        Height = 23
        Caption = 'RELEASE'
        TabOrder = 2
        OnClick = buttonReleaseClick
      end
      object buttonSendMarker: TButton
        Left = 238
        Top = 14
        Width = 184
        Height = 23
        Caption = 'Send Marker'
        TabOrder = 3
        OnClick = buttonSendMarkerClick
      end
    end
  end
  object gbReliabilityStatus: TGroupBox
    Left = 0
    Top = 215
    Width = 694
    Height = 47
    Align = alBottom
    Caption = ' Reliability Status '
    TabOrder = 4
    object labelInput: TLabel
      Left = 191
      Top = 13
      Width = 29
      Height = 16
      Caption = 'Input'
    end
    object labelOutput: TLabel
      Left = 265
      Top = 13
      Width = 38
      Height = 16
      Caption = 'Output'
    end
    object labelRing: TLabel
      Left = 350
      Top = 13
      Width = 25
      Height = 16
      Caption = 'Ring'
    end
    object labelFailure: TLabel
      Left = 420
      Top = 13
      Width = 39
      Height = 16
      Caption = 'Failure'
    end
    object panelInput: TPanel
      Left = 186
      Top = 29
      Width = 41
      Height = 10
      Color = clRed
      ParentBackground = False
      TabOrder = 0
    end
    object panelOutput: TPanel
      Left = 264
      Top = 29
      Width = 41
      Height = 10
      Color = clRed
      ParentBackground = False
      TabOrder = 1
    end
    object panelRingUp: TPanel
      Left = 342
      Top = 29
      Width = 41
      Height = 10
      Color = clRed
      ParentBackground = False
      TabOrder = 2
    end
    object panelFailure: TPanel
      Left = 420
      Top = 29
      Width = 41
      Height = 10
      Color = clRed
      ParentBackground = False
      TabOrder = 3
      OnDblClick = panelFailureDblClick
    end
    object stTimeMrkME: TStaticText
      Left = 502
      Top = 18
      Width = 184
      Height = 21
      Alignment = taCenter
      AutoSize = False
      BevelInner = bvLowered
      BevelOuter = bvRaised
      BorderStyle = sbsSunken
      TabOrder = 4
    end
  end
  object panelShowHideProtocol: TPanel
    Left = 0
    Top = 0
    Width = 694
    Height = 12
    Margins.Left = 2
    Margins.Top = 2
    Margins.Right = 2
    Margins.Bottom = 2
    Align = alTop
    Anchors = [akLeft, akRight]
    TabOrder = 5
    object labelShowHideLog: TLabel
      Left = 10
      Top = -1
      Width = 11
      Height = 12
      Cursor = crSizeNS
      Hint = 'Show/Hide Log'
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Caption = '+ '
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -10
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
      ParentShowHint = False
      ShowHint = True
      OnClick = labelShowHideLogClick
    end
    object bevelShowHideProtokol: TBevel
      Left = 22
      Top = 2
      Width = 664
      Height = 5
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Shape = bsBottomLine
      Style = bsRaised
    end
  end
  object panelMrkME: TPanel
    Left = 0
    Top = 342
    Width = 694
    Height = 10
    Align = alBottom
    Color = clRed
    ParentBackground = False
    TabOrder = 6
  end
  object pupLog: TPopupMenu
    Left = 56
    Top = 40
    object miSave: TMenuItem
      Caption = 'Save'
      OnClick = miSaveClick
    end
  end
end
