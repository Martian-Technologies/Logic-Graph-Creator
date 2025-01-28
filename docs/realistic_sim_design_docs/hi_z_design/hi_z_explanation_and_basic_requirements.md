# High Z Documentation 

## Introduction

## High-Z Explanation

### What is High Z

In logic circuits, states refer to the binary conditions that a siganl can be in, where on and off is usually represented in binary as 1 and 0 respectively.

"High Z" in particular refers to a high impedance state - where the output is neither a high (1) nor low (0). This would cause no signal to be driven, essentially acting like an open circuit.

This state is typically associated with a "tri-state logic" where a circuit can output a logical high, a logical low, or a high impedance state.

Impedance in electrical circuits is basically the resistance except it also takes into account frequencies.  For our purpose though we can essentially treat high impedance as an area of the wire with an extremely high resistance.  The output being high resistance allows another output on the line to take control of it.  For example take the circuit below.  In this circuit treat P and Q as inputs onto the line and R as the output.  However, the output resistance of Q is really high meaning that the output of R will not be effected by the output of Q.

![image](https://github.com/user-attachments/assets/95d536a3-ebd9-493a-b333-f2542550908d)


We can show this through simulation by plotting P, Q, and R in the graph below showing that R is unaffected by Q.  In this case, the ouput of Q is essentially High-Z.

![image](https://github.com/user-attachments/assets/754d1f60-05ec-4162-9e66-1da8f8ba43d7)

### Why does High Z happen

Current flows down path with least resistance. Since High Z occurs when a circuit or component has a very high resistance to the flow of electrical current, it essentially acting like an open circuit. 

In the case of logic circuit simulations, the state would occur in conjunction with tri-state buffers. 

### Effects of High Z on a complex circuit

### Application of High Z

High Z states are particularly useful in situations where multiple devices need to communicate on a shared bus (a shared physical pathway between electronic components). By setting their outputs to high Z when not actively transmitting data, devices avoid interfering with each other, allow other output 


## Requirements

## Test Cases

## Extra Notes
