object dmChannels: TdmChannels
  OldCreateOrder = False
  Height = 108
  Width = 512
  object ssIn: TServerSocket
    Active = False
    Port = 5000
    ServerType = stNonBlocking
    OnClientConnect = ssInClientConnect
    OnClientDisconnect = ssInClientDisconnect
    OnClientRead = ssInClientRead
    OnClientError = ssInClientError
    Left = 40
    Top = 24
  end
  object csOut: TClientSocket
    Active = False
    ClientType = ctNonBlocking
    Port = 5000
    OnConnect = csOutConnect
    OnDisconnect = csOutDisconnect
    OnError = csOutError
    Left = 141
    Top = 26
  end
  object timerChannelOut: TTimer
    Enabled = False
    OnTimer = timerChannelOutTimer
    Left = 244
    Top = 23
  end
  object timerRingUp: TTimer
    Enabled = False
    OnTimer = timerRingUpTimer
    Left = 381
    Top = 23
  end
end
