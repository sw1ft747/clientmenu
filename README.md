# Client Menu
This is a [SvenMod](https://github.com/sw1ft747/svenmod) plugin that lets you create and customize client-side menu.

Client menu has similar syntax of menu as in Left 4 Dead 2.

All client menus are loaded from main file `clientmenu.txt`. For example, check file `example.txt`.

To do: I will complete description

# How to open a client menu
When your client menu file is successfully loaded, you can bind the name of menu to a key.

Example:
```
bind i "cl_menu_show example_menu"
```

# Console Variables
ConVar | Default Value | Type | Description
--- | --- | --- | ---
cl_menu_duration | 10 | float | Duration of menu before it will autoclose, use value `-1` to disable it
cl_menu_fade_duration | 0.5 | float | Duration of fade when menu is closed
cl_menu_align_center | 1 | bool | Align menu to center of its tall
cl_menu_width_fraction | 0.0125 | float | The screen's fraction of width
cl_menu_height_fraction | 0.5 | float | The screen's fraction of height
cl_menu_title_color | "255 255 112 255" | color4 | Color of the menu's title
cl_menu_label_color | "255 255 255 255" | color4 | Color of the menu's label
cl_menu_label_number_color | "255 96 96 255" | color4 | Color of the menu's number of label

# Console Commands
ConCommand | Argument #1 | Description
--- | --- | ---
cl_menu_show | menu name | Show client menu by its name
cl_menu_reload | - | Reload all menus from main file `clientmenu.txt`
cl_menu_playsound | file name | Play a sound in 2D

# Config File
When the plugin is loaded, it will execute file `clientmenu.cfg` from folder `Sven Co-op/svencoop/`.

You can use that file to save console variables (colors, menu duration etc.).
