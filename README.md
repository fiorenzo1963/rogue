# ROGUE

The original version of rogue, version 5.4.4.

Original Copyright:
```
Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman.
```

Portions Copyright:
```
Copyright (C) 1999, 2000, 2005 Nicholas J. Kisseberth.
Copyright (C) 1994 David Burren.
Copyright (C) 2018 Fio Cattaneo.
```

Distribution:
```
Distributed under BSD License (see LICENSE.TXT).
```

Cloned from https://github.com/Davidslv/rogue.git

Build as follows:

```
         ./configure --enable-ncurses --enable-wizardmode
```

## Options and Features

- This version has modified to run on 2018 versions of MacOS, Linux and FreeBSD.
  - Several bug fixes.
  - Added Rookie/Veteran mode. The "R" flag will be shown at the end of the status line when in Rookie mode.
    - Veteran mode is the original behavior.
    - Rookie mode gives you better weapon and armor, and much higher chances of finding a roomfull of monsters. This gives you more chances of acquiring resources.

- To run in "Veteran" mode: rogue
- To run in "Rookie" mode: rogue -r
- To print scores for "Veteran" mode: rogue -s
- To print scores for "Rookie" mode: rogue -r -s
- To restore a saved game: rogue save_file
