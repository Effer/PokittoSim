#include "Pokitto.h"
#include "Santorini.h"

Pokitto::Core game;

#define TILE_WIDTH 16
#define TILE_HEIGHT 16

#define TILE_TERRAIN 0
#define TILE_LEVEL_1 1
#define TILE_LEVEL_2 2
#define TILE_LEVEL_3 3
#define TILE_LEVEL_4 4
#define TILE_SEA 5

#define TILE_P1_LEVEL_1 6
#define TILE_P1_LEVEL_2 7
#define TILE_P1_LEVEL_3 8

#define TILE_P2_LEVEL_1 9
#define TILE_P2_LEVEL_2 10
#define TILE_P2_LEVEL_3 11

#define TILE_CURSOR_P1 12
#define TILE_CURSOR_P2 13

#define TILE_CURSOR_P1_SEL  14
#define TILE_CURSOR_P2_SEL  15

#define BOARDSIZE 7

uint16_t board[BOARDSIZE][BOARDSIZE];
uint16_t pieces[BOARDSIZE][BOARDSIZE];
bool refresh=true;

typedef struct
{
    int16_t x;
    int16_t y;
} Point;

Point cursor;
Point selected;
Point destination;
short playerTurn=1;

typedef enum  GameStates
{
    Intro,
    InitGame,
    Move,
    MoveCheck,
    Build,
    BuildCheck,
    Credits
} GameStates;
GameStates gameState;


bool anyKey()
{
    for (char i; i<NUM_BTN; i++)
    {
        if (game.buttons.states[i]!=0)
            return true;
    }
    return false;
}

void toIso(Point * pp)
{
    int16_t x=pp->x;
    int16_t y=pp->y;
    pp->x=x-y;
    pp->y=(x+y)/2;

    //Center screen
    pp->x+=(game.display.width/2-TILE_WIDTH);
    pp->y+=(game.display.height/12);
}

void to2d(Point * pp)
{
    int16_t x=pp->x;
    int16_t y=pp->y;
    pp->x=(2*x+y)/2;
    pp->y=(2*y-x)/2;
}

bool areNeighbor(Point p1,Point p2)
{
    short dx=p1.x-p2.x;
    short dy=p1.y-p2.y;

    bool tl=(dx==1) && (dy==1);
    bool tc=(dx==0) && (dy==1);
    bool tr=(dx==-1) && (dy==1);

    bool cl=(dx==1) && (dy==0);
    bool cr=(dx==-1) && (dy==0);

    bool bl=(dx==1) && (dy==-1);
    bool bc=(dx==0) && (dy==-1);
    bool br=(dx==-1) && (dy==-1);

    //Only neighbor cells are allowed
    return (tl || tc || tr || cl || cr || bl || bc || br);
}

void initBoard()
{
    cursor.x=1;
    cursor.y=1;
    selected.x=-1;
    selected.y=-1;

    //Players tiles
    char x;
    char y;
    char players[]= {TILE_P1_LEVEL_1,TILE_P1_LEVEL_1,TILE_P2_LEVEL_1,TILE_P2_LEVEL_1};
    playerTurn=random(1,2);

    //Reset pieces
    for(uint8_t x=0; x<BOARDSIZE; x++)
    {
        for(uint8_t y=0; y<BOARDSIZE; y++)
        {
            pieces[x][y]=0;
        }
    }
    //Place random
    for (char i=0; i<4; i++)
    {
        while(true)
        {
            x=random(1,BOARDSIZE-2);
            y=random(1,BOARDSIZE-2);

            if(pieces[x][y]==0)
            {
                pieces[x][y]=players[i];
                break;
            }
        }
    }

    //
    for(uint8_t x=0; x<BOARDSIZE; x++)
    {
        for(uint8_t y=0; y<BOARDSIZE; y++)
        {
            board[x][y]=TILE_TERRAIN;
            if (x==0 || y==0 || x==BOARDSIZE-1 || y==BOARDSIZE-1) board[x][y]=TILE_SEA;
        }
    }
}

void moveCursor()
{
    if (game.buttons.pressed(BTN_UP))
    {
        cursor.x-=1;
        refresh=true;
    }

    if (game.buttons.pressed(BTN_DOWN))
    {
        cursor.x+=1;
        refresh=true;
    }

    if (game.buttons.pressed(BTN_LEFT))
    {
        cursor.y+=1;
        refresh=true;
    }

    if (game.buttons.pressed(BTN_RIGHT))
    {
        cursor.y-=1;
        refresh=true;
    }
}

void moveRules()
{
    moveCursor();

    //First select, then move if possible
    if (game.buttons.pressed(BTN_A))
    {
        //Check  if I'm selecting a piece I can move
        short piece=pieces[cursor.x][cursor.y];
        if((playerTurn==1 &&(piece==TILE_P1_LEVEL_1 || piece==TILE_P1_LEVEL_1 || piece==TILE_P1_LEVEL_1)) ||
                (playerTurn==2 &&(piece==TILE_P2_LEVEL_1 || piece==TILE_P2_LEVEL_1 || piece==TILE_P2_LEVEL_1)))
        {
            selected.x=cursor.x;
            selected.y=cursor.y;
        }
        else
        {
            //Move piece
            if(areNeighbor(selected,cursor) && //only near
                    pieces[cursor.x][cursor.y]==0) //and free
            {
                //copy piece
                pieces[cursor.x][cursor.y]=pieces[selected.x][selected.y];
                //delete old one
                pieces[selected.x][selected.y]=0;

                //Select new position
                selected.x=cursor.x;
                selected.y=cursor.y;

                //Goto to build phase
                gameState=Build;
            }
            else
            {
                //Reset selection
                selected.x=-1;
                selected.y=-1;
            }


        }
        refresh=true;
    }

}

void buildRules()
{
    moveCursor();

    //First select, then build if possible
    if (game.buttons.pressed(BTN_A))
    {
        short piece=pieces[cursor.x][cursor.y];
        short tile=board[cursor.x][cursor.y];
        bool validBuildTile=(tile==TILE_TERRAIN || tile==TILE_LEVEL_1 || tile==TILE_LEVEL_2 || tile==TILE_LEVEL_3);


        if(areNeighbor(selected,cursor) && //only near
                pieces[cursor.x][cursor.y]==0 && //and free
                validBuildTile ) //and valid build tile
        {
            board[cursor.x][cursor.y]+=1;

            playerTurn=playerTurn==1?2:1;
            gameState=Move;

            //deselect
            selected.x=-1;
            selected.y=-1;
        }
          refresh=true;
    }

}

void drawBoard()
{
    Point p;
    uint8_t boardTile=0;
    uint8_t pieceTile=0;
    int16_t zOff=0;

    game.display.invisiblecolor=0;

    for(uint8_t x=0; x<BOARDSIZE; x++)
    {
        for(uint8_t y=0; y<BOARDSIZE; y++)
        {
            boardTile=board[x][y];
            pieceTile=pieces[x][y];

            //Calc drawing coordinates
            p.x=x*TILE_WIDTH;
            p.y=y*TILE_HEIGHT;
            toIso(&p);

            //Draw Tile
            game.display.directBitmap(p.x,p.y,sprites[boardTile],4,1);

            //Draw pieces
            if (pieceTile!=0)
            {
                zOff=0;
                if(boardTile==TILE_LEVEL_2) zOff=1;
                if(boardTile==TILE_LEVEL_3) zOff=2;
                game.display.directBitmap(p.x,p.y,sprites[pieceTile+zOff],4,1);
            }

            //Draw Cursor
            if (x==cursor.x && y==cursor.y)
            {
                if (playerTurn==1)
                    game.display.directBitmap(p.x,p.y,sprites[TILE_CURSOR_P1],4,1);
                else
                    game.display.directBitmap(p.x,p.y,sprites[TILE_CURSOR_P2],4,1);
            }

            //Draw selection
            if (x==selected.x && y==selected.y)
            {
                if (playerTurn==1)
                    game.display.directBitmap(p.x,p.y,sprites[TILE_CURSOR_P1_SEL],4,1);
                else
                    game.display.directBitmap(p.x,p.y,sprites[TILE_CURSOR_P2_SEL],4,1);

            }
        }
    }
    game.display.invisiblecolor=-1;
}

void drawIntro()
{
    game.display.clear();

    int x=random(0,game.display.width);
    int y=random(0,game.display.height);
    int w=random(0,game.display.width);
    int h=random(0,game.display.height);
    int c=random(65535);
    game.display.directRectangle(x,y,w,h,c);

    game.display.invisiblecolor=0;
    game.display.directBitmap(60,60,sprites[TILE_LEVEL_3],4,1);

    if (anyKey())
        gameState=Move;

    game.display.print(game.display.width/2,game.display.height/2, "Press any key..");
}

void drawGame()
{
    if(refresh)
    {
        refresh=false;
        //Clear screen
        game.display.directRectangle(0,0,game.display.width,game.display.height,0);
        //Draw
        drawBoard();
    }
}

int main ()
{
    game.begin();

    //load sprite palette
    game.display.load565Palette(sprite_pal);
    game.display.enableDirectPrinting(1);

    initBoard();

    while (game.isRunning())
    {
        if (game.update(true))
        {


            switch(gameState)
            {
            case Intro:
                drawIntro();
                break;
            case Move:
                moveRules();
                drawGame();
                break;
            case MoveCheck:
                break;
            case Build:
                buildRules();
                drawGame();
                break;
            case BuildCheck:
                break;
            case Credits:
                break;
            }



            game.display.print(0,0,gameState);

        }
    }

    return 1;
}
