redLED = 5
gpio.mode(redLED,gpio.OUTPUT)
gpio.write(redLED, gpio.HIGH)
wifi.setmode(wifi.STATION) 
station_cfg={}
station_cfg.ssid="xxxx"
station_cfg.pwd="xxxx"
wifi.sta.config(station_cfg)
wifi.sta.connect()
print("Client IP Address:",wifi.sta.getip())
print("Looking for a connection")
counter=0
tmr.alarm(1, 10000, 1, function()
    sntp.sync("clock.via.net",
      function(sec, usec, server, info)
        print('sync', sec, usec, server)
        testtm = rtctime.epoch2cal(rtctime.get())
        counter=counter+1 
        print(string.format("%04d/%02d/%02d %02d:%02d:%02d", testtm["year"], testtm["mon"], testtm["day"], testtm["hour"], testtm["min"], testtm["sec"]))
        gpio.mode(redLED, gpio.LOW)
      end,
        function()
         print('failed!')
         gpio.mode(redLED, gpio.HIGH)
        end,
        0
      )
    print(rtctime.get())  
    if(wifi.sta.getip()~=nil and counter ~= 0 ) then
        tmr.stop(1)
        tmr.unregister(1)
        print("Connected!")
        print("Client IP Address:",wifi.sta.getip())
        dofile("clientSendDHT.lua")
    end
end)
