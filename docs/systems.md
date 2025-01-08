# Systems

## The Backend

### Containers
Containers `BlockContainer`/`BlockContainerWrapper` are used to store and modify the gird of blocks and the connections betweens the blocks.

### Evaluator
Evaluators `Evaluator` are used to simulate the circuit made of containers.

## Block Container View

### Renderers

Renderers are made to interface with both the windowing library and the backend. `BlockContainerView` takes a renderer as a template argument.

### Tools

Tools are mannaged by the tool mannager which deals with swapping and registering tools for a view. You can create tools by inheriting `BlockContainerTool`.

## QT

## Actions

TODO

## Settings

TODO
