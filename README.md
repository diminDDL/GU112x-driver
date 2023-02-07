## An RP2040 driver for the GU112x16G-7000 display. 

This repostitory demonstrates how one can stream real time video to a GU112X16G-7000.
The RP2040 firmware implements the most important methods for the above display.
The IO is handled by PIO, sending it a byte will output it on the parallel port and wait until the BUSY signal goes down.
This is then wrapped into a simple library with methods to interact with the display.

The data is streamed by using a simple byte stream over USB CDC. This is quite unstable and whenever there is USB activity this thing stops working correctly.
It can be improved quite a lot but this is just a proof of concept.

The desktop application is built in rust. It takes a screenshot, goes through each Nth pixel, averages it and finally compares it to a threshold.
This forms the framebuffer that is later sent over USB.
