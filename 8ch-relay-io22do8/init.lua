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

-- --------------
-- controls the 7 digit display
-- --------------
function setNumber(num, val)
  sendI2CW(0x5E,2,num,val)
end

function setDigitOnOff(num, val)
  sendI2CW(0x5E,3,num,val)
end


