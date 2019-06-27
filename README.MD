# JKSV

~~Multipurpose~~ Save tool for Switch.

## Info
JKSV on Switch started as a small project/port to test some things and get familiar with libnx. A list of what it currently can do:
1. Dump and restore save data.
    ~~* This includes the ability to dump and restore to/from any location on SD by pressing minus and using the Advanced Mode.~~
~~2. Dump system save data
    * Pressing all four shoulder buttons at once will rescan and include the previously unlisted system saves.
    * Dumping this data is allowed, but writing back is not.~~

# Building:
1. Requires [devkitPro](https://devkitpro.org/) and [libnx](https://github.com/switchbrew/libnx)
2. Requires switch-freetype, libpng, and libjpeg-turbo

# Credits and Thanks:
* [shared-font](https://github.com/switchbrew/switch-portlibs-examples) example by yellows8
* Authors of switch-examples for account and save mounting code.
* [Iguniisu](https://github.com/igniscitrinus) for the icon.

---

r4dius fork :
- Design overhaul to match switch menus
- Removed non game save stuff

To do :
- add battery / time info
- popup confirmation when writing to a name that already exists

Stuff I'd like to add :
- scrolling effect
