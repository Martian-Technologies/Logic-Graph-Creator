# multiTypeMap.h 
Generic file that currently holds 5 types:
- booleans
- int
- string
- color
- float

If you would like to update types with a unique one, change varient type or ask `@Pokun`

## Accessing Values
To access these values: `#include "util/config/config.toml"`

Get Values: `Settings::get<"type">(key)`
- when you get a value, you must provide a type, view 'config.cpp' for types, that is the same value that is returned
- key is a string defined as `group.subgroup.key_name`

Set Values: `Settings::set(group.subgroup.key_name, value)`
- when setting a value, the value must be of same type that its set as in config.toml 

# config.toml
This file will hold all values that the user should be able to modify within particular parameters, found in `resources/config.toml`
``` toml
[graphical.block_color]
AND = "0xff00ff"
```
Each line will be under a "grouping.subgrouping"

Each "subgroup" is defined after a "grouping"
- **Groupings:** graphical, appearance, keybinds (more to come)
- **Subgrouping:** these you'll define or append to previously created subgroupings, when adding please go to the next section for general instructions 

# config.cpp
This only matters if you want to add a setting value
- Depending on the grouping you add, go to `src/util/config/config.cpp` to add to the std::vector
- there's a vector for each group, choice one and add:
 - at the top of the file is a **FORM TYPE** area, add in the


  

## Formatting
``` toml
# Example
[general.general]
visual = "Dark"

[graphical.block_color]
AND = "0x111111" 
```
``` cpp
// requires #include "gui/circuitView/renderer/color.h" 
Color color_obj = Settings::get<Color>("graphical.block_color.AND");
// or
std::string visual_string = Settings::get<std::string>("general.general.visual");
```


