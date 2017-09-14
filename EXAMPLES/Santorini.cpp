#define DEBUG

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
#define TILE_P1_LEVEL_4 9

#define TILE_P2_LEVEL_1 10
#define TILE_P2_LEVEL_2 11
#define TILE_P2_LEVEL_3 12
#define TILE_P2_LEVEL_4 13

#define TILE_CURSOR_P1 14
#define TILE_CURSOR_P2 15

#define TILE_CURSOR_P1_SEL  16
#define TILE_CURSOR_P2_SEL  17

#define BOARDSIZE 7


bool useAI=true;
uint16_t intro[BOARDSIZE][BOARDSIZE];
uint16_t board[BOARDSIZE][BOARDSIZE];
uint16_t pieces[BOARDSIZE][BOARDSIZE];
bool _refresh=true;

typedef struct
{
    int16_t x;
    int16_t y;
} Point;

typedef struct
{
    Point from;
    Point to;
} Movement;

Point cursor;
Point selected;
Point destination;
short playerTurn=1;

typedef enum  GameStates
{
    Intro,
    InitGame,
    Place,
    Move,
    Build,
    End,
    Credits
} GameStates;
GameStates gameState;

const char * stateDescription[] =
{
    "Intro",
    "Initialize",
    "Place",
    "Move",
    "Build",
    "End",
};


template <typename T>
T clip(T n, T lower, T upper)
{
    if (n<lower) return lower;
    if (n>upper) return upper;
    return n;
}

template <typename T>
T lerp(T a, T b, T t)
{
    return (a + (b - a) * t/100.0);
}


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

short distance(short x1,short y1,short x2, short y2)
{
    short dx=x1-x2;
    short dy=y1-y2;
    return (short)((dx*dx)+(dy*dy));
}

bool isNear(Point p1,Point p2)
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

short getBoardHeight(short x,short y)
{
    short tile=board[x][y];
    if(tile==TILE_LEVEL_1) return 1;
    if(tile==TILE_LEVEL_2) return 2;
    if(tile==TILE_LEVEL_3) return 3;
    if(tile==TILE_LEVEL_4) return 4;
    return 0;
}

void resetSelection()
{
    selected.x=-1;
    selected.y=-1;
}

bool isSelected()
{
    return (selected.x!=-1) && (selected.y!=-1);
}

bool isSelecteable(short playerTurn,short x,short y)
{
    bool selecteableP1;
    bool selecteableP2;
    short piece=pieces[x][y];

    selecteableP1 = (playerTurn==0 || playerTurn==1) && (piece==1);
    selecteableP2 = (playerTurn==0 || playerTurn==2) && (piece==2);

    return selecteableP1 || selecteableP2;
}

bool moveAllowed(short playerTurn,Point from,Point to)
{
    short fromLevel=getBoardHeight(from.x,from.y);
    short toLevel=getBoardHeight(to.x,to.y);

    short dZ=toLevel-fromLevel;
    //Move piece
    if(isNear(from,to) && //near
            pieces[to.x][to.y]==0 && //free
            dZ<2 && //max 1 level up
            toLevel!=TILE_LEVEL_4 && //not blocked
            to.x>0 && to.x<6 && to.y>0 && to.y<6) //inside board
    {
        return true;
    }
    return false;
}

bool anyMoveAllowed(short playerTurn)
{
    Point from;
    Point to;
    for(short x=0; x<BOARDSIZE; x++)
    {
        for(short y=0; y<BOARDSIZE; y++)
        {
            //Search players pieces
            if (isSelecteable(playerTurn,x,y))
            {
                from.x=x;
                from.y=y;
                //Check if can do any move around
                for(short xt=-1; xt<2; xt++)
                {
                    for(short yt=-1; yt<2; yt++)
                    {
                        to.x=x+xt;
                        to.y=y+yt;
                        if(moveAllowed(playerTurn,from,to))
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool findWinMove(Movement winMove)
{
    short fromLevel;
    short toLevel;
    for(short x=0; x<BOARDSIZE; x++)
    {
        for(short y=0; y<BOARDSIZE; y++)
        {
            winMove.from.x=x;
            winMove.from.y=y;
            fromLevel=board[x][y];

            //Search players pieces
            if (isSelecteable(0,x,y))
            {
                //look surround pieces
                for(short xi=0; xi<BOARDSIZE; xi++)
                {
                    for(short yi=0; yi<BOARDSIZE; yi++)
                    {
                        winMove.to.x=xi;
                        winMove.to.y=yi;
                        toLevel=board[xi][yi];

                        if(isNear(winMove.from,winMove.to))
                        {
                            if ((toLevel==4 && fromLevel==3))
                            {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

Point getNearestPiece(short playerTurn,Point from)
{
    //Search around for
    //find all and take nearest
    short bestDistance=9999;
    short d=0;
    Point nearest;
    nearest.x=from.x;
    nearest.y=from.y;

    for(short x=0; x<BOARDSIZE; x++)
    {
        for(short y=0; y<BOARDSIZE; y++)
        {
            if((x!=from.x || y!=from.y) && isSelecteable(playerTurn,x,y))
            {
                d=distance(x,y,from.x,from.y);
                if(d<bestDistance)
                {
                    bestDistance=d;
                    nearest.x=x;
                    nearest.y=y;
                }
            }
        }
    }
    return nearest;
}

short countPlayerPieces()
{
    //Count players pieces
    short c=0;
    for(short x=0; x<BOARDSIZE; x++)
    {
        for(short y=0; y<BOARDSIZE; y++)
        {
            if (pieces[x][y]!=0)
            {
                c++;
            }
        }
    }

    return c;
}

short nextTurn()
{
    playerTurn=playerTurn==1?2:1;
}

void refresh()
{
    _refresh=true;
}

void AI()
{
    short x;
    short y;

    if(gameState==Place)
    {
        //Place near other players
        if(countPlayerPieces()>1)
        {
            Point pp=getNearestPiece(random(1,2),selected);

            while(true)
            {
                x=clip(pp.x+random(-2,2),1,5);
                y=clip(pp.y+random(-2,2),1,5);
                if (pieces[x][y]==0)
                {
                    pieces[x][y] = playerTurn;
                    refresh();
                    return;
                }
            }
        }
        else
        {
            //Place random
            while(true)
            {
                x=random(1,5);
                y=random(1,5);
                if (pieces[x][y]==0)
                {
                    pieces[x][y] = playerTurn;
                    refresh();
                    return;
                }
            }
        }

    }


    if(gameState==Move)
    {
        //Try to win moving at level 4
        //Look if a piece can move at top
        Movement winMove;
        bool win=findWinMove(winMove);
        short winPiece=-1;
        if(win)
        {
            winPiece=pieces[winMove.from.x][winMove.from.y];

        }

        //AI can move to win?
        if(win && winPiece==playerTurn)
        {
            pieces[winMove.from.x][winMove.from.y]=0;
            pieces[winMove.to.x][winMove.to.y]=playerTurn;
            refresh();
            return;
        }

        //if opponent can go to level 4 -> move near to later block it
        if(win && winPiece!=playerTurn)
        {

            refresh();
            return;
        }

        //Move near higher build
    }

    if(gameState==Build)
    {
        //if opponent can go to level 4 -> Build a Dome

        //Build to reach higher build

    }



}

void initBoard()
{
    cursor.x=1;
    cursor.y=1;
    resetSelection();

    //Players tiles
    char x;
    char y;
    char players[]= {1,1,2,2};
    playerTurn=random(1,2);

    //Reset pieces
    for(uint8_t x=0; x<BOARDSIZE; x++)
    {
        for(uint8_t y=0; y<BOARDSIZE; y++)
        {
            pieces[x][y]=0;
        }
    }

//    //Place random
//    for (char i=0; i<4; i++)
//    {
//        while(true)
//        {
//            x=random(1,BOARDSIZE-2);
//            y=random(1,BOARDSIZE-2);
//
//            if(pieces[x][y]==0)
//            {
//                pieces[x][y]=players[i];
//                break;
//            }
//        }
//    }

    //Initialize board with flat land
    for(uint8_t x=0; x<BOARDSIZE; x++)
    {
        for(uint8_t y=0; y<BOARDSIZE; y++)
        {
            //Land
            board[x][y]=TILE_TERRAIN;
            //Sea at boarder
            if (x==0 || y==0 || x==BOARDSIZE-1 || y==BOARDSIZE-1)
                board[x][y]=TILE_SEA;
        }
    }

    //Select player
    cursor=getNearestPiece(playerTurn,cursor);
}

void moveCursorSnap()
{
    if (game.buttons.pressed(BTN_UP) ||
            game.buttons.pressed(BTN_DOWN) ||
            game.buttons.pressed(BTN_LEFT) ||
            game.buttons.pressed(BTN_RIGHT))
    {
        cursor=getNearestPiece(playerTurn,cursor);
        refresh();
    }
}

void moveCursor(bool onlyNearSelected)
{

    Point newCursor;
    newCursor.x=cursor.x;
    newCursor.y=cursor.y;

    if (game.buttons.pressed(BTN_UP))
    {
        newCursor.x-=1;
        refresh();
    }

    if (game.buttons.pressed(BTN_DOWN))
    {
        newCursor.x+=1;
        refresh();
    }

    if (game.buttons.pressed(BTN_LEFT))
    {
        newCursor.y+=1;
        refresh();
    }

    if (game.buttons.pressed(BTN_RIGHT))
    {
        newCursor.y-=1;
        refresh();
    }

    //Limit cursor inside board
    if (newCursor.x<1)newCursor.x=1;
    if (newCursor.x>5)newCursor.x=5;
    if (newCursor.y<1)newCursor.y=1;
    if (newCursor.y>5)newCursor.y=5;

    //Allow move only near the selection
    if(onlyNearSelected)
    {
        if(!isNear(selected,newCursor))
        {
            //Restore old position
            newCursor.x=cursor.x;
            newCursor.y=cursor.y;
        }
    }

    if(refresh)
    {
        cursor.x=newCursor.x;
        cursor.y=newCursor.y;
    }

#ifdef DEBUG
    if (game.buttons.pressed(BTN_C))
    {
        board[cursor.x][cursor.y]+=1;
        refresh();
    }
#endif // DEBUG
}

void placeRules()
{
    if(useAI && playerTurn==2)
    {
        AI();
        nextTurn();
    }
    else
    {
        moveCursor(false);

        if (game.buttons.pressed(BTN_A))
        {
            pieces[cursor.x][cursor.y]=playerTurn;
            nextTurn();
            refresh();
        }
    }

    //all pieces dropped --> change state
    if (countPlayerPieces()>=4)
    {
        cursor=getNearestPiece(playerTurn,cursor);
        gameState=Move;
    }
}


void moveRules()
{
    if(useAI && playerTurn==2)
    {
        AI();
        nextTurn();
    }
    else
    {

        //free move if selected, otherwise snap to players
        if(isSelected())
        {
            moveCursor(true);
        }
        else
        {
            moveCursorSnap();
        }

        //First select, then move if possible
        if (game.buttons.pressed(BTN_A))
        {
            //Check  if I'm selecting a piece I can move
            if(isSelecteable(playerTurn,cursor.x,cursor.y))
            {
                selected.x=cursor.x;
                selected.y=cursor.y;
            }
            else
            {
                if(moveAllowed(playerTurn,selected,cursor))
                {
                    //copy piece
                    pieces[cursor.x][cursor.y]=pieces[selected.x][selected.y];
                    //delete old one
                    pieces[selected.x][selected.y]=0;

                    //Select new position
                    selected.x=cursor.x;
                    selected.y=cursor.y;

                    //Next game state
                    gameState=Build;
                }
                else
                {
                    //Reset selection
                    resetSelection();
                }
            }
            refresh();
        }

        //Reset selection
        if (game.buttons.pressed(BTN_B))
        {
            resetSelection();
            refresh();
        }
    }

    //Check if player can't move or reach top (win)
    if(!anyMoveAllowed(playerTurn) || board[selected.x][selected.y]==TILE_LEVEL_3)
    {
        gameState=End;
    }
}


void buildRules()
{
    moveCursor(true);

    //First select, then build if possible
    if (game.buttons.pressed(BTN_A))
    {
        short piece=pieces[cursor.x][cursor.y];
        short tile=board[cursor.x][cursor.y];
        bool validBuildTile=(tile==TILE_TERRAIN || tile==TILE_LEVEL_1 || tile==TILE_LEVEL_2 || tile==TILE_LEVEL_3);

        if(isNear(selected,cursor) && //only near
                pieces[cursor.x][cursor.y]==0 && //and free
                validBuildTile ) //and valid build tile
        {
            //Build up
            board[cursor.x][cursor.y]+=1;

            //Next turn
            nextTurn();
            gameState=Move;

            //select none
            resetSelection();

            //Move cursor to nearest
            cursor=getNearestPiece(playerTurn,cursor);
        }
        refresh();
    }
}

void drawBoard()
{
    Point p;
    short boardTile=0;
    short pieceTile=0;
    short zOff=0;

    game.display.invisiblecolor=0;

    for(short x=0; x<BOARDSIZE; x++)
    {
        for(short y=0; y<BOARDSIZE; y++)
        {
            boardTile=board[x][y];

            pieceTile=0;
            if(pieces[x][y]==1)
                pieceTile=TILE_P1_LEVEL_1;
            if(pieces[x][y]==2)
                pieceTile=TILE_P2_LEVEL_1;

            //Calculate drawing coordinates
            p.x=x*TILE_WIDTH;
            p.y=y*TILE_HEIGHT;
            toIso(&p);

            //Draw Tile
            game.display.directBitmap(p.x,p.y,sprites[boardTile],4,1);

            //Draw players pieces at correct height
            if (pieceTile!=0)
            {
                zOff=getBoardHeight(x,y);
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
    //game.display.directRectangle(x,y,w,h,c);

    game.display.directRectangle(0,0,game.display.width,game.display.height,0);

    game.display.invisiblecolor=0;
    for(short x=0; x<BOARDSIZE; x++)
    {
        for(short y=0; y<BOARDSIZE; y++)
        {
            Point p;
            p.x=x*TILE_WIDTH;
            p.y=y*TILE_HEIGHT;
            toIso(&p);

            short s=intro[x][y];
            game.display.directBitmap(p.x,p.y,sprites[s],4,1);
        }
    }

    x=random(0,6);
    y=random(0,6);
    intro[x][y]=(intro[x][y]+1)%6;

    game.display.print(game.display.width/2,game.display.height-16, "Press any key..");
    game.display.invisiblecolor=-1;

    if (anyKey())
        gameState=Place;
}

void drawGame()
{
    if(_refresh)
    {
        _refresh=false;
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
            case Place:
                placeRules();
                drawGame();
                break;
            case Move:
                moveRules();
                drawGame();
                break;
            case Build:
                buildRules();
                drawGame();
                break;

            case End:
                drawGame();
                break;
            case Credits:
                break;
            }


            game.display.print(0,0,stateDescription[gameState]);
            if(!isSelected())
                game.display.print(0,10,"select piece");

        }
    }

    return 1;
}
