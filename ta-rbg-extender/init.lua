function setColorForRing(ring, color)
  sendI2CW(0x60,ring,color)
end

function setBrightness(brightness)
  sendI2CW(0x60,2,brightness)
end

function allOff() {
  sendI2CW(0x60,3,0)
}


