# Propagation Delay

## Introduction

Propagation delay (or gate delay) is the time it takes for the output of a gate to reflect its inputs.  
This is naturally built into gatality when vizualizing how an input propagates throughout the circuit.  However, currently all delays are considered to be the same and does not take into account the addition of clocks.
Propation delay generally happens in digital circuits due to the "slew rate" for MOSFET's.  The most comment form of logic gates are made using CMOS technology.  Below we show the gate implementation of an OR gate.


![image](https://github.com/user-attachments/assets/5eb520fb-26a7-4a44-af62-56edc7655f3e)

Image from https://www.allaboutelectronics.org/cmos-logic-gates-explained/#google_vignette 

The slew rate of a FET (field effect transistor) is the rise/fall time for the transistor to react to a change in the input.

However, in real life gates and chips have different propagation delays.  Therefore it would be useful in the simulator to allow users to set the propagation delay.  Additionally, whatever implementation for propagation delay with need to account for clocks.

## Requirements

1. Gates need to have a way to set the propagation delay
2. Gates should have a time the input needs to be held in order for the output to change.
3. When a gate's hold time is not met it should output the X state discussed in the Hi-Z design document.

## Benchmarking of existing software and room for improvement.

1. Logicworks does not have the ability to account for hold time of a gate.  Having this in Gatality would be a huge plus.
