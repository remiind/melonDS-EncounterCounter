# melonDS-EncounterCounter

Fork of [melonDS](https://github.com/melonDS-emu/melonDS) that reports wild Pokémon encounters to a **PokéCounter** desk display (ESP32) on the local network.

- Supported: Pokémon HeartGold, SoulSilver, Platinum, Diamond and Pearl (all languages).
- Every emulator instance reports on its own, so several instances (*System → Multiplayer → Launch new instance*, or several melonDS windows) can hunt in parallel. Fast forward is fine.
- Every wild battle is sent as a small UDP/JSON datagram to port 4210 of the display, with species, level, gender, shiny flag, PID, IVs, ability, held item and the game code.
- The display is found via `pokecounter.local` (system resolver, then a direct mDNS query) or UDP broadcast. To set it explicitly, start melonDS with the environment variable `POKECOUNTER_HOST=<ip>`.
- If Windows asks whether melonDS may use the network, allow it for private networks.
- **PC boxes**: every 2 s the 18 boxes of the last in-game save (save memory) are checked; boxes that changed are sent as `{"t":"box",...}` (half a box per datagram) for the display's Boxes tab. Several instances of the same game send a box only once. `POKECOUNTER_BOXES=0` switches it off. Save layout from PKHeX (SAV4Sinnoh / SAV4HGSS), growth rates for the level from PokeAPI.
- **Auto-hunt (soft reset)**, per window, toggled with **F10** (*Config → Input and hotkeys → Hotkeys: "Pokémon auto-hunt"*): save the game in front of a static Pokémon (legendary, Snorlax ...), then press F10. The window resets, waits a random number of frames, presses A through intro, title, "Continue" and the dialogue until the battle starts, and checks the Pokémon: shiny → the hunt stops and the shiny auto-pause holds the window; otherwise it resets again, as fast as the PC allows. The random waits give every reset its own start delay and with it its own RNG seed. After three resets in a row without a battle (2 emulated minutes each) it stops. Every encounter is counted on the display as usual. Several windows can hunt at once (*System → Multiplayer → Launch new instance*, load the same ROM, F10 in each): an additional instance without its own `<game>.sav.2` ... starts from `<game>.sav`; the hunt never saves.

Changes against upstream (GPLv3): `src/frontend/qt_sdl/PokeCounter.cpp/.h` and `AutoHunt.cpp/.h` (new), the `HK_AutoHunt` hotkey (F10), hooks in `EmuThread.cpp` (after every frame) and `EmuInstance.cpp` (savestate loaded, instance destroyed), `ws2_32` in `src/frontend/qt_sdl/CMakeLists.txt`, CI builds only Windows x64. RAM layouts from [pokebot-nds](https://github.com/wyanido/pokebot-nds) (MIT).

---

<p align="center"><img src="https://raw.githubusercontent.com/melonDS-emu/melonDS/master/res/icon/melon_128x128.png"></p>
<h2 align="center"><b>melonDS</b></h2>
<p align="center">
<a href="http://melonds.kuribo64.net/" alt="melonDS website"><img src="https://img.shields.io/badge/website-melonds.kuribo64.net-%2331352e.svg"></a>
<a href="http://melonds.kuribo64.net/downloads.php" alt="Release: 1.1"><img src="https://img.shields.io/badge/release-1.1-%235c913b.svg"></a>
<a href="https://www.gnu.org/licenses/gpl-3.0" alt="License: GPLv3"><img src="https://img.shields.io/badge/License-GPL%20v3-%23ff554d.svg"></a>
<a href="https://kiwiirc.com/client/irc.badnik.net/?nick=IRC-Source_?#melonds" alt="IRC channel: #melonds"><img src="https://img.shields.io/badge/IRC%20chat-%23melonds-%23dd2e44.svg"></a>
<a href="https://discord.gg/pAMAtExcqV" alt="Discord"><img src="https://img.shields.io/badge/Discord-Kuribo64-7289da?logo=discord&logoColor=white"></a>
<br>
<a href="https://github.com/melonDS-emu/melonDS/actions/workflows/build-windows.yml?query=event%3Apush"><img src="https://github.com/melonDS-emu/melonDS/actions/workflows/build-windows.yml/badge.svg" /></a>
<a href="https://github.com/melonDS-emu/melonDS/actions/workflows/build-ubuntu.yml?query=event%3Apush"><img src="https://github.com/melonDS-emu/melonDS/actions/workflows/build-ubuntu.yml/badge.svg" /></a>
<a href="https://github.com/melonDS-emu/melonDS/actions/workflows/build-macos.yml?query=event%3Apush"><img src="https://github.com/melonDS-emu/melonDS/actions/workflows/build-macos.yml/badge.svg" /></a>
<a href="https://github.com/melonDS-emu/melonDS/actions/workflows/build-bsd.yml?query=event%3Apush"><img src="https://github.com/melonDS-emu/melonDS/actions/workflows/build-bsd.yml/badge.svg" /></a>
</p>
DS emulator, sorta

The goal is to do things right and fast, akin to blargSNES (but hopefully better). But also to, you know, have a fun challenge :)
<hr>

## How to use

Firmware boot (not direct boot) requires a BIOS/firmware dump from an original DS or DS Lite.
DS firmwares dumped from a DSi or 3DS aren't bootable and only contain configuration data, thus they are only suitable when booting games directly.

### Possible firmware sizes

 * 128KB: DSi/3DS DS-mode firmware (reduced size due to lacking bootcode)
 * 256KB: regular DS firmware
 * 512KB: iQue DS firmware

DS BIOS dumps from a DSi or 3DS can be used with no compatibility issues. DSi BIOS dumps (in DSi mode) are not compatible. Or maybe they are. I don't know.

As for the rest, the interface should be pretty straightforward. If you have a question, don't hesitate to ask, though!

## How to build
See [BUILD.md](./BUILD.md) for build instructions.

## TODO LIST

 * better DSi emulation
 * better OpenGL rendering
 * netplay
 * the impossible quest of pixel-perfect 3D graphics
 * support for rendering screens to separate windows
 * emulating some fancy addons
 * other non-core shit (debugger, graphics viewers, etc)

### TODO LIST FOR LATER (low priority)

 * big-endian compatibility (Wii, etc)
 * LCD refresh time (used by some games for blending effects)
 * any feature you can eventually ask for that isn't outright stupid

## Credits

 * Martin for GBAtek, a good piece of documentation
 * Cydrak for the extra 3D GPU research
 * limittox for the icon
 * All of you comrades who have been testing melonDS, reporting issues, suggesting shit, etc

## Licenses

[![GNU GPLv3 Image](https://www.gnu.org/graphics/gplv3-127x51.png)](http://www.gnu.org/licenses/gpl-3.0.en.html)

melonDS is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

### External
* Images used in the Input Config Dialog - see `src/frontend/qt_sdl/InputConfig/resources/LICENSE.md`
