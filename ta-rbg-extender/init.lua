function setColorForRing(ring, color)
  sendI2C(0x60,3,ring)
  sendI2C(0x60,1,color)
end

function setBrightness(brightness)
  sendI2C(0x60,2,brightness)
end


