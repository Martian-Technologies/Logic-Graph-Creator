# Systems

## The Backend

The Backend is where `Circuit`s and `Evaluator`s are created, managed.

### BlockContainer
`BlockContainer` is a container used for storing and modifying the gird of `Cell`s and the related `Block`s.

When modifying a `Block` the cells for that block will get automatically modified.
When modifying connections both `Block`s at each end of connection will get modified.

**Cell**
The `Cell` class is used to store data at locations on the grid.
It currently only stores a `block_id_t` of its `Block`.

**Block**
The `Block` class is used to store all the information about a block including:
- `Type`
- `block_id_t`
- `Position`
- `Rotation`
- `ConnectionContainer`
- Simple data (rn its a int that can hold dif stuff)

**Connection Container**
The `ConnectionContainer` just holds and allows added and removing connections.
Each connection is a `connection_end_id_t` and `ConnectionEnd`.
A `connection_end_id_t` is local to a block and says which input or output the connection goes into.
A `ConnectionEnd` stores a `block_id_t` of the other block and a `connection_end_id_t` for which connection in that other block.

### Circuit
`Circuit` is a higher level `BlockContainer` ment to manage the modifications coming from other parts of the program.
When you tell `Circuit` to modify `BlockContainer` it will create a `Difference` that contain the effects that the modification had.
This `Difference` is then sent out to all registered functions. It is also added to the undo stack which allows undo and redoing.

### Evaluator
Evaluators `Evaluator` are used to simulate the circuit made of containers.

## Block Container View

### Renderers

Renderers are made to interface with both the windowing library and the backend. `CircuitView` takes a renderer as a template argument.

### Tools

Tools are mannaged by the tool mannager which deals with swapping and registering tools for a view. You can create tools by inheriting `CircuitTool`.

## QT

## Actions

TODO

## Settings

TODO
