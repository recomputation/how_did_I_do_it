# How did I do it?

The utility developed allows the user to run anything and produce a list of input/output files used by the program. Such an approach allows reproducibility and lets people see what generated a particular programs.

#Currently it supports:
- Tracer written (24 May 2016)
- Running programs through it (24 May 2016)
- Proper Makefile added (24 May 2016)
- Support for fork/vfork programs (25 May 2016)
- Identification of the read/writes
- Saves the files by their md5 to a specific folder (25 May 2016)
- Saves recipes to the designated folder (26 May 2016)
- Allows lookup by a path to the file (29 May 2016)
- Allows lookup by a md5 digest (29 May 2016)
- Transition to C++ (29-30 May 2016)
- Migrate to sha512 (31 May 2016)

#TODO:
- Check support for cloning (Low priority)
- Keeping stdin/stdout
- Check for non-closed files
- NixOS(?) - Preserve the folder hierarchy NOT YET
- Warnings on non-relative file names
- Files name rules
- Follow current working directory (Mid priority)
- Keep the current working directory in the recipe file (Mid priority)

#Currently working on the:
- Migration to C++, to make development faster
- Add script generation (High priority)
