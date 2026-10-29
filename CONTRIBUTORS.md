# Contributors to 0700-AAM

This file documents contributions from external authors whose work has been integrated into this project.

## General Principles

Following the project's code of conduct: **"Share credit. Name the people whose work you build on."**

All external contributions must:
- Be properly licensed (compatible with AGPL-3.0-or-later)
- Include original author information
- Reference the source repository
- Maintain the original license information

---

## Plugin Contributions

### FSM Kick VST3 Plugin & Internal Engine

**Original Author**: Krzysztof Foltman <kfoltman@users.sourceforge.net>
- **Source**: [Buzztrax/buzzmachines](https://github.com/Buzztrax/buzzmachines/tree/master/FSM/Kick)
- **License**: LGPL 2.1 (see buzzmachines/FSM/COPYING)
- **Contributions**: Original FSM Kick synthesis algorithm
- **Integration**: 
  - Ported to VST3 plugin: `Kick/apps/kickvst_fsm/`
  - Ported to internal engine: `Kick/modules/engine/audio_program/instrument/fsm_kick_engine.h/.cpp`
  - Available as both standalone VST3 plugin and internal instrument engine in emulator

**Integration Author**: [To be filled with your name]
- **Date**: [Current date]
- **Work**: Ported FSM Kick algorithm from BuzzMachine to JUCE/C++

---

## Internal Porting Work

The following porting work has been performed by the 0700-AAM team:

### BuzzMachines Ports

1. **FSM Kick** - Ported by [Your Name] <[your.email]>
   - Source: buzzmachines/FSM/Kick/
   - License: LGPL 2.1
   - Implementation: Complete
   - Status: Working in both VST3 plugin and internal engine
   - Parameters:
     - Start Freq: Starting frequency of the kick (20-500 Hz)
     - End Freq: Frequency the kick decays to (20-500 Hz)
     - Tone Decay: How fast the pitch decays (0.001-2 seconds)
     - Tone Shape: Curve of the pitch decay (0-1, linear to exponential)
     - Amp Decay: Envelope decay time (0.001-4 seconds)
     - Volume: Output level (0-1)

---

## Adding New Contributions

When adding a new external contribution:

1. Create a new section under "Plugin Contributions" or "Internal Porting Work"
2. Include:
   - Original author name and contact
   - Source repository URL
   - License information
   - Brief description of what was contributed
   - Where it was integrated in this project
   - Your name as integration author

3. Update the relevant skill documentation in `docs/skills/`

4. Commit with proper attribution following `docs/skills/commit_principles.md`

---

## References

- [Buzztrax/buzzmachines GitHub Repository](https://github.com/Buzztrax/buzzmachines)
- [LGPL 2.1 License](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
- [AGPL 3.0 License](https://www.gnu.org/licenses/agpl-3.0.html)
