#ifdef __no_sdl__ //define for archs where is no SDL
class display
{
public:
  void init(int gui_enabled = 1) {}
  void drawNetwork(Network &) {}
  void drawEnvironment(Environment &) {}
  void toggleVideo(Environment &) {}
}; //display
#else

#include <SDL/SDL.h>
#include "./classes/timeStructure.h"
#include "./classes/network.h"
#include "./classes/outputLogService.h"

//these are used just to initialize the cachen network. they should never actually be used
TimeStructure ts;
OutputLogService ls(std::string("SdlFolder"), std::string("1"), ts);

class display
{
  SDL_Surface *screen; //NULL signals that we don't use GUI at all

public:
  bool view_screen; //whether the movie is on or off
  int delay_;       //delay between movie steps
  Network cachen;   //cache to avoid unnecessary painting
  Environment cachee;
  bool cache_dirty; //is cache different from new state?
  display() : screen(NULL), cachen(ls), cachee(ls){};

  int marginy, marginx; //margin for environment paint

  void init(int gui_enabled = 1)
  {
    if (!gui_enabled)
      return;

    view_screen = 1;
    delay_ = 5;
    int width = 1500;
    int height = 600;
    int bpp = 32;

    marginx = 10;
    marginy = 20;

    assert(SDL_Init(SDL_INIT_EVERYTHING) != -1);
    assert((screen = SDL_SetVideoMode(width, height, bpp, SDL_SWSURFACE)) != NULL);
    //SDL_WM_SetCaption("Hello world", NULL);

    for (int i = 0; i < cachee.size; i++)
      for (int j = 0; j < cachee.size; j++)
        cachee.env[i][j] = -10;
    for (int z = 0; z < 3; z++)
      for (int x = 0; x < layer_size[z]; x++)
        for (int y = 0; y < layer_size[z]; y++)
          cachen.get_cell(x, y, z).voltage = -10;

  } //init

  //draws rectangle onto screen
  void rect(short int x, short int y, short unsigned int w, short unsigned int h, int color)
  {
    SDL_Rect rect = {x, y, w, h};
    SDL_FillRect(screen, &rect, color);
  }
  bool cached(int l, int x, int y, int colo)
  { //is the new value equal to cached one; avoid painting
    if (cachen.get_cell(x, y, l).voltage == colo)
      return true;
    cachen.get_cell(x, y, l).voltage = colo;
    cache_dirty = 1;
    return false;
  }
  void drawNetwork(Network &n)
  {
    if (!screen || !view_screen)
      return;
    cache_dirty = 0;
    int colo;
    int i;
    int j;
    int v;
    if (delay_ < 0)
      return; //negative delay let only environment movie
    for (i = 0; i < n.input_size; i++)
    {
      for (j = 0; j < n.input_size; j++)
      {
        v = 256 * (n.input_layer[i][j].voltage + 1) / 2;
        if (v < -1)
          v = -1;
        colo = v * 256 * 256 + 255 - v;
        if (cached(0, i, j, colo))
          continue;
        rect((550 + i * 10), (20 + j * 10), 9, 9, colo);
      }
    }
    for (i = 0; i < n.middle_size; i++)
    {
      for (j = 0; j < n.middle_size; j++)
      {
        v = 256 * (n.middle_layer[i][j].voltage + 1) / 2;
        if (v < -1)
          v = -1;
        colo = v * 256 * 256 + 255 - v;
        if (cached(1, i, j, colo))
          continue;
        rect((640 + i * 10), (20 + j * 10), 9, 9, colo);
      }
    }
    for (i = 0; i < n.output_size; i++)
    {
      for (j = 0; j < n.output_size; j++)
      {
        v = 256 * (n.output_layer[i][j].voltage + 1) / 2;
        if (v < -1)
          v = -1;
        colo = v * 256 * 256 + 255 - v;
        if (cached(2, i, j, colo))
          continue;
        rect((940 + i * 10), (20 + j * 10), 9, 9, colo);
      }
    }
    //toggleVideo();
    if (!cache_dirty)
      return;
    SDL_Flip(screen);
    delay();
  }
  bool cached(int x, int y, int colo)
  { //is the new value equal to cached one; avoid painting
    if (cachee.env[x][y] == colo)
      return true;
    cachee.env[x][y] = colo;
    cache_dirty = 1;
    return false;
  }
  void drawEnvironment(Environment &e)
  {
    if (!screen || !view_screen)
      return;
    cache_dirty = 0;
    int colo;
    int i;
    int j;
    for (i = 0; i < e.size; i++)
    {
      for (j = 0; j < e.size; j++)
      {
        colo = 255 * 256;
        if (e.env[i][j] == 0)
          colo = 255 * 256 * 256;
        if (e.critter.position.x == i and e.critter.position.y == j)
          colo = 255;
        if (cached(i, j, colo))
          continue;
        rect((marginx + i * 10), (marginy + j * 10), 9, 9, colo);
      }
    } //(X,Y,L,H,C)(0-255 bluescale)(0-255*256 green)(0-255*256*256 red)

    //rect(60,20,49,49,500);//(X,Y,L,H,C)
    //toggleVideo();
    //rect(10,20,49,49,2221);
    if (!cache_dirty)
      return;
    SDL_Flip(screen); //update screen
    delay();
  }
  void delay() { delay(abs(delay_)); }
  void delay(int t) { SDL_Delay(t < 0 ? 0 : t); }
  //toggle video on/off based on key press 'v'
  void toggleVideo(Environment &e)
  {
    if (!screen)
      return;
    SDL_Event event;
    string key;
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_KEYDOWN:
        key = SDL_GetKeyName(event.key.keysym.sym);
        if (key == "m")
        {
          if (view_screen)
          {
            cerr << "movie off.\n";
            view_screen = 0;
          }
          else if (!view_screen)
          {
            cerr << "movie on.\n";
            view_screen = 1;
          }
        }
        if (key == "q")
          exit(0);
        if (key == "+" || key == "=")
          cerr << "delay " << ++delay_ << "\n"; //shift is not seen this way
        if (key == "-")
          cerr << "delay " << --delay_ << "\n";
        if (key == "s")
        {
          e.starving = !e.starving;
          cerr << "starving " << e.starving << "\n";
        }
        //printf("The %s key was pressed!\n",SDL_GetKeyName(event.key.keysym.sym));
        break;
      case SDL_MOUSEBUTTONDOWN:
      {
        int x = event.button.x - marginx;
        int y = event.button.y - marginy;
        if (x < 0 || y < 0)
          break;
        x = x / 10;
        y = y / 10;
        e.env[x][y] = 1;
        break;
      }
      case SDL_QUIT:
        exit(0);
      }
    }
  }
}; //display
#endif
