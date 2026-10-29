# Contributors to Behringer KICK (BE 0700-AAM) Desktop Build

This file documents contributors whose work is included in this repository beyond the original Music Tribe release.

## Desktop Build & Compatibility Layer

The following contributions enable building and running the KICK simulator (KT_KickEM) on desktop platforms:

### Jacob Schooley <jacob@schooley.com>
- **Source**: [jbschooley/0700-AAM](https://github.com/jbschooley/0700-AAM/tree/desktop-build)
- **Contributions**:
  - `Kick/modules/compat/` - Complete compatibility layer for missing private modules:
    - `compat/AudIO_constants/` - Reconstructed AudIO constants (MIDI, GAIN, PAN, POLYPHONY, DB, etc.)
    - `compat/AudIO_juce/` - JUCE audio extensions with Recorder, SampleRecorder, Looper implementations
    - `compat/juce_serialport/` - POSIX-based serial port replacement
  - `Kick/3rdParty.cmake` - Added support for in-repo AudIO and ComProtocol sources
  - `Kick/modules/CMakeLists.txt` - Added compat modules to build
  - `Kick/apps/CMakeLists.txt` - Made kickem conditional on KICK_SIMULATOR option
  - `Kick/CMakeLists.txt` - Added KICK_SIMULATOR, KICK_ENGINE_TEST, KICK_PROGRAM_BUILDER options, ASIO support
  - `Kick/apps/kickem/` - Complete emulator implementation
  - `Kick/build-mac.sh` - macOS build script
  - Various other Kick and AudIO module files needed for the desktop build

### Eight4aWish
- **Original vendor** for juce_serialport module (as indicated in module declaration)
- **Website**: https://github.com/Eight4aWish

## FSM Kick VST3 Plugin

### Krzysztof Foltman <kfoltman@users.sourceforge.net>
- **Source**: [Buzztrax/buzzmachines](https://github.com/Buzztrax/buzzmachines) (FSM/Kick/)
- **License**: LGPL 2.1 (see buzzmachines/FSM/COPYING)
- **Contributions**:
  - Original FSM Kick synthesis algorithm adapted for VST3 plugin
  - `Kick/apps/kickvst_fsm/` - VST3 plugin implementation based on FSM Kick

## License

All compat modules and modifications are licensed under **AGPL-3.0-or-later**, consistent with the original project license.
FSM Kick VST3 plugin code incorporates work under **LGPL 2.1** (compatible with AGPL-3.0).

## Code of Conduct Compliance

This attribution follows the project's code of conduct: "Share credit. Name the people whose work you build on."

---

*Last updated: 2026-10-29*

