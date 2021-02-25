# Civ 3 Enhanced Map Replays

A utility you can run on Civ 3 SAV files for a moderately better looking replay map. To use it, download the files "map" and "tiles.png" if on Linux or "map.exe" and "tiles.png" if on Windows. It's necessary to keep the png file in the same folder as the program itself because it has the required textures. You can also build the program itself if you have the TCLAP and SFML libraries installed.

```
USAGE:

   ./map  [-x <integer>] [-y <integer>] [-f <integer>] [-s] [-l] [--]
          [--version] [-h] <yourfile.SAV>


Examples:
  Linux:
    ./map -f 24 -sl "Super Portugal.SAV"
  Windows:
    map.exe -x 15 -y -20 -s -f 12 path/to/myfile.SAV

Where:

   -x <integer>,  --x-offset <integer>
     Shift the map horizontally by x tiles

   -y <integer>,  --y-offset <integer>
     Shift the map vertically by y tiles

   -f <integer>,  --frame-rate <integer>
     Set the number of turns per second

   -s,  --stretch
     If set, the map will be stretched to fit the viewport. Otherwise,
     black bars will be used

   -l,  --loop
     If set, the replay will loop back to the beginning after finishing

   --,  --ignore_rest
     Ignores the rest of the labeled arguments following this flag.

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.

   <yourfile.SAV>
     (required)  A Civ 3 Save File


   Utility for creating pretty video replay maps from Civilization 3 SAV
   files

```
