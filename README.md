# Greyhound - A fork of Wraith Archon, Call of Duty Asset Extractor
[![Releases](https://img.shields.io/github/downloads/Scobalula/Greyhound/total.svg)](https://github.com/Scobalula/Greyhound/releases) [![License](https://img.shields.io/github/license/Scobalula/Greyhound.svg)](https://github.com/Scobalula/Greyhound/blob/master/LICENSE) [![Discord](https://img.shields.io/badge/chat-Discord-blue.svg)](https://discord.gg/RyqyThu)

Greyhound is an asset extractor various titles running on the IW Engine (primarily Call of Duty games), and is based off the famous [Wraith Archon](https://github.com/dtzxporter/WraithXArchon/) that was originally developed by DTZxPorter. Greyhound's aim is to provide people access to assets from the games for various purposes including working mods such custom zombie maps or weapon mods, 3D Art, and thumbnails for content creation.

For detailed information such as game support, settings, FAQs, etc. please refer to the wiki: [Wiki](https://scobalula.github.io/Greyhound/) Work is being done to populate the wiki with information and tutorials so stay tuned!

For more support you can hop into our Discord: [https://discord.gg/RyqyThu](https://discord.gg/RyqyThu)

## Requirements

* Windows 10 x64 or above (Windows 7/8/8.1 should work, but are untested)
* Microsoft Visual Studio 2017 Runtime ([x86](https://aka.ms/vs/16/release/vc_redist.x86.exe) and [x64](https://aka.ms/vs/16/release/vc_redist.x64.exe))
* Official copies of the games (only the latest copies from official distributors are tested)

The following tools/plugins are required/recommended for some assets/games:

* [SETools](https://github.com/dtzxporter/SETools) by DTZxPorter (.seanim & .semodel) (Autodesk Maya)
* [io_anim_seanim](https://github.com/SE2Dev/io_anim_seanim) by SE2Dev (.seanim) (Blender)
* [io_model_semodel](https://github.com/dtzxporter/io_model_semodel) by DTZxPorter (.semodel) (Blender)
* [FileTypeDDS](https://github.com/dtzxporter/FileTypeDDS) by DTZxPorter (support in Paint .NET for newer DXGI formats) (Paint .NET)
* [Intel TextureWorks](https://software.intel.com/en-us/articles/intel-texture-works-plugin) by Intel (DDS + Utils) (Photoshop)
* [DarkIris](https://aviacreations.com/modme/index.php?view=topic&tid=831) by DTZxPorter (Texture Utils) (Paint .NET)

## Links:
* Discord Server: [https://discord.gg/RyqyThu](https://discord.gg/RyqyThu)
* Github Repo: [https://github.com/Scobalula/Greyhound](https://github.com/Scobalula/Greyhound)
* Change Log: [https://github.com/Scobalula/Greyhound/blob/master/CHANGELOG.md](https://github.com/Scobalula/Greyhound/blob/master/CHANGELOG.md)
* Latest Release: [ttps://github.com/Scobalula/Greyhound/releases](https://github.com/Scobalula/Greyhound/releases)

## License/Disclaimer

Greyhound, like Wraith Archon and the WraithX Library, is licensed under the General Public License 3.0, you are free to use Greyhound, both it and its source code, under the terms of the GPL. Greyhound is distributed in the hope it will be useful to, but it comes WITHOUT ANY WARRANTY, without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, see the [LICENSE](https://github.com/Scobalula/Greyhound/blob/master/LICENSE) file for more information.

This repo is in no shape or form associated (apart from being a fork of course) with DTZxPorter or Activision and the developers. These tools are developed to allow users access to assets for use in other Call of Duty SDKs (such as the Black Ops III Mod Tools) and for use in 3D art such as YouTube thumbnails, etc. The assets extracted by these tools are property of their respective owners and what you do with the assets is your own responsbility.

Greyhound is distributed with an automatic updater that will check for updates each time the application is launched by requesting the releases data via the Github API. If you do not wish for Greyhound to check for updates you can simply delete the Greyhound Updater executable.

## Credits/Contributors

* Scobalula - Developer/Maintainer/Research
* Blakintosh - MWR Material Support
* Eric Maynard - Help on BOCW from ModelGetter, ZM Hashes for BOCW Base
* DTZxPorter - Original Developer of the entire Wraith Project, Game Research
* ID-Daemon - Game Research

**If you use Greyhound in any of your projects, it would be highly appreciated if you credit the people/parties listed in the Credits list.**
