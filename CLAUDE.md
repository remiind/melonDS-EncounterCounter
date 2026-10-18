# CLAUDE.md - melonDS-EncounterCounter

Fork of melonDS (desktop, Qt) for Daniel's PokéCounter. Branch **`master`**, CI workflow
**"Windows (EncounterCounter)"** builds only Windows x64, artifact **`melonDS-EncounterCounter-windows-x86_64`**
(~40 min). He replaces his `melonDS.exe` with it. Sister fork for Android: remiind/WatermelonDS-LAN (same
PokeCounter core, keep both in sync - see below).

## What the fork adds (all in `src/frontend/qt_sdl/`)

- `PokeCounter.cpp/.h` - the whole bridge, one `Tracker` per emulator instance (several windows hunt in parallel):
  - Gen 4 only: HeartGold/SoulSilver (IPK/IPG), Platinum (CPU), Diamond/Pearl (ADA/APA), all languages.
    RAM layouts from pokebot-nds (anchor pointer + language offset table `kGames`).
  - wild battle detection → `report()` (t=enc); party → t=mon every 60 frames outside battles; PC boxes from the
    **save memory** (`processSave`, PKHeX SAV4 layouts, active partition by footer counter) → t=box, shared hash
    across instances so 10 windows send a box once; EV yield table (`kEvYield`, PokeAPI) → `takeEvSpecies`/`evText`.
  - egg hunt (Masuda) state machine `EggHunt` (`eggStart/eggAfterFrame/eggKeys`): bikes the day-care path
    (coordinates/pointers from pokebot-nds `mode_daycare_eggs`), fetches eggs, **checks each egg at pickup** (PID is
    fixed when the egg is made; shiny vs the player's TID/SID), reports with `egg`/`masuda` (parents' language byte
    0x17 compared; day-care slots = PKHeX DaycareOffset mapped to RAM, verified against pokebot's daycare_egg
    pointer), shiny egg → stop, full party → reset.
  - gift detection for the auto-hunt (`giftArm`/`giftTick`): a Pokémon beyond the saved party size is reported
    like an encounter (starters, Oak's Kanto starter in HGSS, fossils).
  - env switches: POKECOUNTER_HOST, POKECOUNTER_AUTOPAUSE, POKECOUNTER_TEAM, POKECOUNTER_BOXES, POKECOUNTER_EVS (=0 off).
  - `POKECOUNTER_NO_MELONDS` builds the core without melonDS for unit tests.
- `AutoHunt.cpp/.h` - soft-reset hunt (pure logic): reset, random start delay (own RNG seed per reset), random A
  presses until an encounter (battle or gift) is reported; shiny → stop, else reset; 3x2 min without one → stop.
- `EmuThread.cpp` - hotkeys and glue: **F10** auto-hunt, **F11** egg hunt, key mask
  `inputMask & ~(autoHunt.keysForFrame() | eggKeys)`, unlimited speed while hunting, OSD messages (EV yield,
  hunt status), shiny pause. Hotkeys registered in `EmuInstance.h` (HK_AutoHunt, HK_EggHunt),
  `EmuInstanceInput.cpp`, `InputConfig/InputConfigDialog.h`, defaults in `Config.cpp`.
- `Window.cpp` - **Ctrl+Alt+I** = Launch new instance.
- README top section documents all of it for the user.

## Testing without a ROM

No ROMs/saves in the session. Test the core with simulated RAM: compile
`g++ -std=c++17 -DPOKECOUNTER_NO_MELONDS -I src/frontend/qt_sdl test.cpp src/frontend/qt_sdl/PokeCounter.cpp -lpthread`
with a small harness that writes encrypted PK4 structs (block shuffle + LCRNG) into a RAM vector, and run a
fake display (`python` UDP server on 4210 answering `{"ok":true}`) with `POKECOUNTER_HOST=127.0.0.1`. The egg hunt
harness simulates bike movement, the day-care man and resets (HGSS and Platinum coordinates). A full Linux build
of the Qt frontend works too (cmake build dir, `make melonDS`) to catch compile errors before CI.

## Keeping WatermelonDS-LAN in sync

`app/src/main/cpp/PokeCounter.cpp/.h` there is a copy of this repo's files with only the first comment line
changed (`// WatermelonDS-LAN: PokéCounter bridge ...`). After changing the core here: copy both files over (sed the
first line), commit and push there too. Frontend glue differs (Android: MelonInstance.cpp/JNI/Kotlin).

## Known gaps

- Egg hunt and gift hunt were tested only against simulated RAM, not a real game.
- EV yields are PokeAPI's current values (practically identical to Gen 4).

## Owner and how to work with him

- Owner: Daniel (GitHub `remiind`). **Reply in German.** Network/security engineer, comfortable with technical detail.
- No upstream PRs (he declined). Work on the fork's branch named below and push there; CI builds the downloads.
- Session artifacts cannot be downloaded from a Claude session (GitHub blob storage answers 403): give him the
  Actions run link + artifact name and let him download. Logs of failed runs are also blocked; read the errors
  from the check-run annotations instead (`gh api repos/<repo>/check-runs/<id>/annotations`).
- Never kill processes with `pkill -f <pattern>` in the session shell (it matches and kills the shell itself);
  check `/proc/<pid>/cmdline` and kill by pid.
- His Windows PC (when the session is linked): `C:\Users\dschr\Desktop\pokecounter\` holds the display firmware
  project (`pokecounter\`), gen1recomp builds (`gen1recompPRJ\`) and melonDS (`C:\Users\dschr\Desktop\melonDS`).
  Large files go over the device bridge in parts of ≤25 MB into `gen1recompPRJ\_to_delete\`, then are joined
  with device_bash and checked with sha256.

## The PokéCounter system (shared by all four forks)

A desk display (Waveshare ESP32-S3-Touch-LCD-1.47, PlatformIO/Arduino, LVGL; web UI at http://pokecounter.local)
counts Pokémon encounters sent by the emulators/games over UDP.

- Firmware: **not on GitHub**, lives on the PC in `C:\Users\dschr\Desktop\pokecounter\pokecounter` (src/: main.cpp,
  net.cpp, store.cpp, hunts.cpp, boxes.cpp, sdlog.cpp, team.cpp, ui.cpp, web_index.h). Flash it from VS Code
  (PlatformIO "Upload", env `pokecounter`, USB) or via the web UI firmware upload (Verwaltung tab, password in
  `include/secrets.h`). **Never print or copy `secrets.h`** (Wi-Fi credentials). Builds made in a cloud session use a
  placeholder `secrets.h` - never hand those .bin files out (that once put the display off Wi-Fi).
- Transport: JSON datagrams to UDP port 4210 of the display; the display answers `{"ok":true,...}`. Senders find it
  via `pokecounter.local` (resolver, then a direct mDNS query) or broadcast, pin the first replying address,
  repeat a report until acknowledged (300 ms gap after an ack, 0.4 s → 2 s retries) and never drop reports.
- Packets:
  - `{"t":"enc","sp":<natdex>,"lv":..,"g":"M|F|N","shiny":bool,"pid":..,"eid":<random>,"ivs":<packed>,"ab":..,"it":..,
    "tsv":<TID^SID>,"src":"<4-char game code>"[,"egg":1][,"masuda":1]}` - one encounter. `eid` makes repeats
    harmless. IVs packed HP|Atk<<5|Def<<10|Spe<<15|SpA<<20|SpD<<25. `masuda`: egg from parents of different
    languages (Gen 4 rolls the PID 5x; the display counts it like 5 encounters for the odds).
  - `{"t":"mon","slot":1..6,"n":<party size>,...}` - party member for the Team tab (IVs, EVs, stats, moves, names).
  - `{"t":"box","b":1..18,"h":0|1,"nb":..,"cur":..,"name":"..","m":[[slot,sp,lv,fl,pid,ivs,it,ab,"nick",ball],..]}` -
    half a PC box per datagram (fits 1500 bytes); fl = shiny | gender<<1 | egg<<3.
  - `{"t":"ping"}` - find the display.
- Display features that depend on the senders: per-edition profiles (by `src`), target hunt ("Ziel-Jagd") with
  odds incl. Masuda, finished hunts tab, shinies, history on SD card, Team, Boxes, shiny melody.
- Shiny auto-pause in every sender: on a shiny the game pauses once the display confirmed it (max 1.5 s), and
  reset/state loading stay blocked until resumed by hand.
