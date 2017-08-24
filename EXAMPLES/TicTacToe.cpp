#include "Pokitto.h"
#include <stdlib.h>
#include <map>

#define CELL_SIZE 16
#define SIGN_SIZE 12
#define CURSOR_SIZE 16
#define MAX_GRID_SIZE 10

#define MAX_PLAYERS 4
#define MAX_MENU_ITEMS 3

Pokitto::Core game;
Pokitto::Display disp;
Pokitto::Sound snd;
Pokitto::Buttons btn;

typedef enum GameStates
{
    Menu=0,
    InitGame,
    Move,
    Check,
    Winner,
    Drawn,
    Credits
} GameStates;

typedef enum Symbol
{
    None=0,
    Cross,
    Circle,
    Triangle,
    Diamond,
} Symbol;

typedef struct
{
    short x;
    short y;
} Point;

typedef struct
{
    bool winner;
    Point from;
    Point to;
    Symbol symb;
} WinnerLine;

typedef struct
{
    Symbol symb;
    short x;
    short y;
    short diameter=SIGN_SIZE;
    short time;
    float angle;
} Sign;

typedef struct Player
{
    bool AI;
    Symbol symb;
} Player;

typedef struct MenuItem
{
    char * description;
    int minValue;
    int maxValue;
    int value;
} MenuItem;

std::map<std::string,MenuItem> menuMap;

MenuItem mainMenu[MAX_MENU_ITEMS];
char menuIndex=0;

char setBoardSize=3;
char setSigns2Win=3;
char setNumPlayers=2;

Sign board[MAX_GRID_SIZE][MAX_GRID_SIZE];
Point cursor;

Player players[MAX_PLAYERS];
char playerTurn=0;
WinnerLine winnerLine;
GameStates gameState;
Point boardPosition = {.x=5,.y=5};

Sign scoreList[10];
char scoreListIndx=0;

uint32_t timer;


int clip(int n, int lower, int upper)
{
    if (n<lower) return lower;
    if (n>upper) return upper;
    return n;
}

inline float lerpf(float a, float b, float t)
{
    return a + (b - a) * t/100.0;
}

void LoadMenu()
{
    MenuItem menuSize;
    menuSize.description="Board Size";
    menuSize.minValue=2;
    menuSize.maxValue=10;
    menuSize.value=3;
    mainMenu[0]=menuSize;
    menuMap[menuSize.description]=menuSize;

    MenuItem menuSigns2Win;
    menuSigns2Win.description="Signs to win";
    menuSigns2Win.minValue=2;
    menuSigns2Win.maxValue=5;
    menuSigns2Win.value=3;
    mainMenu[1]=menuSigns2Win;
    menuMap[menuSigns2Win.description]=menuSigns2Win;

    MenuItem menuNumPlayers;
    menuNumPlayers.description="Players";
    menuNumPlayers.minValue=2;
    menuNumPlayers.maxValue=4;
    menuNumPlayers.value=2;
    mainMenu[2]=menuNumPlayers;
    menuMap[menuNumPlayers.description]=menuNumPlayers;
}

void drawMenuItem(short x,short y,MenuItem item)
{
    game.display.print(x,y,item.description);
    game.display.print(x+80,y,(int)item.value);
}

void drawMenu()
{
    short xo=10;
    short yo=10;

    //draw cursor
    game.display.print(0,yo+(menuIndex*10),">");
    //draw menu items
    for (char m=0; m<MAX_MENU_ITEMS; m++)
    {
        drawMenuItem(xo,yo,mainMenu[m]);
        yo+=10;
    }

    //std::map<std::string,MenuItem>::iterator it;
    //for (it = menuMap.begin(); it != menuMap.end(); it++ )
    //{
    //    drawMenuItem(xo,yo,it->second);
    //     yo+=10;
    //}
}

void drawPolygon (short xc, short yc, short r, short n,float a)
{
    short lastx;
    short lasty;
    a-=PI/2;
    short x = round ((float)xc + (float)r * cos(a));
    short y= round ((float)yc + (float)r * sin(a));

    for (int i = 0; i < n; i++)
    {
        lastx = x;
        lasty = y;
        a = a + (2 * PI / n);
        x = round ((float)xc + (float)r * cos(a));
        y = round ((float)yc + (float)r * sin(a));
        game.display.drawLine(lastx, lasty, x, y);
    }
}

void drawCross (short xc, short yc, short r, short n,float a)
{
    short x;
    short y;
    a-=PI/4;
    for (int i = 0; i < n; i++)
    {
        x = round ((float)xc + (float)r * cos(a));
        y = round ((float)yc + (float)r * sin(a));
        a = a + (2 * PI / n);
        game.display.drawLine(xc, yc, x, y);
    }
}

void drawSign(Sign symb)
{
    if (symb.symb==Cross)
    {
        disp.color=1;
        drawCross(symb.x,symb.y,symb.diameter/2,4,symb.angle);
    }

    if (symb.symb==Circle)
    {
        disp.color=2;
        disp.drawCircle(symb.x,symb.y,(symb.diameter/2)*2*PI/symb.angle);
    }

    if (symb.symb==Triangle)
    {
        disp.color=3;
        drawPolygon(symb.x,symb.y,symb.diameter/2,3,symb.angle);
    }

    if (symb.symb==Diamond)
    {
        disp.color=1;
        drawPolygon(symb.x,symb.y,symb.diameter/2,4,symb.angle);
    }
}

void drawWinningLine(char xo,char yo)
{
    if(winnerLine.winner)
    {
        disp.color=7;
        disp.drawLine(xo+winnerLine.from.x * CELL_SIZE + CELL_SIZE/2,
                      yo+winnerLine.from.y * CELL_SIZE+ CELL_SIZE/2,
                      xo+winnerLine.to.x * CELL_SIZE+ CELL_SIZE/2,
                      yo+winnerLine.to.y * CELL_SIZE+ CELL_SIZE/2);
    }
}

void drawGameType()
{
    disp.print((int)setBoardSize);
    disp.print("x");
    disp.print((int)setBoardSize);
    disp.print(" ");
    disp.print((int)setNumPlayers);
    disp.print("P ");
    disp.print((int)setSigns2Win);
    disp.print(" to win");
}

void drawBoard(short xo,short yo)
{
    short xc;
    short yc;

    //Vertical lines
    disp.color=1;
    for (char x=1; x<setBoardSize; x++)
    {
        disp.drawLine(xo+(CELL_SIZE*x),yo,xo+(CELL_SIZE*x),yo+(CELL_SIZE*(setBoardSize)));
    }
    //Horizontal lines
    for (char y=1; y<setBoardSize; y++)
    {
        disp.drawLine(xo,yo+(CELL_SIZE*y),xo+CELL_SIZE*(setBoardSize),yo+(CELL_SIZE*y));
    }

    //Draw Cell
    for (char x=0; x<setBoardSize; x++)
    {
        for (char y=0; y<setBoardSize; y++)
        {
            //apply update
            if ( board[x][y].time<100) board[x][y].time+=5;
            board[x][y].angle=lerpf(0,2*PI,board[x][y].time);

            //Calculate center of the cell
            xc=xo+(x*CELL_SIZE)+(CELL_SIZE/2);
            yc=yo+(y*CELL_SIZE)+(CELL_SIZE/2);
            board[x][y].x=xc;
            board[x][y].y=yc;

            //Draw cursor
            if (cursor.x==x && cursor.y==y)
            {
                disp.color=7;
                drawPolygon(xc,yc,CURSOR_SIZE/2,4,-PI/4);
            }

            //Draw symb of the board
            drawSign(board[x][y]);
        }
    }

    //Winner line
    drawWinningLine(xo,yo);

    //draw winner list
    for (int i=0; i<10; i++)
    {
        drawSign(scoreList[i]);
    }

}


void initGame()
{
    game.display.setFont(fontMini);

    boardPosition.x=((game.display.width-20)-(setBoardSize*CELL_SIZE))/2;
    boardPosition.y=6+((game.display.height-6)-(setBoardSize*CELL_SIZE))/2;

    //
    players[0].symb=Cross;
    players[0].AI=true;

    players[1].symb=Diamond;
    players[1].AI=true;

    players[2].symb=Triangle;
    players[2].AI=true;

    players[3].symb=Circle;
    players[3].AI=true;

    //Initialize cursor position
    cursor.x=floor(setBoardSize/2);
    cursor.y=floor(setBoardSize/2);

    //reset winnerLine
    winnerLine.winner=false;

    //Clear board
    for (char x=0; x<setBoardSize; x++)
    {
        for (char y=0; y<setBoardSize; y++)
        {
            board[x][y].symb=None;
        }
    }

    //Begin game
    gameState=Move;
}

void updateMenu()
{
    game.display.setFont(font5x7);

    //Scroll the menu settings
    if (btn.pressed(BTN_DOWN)) menuIndex++;
    if (btn.pressed(BTN_UP)) menuIndex--;
    menuIndex=clip(menuIndex,0,MAX_MENU_ITEMS-1);

    //Change menu values
    if (btn.pressed(BTN_RIGHT))mainMenu[menuIndex].value++;
    if (btn.pressed(BTN_LEFT))mainMenu[menuIndex].value--;
    mainMenu[menuIndex].value=clip(mainMenu[menuIndex].value,mainMenu[menuIndex].minValue,mainMenu[menuIndex].maxValue);

    //Start game
    if (btn.pressed(BTN_A))
    {
        setBoardSize=mainMenu[0].value;
        setSigns2Win=mainMenu[1].value;
        setNumPlayers=mainMenu[2].value;
        gameState=InitGame;
    }

    //Randomize data
    if(btn.pressed((BTN_B)))
    {
        mainMenu[0].value=random(3,5); //board
        mainMenu[1].value=random(2,mainMenu[0].value); //signs to win
        mainMenu[2].value=random(2,4);//players
    }
    drawMenu();
}


bool isWinner(Sign originalBoard[MAX_GRID_SIZE][MAX_GRID_SIZE],Symbol symb,short xm=-1,short ym=-1)
{
    short xc;
    short yc;

    //Copy board to test
    Sign boardTest[setBoardSize][setBoardSize];
    for (char x=0; x<setBoardSize; x++)
    {
        for (char y=0; y<setBoardSize; y++)
        {
            boardTest[x][y]=originalBoard[x][y];
        }
    }

    //If a move is request
    if(xm>-1 && ym>-1)
    {
        //place the symbol, if we can
        if(boardTest[xm][ym].symb==None)
        {
            boardTest[xm][ym].symb=symb;
        }
    }

    //NOW check all winning conditions
    bool win;
    for (char x=0; x<setBoardSize; x++)
    {
        for (char y=0; y<setBoardSize; y++)
        {
            //horizontal win
            win=true;
            for (char c=0; c<setSigns2Win; c++ )
            {
                xc=c+x;
                yc=y;
                if(xc>=0 && xc<setBoardSize && yc>=0 && yc<setBoardSize)
                    win=win && boardTest[xc][yc].symb==symb;
                else
                    win=false;
            }

            if(win)
            {
                winnerLine.winner=true;
                winnerLine.from.x=x;
                winnerLine.from.y=y;
                winnerLine.to.x=x+setSigns2Win-1;
                winnerLine.to.y=y;
                winnerLine.symb=symb;
                return true;
            }

            //vertical win
            win=true;
            for (char c=0; c<setSigns2Win; c++ )
            {
                xc=x;
                yc=c+y;
                if(xc>=0 && xc<setBoardSize && yc>=0 && yc<setBoardSize)
                    win=win && boardTest[xc][yc].symb==symb;
                else
                    win=false;
            }

            if(win)
            {
                winnerLine.winner=true;
                winnerLine.from.x=x;
                winnerLine.from.y=y;
                winnerLine.to.x=x;
                winnerLine.to.y=y+setSigns2Win-1;
                winnerLine.symb=symb;
                return true;
            }

            //diagonal top/right win
            win=true;
            for (char c=0; c<setSigns2Win; c++ )
            {
                xc=c+x;
                yc=c+y;
                if(xc>=0 && xc<setBoardSize && yc>=0 && yc<setBoardSize)
                    win=win && boardTest[xc][yc].symb==symb;
                else
                    win=false;
            }

            if(win)
            {
                winnerLine.winner=true;
                winnerLine.from.x=x;
                winnerLine.from.y=y;
                winnerLine.to.x=x+setSigns2Win-1;
                winnerLine.to.y=y+setSigns2Win-1;
                winnerLine.symb=symb;
                return true;
            }

            //diagonal top/left win
            win=true;
            for (char c=0; c<setSigns2Win; c++ )
            {
                xc=-c+x;
                yc=c+y;
                if(xc>=0 && xc<setBoardSize && yc>=0 && yc<setBoardSize)
                    win=win && boardTest[xc][yc].symb==symb;
                else
                    win=false;
            }

            if(win)
            {
                winnerLine.winner=true;
                winnerLine.from.x=x;
                winnerLine.from.y=y;
                winnerLine.to.x=x-setSigns2Win+1;
                winnerLine.to.y=y+setSigns2Win-1;
                winnerLine.symb=symb;
                return true;
            }
        }
    }

    winnerLine.winner=false;
    return false;
}

bool isDrawn(Sign originalBoard[MAX_GRID_SIZE][MAX_GRID_SIZE])
{
    //All cells not None
    for (char x=0; x<setBoardSize; x++)
    {
        for (char y=0; y<setBoardSize; y++)
        {
            if (originalBoard[x][y].symb==None)
            {
                return false;
            }
        }
    }
    return true;
}


void AI(Symbol symb)
{
    //Check if I can win
    for (char x=0; x<setBoardSize; x++)
    {
        for (char y=0; y<setBoardSize; y++)
        {
            if (isWinner(board,symb,x,y))
            {
                //Place winning move
                board[x][y].time=0;
                board[x][y].symb=symb;
                cursor.x=x;
                cursor.y=y;
                return;
            }
        }
    }

    //Check if anyone can win with a move
    for(char p=0; p<setNumPlayers; p++)
    {
        for (char x=0; x<setBoardSize; x++)
        {
            for (char y=0; y<setBoardSize; y++)
            {
                if (isWinner(board,players[p].symb,x,y))
                {
                    //Place winning move
                    board[x][y].time=0;
                    winnerLine.winner=false; //Please FIXME!! Avoid drawing winning line when not need
                    board[x][y].symb=symb;
                    cursor.x=x;
                    cursor.y=y;
                    return;
                }
            }
        }
    }


    //Check if I can win in 2 moves ???


    //place a random move
    while(true)
    {
        char x=random(0,setBoardSize-1);
        char y=random(0,setBoardSize-1);
        if(board[x][y].symb==None)
        {
            board[x][y].time=0;
            board[x][y].symb=symb;
            cursor.x=x;
            cursor.y=y;
            return;
        }
    }
}

void updateGame()
{
    //Move
    if(players[playerTurn].AI)
    {
        AI(players[playerTurn].symb);
        gameState=Check;
    }
    else
    {
        if (btn.pressed(BTN_UP)) cursor.y-=1;
        if (btn.pressed(BTN_DOWN)) cursor.y+=1;
        if (btn.pressed(BTN_LEFT)) cursor.x-=1;
        if (btn.pressed(BTN_RIGHT)) cursor.x+=1;

        //Limit cursor to the board
        cursor.x=abs(cursor.x%setBoardSize);
        cursor.y=abs(cursor.y%setBoardSize);

        if (btn.pressed(BTN_A))
        {
            if (board[cursor.x][cursor.y].symb==None)
            {
                board[cursor.x][cursor.y].time=0;
                board[cursor.x][cursor.y].symb=players[playerTurn].symb;
                gameState=Check;
            }
        }
    }

    //Draw player symbol at top/right corner
    Sign playerSign;
    playerSign.symb=players[playerTurn].symb;
    playerSign.x=game.display.width-(SIGN_SIZE/2);
    playerSign.y=SIGN_SIZE/2;
    drawSign(playerSign);

    //Draw
    drawBoard(boardPosition.x,boardPosition.y);
    drawGameType();
}

void updateCheck()
{
    if(isWinner(board,players[playerTurn].symb))
    {
        cursor.x=-1;
        cursor.y=-1;
        gameState=Winner;
        timer=game.getTime()+1000;
    }
    else if (isDrawn(board))
    {
        cursor.x=-1;
        cursor.y=-1;
        gameState=Drawn;
        timer=game.getTime()+1000;
    }
    else
    {
        playerTurn++;
        playerTurn=playerTurn%setNumPlayers;
        gameState=Move;
    }

    //Draw
    drawBoard(boardPosition.x,boardPosition.y);
    drawGameType();
}

void updateWin()
{
    drawBoard(boardPosition.x,boardPosition.y);
    disp.println("There's a winner!");

    if (btn.pressed(BTN_A) || game.getTime()>timer)
    {
        //move list symbols
        for (int i=0; i<10; i++)
        {
            scoreList[i].y+=SIGN_SIZE/2;
        }

        //add winner symbol
        Sign winSign;
        winSign.x=game.display.width-(SIGN_SIZE/2);
        winSign.y=SIGN_SIZE*2;
        winSign.symb=winnerLine.symb;
        winSign.diameter=SIGN_SIZE/2;
        scoreList[scoreListIndx]=winSign;
        scoreListIndx=(scoreListIndx+1)%10;

        gameState=InitGame;
    }

}

void updateDrawn()
{
    drawBoard(boardPosition.x,boardPosition.y);
    disp.print("It's a drawn");

    if (btn.pressed(BTN_A) || game.getTime()>timer)
    {
        gameState=InitGame;
    }


}


int main ()
{

    LoadMenu();

    game.begin();
    //game.display.loadRGBPalette(paletteDB16);


    while (game.isRunning())
    {
        if (game.update())
        {
            //Exit
            if(btn.pressed((BTN_C)))
            {
                gameState=Menu;
            }

            switch (gameState)
            {
            case Menu:
                updateMenu();
                break;
            case InitGame:
                initGame();
                break;
            case Move:
                updateGame();
                break;
            case Check:
                updateCheck();
                break;
            case Winner:
                updateWin();
                break;
            case Drawn:
                updateDrawn();
                break;
            default:
                break;
            }
        }
    }

    return 1;
}
