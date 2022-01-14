-- --------------
-- controls relays
-- --------------
function switchOn(relayNo)
  setRelay(relayNo,1)
end

function switchOff(relayNo)
  setRelay(relayNo,0)
end

function setRelay(relayNo, val)
  sendI2CW(0x5E,1,relayNo,val)
end

function setDigit(digit, val)
  sendI2CW(0x5E,2,digit,val)
end



