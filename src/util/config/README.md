This file is for workign in thsi directory

# multiTypeMap.h 

# config.toml
This file will hold all values that the user should be able to modify within particular parameters
`
[graphical.block_color]
AND = "0xff00ff"
`
Each line will be under a "subgrouping"
Each "subgroup" is defined after a "grouping"
** Groupings: ** graphical, input, ... 
If you do want to add a grouping, please consult me(Pojin) on gatality discord

The "grouping" is then seperated by a '.' from the subgrouping, which you'll define or add to previous subgrouping that makes sense
Under it, you can add an item that will be able to be changed by the user that will be in the format `name = "value"`
Integers and booleans should have ""
Each will link to some behavior you want to see graphical(please look at configGraphical.toml for more instructions on graphical look)

## Accessing Values
To access these values, it you'll `#include "util/config/config.toml"`
call `getConfig()` to get a returned object "MultiTypeMap" 
With this you can access elements with `.get("graphical.block_color.AND")` and set values with `.set("graphical.block_color.AND", "0x00ff00)`
As shown, each value is accessible by "group.subgroup.typename" which will return the respective value

** PLEASE REFERENCE NEXT HEADING WHENEVER YOU ADD AN ITEM TO THIS FILE **
  
# configGraphical.toml
Everything in this file is **only** for the use of ./gui/preferences/settingsWindow.cpp. **DO NOT INCLUDE OR USE THIS IN YOUR OWN FILES**
This file will have a **new entry** whenever you add something to **config.toml** or it will **crash the program**

## Formatting
`
// Example
[graphical.block_color]
AND = [ "color" ]
`
### WidgetType 
DROPDOWN, SLIDER, CHECKBOX, USERINPUT

### Key Types
These follow the value and will define how the user can define each setting 

