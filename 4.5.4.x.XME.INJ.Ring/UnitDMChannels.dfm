object dmChannels: TdmChannels
  Height = 135
  Width = 640
  PixelsPerInch = 120
  object ssIn: TServerSocket
    Active = False
    Port = 5000
    ServerType = stNonBlocking
    OnClientConnect = ssInClientConnect
    OnClientDisconnect = ssInClientDisconnect
    OnClientRead = ssInClientRead
    OnClientError = ssInClientError
    Left = 50
    Top = 35
  end
  object csOut: TClientSocket
    Active = False
    ClientType = ctNonBlocking
    Port = 5000
    OnConnect = csOutConnect
    OnDisconnect = csOutDisconnect
    OnError = csOutError
    Left = 135
    Top = 35
  end
  object timerCEH: TTimer
    Enabled = False
    OnTimer = timerCEHTimer
    Left = 221
    Top = 35
  end
  object timerRUP: TTimer
    Enabled = False
    OnTimer = timerRUPTimer
    Left = 306
    Top = 35
  end
  object timerINJ: TTimer
    Enabled = False
    OnTimer = timerINJTimer
    Left = 392
    Top = 35
  end
end
