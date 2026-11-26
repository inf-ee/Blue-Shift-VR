# bsvr-install

Interactive single-file installer (Windows or Linux) for Half-Life: VR Mod - Blue Shift.

It finds Steam through the registry (on Linux, in its usual folders), finds the Half-Life folder from Blue Shift's Steam
app manifest, waits for Half-Life to be installed as its own game because Steam only lists mods under it,
then creates `<Half-Life>\bsvr\` and `<Half-Life>\bsvr_hd\` with:

- the embedded payload: `liblist.gam`, `delta.lst`, the `game.*` icons, `dlls\hl.dll`,
  `cl_dlls\client.dll`, `maps\ba_yard4a.ent`, the SD weapon view models in `models\` and `models\animov\`, licenses, and the
  VR runtime assets from Half-Life: VR (see Payload below): the SteamVR input manifest and default bindings in `actions\`,
  the HUD sprite sheets as PNG in `textures\hud\`, the SteamVR loading skyboxes in `textures\skybox\`, `sprites\black.spr`
  and `sound\plats\train_use2.wav`;
- the HD weapon view models in `bsvr_hd\models\` and `bsvr_hd\models\animov\`. The engine puts `bsvr_hd` ahead of
  `bsvr` when HD models are on (the game's own HD models checkbox, `OriginalModels` in **Options > Advanced**, which runs `_sethdmodels`);
- `openvr_api.dll` and `EasyHook32.dll` next to `hl.exe` in the Half-Life folder, because `client.dll`
  imports them implicitly and Windows does not search `cl_dlls\` for imports;
- Blue Shift's maps, rewritten from the Blue Shift BSP lump order to the standard one (see `Maps.cs`),
  with entities replaced from any `maps\*.ent` in the payload.
- `resource\OptionsSub*.res`: the player's own layouts of the game's Options tabs (from `bshift`, else `valve`) with the
  options that do nothing in VR or that the mod overrides set to `"enabled" "0"`: HEV suit volume, all of Video except
  texture filtering, brightness and gamma (display mode is hidden, the game enables it itself), all of Aim and Keyboard
  (see `OptionsLocks.cs`). Rerun the installer after a game update that changes them.
- `sound\sentences.txt`: the player's own sentences file (from `bshift`, else `valve`) with Half-Life: VR's gunpoint
  and gib reaction lines for guards and scientists added (`BA_GUN_*`, `SC_GUN_*`, `BA_GIB_*`, `SC_GIB_*`; see
  `Sentences.cs`). Blue Shift's file has none of them, so without it NPCs react silently. Rerun the installer after a
  game update that changes it.
- spaces removed from the names of files in `SAVE\`: the new game menu cannot load saves with spaces, which the old one allowed.

The Half-Life folder comes only from Steam; the installer quits with a hint when Steam or Blue Shift is not found.
Half-Life: VR Mod is not needed: everything the mod used from it is in the payload.
If the Half-Life folder already has `bsvr\liblist.gam`, the installer asks whether to update it, does not wait for Steam
to exit (the mod is already in the library) and only waits for the game to quit.

## Payload

`payload/` is the `bsvr` folder and `payload_hd/` the `bsvr_hd` folder (embedded as `halflife/bsvr_hd/...`, see the csproj).
The SD set has no `v_gauss.mdl` (SD gauss is not converted), so `payload/models` and `payload/models/animov` keep the HD gauss
for SD mode. Blue Shift's own NPC and world models are not copied: the engine finds them in `bshift` and, with HD on,
`bshift_hd`. Both animov sets must be complete, because the animov twin of every weapon is precached unconditionally.

The VR runtime assets are tracked, taken from Half-Life: VR (its Steam release, the same files as its repository's `art/` and
`game/`); only the ones the code uses are shipped (see `Docs/VR_ASSETS.md`):
- `payload/actions/`: the SteamVR action manifest, its English strings, `customactions.cfg` and default bindings for Vive,
  Index, Touch and WMR, with the Steam release's CRLF line endings.
- `payload/textures/hud/`: `640hud1`-`640hud9` and `640_train_frame_0`-`4`, the only HUD sheets the VR HUD draws (the
  client always uses the 640 sheets).
- `payload/textures/skybox/`: `desert`, `2desert` and `alien1`, the SteamVR compositor skyboxes shown while the game isn't
  submitting frames (map table in `VRTextureHelper.cpp`; Windows only).
- `payload/sprites/black.spr`: the invisible model of the VR flashlight's light entity, precached by `ClientPrecache`;
  without it no map loads.
- `payload/sound/plats/train_use2.wav`: played when a VR train button changes the speed.

Earlier versions copied these from the player's Half-Life: VR install, with more than was used. Updates remove the rest
(the `Obsolete` list in `Assets.cs`): the 320 HUD sheets, `textures/hud/HD/`, 16 unused skybox sets, `textures/skybox/hd/`
and `textures/background.png`.

`payload/liblist.gam`, the icons and `maps/ba_yard4a.ent` are tracked; the last four come from the
Blue Shift Updated release. `payload/resource/GameMenu.res` is tracked too: the game's main menu with a "VR Options" entry
(console command `vr_options`), it replaces the one the engine would find in `bshift`. `payload/resource/VROptionsScheme.res`
is the look of the VR Options dialog, based on the game's `TrackerScheme.res`; the game writes a compiled copy next to it
(`VROptionsScheme.res.i`) the first time the dialog opens. `skill.cfg`, `settings.scr` and `user.scr` are deliberately not shipped:
the engine finds retail Blue Shift's through `fallback_dir`, which keeps Blue Shift's own skill values. `payload/dlls/hl.dll` and `payload/cl_dlls/client.dll` are not; the Visual Studio post-build step of
`hldll` and `hl_cdll` copies them there, so publish after building the configuration you want to ship.
`delta.lst`, `LICENSE`, `THIRD_PARTY_LICENSES`,
`lib/public/openvr_api.dll` and `cl_dll/EasyHook/bin/EasyHook32.dll` are embedded straight from the
repository (see the csproj).

## Build

The target runtime is chosen with `-r`; without it the build defaults to `win-x64`. Every build is self-contained
and a single file, and can be cross-published from any host OS.

```
# Windows (default)
dotnet publish -c Release

# Linux
dotnet publish -c Release -r linux-x64
```

The output is in `bin/Release/net10.0/<rid>/publish/`.

Delete `bin/` and `obj/` before publishing after replacing payload files: the incremental build compares timestamps,
and a file moved or copied with its old timestamp (`mv`, `cp -p`) is not embedded again.

### Release

Pushing a `v*` tag runs `.github/workflows/release.yml`: it builds the Linux libraries in the Steam Runtime container
and the Windows ones on a Visual Studio 2026 runner, puts all four into the payload, and publishes both installers into a
draft release as `bsvr-install-<tag>.exe` and `bsvr-install-<tag>-linux`, with `SHA256SUMS`. Both installers carry both
library sets, so each one works for native Linux and Proton alike. Release files lose the executable flag on download,
so the release notes and the main README tell Linux players to `chmod +x` the installer and run it from a terminal.

### Linux

The installer supports both a native Linux Half-Life (it has `hl_linux` and `hl.sh`) and a Windows one run through
Proton (it has `hl.exe`). It installs whatever is embedded: `hl.so`/`client.so` for native, `hl.dll`/`client.dll` for Proton.
`liblist.gam` lists both game libraries, and the engine picks the one for its platform.

Build the native libraries with `linux/build-steamrt.sh` (needs docker). It runs `make` in the pinned Steam Runtime 1
(scout) SDK container, so the libraries need only glibc 2.6 and no system `libstdc++`. A plain `make` in `linux/`
(needs `g++ -m32`) works for testing, but the result needs the build machine's glibc. Either way the `installer_payload`
step of the default target copies `release/hl.so` to `payload/dlls/` and `release/client.so` to `payload/cl_dlls/`:

```
../linux/build-steamrt.sh
```

`external/reactphysics3d/lib/Linux/libreactphysics3d.a` is prebuilt and tracked; the default target links it without
rebuilding. After changing its sources, rebuild it with `../linux/build-steamrt.sh rp3d` (byte-identical for the same
sources) and commit it.

Then publish the installer as above. `lib/public/libopenvr_api.so` (OpenVR 2.15.6, from the SDK's `lib/linux32/`) is embedded too, and is installed
into the Half-Life folder, which `hl.sh` puts on `LD_LIBRARY_PATH`. `client.so` also needs `vgui.so` (shipped with
Half-Life) and `libSDL2-2.0.so.0`, which is expected to come from the Steam runtime.

The installer looks for Steam in `~/.steam/steam`, `~/.local/share/Steam`, the Flatpak location
(`~/.var/app/com.valvesoftware.Steam/.local/share/Steam`) and the Snap one (`~/snap/steam/common/.local/share/Steam`),
and quits if it is in none of them.

To run the Windows build instead, force Proton on Half-Life (Steam > Half-Life > Properties > Compatibility) so the
Windows files, including `hl.exe`, are installed; the Linux installer handles both.

Nothing is locked on Linux, but overwriting a `.so` the game has loaded crashes it, so the installer still waits for
the game from this Half-Life folder to quit: `hl_linux`, or `hl.exe` under Proton (found by its `Z:\...` command line).
