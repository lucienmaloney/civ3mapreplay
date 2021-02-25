#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include "civ3.h"

namespace Civ3 {
  class Player {
    public:
      std::string name;
      sf::Color primarycolor; // The civ's in-game color, roughly
      sf::Color secondarycolor; // A lighter shade of the primary color to mark border changes

      Player(std::string, unsigned int, unsigned int);
  };

  Player::Player(std::string name, unsigned int hexcolor1, unsigned int hexcolor2) {
    this->name = name;
    this->primarycolor = sf::Color(hexcolor1);
    this->secondarycolor = sf::Color(hexcolor2);
  }

  // 32 bit color. 1st byte red, 2nd byte green, 3rd byte blue, 4th byte alpha
  const Player PLAYERS[] = {
    Player("", 0xffffffff, 0xffffffff),
    Player("Rome", 0xe30a0aff, 0xe33636ff),
    Player("Egypt", 0xe6e600ff, 0xf4f402ff),
    Player("Greece", 0x78dd00ff, 0x83ef02ff),
    Player("Babylon", 0x002792ff, 0x0039d8ff),
    Player("Germany", 0x3463ffff, 0x5980ffff),
    Player("Russia", 0x906a06ff, 0xad7f05ff),
    Player("China", 0xf3c2fcff, 0xf7dbfcff),
    Player("America", 0x1dd8daff, 0x5ed9daff),
    Player("Japan", 0x485b2cff, 0x5d7538ff),
    Player("France", 0xff4eedff, 0xff75f1ff),
    Player("India", 0x9f6fe8ff, 0xb687ffff),
    Player("Persia", 0x0e8797ff, 0x0facc1ff),
    Player("Aztecs", 0x077607ff, 0x0b910bff),
    Player("Zulu", 0x3c3c3cff, 0x575757ff),
    Player("Iroquois", 0x79009cff, 0x801f9cff),
    Player("England", 0xff8c00ff, 0xffa333ff),
    Player("Mongols", 0xbdc844ff, 0xc2c880ff),
    Player("Spain", 0xacd9eaff, 0xc5e0eaff),
    Player("Vikings", 0xc6a7edff, 0xd3beedff),
    Player("Ottomans", 0xbba12dff, 0xd3b632ff),
    Player("Celts", 0x75ffadff, 0xa8ffcbff),
    Player("Arabia", 0xfbc7ffff, 0xfde0ffff),
    Player("Carthage", 0x6f530aff, 0x8e6a0bff),
    Player("Korea", 0x7e00ffff, 0x9126ffff),
    Player("Sumeria", 0x728637ff, 0x90aa44ff),
    Player("Hittites", 0xffc000ff, 0xffd040ff),
    Player("Netherlands", 0xff6c00ff, 0xff8226ff),
    Player("Portugal", 0xc000ffff, 0xcd33ffff),
    Player("Byzantines", 0x940808ff, 0x942525ff),
    Player("Inca", 0xa35ca5ff, 0xba6fbcff),
    Player("Maya", 0xaf8652ff, 0xaf926dff),
  };

  class MapRender {
    SAV* sav;
    // The user flags
    int xo;
    int yo;
    int framerate;
    bool stretch;
    bool loop;
    // The viewport pixel dimensions
    int mapwidth;
    int mapheight;

    // It would be nice if some of these were local to their dependent functions, but having certain
    // SFML variables go out of scope causes rendering problems
    sf::RenderWindow* window;
    sf::Sprite* mapsprite;
    sf::Texture texture;
    sf::Sprite sprite;
    sf::RenderTexture map;
    sf::ConvexShape diamond;
    sf::Event event;
    sf::View view;

    // Stuff for correctly positioning the map within the overall window
    float scalex;
    float scaley;
    float posx;
    float posy;

    void setmapsprite();
    void handleresize();
    void handleevents();
    void render();
    int posmod(int, int);
    sf::Vector2f mapposition(int, int);
    // A template because we need to be able to pass both Sprites and Shapes, which inherit from both
    // sf::Drawable and sf::Transformable, and I don't know enouogh about c++ types to make that work
    // God, I miss you Julia
    template<typename T> void draw(sf::RenderTexture*, T*, int, int);

    public:
      MapRender(SAV*, int, int, int, bool, bool);
  };

  MapRender::MapRender(SAV* sav, int xoffset, int yoffset, int framerate, bool stretch, bool loop) {
    this->sav = sav;
    // Remove negatives from offsets
    this->xo = posmod(xoffset, sav->mapwidth * 2);
    this->yo = posmod(yoffset, sav->mapheight * 2);
    this->framerate = framerate;
    this->stretch = stretch;
    this->loop = loop;
    this->mapwidth = (sav->mapwidth + sav->wrapx - 1) * TILEHEIGHT;
    this->mapheight = (sav->mapheight / 2 + sav->wrapy - 1) * TILEHEIGHT;
    window = new sf::RenderWindow(sf::VideoMode::getDesktopMode(), "Civ 3 Enhanced Replays");
    if (!texture.loadFromFile("tiles.png")) {
      std::cerr << "Error loading texture. Does the file 'tiles.png' exist in the current directory?" << std::endl;
    } else {
      // Setting up the tile shape. Should this be in its own function? Eh, whatever
      diamond.setPointCount(4);
      diamond.setPoint(0, sf::Vector2f(0, TILEHEIGHT / 2));
      diamond.setPoint(1, sf::Vector2f(TILEWIDTH / 2, 0));
      diamond.setPoint(2, sf::Vector2f(TILEWIDTH, TILEHEIGHT / 2));
      diamond.setPoint(3, sf::Vector2f(TILEWIDTH / 2, TILEHEIGHT));

      setmapsprite();

      render();
    }
  }

  int MapRender::posmod(int a, int b) {
    return ((a % b) + b) % b;
  }

  sf::Vector2f MapRender::mapposition(int x, int y) {
    // Get the x and y drawing position in pixels from the tile x and y
    float xpos = (x - 1) * TILEWIDTH / 2;
    float ypos = (y - 1) * TILEHEIGHT / 2;
    return sf::Vector2f(xpos, ypos);
  }

  template<typename T> void MapRender::draw(sf::RenderTexture* canvas, T* sprite, int x, int y) {
    // Draw the sprite, taking into account the user offsets for positioning
    int ax = (x + xo) % sav->mapwidth;
    int ay = (y + yo) % sav->mapheight;
    sprite->setPosition(mapposition(ax, ay));
    canvas->draw(*sprite);

    // If the map wraps in the x or y direction, tiles on left and upper edges respectivelly need
    // to be drawn again on the right or lower edges because they get split down the middle
    if (sav->wrapx && ax == 0) {
      ax += sav->mapwidth;
      sprite->setPosition(mapposition(ax, ay));
      canvas->draw(*sprite);
    }
    if (sav->wrapy && ay == 0) {
      ay += sav->mapheight;
      sprite->setPosition(mapposition(ax, ay));
      canvas->draw(*sprite);
    }
  }

  void MapRender::setmapsprite() {
    sprite.setTexture(texture);

    map.create(mapwidth, mapheight);
    map.clear(sf::Color::Transparent);

    Civ3::Tile t;
    // For each tile, get its texture and draw it onto the map RenderTexture
    for (int i = 0; i < sav->tilecount; i++) {
      t = sav->tiles[i];
      sprite.setTextureRect(sf::IntRect(t.texturex, t.texturey, TILEWIDTH, TILEHEIGHT));
      draw(&map, &sprite, t.x, t.y);
    }

    map.display();
    mapsprite = new sf::Sprite(map.getTexture());
  }

  void MapRender::handleresize() {
    view = window->getDefaultView();
    view.reset(sf::FloatRect(0.f, 0.f, event.size.width, event.size.height));
    window->setView(view);

    // Determine the correct factor to scale by so that the contents doesn't overflow either horizontally or vertically
    sf::Vector2u size = window->getSize();
    scalex = size.x * 1.f / mapwidth;
    scaley = size.y * 1.f / mapheight;
    float scale = std::min(scalex, scaley);

    if (!stretch) {
      // Make the dimension scaling uniform unless the user specified stretching
      scalex = scale;
      scaley = scale;
    }

    posx = (size.x - mapwidth * scalex) / 2;
    posy = (size.y - mapheight * scaley) / 2;
  }

  void MapRender::handleevents() {
    while (window->pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window->close();
      }

      if (event.type == sf::Event::Resized) {
        handleresize();
      }
    }
  }

  void MapRender::render() {
    window->setFramerateLimit(framerate);
    Event* e = sav->eventhead;
    Event* e2 = sav->eventhead;

    sf::RenderTexture tiles;
    tiles.create(mapwidth, mapheight);
    tiles.clear(sf::Color::White);

    int turn = 0;
    while (window->isOpen()) {
      handleevents();

      window->clear(sf::Color::Black);

      // This loop updates the tiles as they change with the lighter color
      while (e != NULL && e->turn - 1 < turn) {
        diamond.setFillColor(PLAYERS[sav->leaders[e->civ]].secondarycolor);
        draw(&tiles, &diamond, e->x, e->y);
        e = e->next;
      }

      // This loop updates the tiles one turn after they change in a darker color to imply permanence
      // Going for the video map type effect
      while (e2 != NULL && e2->turn < turn) {
        diamond.setFillColor(PLAYERS[sav->leaders[e2->civ]].primarycolor);
        draw(&tiles, &diamond, e2->x, e2->y);
        e2 = e2->next;
      }

      tiles.display();
      turn++;

      sf::Sprite tilessprite(tiles.getTexture());

      mapsprite->setScale(sf::Vector2f(scalex, scaley));
      mapsprite->setPosition(sf::Vector2f(posx, posy));
      tilessprite.setScale(sf::Vector2f(scalex, scaley));
      tilessprite.setPosition(sf::Vector2f(posx, posy));

      window->draw(tilessprite);
      window->draw(*mapsprite);

      window->display();

      // If the user specified looping, reset everything after reaching the end of the linked list
      if (e == NULL && loop) {
        tiles.clear(sf::Color::White);
        e = sav->eventhead;
        e2 = sav->eventhead;
        turn = 0;
      }
    }
  }
};
