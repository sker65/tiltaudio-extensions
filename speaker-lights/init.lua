-- --------------
-- controls wheter or not the second ring does effects rotation reverse
-- --------------
function setReverseRotation(rotation)
  sendI2CW(0x60,2,rotation)
end

-- --------------
-- controls current effect, if cancel is set ti 0 the running effect will end first, then start next effect
-- if cancel set to 1 (also default), current effect is canceled
-- --------------
function setEffect(effectNumber, cancel)
  cancel = cancel or 1  -- cancel is default
  sendI2CW(0x60,1,cancel,effectNumber)
end

function playCallout(callout)
  sendI2CW(0x60,3,0,callout)
end

