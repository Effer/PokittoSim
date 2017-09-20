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

short getBuildHeight(short x,short y)
{
    short tile=board[x][y];
    switch (tile)
    {
    case TILE_LEVEL_1:
        return 1;
    case TILE_LEVEL_2:
        return 2;
    case TILE_LEVEL_3:
        return 3;
    case TILE_LEVEL_4:
        return 4;
    default:
        return 0;
    }
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
    short piece=pieces[x][y];
    return (playerTurn==piece) || (playerTurn==0 && piece!=0);
}

bool isMoveAllowed(Point from,Point to)
{
    short fromLevel=getBuildHeight(from.x,from.y);
    short toLevel=getBuildHeight(to.x,to.y);

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


bool isBuildAllowed(Point where)
{
    bool insideBoard=where.x>0 && where.x<6 && where.y>0 && where.y<6;

    bool boardOK=board[where.x][where.y]==TILE_TERRAIN ||
                 board[where.x][where.y]==TILE_LEVEL_1 ||
                 board[where.x][where.y]==TILE_LEVEL_2 ||
                 board[where.x][where.y]==TILE_LEVEL_3;

    bool pieceOK=pieces[where.x][where.y]==0;

    return insideBoard && boardOK && pieceOK;
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
                        if(isMoveAllowed(from,to))
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

bool anyCanMoveThere(short playerTurn,Point there,Point * who)
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
                        to.x=there.x+xt;
                        to.y=there.y+yt;
                        if(isMoveAllowed(from,to))
                        {
                            who->x=from.x;
                            who->y=from.y;
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool findMovement(short fromLevel,short toLevel,Point * from,Point * to)
{
    short bLevelFrom;
    short bLevelTo;
    for(short x=1; x<BOARDSIZE-1; x++)
    {
        for(short y=1; y<BOARDSIZE-1; y++)
        {
            from->x=x;
            from->y=y;
            bLevelFrom=board[x][y];

            //Search players pieces
            if (isSelecteable(0,x,y))
            {
                //look surround pieces
                for(short xi=1; xi<BOARDSIZE-1; xi++)
                {
                    for(short yi=1; yi<BOARDSIZE-1; yi++)
                    {
                        to->x=xi;
                        to->y=yi;
                        bLevelTo=board[xi][yi];

                        if(isNear(*from,*to))
                        {
                            if ((bLevelFrom==fromLevel && bLevelTo==toLevel))
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
    //find all and take nearest != actual position
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

Point searchReacheableNear(short playerTurn,Point from,Point to)
{
    //Search all points around destination if any is allowed, return it
    Point mto;
    mto.x=-1;
    mto.y=-1;
    for(short xt=-1; xt<2; xt++)
    {
        for(short yt=-1; yt<2; yt++)
        {
            mto.x=to.x+xt;
            mto.y=to.y+yt;
            if(isMoveAllowed(from,mto))
            {
                mto.x=mto.x;
                mto.y=mto.y;
                return mto;
            }
        }
    }
    return mto;
}

Point findHighestBuild()
{
    Point highestPoint;
    short maxHeight=-1;
    short heigth=0;

    for(short x=0; x<BOARDSIZE; x++)
    {
        for(short y=0; y<BOARDSIZE; y++)
        {
            heigth=getBuildHeight(x,y);
            if(heigth>maxHeight)
            {
                maxHeight=heigth;
                highestPoint.x=x;
                highestPoint.y=y;
            }
        }
    }
    return highestPoint;
}

Point moveTowards(Point from,Point to)
{
    //get direction
    short dx=to.x-from.x;
    short dy=to.y-from.y;

    dx=clip<short>(dx,-1,1);
    dy=clip<short>(dy,-1,1);
    //

    Point towards;
    towards.x=clip(from.x+dx,1,5);
    towards.y=clip(from.y+dy,1,5);
    return towards;
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

void movePiece(Point from,Point to)
{
    pieces[from.x][from.y]=0;
    pieces[to.x][to.y]=playerTurn;
    selected.x=to.x;
    selected.y=to.y;
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
    Point towards;
    Point who;

    //Prepare some useful data
    //Look if a piece can move at top
    Point winFrom;
    Point winTo;
    bool win=findMovement(TILE_LEVEL_2,TILE_LEVEL_3,&winFrom,&winTo);
    short winPiece=-1;
    if(win)
    {
        winPiece=pieces[winFrom.x][winFrom.y];
    }

    Point highestBuild=findHighestBuild();


    //-----------------------------------------------------
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
                    return;
                }
            }
        }
    }

    //-----------------------------------------------------
    if(gameState==Move)
    {
        //Try to win moving at level 3

        //AI can move to win?
        if(win && winPiece==playerTurn)
        {
            movePiece(winFrom,winTo);
            printf("(m1) move to win\n");
            return;
        }

        //Find if I can reach any tiles around win position (to later build there)
        if(win && winPiece!=playerTurn)
        {
            Point nearWin;
            for(short xt=-1; xt<2; xt++)
            {
                for(short yt=-1; yt<2; yt++)
                {
                    nearWin.x=winTo.x+xt;
                    nearWin.y=winTo.y+yt;
                    if(anyCanMoveThere(playerTurn,nearWin,&who))
                    {
                        movePiece(who,nearWin);
                        printf("(m2) move near win pos\n");
                        return;
                    }
                }
            }
        }


        //Move near highest build
        Point nearestPiece=getNearestPiece(playerTurn,highestBuild);
        towards= moveTowards(nearestPiece,highestBuild);
        Point nearHighest;
        for(short xt=-1; xt<2; xt++)
        {
            for(short yt=-1; yt<2; yt++)
            {
                nearHighest.x=winTo.x+xt;
                nearHighest.y=winTo.y+yt;
                if(anyCanMoveThere(playerTurn,nearHighest,&who))
                {
                    movePiece(who,nearHighest);
                    printf("(m3) move near high pos\n");
                    return;
                }
            }
        }


        //Try a valid random move
        while(true)
        {
            nearestPiece=getNearestPiece(playerTurn,nearestPiece);
            Point randMove;
            randMove.x=nearestPiece.x+random(-1,1);
            randMove.y=nearestPiece.y+random(-1,1);
            if(isMoveAllowed(nearestPiece,randMove))
            {
                movePiece(nearestPiece,randMove);
                printf("(m4) random move\n");
                return;
            }
        }
    }

    //-----------------------------------------------------
    if(gameState==Build)
    {
        //if opponent can go to level 3 -> Build a Dome

        //if opponent can go to level 3 try to block it building a dome
        if(win && winPiece!=playerTurn)
        {
            if(isNear(selected,winTo))
            {
                board[winTo.x][winTo.y]+=1;
                refresh();
                printf("(b1) block with a dome\n");
                return;
            }
        }

        //Build to reach higher build

        //avoid to build a win position for the opponent
        //---->>>>>>


        //Build random around this piece
        while(true)
        {
            Point randBuild;
            randBuild.x=selected.x+random(-1,1);
            randBuild.y=selected.y+random(-1,1);
            if(isBuildAllowed(randBuild))
            {
                board[randBuild.x][randBuild.y]+=1;
                refresh();
                printf("(b2) build random\n");
                return;
            }
        }

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
        refresh();
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
        gameState=Build;
        refresh();
        simulator.waitSDL(1000);
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
                if(isMoveAllowed(selected,cursor))
                {
                    movePiece(selected,cursor);
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
    if(useAI && playerTurn==2)
    {
        AI();
        nextTurn();
        gameState=Move;
        resetSelection();

        //Move cursor to nearest
        cursor=getNearestPiece(playerTurn,cursor);

        refresh();
        simulator.waitSDL(1000);
    }
    else
    {
        moveCursor(true);
        //First select, then build if possible
        if (game.buttons.pressed(BTN_A))
        {
            if(isNear(selected,cursor) && //only near
                    isBuildAllowed(cursor)) //and valid build tile
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
                zOff=getBuildHeight(x,y);
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
            else
                game.display.print(0,10,board[selected.x][selected.y]);
            //debug info


        }
    }

    return 1;
}
