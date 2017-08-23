#include "Pokitto.h"

#define CELL_SIZE 16
#define SIGN_SIZE 12
#define CURSOR_SIZE 16

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

typedef enum SignType
{
    None=0,
    Cross,
    Circle,
    Triangle,
    Diamond,
} SignType;

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
    SignType sign;
} WinnerLine;

typedef struct
{
    char x;
    char y;
    SignType sign;
    short diameter=SIGN_SIZE;
    short time;
    short angle;
} Sign;

typedef struct Player
{
    bool AI;
    SignType sign;

} Player;

char BOARDSIZE=3;
char NUMPLAYERS=2;
char SIGNS2WIN=3;

Sign board[5][5];
Point cursor;

Player players[4];
char playerTurn=0;
WinnerLine winnerLine;
GameStates gameState;
Point boardPosition= {.x=5,.y=5};

Sign scoreList[10];
char scoreListIndx=0;

uint32_t timer;

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

void drawSign(Sign sign)
{
    if (sign.sign==Cross)
    {
        disp.color=5;
        drawCross(sign.x,sign.y,sign.diameter/2,4,-PI/4);
    }

    if (sign.sign==Circle)
    {
        disp.color=6;
        disp.drawCircle(sign.x,sign.y,sign.diameter/2);
    }

    if (sign.sign==Triangle)
    {
        disp.color=10;
        drawPolygon(sign.x,sign.y,sign.diameter/2,3,-PI/2);
    }

    if (sign.sign==Diamond)
    {
        disp.color=11;
        drawPolygon(sign.x,sign.y,sign.diameter/2,4,-PI/2);
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
    disp.print((int)NUMPLAYERS);
    disp.print("P ");
    disp.print((int)BOARDSIZE);
    disp.print("x");
    disp.print((int)BOARDSIZE);
    disp.print("  ");
    disp.print((int)SIGNS2WIN);
    disp.print(" to win");
}

void drawBoard(char xo,char yo)
{
    char xc;
    char yc;

    //Vertical lines
    disp.color=3;
    for (char x=1; x<BOARDSIZE; x++)
    {
        disp.drawLine(xo+(CELL_SIZE*x),yo,xo+(CELL_SIZE*x),yo+(CELL_SIZE*(BOARDSIZE)));
    }
    //Horizontal lines
    for (char y=1; y<BOARDSIZE; y++)
    {
        disp.drawLine(xo,yo+(CELL_SIZE*y),xo+CELL_SIZE*(BOARDSIZE),yo+(CELL_SIZE*y));
    }

    //Draw Cell
    for (char x=0; x<BOARDSIZE; x++)
    {
        for (char y=0; y<BOARDSIZE; y++)
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

            //Draw sign of the board
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
    BOARDSIZE=random(3,5);
    NUMPLAYERS=random(2,4);
    SIGNS2WIN=random(2,BOARDSIZE);

    //
    players[0].sign=Cross;
    players[0].AI=true;

    players[1].sign=Diamond;
    players[1].AI=true;

    players[2].sign=Triangle;
    players[2].AI=true;

    players[3].sign=Circle;
    players[3].AI=true;

    //Initialize cursor position
    cursor.x=floor(BOARDSIZE/2);
    cursor.y=floor(BOARDSIZE/2);

    //reset winnerLine
    winnerLine.winner=false;

    //Initialize board
    for (char x=0; x<BOARDSIZE; x++)
    {
        for (char y=0; y<BOARDSIZE; y++)
        {
            board[x][y].sign=None;
        }
    }

}

void updateMenu()
{
    if (btn.pressed(BTN_A) || true)
    {
        initGame();
        gameState=Move;
    }
}


bool isWinner(SignType sign,char xm=-1,char ym=-1)
{
    char xc;
    char yc;

    //Copy board to test
    Sign boardTest[BOARDSIZE][BOARDSIZE];
    for (char x=0; x<BOARDSIZE; x++)
    {
        for (char y=0; y<BOARDSIZE; y++)
        {
            boardTest[x][y]=board[x][y];
        }
    }

    //If a move is request
    if(xm>-1 && ym>-1)
    {
        //place the sign, if we can
        if(boardTest[xm][ym].sign==None)
        {
            boardTest[xm][ym].sign=sign;
        }
    }

    //NOW check all winning conditions

    //horizontal win
    for (char x=0; x<BOARDSIZE; x++)
    {
        for (char y=0; y<BOARDSIZE; y++)
        {
            bool win=true;
            for (char c=0; c<SIGNS2WIN; c++ )
            {
                xc=c+x;
                yc=y;
                if(xc>=0 && xc<BOARDSIZE && yc>=0 && yc<BOARDSIZE)
                    win=win && boardTest[xc][yc].sign==sign;
                else
                    win=false;
            }

            if(win)
            {
                winnerLine.winner=true;
                winnerLine.from.x=x;
                winnerLine.from.y=y;
                winnerLine.to.x=x+SIGNS2WIN-1;
                winnerLine.to.y=y;
                winnerLine.sign=sign;
                return true;
            }
        }
    }

    //vertical win
    for (char y=0; y<BOARDSIZE; y++)
    {
        for (char x=0; x<BOARDSIZE; x++)
        {
            bool win=true;
            for (char c=0; c<SIGNS2WIN; c++ )
            {
                xc=x;
                yc=c+y;
                if(xc>=0 && xc<BOARDSIZE && yc>=0 && yc<BOARDSIZE)
                    win=win && boardTest[xc][yc].sign==sign;
                else
                    win=false;
            }

            if(win)
            {
                winnerLine.winner=true;
                winnerLine.from.x=x;
                winnerLine.from.y=0+y;
                winnerLine.to.x=x;
                winnerLine.to.y=y+SIGNS2WIN-1;
                winnerLine.sign=sign;
                return true;
            }
        }
    }

    //diagonal top/right win
    for (char x=0; x<BOARDSIZE; x++)
    {
        for (char y=0; y<BOARDSIZE; y++)
        {
            bool win=true;
            for (char c=0; c<SIGNS2WIN; c++ )
            {
                xc=c+x;
                yc=c+y;
                if(xc>=0 && xc<BOARDSIZE && yc>=0 && yc<BOARDSIZE)
                    win=win && boardTest[xc][yc].sign==sign;
                else
                    win=false;
            }

            if(win)
            {
                winnerLine.winner=true;
                winnerLine.from.x=x;
                winnerLine.from.y=y;
                winnerLine.to.x=x+SIGNS2WIN-1;
                winnerLine.to.y=y+SIGNS2WIN-1;
                winnerLine.sign=sign;
                return true;
            }
        }
    }

    //diagonal top/left win
    for (char x=0; x<BOARDSIZE; x++)
    {
        for (char y=0; y<BOARDSIZE; y++)
        {
            bool win=true;
            for (char c=0; c<SIGNS2WIN; c++ )
            {
                xc=-c+x;
                yc=c+y;
                if(xc>=0 && xc<BOARDSIZE && yc>=0 && yc<BOARDSIZE)
                    win=win && boardTest[xc][yc].sign==sign;
                else
                    win=false;
            }

            if(win)
            {
                winnerLine.winner=true;
                winnerLine.from.x=x;
                winnerLine.from.y=y;
                winnerLine.to.x=x-SIGNS2WIN+1;
                winnerLine.to.y=y+SIGNS2WIN-1;
                winnerLine.sign=sign;
                return true;
            }
        }
    }

    winnerLine.winner=false;
    return false;
}

bool isDrawn()
{
    //All cells not None
    for (char x=0; x<BOARDSIZE; x++)
    {
        for (char y=0; y<BOARDSIZE; y++)
        {
            if (board[x][y].sign==None)
            {
                return false;
            }
        }
    }
    return true;
}


void AI(SignType sign)
{
    //Check if I can win
    for (char x=0; x<BOARDSIZE; x++)
    {
        for (char y=0; y<BOARDSIZE; y++)
        {
            if (isWinner(sign,x,y))
            {
                //Place winning move
                board[x][y].sign=sign;
                cursor.x=x;
                cursor.y=y;
                return;
            }
        }
    }

    //Check if anyone can win
    for(char p=0; p<NUMPLAYERS; p++)
    {
        for (char x=0; x<BOARDSIZE; x++)
        {
            for (char y=0; y<BOARDSIZE; y++)
            {
                if (isWinner(players[p].sign,x,y))
                {
                    //Place winning move
                    winnerLine.winner=false; //Please FIXME!! Avoid drawing winning line when not need
                    board[x][y].sign=sign;
                    cursor.x=x;
                    cursor.y=y;
                    return;
                }
            }
        }
    }

    //place a random move
    while(true)
    {
        char x=random(0,BOARDSIZE-1);
        char y=random(0,BOARDSIZE-1);
        if(board[x][y].sign==None)
        {
            board[x][y].sign=sign;
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
        AI(players[playerTurn].sign);
        gameState=Check;
    }
    else
    {
        if (btn.pressed(BTN_UP)) cursor.y-=1;
        if (btn.pressed(BTN_DOWN)) cursor.y+=1;
        if (btn.pressed(BTN_LEFT)) cursor.x-=1;
        if (btn.pressed(BTN_RIGHT)) cursor.x+=1;

        //Limit cursor to the board
        cursor.x=abs(cursor.x%BOARDSIZE);
        cursor.y=abs(cursor.y%BOARDSIZE);

        if (btn.pressed(BTN_A))
        {
            if (board[cursor.x][cursor.y].sign==None)
            {
                board[cursor.x][cursor.y].sign=players[playerTurn].sign;
                gameState=Check;
            }
        }
    }

    //Draw player sign
    Sign playerSign;
    playerSign.sign=players[playerTurn].sign;
    playerSign.x=100;
    playerSign.y=10;
    drawSign(playerSign);

    //Draw
    drawBoard(boardPosition.x,boardPosition.y);
    drawGameType();
}

void updateCheck()
{
    if(isWinner(players[playerTurn].sign))
    {
        cursor.x=-1;
        cursor.y=-1;
        gameState=Winner;
        timer=game.getTime()+1000;
    }
    else if (isDrawn())
    {
        cursor.x=-1;
        cursor.y=-1;
        gameState=Drawn;
        timer=game.getTime()+1000;
    }
    else
    {
        playerTurn++;
        playerTurn=playerTurn%NUMPLAYERS;
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

        //add winner sign
        Sign winSign;
        winSign.x=100;
        winSign.y=20;
        winSign.sign=winnerLine.sign;
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
