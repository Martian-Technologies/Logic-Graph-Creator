# Propagation Delay

## Introduction

Propagation delay (or gate delay) is the time it takes for the output of a gate to reflect its inputs.  
This is naturally built into Connection Machine when vizualizing how an input propagates throughout the circuit.  However, currently all delays are considered to be the same and does not take into account the addition of clocks.
Propation delay generally happens in digital circuits due to the "slew rate" for MOSFET's.  The most comment form of logic gates are made using CMOS technology.  Below we show the gate implementation of an OR gate.


![image](https://github.com/user-attachments/assets/5eb520fb-26a7-4a44-af62-56edc7655f3e)

Image from https://www.allaboutelectronics.org/cmos-logic-gates-explained/#google_vignette 

The slew rate of a FET (field effect transistor) is the rise/fall time for the transistor to react to a change in the input.

However, in real life gates and chips have different propagation delays.  Therefore it would be useful in the simulator to allow users to set the propagation delay.  Additionally, whatever implementation for propagation delay with need to account for clocks.

## Requirements

1. Gates need to have a way to set the propagation delay
2. Gate output should be equivalent to Operation(In1[t - t_prop], In2[t - t_prop]).
3. Not gates should be a valid gate so that users can create additional delays via gates.

## Benchmarking of existing software and room for improvement.

Logicworks gate delay seems to be buggy.  Having an intuitive way to set gate delay would make Connection Machine easier to use than logicworks.


## Benchmarking Tests with other software and edge cases

Edge Cases :

1. If the input changes BEFORE the propagation delay expires, then the output does not change.

Below in the following example we set a voltage clock to 0.5ns.  We also set the gate delay to 1ns.  As shown below, the output never goes above 0 when this is the case.  This is most likely due to FETs controlling the flow of electrons via a capacitace.  Most likely, the capacitor is not being charged enough to make the gate go to +5.  

![image](https://github.com/user-attachments/assets/cf0fa186-0994-4d64-be79-81fcd09feacc)

One interesting edge case is if we set the prop delay = 1/2 timer period.  As shown, the output goes up but never comes back down.  It is not known why this happens.

![image](https://github.com/user-attachments/assets/35d8bcdf-cd59-4356-952c-a8e940a0b43d)


Of course, if we set the t_prop < 1/2 f_timer then the testcase works as expected.

![image](https://github.com/user-attachments/assets/b832d18c-8f4c-4121-8cda-4f10e640afde)

2. Propogation delay CAN be used to create a clock with a XOR gate and does not break it.  (See below)

![image](https://github.com/user-attachments/assets/36b67c96-50ba-4324-b745-7b23f7cfb1ba)

3. This is an edge case when tested in LT spice.  Its kind of conusing but I think it should work

This test was t_prop = 0.8ns and a clock with 1s high and 2s low.  This is the output.

![image](https://github.com/user-attachments/assets/bc3c0908-2e9f-4c5a-ad5c-d63b8b5b703a)



4. Heres a fun Testcase too

![image](https://github.com/user-attachments/assets/49ea4515-098f-4679-a3d6-03f1cefdd498)

![image](https://github.com/user-attachments/assets/c8e14538-590b-4cad-8a89-b720707e3ab4)

