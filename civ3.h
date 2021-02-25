#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <string.h>
#include "decompress.h"

namespace Civ3 {
  // The dimensions of the tiles and tile sections in tiles.png
  #define TILEWIDTH 128
  #define TILEHEIGHT 64
  #define SECTIONHEIGHT (TILEHEIGHT * 9)

  // All sections in civ 3 data files begin with 4 character long headers
  // See https://forums.civfanatics.com/threads/the-civilization-iii-save-format.48270/
  char CIV3[] = "CIV3";
  char WRLD[] = "WRLD";
  char TILE[] = "TILE";
  char RPLS[] = "RPLS";
  char RPLE[] = "RPLE";
  char RPLT[] = "RPLT";
  char LEAD[] = "LEAD";

  class Tile {
    public:
      // X and Y coordinates of tile on the map
      int x;
      int y;
      // X and Y coordinates of the tile's texture (in pixels) on the texture grid
      int texturex;
      int texturey;

      Tile() {};
      Tile(int, int, int, int);
  };

  Tile::Tile(int i, int section, int sectionindex, int mapwidth) {
    // Tiles go (0,0), (2,0), (4,0) ... (n,0), (1,1), (3,1), (5,1) ... (n+1,1), (0,2), (2,2) ...
    // Note that this means half of all coordinates on the map go unused
    y = i / (mapwidth / 2);
    x = i % (mapwidth / 2) * 2 + y % 2;
    texturex = (sectionindex % 9) * TILEWIDTH;
    texturey = (sectionindex / 9) * TILEHEIGHT + section * SECTIONHEIGHT;
  };

  class Event {
    public:
      // You only ever really need to go through the events sequentially, so make it a linked list why not
      Event* next = NULL;
      int turn; // When
      int x; // Where
      int y; // Where
      int civ; // Who

      Event() {};
      Event(int, int, int, int);
      Event(int, char*);
  };

  Event::Event(int turn, int x, int y, int civ) {
    this->turn = turn;
    this->x = x;
    this->y = y;
    this->civ = civ;
  };

  Event::Event(int turn, char* pointer) {
    this->turn = turn;
    this->x = *(pointer + 10) & 255;
    this->y = *(pointer + 12) & 255;
    this->civ = *(pointer + 9) & 255;
  };

  class SAV {
      char* buffer; // The civ 3 sav file hex data; *not* null terminated!
      int buflength; // Length of data in bytes
      char* end; // Memory location of where the buffer ends
      Decompress* d;

      bool gotostring(char*);
      char* findstring(char*, char* pointer);
      void settiledata();
      void setleaderdata();
      void seteventdata();

    public:
      unsigned int mapwidth;
      unsigned int mapheight;
      bool wrapx; // Whether or not you can sail east and end up west
      bool wrapy; // Whether or not you can sail north and end up south
      int tilecount;
      Tile* tiles;
      Event* eventhead;
      int leaders[32];

      SAV(std::string);
  };

  SAV::SAV(std::string filename) {
    std::ifstream in;
    in.open(filename, std::ios::binary);

    if (in.is_open()) {
      std::vector<char> vbuf(std::istreambuf_iterator<char>(in), {});
      buflength = vbuf.size();
      buffer = vbuf.data();
      end = buffer + buflength;

      // First 4 bytes should be the "CIV3" header string
      // If they're not, try decompressing the file
      if (strncmp(buffer, CIV3, 4)) {
        d = new Decompress(buffer, buflength);

        buffer = d->output.data();
        buflength = d->output.size();
        end = buffer + buflength;
      }

      // If still not the right header, give up
      if (strncmp(buffer, CIV3, 4)) {
        std::cerr << filename << " was not a valid civ 3 sav file." << std::endl;
      }

      this->settiledata();
      this->setleaderdata();
      this->seteventdata();

    } else {
      std::cerr << "There was an error opening file " << filename << std::endl;
      std::cerr << "Check to be sure the input argument is a valid filename" << std::endl;
    }

    in.close();
  };

  void SAV::settiledata() {
    gotostring(WRLD);
    // This can't be the most elegant way to extract integers from chars
    mapwidth = (*(buffer + 42) & 255);
    mapheight = (*(buffer + 22) & 255);
    // Parse the wrapping bitmap
    char wrapflags = *(buffer + 178);
    wrapx = (bool)(wrapflags & 1);
    wrapy = (bool)(wrapflags & 2);
    // Every other tile location is empty, so total tile count is half of the width * height
    tilecount = mapwidth * mapheight / 2;
    tiles = new Tile[tilecount];

    // The section header "TILE" actually appears 4 times for every 1 tile, so only extract every fourth
    for (int i = 0; i < tilecount * 4; i++) {
      buffer++;
      gotostring(TILE);
      if (i % 4 == 0) {
        int t = i / 4;
        tiles[t] = Tile(t, *(buffer + 21), *(buffer + 20), mapwidth);
      };
    };
  };

  void SAV::setleaderdata() {
    // Put the players in the right order. Player 0 is an empty player. Player 1 is always the human in singleplayer
    while (gotostring(LEAD)) {
      leaders[*(buffer + 8)] = *(buffer + 12);
      buffer++;
    };
  };

  void SAV::seteventdata() {
    int turn = 1;
    // Ignore RPLE events that come before the RPLS section by jumping the buffer past it
    while (gotostring(RPLS)) {
      buffer++;
    };

    // Each RPLE from here on marks a game event, though only some are tile update events
    char* rple = findstring(RPLE, buffer);
    if (rple != NULL) {
      // Each RPLT marks a new turn, which is used to determine the turn number of RPLE events
      char* rplt = findstring(RPLT, rple);

      eventhead = new Event(turn, rple);
      Event* e = eventhead;

      while (rple != NULL) {
        while (rple > rplt && rplt != NULL) {
          rplt = findstring(RPLT, ++rplt);
          turn++;
        }

        // If event type is a tile change
        if (*(rple + 8) == 6) {
          e->next = new Event(turn, rple);
          e = e->next;
        }

        rple = findstring(RPLE, ++rple);
      }
    }
  };

  char* SAV::findstring(char* search, char* pointer) {
    int i = 0;
    while (pointer < end - 4) { // Make sure we don't go past the end of the char buffer. That would be bad
      if (strncmp(pointer, search, 4) == 0) {
        return pointer;
      }
      pointer++;
    }
    return NULL;
  };

  bool SAV::gotostring(char* search) {
    auto index = findstring(search, buffer);
    if (index == NULL) {
      return 0;
    }
    buffer = index;
    return 1;
  };

};
