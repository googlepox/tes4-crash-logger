# tes4-crash-logger

A DLL that logs crashes for The Elder Scrolls IV: Oblivion.

---

Fork of Yvile's fork of Cobb's Crash Logger that reimplements some of Yvile's features to provide a far more readable crash log for Oblivion. CrashLogger.log in the root Oblivion folder (next to Oblivion.exe) will now have more information like specific game objects, forms, and classes loaded at the time of the crash.

Known Issues

- The game may show an error message when crashing, although the log will still print normally. Currently debugging this but it poses no functional issue at this time.
- Some feature's of Yvile's Crash Logger are not yet implemented - Crash Logs folder, heap info, thread info, and not all game classes are supported yet. Work is currently ongoing.
