# Settings
whenever you add something that you think should be controllable by the user, add it to util/config/config.toml

# config.toml
## Groupings
 - there will be a few major groups: graphics, input, 
 - everything should be a subclass of these groups notated by [graphics."$name"]

## Conventions
 - everything should be obvious on how you want it to be read, i.e. AND = and gates
 - everything will be an array: first value its default value(int, string, double, bool), and second should be what kind of type of way the value can be changed
   - the values can be changed 

## Access in Code Base
`
#include "util/config/multiTypeMap.h"
`
 - access settings elements by

