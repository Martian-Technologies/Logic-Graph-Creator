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

1. An additional state for high-z and an "X state" must support where the output of a tri-state buffer is in high impedance mode and where the state cannot be defined respectively.

2. The output of a tri-state buffer is the "Z state" when the control input is low (see test for truth table and image).

3. The high-z state allows combining multiple inputs into 1 output where the output is dependant upon the non-high-z input.

4. If wires outputs are connected and collide ie. 1 gate outputs 0 while the other outputs a 1 results in the wire being in the X state.

5. Gates that take in multiple inputs with a Z or X input should attempt to resolve the output but otherwise should output the X state.  See testcases of the specific truth table on this.  Note this also applies to tri-state buffers.

## Test Cases

### The testcases below show a circuit with inputs and outputs and below show a truth table of the following values.

1. Tri-State Buffer Functionality.  NOTE* The buffer used in the diagram shows hi-z when the control is high but the truth table assumes hi-z is output for a low input.

| P | Q | Out |
|---|---|-----|
| 0 | 0 |  Z  |
| 0 | 1 |  0  |
| 0 | Z |  X  |
| 0 | X |  X  |
| 1 | 0 |  Z  |
| 1 | 1 |  1  |
| 1 | Z |  X  |
| 1 | X |  X  |
| Z | 0 |  Z  |
| Z | 1 |  X  |
| Z | Z |  X  |
| Z | X |  X  |
| X | 0 |  Z  |
| X | 1 |  X  |
| X | Z |  X  |
| X | X |  X  |


  
2. Multiple Inputs can be combined into 1

![image](https://github.com/user-attachments/assets/aa58d297-76b2-4d2a-9a9e-262df634a7fb)

| B0  | C0  | B1  | C1  | Out |
|-----|-----|-----|-----|-----|
|  0  |  0  |  0  |  0  |  Z  |
|  0  |  0  |  0  |  1  |  0  |
|  0  |  0  |  1  |  0  |  Z  |
|  0  |  0  |  1  |  1  |  1  |
|  0  |  1  |  0  |  0  |  0  |
|  0  |  1  |  0  |  1  |  0  |
|  0  |  1  |  1  |  0  |  0  |
|  0  |  1  |  1  |  1  |  X  |
|  1  |  0  |  0  |  0  |  Z  |
|  1  |  0  |  0  |  1  |  0  |
|  1  |  0  |  1  |  0  |  Z  |
|  1  |  0  |  1  |  1  |  1  |
|  1  |  1  |  0  |  0  |  1  |
|  1  |  1  |  0  |  1  |  X  |
|  1  |  1  |  1  |  0  |  1  |
|  1  |  1  |  1  |  1  |  1  |
  
3. And gate functionality works with new states

![image](https://github.com/user-attachments/assets/a3136ac5-b792-43ca-97b4-0d407ac5d9fd)

| P | Q | Out |
|---|---|-----|
| 0 | 0 |  0  |
| 0 | 1 |  0  |
| 0 | Z |  0  |
| 0 | X |  0  |
| 1 | 0 |  0  |
| 1 | 1 |  1  |
| 1 | Z |  X  |
| 1 | X |  X  |
| Z | 0 |  0  |
| Z | 1 |  X  |
| Z | Z |  X  |
| Z | X |  X  |
| X | 0 |  0  |
| X | 1 |  X  |
| X | Z |  X  |
| X | X |  X  |
   
4. Nor gate functionality works with new states

![image](https://github.com/user-attachments/assets/238cbda2-fffe-4cd6-9fae-468b89bbfe48)

| P | Q | Out |
|---|---|-----|
| 0 | 0 |  1  |
| 0 | 1 |  0  |
| 0 | Z |  X  |
| 0 | X |  X  |
| 1 | 0 |  0  |
| 1 | 1 |  0  |
| 1 | Z |  0  |
| 1 | X |  0  |
| Z | 0 |  X  |
| Z | 1 |  0  |
| Z | Z |  X  |
| Z | X |  X  |
| X | 0 |  X  |
| X | 1 |  0  |
| X | Z |  X  |
| X | X |  X  |
   
5. Xor gate works with new states

![image](https://github.com/user-attachments/assets/f71ae9dc-d246-4fc3-939f-789500cd95e9)

| P | Q | Out |
|---|---|-----|
| 0 | 0 |  0  |
| 0 | 1 |  1  |
| 0 | Z |  X  |
| 0 | X |  X  |
| 1 | 0 |  1  |
| 1 | 1 |  0  |
| 1 | Z |  X  |
| 1 | X |  X  |
| Z | 0 |  X  |
| Z | 1 |  X  |
| Z | Z |  X  |
| Z | X |  X  |
| X | 0 |  X  |
| X | 1 |  X  |
| X | Z |  X  |
| X | X |  X  |

## Extra Notes
