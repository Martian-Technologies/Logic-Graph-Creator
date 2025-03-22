# Clock Documentation 

## Introduction

A clock signal is a waveform generated my applying an electrical signal to a crystal where the output will be a square wave with a period of the clock frequency.  Clocks and extremely useful for electrical gate logic design since some logic components trigger a read / write based on a clock signal.  An example of what a clock
  signal looks like can be seen below.

![image](https://github.com/user-attachments/assets/2ceec936-a95e-422e-ba9f-2f7b6e8467d7)

In the clock signal above the jump from 0 to 5 is known as a "rising edge" while going form 5 to 0 is known as a "falling edge".  CPUs, ALUs, registers, and a lot of other hardware are triggered by either a rising or falling edge.
Most logic circuits use 0V for low and (3.3V or 5.0) for high.  In this example the frequency of the clock is 500Hz while the period is 2ms.

## Why the implementation of clocks is necessary.

1. Clocks drive all the operations on our computers.  To truly simulate gate logic we will need clock inputs to a circuit.

## Requirements

1. Clocks must be able to run at different frequencies.  This is because microcontrollers have tons of clocks all that run at different speeds.  Additionally, some clocks are used for driving things like GPIO pins.
2. Designed chips (like reigsters) must have functionality to change output on a clock edge.
3. Clocks should be able to be configured for pulse width - ie) high for 9us low for 1us.
4. A clock should be able to tick even when a signal is propagating through logic gates.  This will allow the user to see where a clock cycle interferes with propagation.

## Design Inferences

1. The way that we currently evaluate logic will not work since a clock tick could occur while the signal is still propagating.
2. Using a scheduling system may be the most accurate way to simulate -> may be too slow though.  Can we optimize for nodes that don't depend on each other?
