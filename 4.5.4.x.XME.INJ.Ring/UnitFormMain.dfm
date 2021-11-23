object formMain: TformMain
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Ringbased Distributed ME'
  ClientHeight = 297
  ClientWidth = 555
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -10
  Font.Name = 'Tahoma'
  Font.Style = []
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 12
  object stbarOutStatus: TStatusBar
    Left = 0
    Top = 282
    Width = 555
    Height = 15
    Margins.Left = 2
    Margins.Top = 2
    Margins.Right = 2
    Margins.Bottom = 2
    Panels = <>
    SimplePanel = True
  end
  object pbarMarker: TProgressBar
    Left = 0
    Top = 264
    Width = 555
    Height = 10
    Margins.Left = 2
    Margins.Top = 2
    Margins.Right = 2
    Margins.Bottom = 2
    Align = alBottom
    Step = 1
    TabOrder = 1
  end
  object memoLog: TMemo
    Left = 0
    Top = 10
    Width = 555
    Height = 162
    Margins.Left = 2
    Margins.Top = 2
    Margins.Right = 2
    Margins.Bottom = 2
    Align = alClient
    PopupMenu = pupLog
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 2
  end
  object GroupBox1: TGroupBox
    Left = 0
    Top = 210
    Width = 555
    Height = 54
    Margins.Left = 2
    Margins.Top = 2
    Margins.Right = 2
    Margins.Bottom = 2
    Align = alBottom
    TabOrder = 3
    object gbE: TGroupBox
      Left = 0
      Top = 0
      Width = 209
      Height = 53
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Align = alCustom
      Caption = ' E Control '
      TabOrder = 0
      object labelCoordinator: TLabel
        Left = 11
        Top = 31
        Width = 55
        Height = 12
        Margins.Left = 2
        Margins.Top = 2
        Margins.Right = 2
        Margins.Bottom = 2
        Caption = 'Coordinator'
      end
      object labelPId: TLabel
        Left = 26
        Top = 16
        Width = 40
        Height = 12
        Margins.Left = 2
        Margins.Top = 2
        Margins.Right = 2
        Margins.Bottom = 2
        Caption = 'PId/Next'
      end
      object stCoordinator: TStaticText
        Left = 69
        Top = 30
        Width = 139
        Height = 17
        Hint = 'Doubleclick to start election'
        Margins.Left = 2
        Margins.Top = 2
        Margins.Right = 2
        Margins.Bottom = 2
        AutoSize = False
        BevelInner = bvLowered
        BevelOuter = bvRaised
        BorderStyle = sbsSunken
        ParentShowHint = False
        ShowHint = True
        TabOrder = 0
      end
      object stPId: TStaticText
        Left = 69
        Top = 13
        Width = 69
        Height = 16
        Margins.Left = 2
        Margins.Top = 2
        Margins.Right = 2
        Margins.Bottom = 2
        AutoSize = False
        BevelInner = bvLowered
        BevelOuter = bvRaised
        BorderStyle = sbsSunken
        TabOrder = 1
      end
      object stPIdNext: TStaticText
        Left = 139
        Top = 13
        Width = 69
        Height = 16
        Margins.Left = 2
        Margins.Top = 2
        Margins.Right = 2
        Margins.Bottom = 2
        AutoSize = False
        BevelInner = bvLowered
        BevelOuter = bvRaised
        BorderStyle = sbsSunken
        TabOrder = 2
      end
    end
    object gbME: TGroupBox
      Left = 211
      Top = 0
      Width = 343
      Height = 53
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Align = alCustom
      Caption = ' ME Control '
      TabOrder = 1
      object labelStatus: TLabel
        Left = 13
        Top = 33
        Width = 27
        Height = 12
        Margins.Left = 2
        Margins.Top = 2
        Margins.Right = 2
        Margins.Bottom = 2
        Caption = 'Status'
      end
      object labelStep: TLabel
        Left = 101
        Top = 14
        Width = 83
        Height = 12
        Margins.Left = 2
        Margins.Top = 2
        Margins.Right = 2
        Margins.Bottom = 2
        Alignment = taRightJustify
        Caption = 'Marker Step Mode'
      end
      object stStatus: TStaticText
        Left = 44
        Top = 30
        Width = 140
        Height = 17
        Margins.Left = 2
        Margins.Top = 2
        Margins.Right = 2
        Margins.Bottom = 2
        AutoSize = False
        BevelInner = bvLowered
        BevelOuter = bvRaised
        BorderStyle = sbsSunken
        TabOrder = 0
      end
      object buttonEnter: TButton
        Left = 190
        Top = 29
        Width = 75
        Height = 18
        Margins.Left = 2
        Margins.Top = 2
        Margins.Right = 2
        Margins.Bottom = 2
        Caption = 'ENTER'
        TabOrder = 1
        OnClick = buttonEnterClick
      end
      object buttonRelease: TButton
        Left = 263
        Top = 29
        Width = 75
        Height = 18
        Margins.Left = 2
        Margins.Top = 2
        Margins.Right = 2
        Margins.Bottom = 2
        Caption = 'RELEASE'
        TabOrder = 2
        OnClick = buttonReleaseClick
      end
      object buttonSendMarker: TButton
        Left = 190
        Top = 11
        Width = 148
        Height = 19
        Margins.Left = 2
        Margins.Top = 2
        Margins.Right = 2
        Margins.Bottom = 2
        Caption = 'Send Marker'
        TabOrder = 3
        OnClick = buttonSendMarkerClick
      end
    end
  end
  object gbReliabilityStatus: TGroupBox
    Left = 0
    Top = 172
    Width = 555
    Height = 38
    Margins.Left = 2
    Margins.Top = 2
    Margins.Right = 2
    Margins.Bottom = 2
    Align = alBottom
    Caption = ' Reliability Status '
    TabOrder = 4
    object labelInput: TLabel
      Left = 153
      Top = 10
      Width = 25
      Height = 12
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Caption = 'Input'
    end
    object labelOutput: TLabel
      Left = 212
      Top = 10
      Width = 32
      Height = 12
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Caption = 'Output'
    end
    object labelRing: TLabel
      Left = 280
      Top = 10
      Width = 20
      Height = 12
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Caption = 'Ring'
    end
    object labelFailure: TLabel
      Left = 336
      Top = 10
      Width = 30
      Height = 12
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Caption = 'Failure'
    end
    object labelCurrentScale: TLabel
      Left = 69
      Top = 15
      Width = 5
      Height = 12
      Hint = 'Current Scale'
      Caption = '0'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clBlack
      Font.Height = -10
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      ParentShowHint = False
      ShowHint = True
    end
    object labelBackupScale: TLabel
      Left = 86
      Top = 15
      Width = 5
      Height = 12
      Hint = 'Backup Scale'
      Caption = '0'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clSilver
      Font.Height = -10
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      ParentShowHint = False
      ShowHint = True
    end
    object labelInjection: TLabel
      Left = 11
      Top = 15
      Width = 38
      Height = 12
      Caption = 'injection'
      Color = clBtnFace
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clRed
      Font.Height = -10
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object panelInput: TPanel
      Left = 149
      Top = 23
      Width = 33
      Height = 8
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Color = clRed
      ParentBackground = False
      TabOrder = 0
    end
    object panelOutput: TPanel
      Left = 211
      Top = 23
      Width = 33
      Height = 8
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Color = clRed
      ParentBackground = False
      TabOrder = 1
    end
    object panelRingUp: TPanel
      Left = 274
      Top = 23
      Width = 32
      Height = 8
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Color = clRed
      ParentBackground = False
      TabOrder = 2
    end
    object panelFailure: TPanel
      Left = 336
      Top = 23
      Width = 33
      Height = 8
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Color = clRed
      ParentBackground = False
      TabOrder = 3
      OnDblClick = panelFailureDblClick
    end
    object stTimeMrkME: TStaticText
      Left = 402
      Top = 14
      Width = 147
      Height = 17
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
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
    Width = 555
    Height = 10
    Margins.Left = 2
    Margins.Top = 2
    Margins.Right = 2
    Margins.Bottom = 2
    Align = alTop
    Anchors = [akLeft, akRight]
    TabOrder = 5
    object labelShowHideLog: TLabel
      Left = 8
      Top = -1
      Width = 9
      Height = 10
      Cursor = crSizeNS
      Hint = 'Show/Hide Log'
      Margins.Left = 2
      Margins.Top = 2
      Margins.Right = 2
      Margins.Bottom = 2
      Caption = '+ '
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -8
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
      ParentShowHint = False
      ShowHint = True
      OnClick = labelShowHideLogClick
    end
    object bevelShowHideProtokol: TBevel
      Left = 18
      Top = 2
      Width = 531
      Height = 4
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
    Top = 274
    Width = 555
    Height = 8
    Margins.Left = 2
    Margins.Top = 2
    Margins.Right = 2
    Margins.Bottom = 2
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
