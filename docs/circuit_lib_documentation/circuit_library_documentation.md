# Introduction 

Connection Machine comes with a set amount of default circuits that can be imported and modified as a base.  
Due to at the time of writing this documentation there is no way to label pins on parts of the circuit.  Therefore, this doc is written to guide users on what each switch input does for a circuitlib circuit.
This document will divide the circuits into different group by functionality and discuss each.

# Circuits

## Encoders

### Overview

These circuits have 2^n inputs where n in the number of outputs.  Each input corresponds to a particular number which is outputted as a binary number.
For example, in this 3-bit encoder, when the 5th input from the top is selected the output is 101 as shown below.

![image](https://github.com/user-attachments/assets/1824e925-7ad0-40b0-bc39-b624048dd2cf)

The default library comes with different sizes of encoders that users can import into their own circuit.

### 3-bit decoder.

The input to the below cirucit is 8 lines with P0 being the top line.  P0 corresponds to an input of 0 while P7 corresponds to an input of 7.
The output is the encoded binary number where Q0 is the least significant bit of the output and Q2 is the most significant output.

![image](https://github.com/user-attachments/assets/22fc5f9e-c6fd-4697-8edc-75f1cb5c3f0d)

### 4-bit decoder

Similar to the 3-bit encoder this circuit has it least significant bit input at the top as P0 and the most significant as P15.  This encoder encodes 4 bits meaning that if for exampke, P15 is enabled, then Q0-Q3 will be on.

![image](https://github.com/user-attachments/assets/ee457bc4-3b98-4100-83ab-3cf2787a7bfa)

### Decoders

Decoders are the inverse of encoders meaning this type of circuit block takes in a number in binary and drives one out of 2^n outputs high where n is the number of input bits.  The default library also comes with different types of encoders depending on the size of input/output.

### 3-bit decoder

This decoder has 3 inputs, P0-P2 and 8 outputs, Q0-Q7.  The binary representation of the input is the output line that is driven high.  For example if (P2, P1, P0) = (HIGH, LOW, HIGH) then Q5 will be driven high for the output.  An example of the circuit diagram is shown below

![image](https://github.com/user-attachments/assets/ccbe977a-d2d5-481b-957c-edc4acc6eb31)

### 4-bit decoder

This decoder has a 4-bit input and continues the pattern the top input being the least significant bit for the binary number represented as the input.

![image](https://github.com/user-attachments/assets/e0d67b2f-9872-40a4-af12-8da3c518efbf)


## Multiplexors

### Overview

A multiplexor is a gate that takes in two inputs.  The first input is the set of inputs to select from (on the left side of the MUX).  The second input is the select input which is a input for a binary number of which input to select.  The output is whichever whichever line was selected as the input.  So for example in this when S0 = 0, the output of the MUX is equal to P0 which in this case in low.

### 2:1 MUX

For the 2:1 mux the top input corresponds to being selected when S0 = 0 and the bottom bit corresponds to being the output when S1 = 1.

![image](https://github.com/user-attachments/assets/d6f451a1-d4b7-4883-a5be-f10da07c9999)

16:1 MUX

This uses the same principle as the 2:1 MUX expect there are 16 inputs and 4 select lines to select 1 of the 16 inputs to be the output.  S0 of course is the least significant bit for the select lines while S3 is the most significant.

![image](https://github.com/user-attachments/assets/b1731128-71bc-4c68-97c8-62df1cf4d72f)

## Adders

### Overview

Adders are circuits which take in two inputs of the same size and add the numbers represented in binary by the inputs.  Generally there is one more output than the number of bits in one number that can be added in order to account for overflow.

### 4-bit Adder

This adder takes in 2 four bit inputs where the numbers are are inputted in binary representation as (P3 ... P0) + (Q3 ... Q0).  The output (R4 ... R0) is the sum of these two numbers.  As stated R4 is the overflow bit of the sum if one occurs.  In this case the number represented by (P3 .. P0) is (0110) = 6 and the number represented by (Q3 .. Q0) is (0010) = 2.  As seen in the output R3 is high implying the output of (01000) = 8.  Therefore it is shown that the addition is correct.

![image](https://github.com/user-attachments/assets/d17d16cb-c90e-46b7-abf9-3a7cf446dd4e)

### 8-bit Adder

This is the same as the 4-bit adder except each input is a whole byte and the output is a byte plus an overflow bit.  Notice how in this example the sum of both inputs is 9 bits long and therefore the overflow bit is used.

![image](https://github.com/user-attachments/assets/c669b8fd-a775-4fda-9eca-7bd56fa2f6c4)


### Full-word adder

A lot of computers either use 32-bit or 64-bit hardware so a 32-bit adder is extremely useful to have for computer architecture.
One key thing to note is that the two input binary numbers (P31 ... P0) and (Q31 ... Q0) are parallel to each other to save vertical room.  The output (R32 ... R0) is still on the right side though.

![image](https://github.com/user-attachments/assets/bb60f85d-e107-47d7-9070-05359cc90330)

## Reigsters

### Overview

Registers are the fastest type of "memory" on a computer.  Obviously it isnt true memory like SRAM or DRAM but it does store information and replaces it upon clock tics.  Registers are the direct piece of hardware used for feeding information into components such as an ALU because of how fast they are.  Additionally, registers are used for microcontroller peripherals such as for the use of a timer.

### Basic Register

This register only has CLR functionality and loads on a clock ege.  On a rising clock edge the reigster will load in whatever the input is and return it as the output.  This is useful for for computer since it allows for operations to be performed on a clock edge.  One thing to watch out for is ensuring that the clock edge does not trigger too quickly or else unknown behavior may occur.

#### Pin Usage

CLK - A RISING edge (Low -> High) triggers the register to read in the current input and hold as its output until the next clock cycle.
CLR - When input is high the register clears its current output to 0s.  This is useful on startup for ensuring registers do not contain bad data.
(P0 - P7) - The byte to load into the register.  P0 is the least significant bit and P7 is the most significant.
(Q0 - Q7) - This is the output byte of the register.  Note that Q0 becomes the state of P0 on a rising edge while Qn becomes the state of Pn on the clock egde.

#### Circuit Implementation:

![image](https://github.com/user-attachments/assets/438246fe-f033-45a6-a170-f115df1a97af)



#### Pinout on IC:

![image](https://github.com/user-attachments/assets/e4b62f58-4d1d-4dda-ab32-7effe3ad73c0)



### Shift Register

This compoenent is a special register which acts like a normal register loading in data on a clock pulse but also has the ability to shift the output bits of the register.  This application is useful for things like counting or for serial communication hardware such as UART which transmits one bit at a time between microcontrollers.  This chip has a variety of inputs so a list will be given below of what each input does.

#### Pin Usage

CLK - On rising or falling edge triggers either a load, shift or nothing depending on S0 and S1.
EN - Must be high in order for the register to work correctly
(P0 - P7) - The byte to load in when (S1, S0) = (11) and a clock edge occurs
(S1, S0) - Selects the action of the register when a clock edge occurs : {00, 01, 10, 11} -> {HOLD, RIGHT SHIFT, LEFT SHIFT, LOAD}.  Note that right shifts (R7 .. R1) into bits (R6 .. R0).
LI - Set this input to high if a 1 should replace R0 on a left shift.
RI - Set this input to high if a 1 should replace R7 on a right shift.
(R7 .. R0) - This is the registers output where R7 is the most significant bit of the output.

NOTE* : if (P7 .. P0) = (00111010) and (S1, S0) = (11) when a clock edge triggers, then (R7 .. R0) -> (00111010)

![image](https://github.com/user-attachments/assets/2ee9653f-c27f-46b2-a01e-ef3fdb835831)


## ALU's

### Overview

ALUs are ICs that take in an input and perform bitwise calculations to generate an output.  ALUs also take in an additional byte to perform mathematical operations.  The different "Select Pins" on the ALU select which operation is performed.  These operations can vary from bitwise logical operations, addition, subtraction, multiplication and others.

### Basic ALU (8-bit)

This is the default ALU.  ** Note : This IC is still a work in progress so additional operations should be added to the ALU.  When operations are added this documentation should be updated! **.  This default ALU currently supports addition, bitwise comparion, bitwise OR, and a move operation.  The operations listed in the table below depend the combination of select pins are enabled.

#### Pin Usage

| Pin Number | Explanation |
| ---------- | ----------- |
| (P0 - P7)  | This is the first byte of the input to the ALU with P0 being the least significant bit input and P7 being the most significant bit |
| (Q0 - Q7)  | This is the second byte of input with Q0 again being the least significant bit |
| (R0 - R7)  | This is the output byte of the ALU with the operations performed on (P0 - P7) and (Q0 - Q7). |
| (S0 - S3)  | The select bits for which operation is selected.  Each operation is listed in the table below |

#### Operation Opcodes

The ALU takes in what are known as opcodes through the select bits.  These opcodes decide which operation is output by the ALU.

A table outlining which opecode performs which operation is outlined in the table below.  When these opcodes are updated these new opcodes should be updated here in the library.

| Opcode Number | Opcode bits (S3 - S0) | Operation | Explanation |
| ------------- | ------------- | ------------- | ------------- |
|  0  | (0000) | Addtion | This performs the operation (R7 - R0) = (P7 - P0) + (Q7 - Q0) where (R7 - R0) is the byte represented by bits (R7 - R0) with R7 being the most significant bit and R0 being the last.
| 1 | (0001) | Bitewise Equals | This performs the operation (R7 - R0) = (P7 - P0) == (Q7 - Q0) where (R7 - R0) is the byte represented by bits (R7 - R0) with R7 being the most significant bit and R0 being the last.  The == sign performs a bitwise (NXOR) or comparison.  For exampke, is (P7 - P0) = (00011100) and (Q7 - Q0) = (01001000) then the output (R7 - R0) will be (10101011).  This is useful to check if bytes are equal since if bytes are equal then the result will be all 1s.
| 2 | (0010) | Move | This simply sets (R7 - R0) to the input (P7 - P0)
| 3 | (0011) | Bitwise AND | This performs the operation (R7 - R0) = (P7 - P0) & (Q7 - Q0) where (R7 - R0) is the byte represented by bits (R7 - R0) with R7 being the most significant bit and R0 being the last.  This is useful for bitmasking registers within a PC.  An example of this is (P7 - P0) = (11100011) and (Q7 - Q0) = (10011001) results in (R7 - R0) = (10000001).

#### Circuit Implementation

In this image the addition operation is utilized.  As is seen the input (P7 - P0) = (0001 0100) = (20) and (Q7 - Q0) = (0001 1000) = (24).  Therefore the output should be (20 + 24 = 44) = (0010 1100) which as seen in the figure provided.

![image](https://github.com/user-attachments/assets/a8100516-a115-45aa-9041-04bbd4287033)

#### IC Documentation

This is the circuit in IC form with the pinout shown below.

![image](https://github.com/user-attachments/assets/02174750-36a4-4397-b378-072f85c8ae1a)


## Serial Communication ICs

## Serial Communication Overview

Serial communication is a method is which bits are transferring between ICs in a serial manner.  Transmitting serially means that only 1 bit is transmitted at a time.  Having serial communication is useful since it reduces the number of wires that need to be drawn between ICs in order to communicate information.  There are many types of serial communication, and people can make their own but some are common enough where specific microcontrollers implement the their ability in hardware.  Implementing serial communication through hardware is useful since it means the CPU does not need to worry about sending individual bits and only has to set specific registers in order to begin communication.  Below we list out some hardware components provided in circuitlib.

## UART

UART is an asynchronous serial communication protocol.  This means that both ICs do not share a common clock in order to communicate.  However, the clocks must share a common frequency knwon as a "baud rate".  UART occurs between devices via RX and TX pins where one ICs output is the other ICs input.  Below is a resource discussing UART.  There is an addition to this protocol known as "USART" (Universal Synchronous-Asynchronous Receive-Transmit).  Microcontrollers with this module can transmit (TX) and receive (RX) at the same time.

https://www.circuitbasics.com/basics-uart-communication/

## UART TX Circuit

### Overview

This circuit loads in a byte from input and serially transmits it.  One bit is transmitted with each clock cycle.  This is made specifically for the transmit side of UART.  This IC also comes with a send pin that when driven high sends the data.

### Pin Usage

| Pin Number | Explanation |
| ---------- | ----------- |
| (P0 - P6)  | The data to transmit when the SEND input is given a RISING edge.  The bits are send P0 - P6 with P0 being sent first.  See "Errata" section for this section for why there are only 6 bit |
| Send  | Begin sending the data when this input is driven high.  This input must be driven low again before sending |
| TX  | The transmit bit of what is actually transmitting the bits. |

### Circuit Implementation

This is the circuit implementation of the UART hardware module programmed through logic gates.
![image](https://github.com/user-attachments/assets/24f44b51-7e54-4320-9bca-3e7b4c204c69)


### IC Documentation

Here is the picture of the IC and its pinout.

![image](https://github.com/user-attachments/assets/e4dc7cfa-51cd-4a51-b78c-5c1bf49c652f)

### Errata

This only sends 7 bits instead of 8 because UART requires a start bit transmitted at the beginning to notify the other IC to start clocking data into the register.  We only have a default 8 bit shift register but this would ideally use a 9 or 11 bit shift register to handle start/stop bits.

## UART RX Hardware

This hardware loads in communication data starting for the start bit the TX circuit sends.  A start bit must be sent to start clocking the data in.

### Pin Usage

| Pin Number | Explanation |
| ---------- | ----------- |
| (Q0 - Q6)  | The data received byte via UART.  The first bit recieved is put in Q0 while the last received in put in Q6.  See "Errata" section for this section for why there are only 6 bit |
| RX_Start  | This should be hooked up to the TX pin of the UART TX IC and indicates when the message is started being received |
| RX  | The bits to receive in a serial fashion. |

### Circuit Implementation
![image](https://github.com/user-attachments/assets/f6016f0e-0047-4356-a4b1-d309e10322da)

### IC Doucmentation

![image](https://github.com/user-attachments/assets/c9b57bed-905e-496a-bb90-5bb0b9ff508f)

### Errata

Again, this RX is only 7 bits instead of 8 to make room for the start bit.


















