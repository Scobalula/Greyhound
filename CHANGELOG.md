## Greyhound 1.3.30.0

### CoD 4, 5, MW2, MW3 Loaded Sounds

Support for Loaded Sounds from CoD 4, 5, MW2, and MW3

### Black Ops 4 Update

Update for Call of Duty: Black Ops 4 1.15 Update

## Greyhound 1.3.20.0

### Black Ops 4 Update

Update for Call of Duty: Black Ops 4 1.15 Update

## Greyhound 1.3.10.0

### AW, MWR, Ghosts Primed Audio + Fixes

* Added support for Primed Audio for AW, MWR, and Ghosts (Streamed audio is also supported for Ghosts now)
* Added localization folder names/prefixes for all known regions
* Fixed some streamed audio not showing up

## Greyhound 1.3.0.0

### Images File Updates + Black Ops 4 Update

* Update for Call of Duty: Black Ops 4 1.14 Update
* Added static props names from [Planet](https://github.com/Scobalula/GreyhoundPackageIndex/issues/4#issue-421557868)
* New additions to the Material Images files, with semantic information written now. I have generated a hash table 
from Black Ops 3's Techsets from the Mod Tools and so for Black Ops 3 all these slots should be valid, for Black Ops 4 majority should have names, with some resolving to unk_semantic_(hash), other games should have base images with the same resolving as Black Ops 4 in cases of unidentified ones. ![Example](https://i.imgur.com/GjYbVbN.png)

## Greyhound 1.2.71.0

### Material Images File Fix + Asset Hashes

* Fixed an issue with the material images files showing invalid text (they will be updated again to add more information in the next update)
* Added potential static model names, image names from Ethan, and some misc. models from Faithfullfaun

## Greyhound 1.2.70.0

### New Features + Black Ops 4 Update

* Support for the latest Call of Duty: Black Ops 4 Update
* Fixed issues in the previous update with some strings
* Added options to skip assets that are already exported (Images/Sounds already done this, but it can now be toggled for XModel, XAnim, Sound, and Image assets)

## Greyhound 1.2.62.0

### Black Ops 4 String Fixes

* Added encryption methods for missing keys
* Fixed a slight memory leak in the Black Ops 4 string reader

There's a huge possibility that updates will continue for Bo4, thanks to recent events and new research. 

Also to clarify in any case: Both the prev. update and my twitter post explicitly stated Black Ops 4, I have no plans to abandon Greyhound.
## Greyhound 1.2.60.0

### Final Update for Black Ops 4

* Final update for Call of Duty: Black Ops 4 - Some strings will resolve to their hash due to the encryption
## Greyhound 1.2.50.0

### Black Ops 4 Update

* Support for the latest Call of Duty: Black Ops 4 Update 
## Greyhound 1.2.46.0

### Maya Max Influences + Ghosts Fix

* Reduced Max Influences on Maya Bind Files to 15 (this won't affect exported binds as they are already way under 15, but may have affected modifications to binds, and in some cases where Maya normalizes the weights)
* Fixed an issue with some Ghosts SP levels returning Unsupported Game.
## Greyhound 1.2.45.0

### Bo1 Raw Files + Hashes for Bo4

* Added support for Call of Duty: Black Ops Raw Files 
* Fixed an issue with MWR/AW Loaded Audio being skipped
* Added more static model hashes for Call of Duty: Black Ops 4

I haven't had time to update my hash table generator, so for DLC1 player models here they are: https://pastebin.com/iGKjGcn9

Enjoy!

## Greyhound 1.2.40.0

### Streamed Audio from MWR/AW

* Added support for streamed audio from MWR/AW.
## Greyhound 1.2.33.0

### Black Ops 4 Update

* Support for the Call of Duty: Black Ops 4 Update
## Greyhound 1.2.32.0

### Remove XPAK Checks

* Removed XPAK Check (bigger XPAKs should be fine, smaller localized ones were throwing it off since some are blank so from now on it just skips them)
## Greyhound 1.2.31.0

### Hashes + XPAK Index

* Added officially named icons from EthanC for prestige, ranks, etc. (check Initial.xpak)
* Added officially named XAnim and XModels for a lot of weapons from JerriGaming
* XImage Package Index now works with loading an XPak
## Greyhound 1.2.30.0

### Black Ops 4 Update

* Support for the latest Black Ops 4 Update
## Greyhound 1.2.21.0

### Zone Folder Fix + XPAK Offset Checks

* Fixed a small issue with Bo4 zone folder not being found when in Greyhound's folder.
* Added checks for XPAK offsets, if a Bo4 XPAK is bad, it'll tell you which one.
## Greyhound 1.2.20.0

### BO4 Material & Name Caches for XModel Exporting + More Hashes

* Name caches now work for xmaterial and ximages when exporting xmodels (bo4_xmaterial.wni and bo4_ximage.wni)
* Add hashes for most character models and icons (Some "torso" and "arm" character models might be combined in either one, mostly for zombie characters)
* Streamed images can now be exported via in-game (Bo4)
## Greyhound 1.2.13.0

### More weight fixes for Bo4 Models

* Fixed weights for some more models, tested quite a few, all should be working now, but do report issues!
## Greyhound 1.2.12.0

### Bo4 Vertex Weights

* Fixed some models with incorrect vertex weights (Heads not being binded to j_head, etc)
## Greyhound 1.2.11.0

### CoD 4/5 Issues

* Fixed issues with Load Game returning Unsupported Game on some CoD 4/5 Levels
## Greyhound 1.2.1.0

### Scan Fixes + Bo4 Update

* Fixed potential issues with heuristic scans
* Updated BO4 Memory Addresses
* Added check for zone folder in Greyhound's folder for Bo4 (if you're tight on space on your game drive, you can now dump the zone folder to Greyhound's folder, it'll check for zone in Bo4's folder first)
## Greyhound 1.2.0.0

### Notetrack Fixes, Model Names, Asset Sorting

* Added Asset Detail sorting (Sort by bones for XModels, by frames for XAnims, etc.) (Check settings)
* Fixed notetracks for Black Ops 4 Animations
* Added a check for if the zone folder exists for Black Ops 4.
* Generated hash dictionaries for more Black Ops 4 assets (Weapon Models and Anims) and AU anims are prefixed with vm_auanim so that they will export correctly.
## Greyhound 1.1.11.0

### Some XAnim and XModel Names

* Generated some XAnim and XModel Names (Mostly prop models and AI animations, most maps should have prop model names now)
## Greyhound 1.1.1.0

### BO4 Corrupt Normals + UVs 

* Fixed corrupt UVs and Normals on some models with extended vertex information.
* Fixed Updater throwing corrupt error (requires re-download from Repo)

## Greyhound 1.1.0.0

### Call of Duty: Black Ops 4

* Support for Call of Duty: Black Ops 4
* External Updater (so you don't need to track when we break things)

Keep an eye on the repo as we'll be introducing more tools and package indexes to aid in working with content exported from Bo4. 

Please note you must run the [T8ZoneRipper](https://github.com/Scobalula/T8ZoneRipper) (and on each update) to extract the XPAK files as we require them.
## Greyhound 1.0.1.0

### PAK Images not being exported

* Fixed PAK Images not being exported for MWR, AW, and Ghosts.
* Fixed Wiki Link.
## Greyhound 1.0.0.0

### Initial Release

Initial Release, updates from original:

* Support for latest WW2 Patch (An update should no longer be required for WW2 Game Updates)
* Support for Vertex Colors (some games/models zero them out, use them for foliage, etc. exports)
* Support for Loaded Sounds from: Ghosts, AW, and MWR
* Material Image Name Exporting (Useful for Global Exports, or for weird image names)

