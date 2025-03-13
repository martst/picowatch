Pico Watch

This software controls the BATC Pico Tuner and allows the video streams to be watched.

The initial aim of the software was to allow control of the LNB voltage. In my configuration the QO-100 receiving equipment is connected
by ethernet to my operating desk. The output of the LNB on the dish is split with power coming from the Picotuner. An SDR is connected
to the other output of the splitter allow reception of the narrow band signals. The DATV signals requier 18V on the LNB and the narrow band
require 12V. Using this software I can switch the power to 12V allow the SDR to receive the narrow band signals.

Having done the work to control the Picotuner it was not a lot of extra work to provide functionality to view the 2 video streams.

For the software to work a number of UDP ports must be open on the PC. 

Port 9997 is the destination port for the broadcast message from the Picotuner. This message is sent all the time from the Picotuner.

Port 9901 and 9902 are the destination ports for technical details about the video streams

Port 9941 and 9942 are the destination ports for the 2 video streams

All the above ports must be open for UDP traffic

This software allows the data received on all these streams to be viewed.
