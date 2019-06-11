wifi.sta.autoconnect(1)
soilHumi = adc.read(0)
sha = crypto.toHex(crypto.hash("sha256","secretWord")) --for testing, ensure secret word is set in both code and sql, need to improve this security test
print(sha)
print(soilHumi)
if(wifi.sta.getip()~=nil) then
    if(testtm["year"] ~= 1970) then
        curMins = 60 - testtm["min"]
        curSecs =  testtm["sec"]
        convMins = curMins*60*1000000
        convSecs = curSecs*1000000
        convTime = (convMins-convSecs)
        print("time =")
        print(convTime)
        http.post('https://192.168.0.221/garden/soilData.php',
            'Content-Type: application/x-www-form-urlencoded\r\n',
            'soilHumi='..soilHumi..'&'..'auth='..sha,
            function(code, data)
                if(code < 0) then
                    print("Request Failed")
                else
                    print(code, data)
                    print("Got Disconnection")
                    rtctime.dsleep(convTime,1)
                end 
        end)
    end
else
    print("Error: Not connected")
    dofile("init.lua")
end
