# Skills Documentation

This directory contains skill documentation for the 0700-AAM project, capturing best practices and procedures developed during the project's development.

## Available Skills

### 📝 [Commit Principles](commit_principles.md)
Guidelines for creating proper commits in this project:
- Developer Certificate of Origin (DCO) sign-off
- AI tools attribution
- Credit to original authors
- Commit message format and examples
- Atomic commit practices

### 🔧 [Porting BuzzMachines Plugins to Kick](porting_buzzmachines_to_kick.md)
Complete guide for porting BuzzMachines synthesis algorithms:
- Two approaches: Standalone VST3 plugin OR Internal instrument engine
- Step-by-step instructions for both methods
- Directory structure
- Code examples for porting from Buzz to JUCE
- CMake configuration
- Build integration
- Common challenges and solutions

## Usage

These documents serve as:
1. **Reference**: For team members to follow established practices
2. **Onboarding**: For new contributors to understand project conventions
3. **Continuity**: To preserve knowledge across team changes

## Adding New Skills

When you develop a new procedure or best practice:

1. Create a new `.md` file in this directory
2. Use clear, actionable language
3. Include code examples where applicable
4. Reference related files in the repository
5. Add cross-references to other skill documents
6. Update this README with the new skill

## Related Files

- **[CONTRIBUTORS.md](../CONTRIBUTORS.md)**: Lists external contributions and their original authors
- **[Kick/build-mac.sh](../Kick/build-mac.sh)**: Build script for macOS builds
- **[Kick/apps/kickvst_fsm/](../Kick/apps/kickvst_fsm/)**: Example VST3 plugin (FSM Kick)

## Maintenance

Keep these documents updated as:
- New tools or procedures are adopted
- Lessons are learned from issues or bugs
- Project structure evolves
- New contribution patterns emerge

## Version History

- **2024-10-29**: Initial skill documents created
  - commit_principles.md
  - porting_buzzmachines_to_kick.md
  - README.md
