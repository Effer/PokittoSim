#include "Pokitto.h"

#define CELL_SIZE 16
#define SIGN_SIZE 12
#define CURSOR_SIZE 16
#define MAX_GRID_SIZE 10

Pokitto::Core game;
Pokitto::Display disp;
Pokitto::Sound snd;
Pokitto::Buttons btn;

typedef enum GameStates
{
    Menu=0,
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
    char x;
    char y;
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
    char x;
    char y;
    short diameter=SIGN_SIZE;
    short time;
    short angle;
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
}MenuItem;

MenuItem menuSize;
MenuItem menuSigns2Win;
MenuItem menuNumPlayers;

char setBoardSize=3;
char setSigns2Win=3;
char setNumPlayers=2;

Sign board[MAX_GRID_SIZE][MAX_GRID_SIZE];
Point cursor;

Player players[4];
char playerTurn=0;
WinnerLine winnerLine;
GameStates gameState;
Point boardPosition = {.x=5,.y=5};

Sign scoreList[10];
char scoreListIndx=0;

uint32_t timer;

void init()
{
    menuSize.description="Grid Size";
    menuSize.minValue=2;
    menuSize.maxValue=5;
    menuSize.value=3;

    menuSigns2Win.description="Signs to win";
    menuSigns2Win.minValue=2;
    menuSigns2Win.maxValue=5;
    menuSigns2Win.value=3;

    menuNumPlayers.description="# Players";
    menuNumPlayers.minValue=2;
    menuNumPlayers.maxValue=4;
    menuNumPlayers.value=2;
}

void drawMenuItem(char x,char y,MenuItem item)
{
    game.display.print(x,y,item.description);
    game.display.print(x+50,y,(int)item.value);
}

void drawMenu()
{
    char xo=10;
    char yo=20;
    drawMenuItem(xo,yo,menuSize);
    yo+=10;
    drawMenuItem(xo,yo,menuSigns2Win);
    yo+=10;
    drawMenuItem(xo,yo,menuNumPlayers);
}

void drawPolygon (short xc, short yc, short r, short n,float a)
{
    short lastx;
    short lasty;
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
        disp.color=5;
        drawCross(symb.x,symb.y,symb.diameter/2,4,-PI/4);
    }

    if (symb.symb==Circle)
    {
        disp.color=6;
        disp.drawCircle(symb.x,symb.y,symb.diameter/2);
    }

    if (symb.symb==Triangle)
    {
        disp.color=10;
        drawPolygon(symb.x,symb.y,symb.diameter/2,3,-PI/2);
    }

    if (symb.symb==Diamond)
    {
        disp.color=3;
        drawPolygon(symb.x,symb.y,symb.diameter/2,4,-PI/2);
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

void drawBoard(char xo,char yo)
{
    char xc;
    char yc;

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
    setBoardSize=random(3,5);
    setNumPlayers=random(2,4);
    setSigns2Win=random(2,setBoardSize);

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

    //Initialize board
    for (char x=0; x<setBoardSize; x++)
    {
        for (char y=0; y<setBoardSize; y++)
        {
            board[x][y].symb=None;
        }
    }
}

void updateMenu()
{
    if (btn.pressed(BTN_A))
    {
        initGame();
        gameState=Move;
    }

    drawMenu();
}


bool isWinner(Sign originalBoard[MAX_GRID_SIZE][MAX_GRID_SIZE],Symbol symb,char xm=-1,char ym=-1)
{
    char xc;
    char yc;

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
                board[cursor.x][cursor.y].symb=players[playerTurn].symb;
                gameState=Check;
            }
        }
    }

    //Draw player symb
    Sign playerSign;
    playerSign.symb=players[playerTurn].symb;
    playerSign.x=100;
    playerSign.y=10;
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
        winSign.x=100;
        winSign.y=20;
        winSign.symb=winnerLine.symb;
        winSign.diameter=SIGN_SIZE/2;
        scoreList[scoreListIndx]=winSign;
        scoreListIndx=(scoreListIndx+1)%10;

        gameState=Menu;
    }
}

void updateDrawn()
{
    drawBoard(boardPosition.x,boardPosition.y);

    disp.print("It's a drawn");

    if (btn.pressed(BTN_A) || game.getTime()>timer)
    {
        gameState=Menu;
    }
}


int main ()
{
    init();

    game.begin();
    //game.display.loadRGBPalette(paletteDB16);
    game.display.setFont(fontMini);

    while (game.isRunning())
    {
        if (game.update())
        {
            switch (gameState)
            {
            case Menu:
                updateMenu();
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
