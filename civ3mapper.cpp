#include <iostream>
#include <tclap/CmdLine.h>
#include <string>
#include "maprender.h"

int main(int argc, char* argv[]) {
  // Set up all the command line stuff
  TCLAP::CmdLine cmd("Utility for creating pretty video replay maps from Civilization 3 SAV files", ' ', "0.0.1");
  TCLAP::UnlabeledValueArg<std::string> savfile("savfile", "A Civ 3 Save File", true, "MySave.SAV", "yourfile.SAV");
  TCLAP::ValueArg<int> xoffset("x", "x-offset", "Shift the map horizontally by x tiles", false, 0, "integer");
  TCLAP::ValueArg<int> yoffset("y", "y-offset", "Shift the map vertically by y tiles", false, 0, "integer");
  TCLAP::ValueArg<int> framerate("f", "frame-rate", "Set the number of turns per second", false, 6, "integer");
  TCLAP::SwitchArg stretch("s", "stretch", "If set, the map will be stretched to fit the viewport. Otherwise, black bars will be used", false);
  TCLAP::SwitchArg loop("l", "loop", "If set, the replay will loop back to the beginning after finishing", false);

  cmd.add(savfile);
  cmd.add(loop);
  cmd.add(stretch);
  cmd.add(framerate);
  cmd.add(yoffset);
  cmd.add(xoffset);
  cmd.parse(argc, argv);

  Civ3::MapRender mr(savfile.getValue(), xoffset.getValue(), yoffset.getValue(), framerate.getValue(), stretch.getValue(), loop.getValue());

  return 0;
};

/*
  TODO:
    [Done] Comments
    Support compressed sav
    Test for vertical maps
    [Done] Loop flag
    [Done] Frame rate flag
    [Done] Investigate Ottoman SAV (seg fault, odd beginning behavior)
    [Done] Fix Colors
    [Done] Fix color tiles not wrapping
    [Done] Fix offset flags
    Fix negative offset
    Better error handling
    Show borders and cities?
*/
