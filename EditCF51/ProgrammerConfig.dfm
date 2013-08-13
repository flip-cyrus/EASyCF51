object ProgConfig: TProgConfig
  Left = 886
  Top = 344
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Programming Configuration'
  ClientHeight = 312
  ClientWidth = 362
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 16
    Top = 48
    Width = 84
    Height = 13
    Caption = 'Program algorithm'
  end
  object Label2: TLabel
    Left = 16
    Top = 160
    Width = 61
    Height = 13
    Caption = 'S-Record file'
  end
  object Label3: TLabel
    Left = 112
    Top = 48
    Width = 125
    Height = 13
    Caption = '(base address is fixed to 0)'
  end
  object cb_Reset: TCheckBox
    Left = 16
    Top = 16
    Width = 97
    Height = 17
    Caption = 'Reset MCU'
    Checked = True
    Enabled = False
    State = cbChecked
    TabOrder = 0
  end
  object cb_Erase: TCheckBox
    Left = 16
    Top = 96
    Width = 97
    Height = 17
    Caption = 'Erase MCU'
    Checked = True
    State = cbChecked
    TabOrder = 3
  end
  object cb_blankcheck: TCheckBox
    Left = 16
    Top = 128
    Width = 121
    Height = 17
    Caption = 'Blank Check MCU'
    TabOrder = 4
  end
  object cb_Program: TCheckBox
    Left = 16
    Top = 208
    Width = 97
    Height = 17
    Caption = 'Program MCU'
    Checked = True
    State = cbChecked
    TabOrder = 6
  end
  object cb_Verify: TCheckBox
    Left = 16
    Top = 240
    Width = 97
    Height = 17
    Caption = 'Verify MCU'
    Checked = True
    State = cbChecked
    TabOrder = 7
  end
  object e_algorithm: TEdit
    Left = 16
    Top = 64
    Width = 257
    Height = 21
    TabOrder = 1
  end
  object btn_CFPBrowse: TButton
    Left = 272
    Top = 64
    Width = 75
    Height = 25
    Caption = 'Browse'
    TabOrder = 2
    OnClick = btn_CFPBrowseClick
  end
  object rb_default: TRadioButton
    Left = 16
    Top = 176
    Width = 57
    Height = 17
    Caption = 'Default'
    Checked = True
    TabOrder = 5
    TabStop = True
  end
  object rb_custom: TRadioButton
    Left = 80
    Top = 176
    Width = 65
    Height = 17
    Caption = 'Custom:'
    Enabled = False
    TabOrder = 10
  end
  object e_srecord: TEdit
    Left = 152
    Top = 176
    Width = 121
    Height = 21
    Enabled = False
    TabOrder = 11
  end
  object btn_S19Browse: TButton
    Left = 272
    Top = 176
    Width = 75
    Height = 25
    Caption = 'Browse'
    Enabled = False
    TabOrder = 12
  end
  object BitBtn1: TBitBtn
    Left = 16
    Top = 272
    Width = 75
    Height = 25
    TabOrder = 8
    OnClick = BitBtn1Click
    Kind = bkOK
  end
  object BitBtn2: TBitBtn
    Left = 272
    Top = 272
    Width = 75
    Height = 25
    TabOrder = 9
    OnClick = BitBtn2Click
    Kind = bkCancel
  end
  object FO_Algorithm: TOpenDialog
    DefaultExt = '*.CFP'
    Filter = 'PE programming algorithm|*.cfp'
    Left = 272
    Top = 16
  end
  object OpenDialog1: TOpenDialog
    Left = 280
    Top = 136
  end
end
