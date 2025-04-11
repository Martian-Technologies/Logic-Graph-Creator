# Introduction 

Gatality comes with a set amount of default circuits that can be imported and modified as a base.  
Due to at the time of writing this documentation there is no way to label pins on parts of the circuit.  Therefore, this doc is written to guide users on what each switch input does for a circuitlib circuit.
This document will divide the circuits into different group by functionality and discuss each.

# Circuits

## Decoders

### Overview

These circuits have 2^n inputs where n in the number of outputs.  Each input corresponds to a particular number which is outputted as a binary number.
For example, in this 3-bit encoder, when the 5th input from the top is selected the output is 101 as shown below.

![image](https://github.com/user-attachments/assets/1824e925-7ad0-40b0-bc39-b624048dd2cf)

The default library comes with different sizes of decoders that users can import into their own circuit.

### 3-bit decoder.

The input to the below cirucit is 8 lines with P0 being the top line.  P0 corresponds to an input of 0 while P7 corresponds to an input of 7.
The output is the encoded binary number where Q0 is the least significant bit of the output and Q2 is the most significant output.

![image](https://github.com/user-attachments/assets/22fc5f9e-c6fd-4697-8edc-75f1cb5c3f0d)




