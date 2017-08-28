#include "Pokitto.h"

#define CELL_SIZE 32
#define SIGN_SIZE 20
#define CURSOR_SIZE 36

#define MAX_GRID_SIZE 10

#define MAX_PLAYERS 4
#define MAX_MENU_SETTINGS 7

#define PI2 PI/2.0
#define PI4 PI/4.0
#define PI3_4 PI*3.0/4.0

const uint8_t logo[] =
{
    188,38,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,21,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,21,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,64,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,0,0,0,0,0,0,0,0,0,0,
    0,1,85,85,85,64,0,0,0,0,0,0,0,0,0,0,85,85,85,80,1,0,0,0,0,0,0,0,0,0,0,0,1,85,85,85,64,0,0,0,0,0,0,0,0,0,0,
    0,5,85,85,85,0,85,0,0,5,85,80,0,0,0,1,85,85,85,64,21,64,0,0,0,0,21,85,64,0,0,0,5,85,85,85,0,21,85,0,0,0,5,85,0,0,0,
    0,21,85,85,64,1,85,0,0,21,85,84,0,0,0,5,85,85,80,0,85,80,0,0,0,1,85,85,64,0,0,0,21,85,85,64,0,85,85,80,0,0,85,85,64,0,0,
    0,80,5,80,0,0,85,0,1,80,5,84,0,0,0,20,1,84,0,0,5,84,0,0,0,5,64,85,64,0,0,0,80,5,80,0,1,85,85,84,0,1,80,21,80,0,0,
    0,0,5,64,0,0,21,0,1,64,0,0,0,0,0,0,1,80,0,0,1,85,0,0,0,20,0,0,0,0,0,0,0,5,64,0,5,64,5,85,0,5,64,5,80,0,0,
    0,0,5,64,0,0,21,0,5,0,0,0,0,0,0,0,1,80,0,0,0,85,64,0,0,84,0,0,0,0,0,0,0,5,64,0,5,0,0,85,64,21,0,5,64,0,0,
    0,0,5,64,0,0,21,0,21,0,0,0,0,0,0,0,1,80,0,0,0,21,80,0,0,84,0,0,0,0,0,0,0,5,64,0,21,0,0,21,80,20,0,85,85,84,0,
    0,0,5,64,0,0,21,0,21,0,0,0,0,0,0,0,1,80,0,0,0,21,80,0,0,80,0,0,0,0,0,0,0,5,64,0,21,0,0,5,80,85,85,85,85,80,0,
    0,0,5,64,0,0,21,0,21,0,0,0,0,0,0,0,1,80,0,0,0,85,84,0,1,80,0,0,0,0,0,0,0,5,64,0,21,0,0,5,80,85,85,85,85,64,0,
    0,0,5,64,0,0,21,0,21,0,0,0,0,0,0,0,1,80,0,0,1,65,85,0,1,80,0,0,0,0,0,0,0,5,64,0,21,0,0,1,80,84,0,0,0,0,0,
    0,0,5,64,0,0,21,0,21,0,0,0,0,0,0,0,1,80,0,0,21,0,85,64,1,84,0,0,0,0,0,0,0,5,64,0,21,64,0,1,64,84,0,0,0,0,0,
    0,0,5,64,0,0,21,0,21,0,0,0,0,0,0,0,1,80,0,0,21,0,21,80,1,84,0,0,0,0,0,0,0,5,64,0,21,64,0,1,64,85,0,0,0,0,0,
    0,0,5,64,0,0,21,0,21,64,0,5,0,0,0,0,1,80,0,0,21,0,21,84,0,85,0,0,16,0,0,0,0,5,64,0,21,80,0,5,0,85,64,0,0,0,0,
    0,0,5,65,0,0,21,0,21,80,0,85,0,0,0,0,1,80,64,0,21,64,85,85,0,85,64,1,80,0,0,0,0,5,65,0,5,85,64,20,0,21,80,0,0,0,0,
    0,0,5,85,0,0,21,84,5,85,85,84,0,0,0,0,1,85,64,0,21,85,81,85,64,85,85,85,80,0,0,0,0,5,85,0,1,85,85,80,0,21,85,85,64,0,0,
    0,0,5,84,0,0,21,80,1,85,85,80,0,0,0,0,1,85,0,0,5,85,0,85,0,21,85,85,0,0,0,0,0,5,84,0,0,85,85,64,0,1,85,85,0,0,0,
    0,0,1,80,0,0,21,64,0,21,85,0,0,0,0,0,0,84,0,0,1,80,0,16,0,1,85,80,0,0,0,0,0,1,80,0,0,5,84,0,0,0,85,80,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


const uint8_t AI_logo[] =
{
    20,14,
    0,0,0,0,0,
    0,85,85,85,0,
    1,85,85,85,64,
    1,0,0,0,64,
    1,0,0,0,64,
    1,1,65,64,64,
    1,1,65,64,64,
    1,0,0,0,64,
    1,0,0,0,64,
    1,0,85,0,64,
    1,0,20,0,64,
    1,0,0,0,64,
    1,0,0,0,64,
    1,85,85,85,64,
};


const uint8_t P_logo[] =
{
    20,14,
    0,0,85,0,0,
    0,1,85,64,0,
    0,1,85,64,0,
    0,1,20,64,0,
    0,5,85,80,0,
    0,5,85,80,0,
    0,1,20,64,0,
    0,1,65,64,0,
    0,0,85,0,0,
    0,0,20,0,0,
    0,21,85,84,0,
    0,85,65,85,0,
    0,85,85,85,0,
    0,85,65,85,0,
};


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
    short time;
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
    bool asBool;
} MenuItem;


MenuItem mainMenu[MAX_MENU_SETTINGS];
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

template <typename T>
T lerp(T a, T b, T t)
{
    return a + (b - a) * cos(PI2*t/100.0);
}

void initMenu()
{
    MenuItem menuSize;
    menuSize.description="Board Size (NxN)";
    menuSize.minValue=2;
    menuSize.maxValue=5;
    menuSize.value=3;
    menuSize.asBool=false;
    mainMenu[0]=menuSize;

    MenuItem menuSigns2Win;
    menuSigns2Win.description="Signs to win";
    menuSigns2Win.minValue=2;
    menuSigns2Win.maxValue=5;
    menuSigns2Win.value=3;
    menuSigns2Win.asBool=false;
    mainMenu[1]=menuSigns2Win;

    MenuItem menuNumPlayers;
    menuNumPlayers.description="Number of players";
    menuNumPlayers.minValue=2;
    menuNumPlayers.maxValue=4;
    menuNumPlayers.value=2;
    menuNumPlayers.asBool=false;
    mainMenu[2]=menuNumPlayers;

    MenuItem P1Settings;
    P1Settings.description="Player 1";
    P1Settings.minValue=0;
    P1Settings.maxValue=1;
    P1Settings.value=1;
    P1Settings.asBool=true;
    mainMenu[3]=P1Settings;

    MenuItem P2Settings;
    P2Settings.description="Player 2";
    P2Settings.minValue=0;
    P2Settings.maxValue=1;
    P2Settings.value=1;
    P2Settings.asBool=true;
    mainMenu[4]=P2Settings;

    MenuItem P3Settings;
    P3Settings.description="Player 3";
    P3Settings.minValue=0;
    P3Settings.maxValue=1;
    P3Settings.value=1;
    P3Settings.asBool=true;
    mainMenu[5]=P3Settings;

    MenuItem P4Settings;
    P4Settings.description="Player 4";
    P4Settings.minValue=0;
    P4Settings.maxValue=1;
    P4Settings.value=1;
    P4Settings.asBool=true;
    mainMenu[6]=P4Settings;

}

void drawMenuItem(short x,short y,MenuItem item)
{
    disp.color=1;
    disp.print(x,y,item.description);


    if(item.asBool)
    {
        if (item.value==1)
        {
            disp.drawBitmap(x+112,y-(AI_logo[1]/2)+3,AI_logo);
        }
        else
        {
            disp.drawBitmap(x+112,y-(AI_logo[1]/2)+3,P_logo);
        }

    }
    else
    {
        disp.color=2;
        disp.print(x+120,y,(int)item.value);
    }
}


void drawPolygon (short xc, short yc, short r, short n,float a)
{
    short lastx;
    short lasty;
    a-=PI2;
    short x = round ((float)xc + (float)r * cos(a));
    short y= round ((float)yc + (float)r * sin(a));

    for (int i = 0; i < n; i++)
    {
        lastx = x;
        lasty = y;
        a = a + (2 * PI / n);
        x = round ((float)xc + (float)r * cos(a));
        y = round ((float)yc + (float)r * sin(a));
        disp.drawLine(lastx, lasty, x, y);
    }
}

void drawCross(short xc, short yc, short r, short n,float a)
{
    short x;
    short y;
    a-=PI/4;
    for (int i = 0; i < n; i++)
    {
        x = round ((float)xc + (float)r * cos(a));
        y = round ((float)yc + (float)r * sin(a));
        a = a + (2 * PI / n);
        disp.drawLine(xc, yc, x, y);
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
        disp.drawCircle(symb.x,symb.y,symb.diameter/2-sin(symb.angle)*symb.diameter/5.0);// symb.diameter/2 + 2*PI/symb.angle );
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

void drawRectangle(short x1,short y1,short x2,short y2,double width,double offsetAngle)
{
    Point pp[4];
    //Orignal angle
    double angle=-atan2(y1-y2,x1-x2)+PI2;

    short xc=(x1+x2)/2;
    short yc=(y1+y2)/2;
    double d2=sqrt(((x1-x2)*(x1-x2))+((y1-y2)*(y1-y2)))/2;

    x1=xc+d2*sin(angle+offsetAngle);
    y1=yc+d2*cos(angle+offsetAngle);
    x2=xc+d2*sin(angle+offsetAngle-PI);
    y2=yc+d2*cos(angle+offsetAngle-PI);

    //Recalculate angle after offset applied
    angle=-atan2(y1-y2,x1-x2);

    pp[0].x=x1+width*sin(angle+PI4);
    pp[0].y=y1+width*cos(angle+PI4);
    pp[1].x=x2+width*sin(angle-PI4);
    pp[1].y=y2+width*cos(angle-PI4);

    pp[2].x=x2+width*sin(angle-PI2-PI4);
    pp[2].y=y2+width*cos(angle-PI2-PI4);
    pp[3].x=x1+width*sin(angle+PI2+PI4);
    pp[3].y=y1+width*cos(angle+PI2+PI4);

    for(short i=0;i<4;i++)
    {
      disp.drawLine(pp[i].x,pp[i].y,pp[(i+1)%4].x,pp[(i+1)%4].y);
      disp.drawLine(xc,yc,pp[i].x,pp[i].y);
    }

    disp.drawLine(x1,y1,x2,y2);

}

void drawWinningLine(short xo,short yo,WinnerLine win)
{
    short x1=xo+win.from.x * CELL_SIZE + CELL_SIZE/2;
    short y1=yo+win.from.y * CELL_SIZE+ CELL_SIZE/2;
    short x2=xo+win.to.x * CELL_SIZE+ CELL_SIZE/2;
    short y2=yo+win.to.y * CELL_SIZE+ CELL_SIZE/2;
    double a=lerp(0.0,2.0*PI,(double)win.time);

    disp.color=3;
    drawRectangle(x1,y1,x2,y2,SIGN_SIZE,a);
}

void drawMenu()
{
    short xo;
    short yo;

    xo=(disp.width-logo[0])/2;

    //draw logo
    disp.drawBitmap(xo,0,logo);

    yo=logo[1];//logo height
    xo=40;

    //draw cursor
    disp.color=3;
    disp.print(xo-10,yo+(menuIndex*16),">");
    //draw menu items
    for (char m=0; m<sizeof(mainMenu)/sizeof(MenuItem); m++)
    {
        drawMenuItem(xo,yo,mainMenu[m]);
        yo+=16;
    }

    //draw instructions
    disp.color=3;
    disp.print(30,disp.height-30,"     Up/Down select menu");
    disp.print(30,disp.height-20,"  Left/Right change value");
    disp.print(30,disp.height-10,"A=Start Game B=Shuffle C=Menu");


    drawRectangle(30,30,160,160,16,game.frameCount*PI/100.0);

}

void drawGameType()
{
    disp.print((int)setNumPlayers);
    disp.print("P ");
    disp.print((int)setBoardSize);
    disp.print("x");
    disp.print((int)setBoardSize);
    disp.print(" ");
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
            //update symbol angle
            if ( board[x][y].time<100) board[x][y].time+=5;
            board[x][y].angle=lerp((float)0,(float)2*PI,(float)board[x][y].time);

            //Calculate center of the cell
            xc=xo+(x*CELL_SIZE)+(CELL_SIZE/2);
            yc=yo+(y*CELL_SIZE)+(CELL_SIZE/2);
            board[x][y].x=xc;
            board[x][y].y=yc;

            //Draw cursor
            if (cursor.x==x && cursor.y==y)
            {
                disp.color=3;
                drawPolygon(xc,yc,CURSOR_SIZE/2,4,-PI/4);
            }

            //Draw symbol of the board
            drawSign(board[x][y]);
        }
    }

    //draw winner list
    for (int i=0; i<10; i++)
    {
        drawSign(scoreList[i]);
    }

}


void initGame()
{
    //Center board
    boardPosition.x=((disp.width-20)-(setBoardSize*CELL_SIZE))/2;
    boardPosition.y=6+((disp.height-6)-(setBoardSize*CELL_SIZE))/2;

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
    //Scroll the menu settings
    if (btn.pressed(BTN_DOWN)) menuIndex++;
    if (btn.pressed(BTN_UP)) menuIndex--;
    menuIndex=clip(menuIndex,0,MAX_MENU_SETTINGS-1);

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

        //
        players[0].symb=Cross;
        players[0].AI=mainMenu[3].value==1;

        players[1].symb=Circle;
        players[1].AI=mainMenu[4].value==1;

        players[2].symb=Triangle;
        players[2].AI=mainMenu[5].value==1;

        players[3].symb=Diamond;
        players[3].AI=mainMenu[6].value==1;


        gameState=InitGame;
    }

    //Randomize data
    if(btn.pressed((BTN_B)))
    {
        mainMenu[0].value=random(3,5); //board
        mainMenu[1].value=random(2,mainMenu[0].value); //signs to win
        mainMenu[2].value=random(2,4);//players

        mainMenu[3].value=random(0,1);//P1 AI
        mainMenu[4].value=random(0,1);//P2 AI
        mainMenu[5].value=random(0,1);//P3 AI
        mainMenu[6].value=random(0,1);//P4 AI
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
    winnerLine.time=0;
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
        if (cursor.x>=setBoardSize) cursor.x=0;
        if (cursor.x<0) cursor.x=setBoardSize-1;
        if (cursor.y>=setBoardSize) cursor.y=0;
        if (cursor.y<0) cursor.y=setBoardSize-1;


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
    playerSign.x=disp.width-(SIGN_SIZE/2);
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
        timer=game.getTime()+1500;
    }
    else if (isDrawn(board))
    {
        cursor.x=-1;
        cursor.y=-1;
        gameState=Drawn;
        timer=game.getTime()+1500;
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

    //Winner line
    if (winnerLine.time <100) winnerLine.time+=10;
    drawWinningLine(boardPosition.x,boardPosition.y,winnerLine);

    if (btn.pressed(BTN_A) || game.getTime()>timer)
    {
        //move list symbols
        for (int i=0; i<10; i++)
        {
            scoreList[i].y+=SIGN_SIZE/2;
        }

        //add winner symbol
        Sign winSign;
        winSign.x=disp.width-(SIGN_SIZE/2);
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
    initMenu();
    game.begin();

    //load palette
    disp.palette[0]=COLOR_BLACK;
    disp.palette[1]=COLOR_WHITE;
    disp.palette[2]=COLOR_GREEN;
    disp.palette[3]=COLOR_RED;

    while (game.isRunning())
    {
        if (game.update())
        {
            //Exit to menu
            if(btn.pressed((BTN_C)))
            {
                gameState=Menu;
            }

            //Game states
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
