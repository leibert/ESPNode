# ESPNode
For ESP8266 / NodeMCU hardware

Can be [herded](https://github.com/leibert/ESPHerder)

Multi-purpose light/logic controller. Has dimming, RGB, and binary control and simple input handling. 

A simple html file is sent via the http connection with a script tag referencing a remote location that can be easily updated if needed.
Once the UI script [(link)](https://github.com/leibert/ESPNode/tree/master/ESPNodeUI) is loaded, it will request the configuration from the ESP and subsequently load the needed channel control elements (switch/color dimming/etc)

##Examples of deployments:##
###RGB LED Flood lights###
[ESP Source](https://github.com/leibert/ESPNode/tree/master/deployed/backyard%20lights%20A/ESPNode/ESPNode.ino)
Uses cheap LED flood lights available from <a target="_blank" href="https://www.amazon.com/gp/product/B01DDJKSPQ/ref=as_li_tl?ie=UTF8&camp=1789&creative=9325&creativeASIN=B01DDJKSPQ&linkCode=as2&tag=cwmcc781-20&linkId=54aae4c8bf253f08c5875fe3f568b863">Amazon</a><img src="//ir-na.amazon-adsystem.com/e/ir?t=cwmcc781-20&l=am2&o=1&a=B01DDJKSPQ" width="1" height="1" border="0" alt="" style="border:none !important; margin:0px !important;" /> or AliExpress. The built-in IR based controller can be bypassed and MOSFETs inserted inline with each LED color channel.

 Most of the cheap LED lights tend to be common anode, so the LED element uses a common +VDC line (ranges from 12-30V) from the power supply and has three seperate wires, one for each color returning to the power supply. The return wires are connected within the power supply to a the IR-based controller. You can cut these, insert the MOSFETs inline and connect the low side of the MOSFET to ground. You can then drive the MOSFET, dimming with PWM, from the ESP.
 <p align="center">
<img src="http://images.cwm.eml.cc/IOSstuff/MOSFET%20Board3.jpg?variant=small" width = "20%" height="20%">
</p>

To save the hassle of building the MOSFET driver circuit, you can get nice multi-channel MOSFET boards that have optical isolation from <a target="_blank" href="https://www.amazon.com/gp/product/B01K7HZEO2/ref=as_li_tl?ie=UTF8&camp=1789&creative=9325&creativeASIN=B01K7HZEO2&linkCode=as2&tag=cwmcc781-20&linkId=bcfd2cce117a7bfe9cd43abd2713001b">Amazon</a><img src="//ir-na.amazon-adsystem.com/e/ir?t=cwmcc781-20&l=am2&o=1&a=B01K7HZEO2" width="1" height="1" border="0" alt="" style="border:none !important; margin:0px !important;" />, eBay, Ali, or Microcenter.



###RGB LED String light controller###
[ESP Source](https://github.com/leibert/ESPNode/blob/master/deployed/RGB%20test/ESPNode/ESPNode.ino)
Same as the flood light controller, but using LED string lights that can be bought from overseas. To make life easier, you can buy "Amplifiers" from ebay or <a target="_blank" href="https://www.amazon.com/gp/product/B00E4JQDKE/ref=as_li_tl?ie=UTF8&camp=1789&creative=9325&creativeASIN=B00E4JQDKE&linkCode=as2&tag=cwmcc781-20&linkId=ce106cace28d18246b059e2eaf14c408">Amazon</a><img src="//ir-na.amazon-adsystem.com/e/ir?t=cwmcc781-20&l=am2&o=1&a=B00E4JQDKE" width="1" height="1" border="0" alt="" style="border:none !important; margin:0px !important;" />. Just connect a PWM output from the ESP to the input of the "Amplifier", connect a 12V power supply, and the LED string light to the output. All of these LED strings, repeaters, and controllers from China tend to use the same connectors and pinout, so you can be up and running without even soldering anything.
 <p align="center">
<img src="http://images.cwm.eml.cc/IOSstuff/IMG_20161108_120950782_BURST000_COVER_TOP.jpg?variant=small" width = "20%" height="20%">
<img src="http://images.cwm.eml.cc/IOSstuff/IMG_20161108_120943637.jpg?variant=small" width = "20%" height="20%">
<br>
###**[Video of both LED strips and LED flood in action](https://goo.gl/photos/xwagtu2RMbbJqEAZA)**###
</p>




###Bus-o-tron###
[ESP Source](https://github.com/leibert/ESPNode/blob/master/deployed/Busotron/ESPNode/ESPNode.ino)
Since the LED sign with the MBTA bus times I build couldn't be seen from our second unit, and since Google Maps and CityMapper are way too convienient to use to find out when the next bus is coming. I built a device with a bunch of blinking lights and a nearly indecipherable UI.

<p align="center">
<img src="http://images.cwm.eml.cc/IOSstuff/IMG_20170111_091142136_BURST000_COVER.jpg?variant=small" width = "20%" height="20%">
</p>

There are 3 bus routes nearby. The center indicators let you know which route is being displayed, with the arrows indicating outbound/inbound. The blink speed of the chevron indicates how close the next bus on the route/direction is getting.

###**[Video of Bus-o-tron in action](https://goo.gl/photos/zDBsrZtRYsdp2fhJ7)**###


This uses the framework of the ESPNode code, and can actually be controlled via the web UI and herded. If it has not recieved a command, it will automatically revert to running the bus tracking code. The bus times are provided by the (datacollectionbot)[https://github.com/leibert/DataCollectionBot] which pulls information every minute from the MBTA / NextBus API and stores it in a text file. The Bus-o-tron (and other devices in the house) reads the text file (stored on a raspberry pi) every 30 seconds. This minimizes the number of hits to the MBTA API from our IP address.









