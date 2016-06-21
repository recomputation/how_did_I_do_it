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
- Transition to C++ (29-31 May 2016)
- Migrate to sha512 (31 May 2016)
- Add script generation (31 May-1 June) // However there are issues in detection itself (see makefile example)
- Warnings on non-relative file names(1 June)
- Keep the current working directory in the recipe file(1 June)
- Close for non-closed files (1 June)
- Check support for cloning (Low priority)
- Files name rules (They exist, just not in a separate config)
- Proper handler management
- Fix the concurrency issues
- Tests work
- Follow current working directory (Mid priority) (that is a bit complicated)
- NixOS(?) - Preserve the folder hierarchy (Kind of)
- Build fake environment in the tmp folder

#TODO:

#Currently working on the:
- Keeping stdin/stdout (Maybe some dup magic here?)
- Path figuring out.
- Beautiful cleaning
- Need to versioning of files
- Execution files
