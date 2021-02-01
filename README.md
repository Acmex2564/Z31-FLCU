# Z31-FLCU
Reverse Engineering of the 300zx (Z31) Analog -> Digital Fuel Tank Gauge Converter Unit

This is a small, silver box located under the hatch of very late Z31s, or cars that have had the original, linear-slide Tank Gauge Sending Unit (TGSU) replaced with the late swing-arm style. 

Analog and digital cars used TGSUs with different resistance curves, to suit the needs of the gauge cluster. The linear-slide parts often demonstrated poor accuracy and high wear-out rates, and the swing arm parts are a commonly sought upgrade. However, Nissan did not produce the same two variants of the later part. Instead, all were produced with the resistance curve for the analog cluster, and if you ordered the replacement part for a digital car you recieved the analog swing-arm, with a small converter box that adjusted the levels to suit the digital cluster.

Now, 30 years later, these simple, hastily-produced boxes are both rare and often affected by corrosion. Their environmental seals are poor, and cases are made of steel in a car with notoriously poor weatherproofing. Perfect for rusting away!

This repository contains my attempt at determing the schematic and method of operation for this tiny circuit. Perhaps one day it will also contain replacement PCB details. 

The key players on the board are:

Zener diode for IC voltage supply (YUP, they removed the actual VR IC to save costs!)
Current limiting resistor to save the Zener
Filter and reservoir caps for the power supply side
Hitachi HA1813P Analog Voltage Comparator
Hitachi D789 NPN transistor
Second Zener diode limiting the voltage from the cluster on the fuel-level sub-gauge sense circuit

The actual function of this part is incredibly simple: at a main level reading of less than 3.6V (that is, more than approximately 1/4 tank) the sub level signal line is grounded, causing the cluster to disable the display. That's it.

Datasheets for the two Hitachi parts are located in the repository, because the datasheet for HA1813PS is difficult to find and I had already saved the other. 
Also found are images of the part in question, and it's wiring harness.
