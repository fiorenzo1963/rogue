# ROGUE 5.3 - 5.4

Rogue version 5.3-4.4.2020.1, derived from Rogue version 5.4.4 at https://github.com/Davidslv/rogue.git,

This is an enchanced version which can either run Monsters, Objects and other behavior like Traps, as version 5.3 (default, 1982) or the latest known version 5.4.4. Use -5.4 to run in 5.4.4 mode, -5.3 otherwise (5.3 is default).

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

```
Cloned from https://github.com/Davidslv/rogue.git
```

Build as follows:

```
         ./configure --enable-ncurses --enable-wizardmode
```

## Options and Features

- This version has miscellanous bug fixes and it now supports Rogue version 5.3 (circa 1980, Italy). Each version has slightly different monsters, objects and traps. By default it runs in 5.3 mode.
- This version has modified to run on 2018 versions of MacOS, Linux and FreeBSD.
  - Major bug fixes in object placement, management, and inventory.
  - Other bug fixes.
  - Added Rookie/Warrior mode. The "R" flag will be shown at the end of the status line when in Rookie mode.
    - Warrior mode is the original behavior.
    - Rookie mode gives you better weapon and armor, a few goodies, and much higher chances of finding a roomfull of monsters. This gives you more chances of acquiring resources.

- To run in 5.3 mode (default): rogue -5.3
- To run in 5.4 mode: rogue -5.4
- To run in "Warrior" mode: rogue [-version]
- To run in "Rookie" mode: rogue -r [-version]
- To print scores for "Warrior" mode: rogue -s [-version]
- To print scores for "Rookie" mode: rogue -r -s [-version]
- To restore a saved game: rogue [-version] save_file

## 5.3 Messages

TBD

## 5.4 Messages

Well, you have the source code, so might as well summarize here messages.

### When you quaff a potion
- "what a tripy feeling", "what's going on here": confusion.
- "everything seems so cosmic": hallucination.
- "cloak of dark", "darkness": blindness.
- "float", "floating in the air": levitation.
- "you have a {normal,strange} feeling for a moment ...": see monsters.
- "you have a {normal,strange} feeling for a moment ...": magic detection.
- "this potion tastes like ... juice": see invisible.
- "you begin to feel much better": extra healing.
- "... It make you feel warm all over": restore strength.

### When you read a scroll
- "your hands begin to glow ...": monster confusion.
- "your amoror glows ... for a moment": enhance armor.
- "you feel a strange sense of loss": hold monster.
- "you hear a faint cry of anguish ...": create monster.
- "your nose tingles ...": food detection.
- "you feel a strange sense of loss": enhance weapon.
- "your ... glows ... for a moment": enhance weapon.
- "you hear maniacal laughter in the distance": scare monsters.
- "you feel as if somebody is watching over you": remove curse.
- "you feel in touch with the ...": remove curse.
- "your armor is covered by a shimmering ...": protect armor.
- "you feel a strange sense of loss": protect armor.

## FIXME

- Fix handling of delta coordinates
