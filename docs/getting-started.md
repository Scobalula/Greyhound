---
layout: default
title: Getting Started
nav_order: 1
---

# Getting Started


Using Greyhound deviates from how other asset extractors work due to the complexity of the `fast file` format used across IW Engine titles, however it is still very easy once you get the hang of it. To use Greyhound in most titles, you must be in-game first and running a supported, legitimate copy of the title.

Some assets can be exported from their package files depending on the asset type and the title you are exporting from, for example XPAK, IPAK, IWD, and SAB files can be loaded directly by Greyhound to export supported assets that are stored within them.

Greyhound lists every asset it finds, if you cannot find the asset you want, you're probably not looking for the right name. A prime example of this are character and weapon models, where actor and code names are heavily used such as `mpapa7` for Call of Duty: Modern Warfare's MP7 and `bailey` for Call of Duty: Vanguard's Polina (actor name).

## Loading Assets In-Game

To load assets from `In-Game` you must first open the game and click `Load Game`. This will allow Greyhound to list all assets currently loaded at that point in the game.

Some assets such as weapons, attachments, and characters can be exported directly from the main menu depending on the title. Others such as map props or AI may require you to load into a map they are loaded in to export them.

Newer titles make heavy use of `Placeholders` for Models and Animations where a slot is kept for the asset but it is only fully loaded once the fast file is loaded. In this case you must prompt the game to load the asset by either hovering over the content within the menus or holding it in-game, and then clicking `Load Game` to refresh the asset list. You should then see it change to `Loaded` and it's details should be populated.

## Loading Assets From Files

As mentioned above, some assets can be exported from their package files depending on the asset type and the title you are exporting from. To do this, click `Load File` and select the file you wish to load, if it's supported and contains assets Greyhound supports, they will be listed in the asset list. You can also drag and drop files onto Greyhound's window.

Greyhound currently only supports loading 1 file at a time.