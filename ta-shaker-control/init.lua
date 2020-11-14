function shakerOn()
  sendI2CW(0x61,1)
end

function shakerOff()
  sendI2CW(0x61,2)
end

function shakerSpeed(speed)
  sendI2CW(0x61,4,speed)
end

function shakerSequence(startIdx)
  sendI2CW(0x61,5,startIdx)
end

