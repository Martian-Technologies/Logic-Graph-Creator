# Introduction 

Gatality comes with a set amount of default circuits that can be imported and modified as a base.  
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



