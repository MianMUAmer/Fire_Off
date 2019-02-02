/*
CTIS164 - Template Source Program
----------
STUDENT : Mian Muhammad Umair Amer
SECTION : 03
HOMEWORK: 03
----------
PROBLEMS: Put GL/ infront of glut.h , Otherwise its working all fine..!!
----------
ADDITIONAL FEATURES:
1) States Gradient Menu,Loadng,GamePlay
2) Start Button in Menu to start game directly with default settings
3) Difficulty Level can be selected from the menu by
4) A Tick will appear infront of the Difficulty level selected
5) About = information of the Software and its Developer can be read from menu
6) Exit = To Exit The Game
7) Loading State With a Countdown ... No click wil work During this time
8) In Run State Player can use 'a' or 'A' button to activate or deactivate the aim (a dotted line showing  bullets paths) for better gameplay
9) Run State displaying options for player i.e. to exit,to return to main menu in middle of game,for	    aim,shows current difficulty level of the game and the TIMMER.
10)When Game Ends it asks Player To play again the Same level if he/she presses 'yes' same level starts again after game has loaded again
11)It also displays the Score according to the time spent on the game and shows a message accordingly (if completed in 10s or in 15s or more than that)
12)In end of the Game message to Exit or return to main menu either to start game with new difficulty level or for any other feature of the menu.
*/

#include <glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 700

#define TIMER_PERIOD    20 // Period for the timer.
#define TIMER_ON         1 // 0:disable timer, 1:enable timer

#define D2R 0.0174532

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight; // current Window width and height

						  //States of the game 
#define BEGINDISPLAY 0  //Menu
#define START 1			//Game Loading State
#define RUN 2			//Game Play

bool aim = false;				//for aim dotted line
bool activetimer = false;		//for timmer
int state = BEGINDISPLAY;
int Bulletspeed = 15;			//To set the speed of bullet
int timecounter = 0, seccount = 0, mincount = 0;
int backCount = 5, Counter = 0; //for Loading Screen Wait

bool Rulemsg = false;		//bool for displaying rules for scoring
bool Aboutmsg = false;		//bool for displaying ABOUT option message
bool levels = false;		//bool to choose Difficulty level
bool B = false;				//bool for Beginner level
bool I = false;				//bool for Intermediate Level
bool E = false;				//bool for Expert
float speed = 0.5;			//for stages speed (a multiple)
int score = 0;				// for score keping

//Structure for point 
typedef struct {
	float x, y;
}cord_t;

//Structure for Balls/targets
typedef struct {
	int r, g, b;
	float radius,
		angle,
		speed,
		direction;
	bool collision = false;
	cord_t cord;
}target_t;

//Structure for Gun
typedef struct {
	cord_t pos;
	float angle;
}gun_t;

//Structure for Bullet
typedef struct {
	float angle;
	bool fire;
	cord_t pos;
}bullet_t;

target_t ball[3]; // 3 targets
bullet_t bullet = { 45,false,{ 0,0 } };
gun_t gun = { { 0,0 },45 };



//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r*cos(angle), y + r*sin(angle));
	}
	glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
	float angle;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r*cos(angle), y + r*sin(angle));
	}
	glEnd();
}

void print(int x, int y, char *string, void *font)
{
	int len, i;

	glRasterPos2f(x, y);
	len = (int)strlen(string);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, string[i]);
	}
}

// display text with variables.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void *font, char *string, ...)
{
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, str[i]);
	}
}

void vprint2(int x, int y, float size, char *string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(size, size, 1);

	int len, i;
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
	}
	glPopMatrix();
}

//displays the Backgound (i.e circles/paths and axis)
void displayBackground()
{
	glColor3ub(150, 150, 150);
	glBegin(GL_LINES);
	glVertex2f(-400, 0); // x-axis
	glVertex2f(400, 0);
	glVertex2f(0, 350); // y-axis
	glVertex2f(0, -350);
	glEnd();

	circle_wire(0, 0, 230);//first circular path
	circle_wire(0, 0, 280);//second circular path
	circle_wire(0, 0, 330);//third circular path
}

//displays Balls/targets
void drawtargets(target_t ball)
{
	glColor3ub(ball.r, ball.g, ball.b);  //random color
	circle(ball.cord.x, ball.cord.y, ball.radius);
	glColor3ub(0, 0, 0);
	vprint(ball.cord.x - 13, ball.cord.y - 5, GLUT_BITMAP_8_BY_13, "%.0f", ball.angle);
}

//Function for displaying vertex according to angle
void vertex(cord_t P, cord_t Tr, float angle)
{
	float xp = (P.x * cos(angle) - P.y * sin(angle)) + Tr.x;
	float yp = (P.x * sin(angle) + P.y * cos(angle)) + Tr.y;
	glVertex2d(xp, yp);
}

//display the Gun
void displaygun(gun_t gun)
{
	//To make sure angle remains in the range 0-360
	if (gun.angle <= 0)
		gun.angle += 360;
	else if (gun.angle >= 360)
		gun.angle -= 360;

	float angle = gun.angle * D2R; // degree to Radians

	glColor3ub(255, 255, 50);
	glBegin(GL_LINE_LOOP);
	//Barrel
	vertex({ -35, 25 }, gun.pos, angle);
	vertex({ 55, 25 }, gun.pos, angle);
	vertex({ 55, 10 }, gun.pos, angle);
	vertex({ 45, 0 }, gun.pos, angle);
	vertex({ -45, 0 }, gun.pos, angle);
	glEnd();

	//Handle
	glColor3ub(200, 0, 150);
	glBegin(GL_LINE_LOOP);
	vertex({ -45, 0 }, gun.pos, angle);
	vertex({ -30, -10 }, gun.pos, angle);
	vertex({ -55, -60 }, gun.pos, angle);
	vertex({ -20, -70 }, gun.pos, angle);
	vertex({ -13, -65 }, gun.pos, angle);
	vertex({ -20, -60 }, gun.pos, angle);
	vertex({ 0, 0 }, gun.pos, angle);
	glEnd();

	//Trigger
	glBegin(GL_LINE_LOOP);
	vertex({ 0, 0 }, gun.pos, angle);
	vertex({ -6, -20 }, gun.pos, angle);
	vertex({ 8, -23 }, gun.pos, angle);
	vertex({ 12, -17 }, gun.pos, angle);
	vertex({ 20, 0 }, gun.pos, angle);
	glEnd();

	//Triger Button
	glColor3ub(255, 255, 0);
	glBegin(GL_LINE_STRIP);
	vertex({ 0, 0 }, gun.pos, angle);
	vertex({ 2, -8 }, gun.pos, angle);
	vertex({ 10, -15 }, gun.pos, angle);
	glEnd();

	//Aim1 (Front)
	glBegin(GL_QUADS);
	vertex({ 32, 25 }, gun.pos, angle);
	vertex({ 42, 30 }, gun.pos, angle);
	vertex({ 50, 30 }, gun.pos, angle);
	vertex({ 55, 25 }, gun.pos, angle);
	glEnd();

	//Aim2 (Back)
	glBegin(GL_QUADS);
	vertex({ -35, 25 }, gun.pos, angle);
	vertex({ -30, 30 }, gun.pos, angle);
	vertex({ -20, 30 }, gun.pos, angle);
	vertex({ -12, 25 }, gun.pos, angle);
	glEnd();

	//Hammer
	glBegin(GL_QUADS);
	vertex({ -41, 13 }, gun.pos, angle);
	vertex({ -47, 13 }, gun.pos, angle);
	vertex({ -50, 8 }, gun.pos, angle);
	vertex({ -42, 8 }, gun.pos, angle);
	vertex({ -45, 5 }, gun.pos, angle);
	glEnd();

	//Pattern on Barrel
	glBegin(GL_LINES);
	vertex({ -25, 20 }, gun.pos, angle);
	vertex({ -35, 5 }, gun.pos, angle);

	vertex({ -15, 20 }, gun.pos, angle);
	vertex({ -25, 5 }, gun.pos, angle);

	vertex({ -5, 20 }, gun.pos, angle);
	vertex({ -15, 5 }, gun.pos, angle);

	vertex({ -1, 20 }, gun.pos, angle);
	vertex({ 48, 20 }, gun.pos, angle);
	vertex({ -1, 17 }, gun.pos, angle);
	vertex({ 48, 17 }, gun.pos, angle);

	//Pattern on Handle
	glColor3ub(200, 0, 150);
	vertex({ -30, -20 }, gun.pos, angle);
	vertex({ -14, -32 }, gun.pos, angle);

	vertex({ -35, -32 }, gun.pos, angle);
	vertex({ -19, -44 }, gun.pos, angle);

	vertex({ -42, -44 }, gun.pos, angle);
	vertex({ -26, -56 }, gun.pos, angle);

	glEnd();

	//if user presses 'A' or 'a' aim(a dotted line for bullets paths) is activated or deactivated
	if (aim)
	{
		glColor3ub(200, 250, 150);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(3, 0xf0f0);
		glBegin(GL_LINES);
		glVertex2f(30 * cos(gun.angle * D2R), 30 * sin(gun.angle * D2R));
		glVertex2f(360 * cos(gun.angle * D2R), 360 * sin(gun.angle * D2R));
		glEnd();
		glLineWidth(1);   // line thickness
		glDisable(GL_LINE_STIPPLE);
	}

	//Printting Angle of the Gun from X-axis
	glColor3f(1, 1, 0);
	vprint(350 * cos(gun.angle *D2R) - 10, 343 * sin(gun.angle *D2R), GLUT_BITMAP_9_BY_15, "%.0f", gun.angle);

}

//displays the bullet when clicked
void fireBullet(bullet_t bullet)
{
	if (bullet.fire)
		circle(bullet.pos.x, bullet.pos.y, 10);
}

//to initialize the targets/Balls different variables
void initialize(target_t ball[], float SPEED)
{
	score = 0;
	for (int i = 0; i < 3; i++)
	{
		ball[i].r = rand() % 256;
		ball[i].g = rand() % 256;
		ball[i].b = rand() % 256;
		ball[i].direction = rand() % 2; //1 for Anti-Clockwise ; 0 for Clockwise
		ball[i].angle = rand() % 361;
		ball[i].radius = rand() % 20 + 15;
		ball[i].speed = (rand() % 3 + 1)*SPEED; //random angular speed * SPEED(the speed of difficulty Level)
		ball[i].collision = false;
	}
	bullet = { 45,false,{ 0,0 } }; //initialize Bullet
}

//displays the Menu Screen
void displayBEGIN_DISPLAYstate()
{
	//Gradient Background
	glBegin(GL_QUADS);
	glColor3ub(0, 0, 0);
	glVertex2f(-400, 350);
	glVertex2f(400, 350);
	glColor3ub(240, 0, 0);
	glVertex2f(400, -350);
	glVertex2f(-400, -350);
	glEnd();

	//Title
	glColor3ub(255, 255, 0);
	vprint2(-340, 230, 0.7, "< FIRE OFF >");
	vprint2(-140, 180, 0.2, "HIT ME IF YOU CAN");

	//Start Button
	glBegin(GL_LINE_LOOP);
	glVertex2f(-380, 0);
	glVertex2f(-250, 0);
	glVertex2f(-250, -50);
	glVertex2f(-380, -50);
	glEnd();
	vprint(-355, -35, GLUT_BITMAP_TIMES_ROMAN_24, "START");

	//Difficulty level button
	glBegin(GL_LINE_LOOP);
	glVertex2f(-380, -70);
	glVertex2f(-120, -70);
	glVertex2f(-120, -120);
	glVertex2f(-380, -120);
	glEnd();
	vprint(-375, -105, GLUT_BITMAP_TIMES_ROMAN_24, "SELECT DIFFICULTY");

	//Rule Button
	glBegin(GL_LINE_LOOP);
	glVertex2f(-380, -140);
	glVertex2f(-120, -140);
	glVertex2f(-120, -190);
	glVertex2f(-380, -190);
	glEnd();
	vprint(-365, -175, GLUT_BITMAP_TIMES_ROMAN_24, "RULES OF SCORING");

	//About Button
	glBegin(GL_LINE_LOOP);
	glVertex2f(-380, -210);
	glVertex2f(-250, -210);
	glVertex2f(-250, -260);
	glVertex2f(-380, -260);
	glEnd();
	vprint(-365, -245, GLUT_BITMAP_TIMES_ROMAN_24, "ABOUT");

	//Exit Button
	glBegin(GL_LINE_LOOP);
	glVertex2f(-380, -280);
	glVertex2f(-250, -280);
	glVertex2f(-250, -330);
	glVertex2f(-380, -330);
	glEnd();
	vprint(-355, -315, GLUT_BITMAP_TIMES_ROMAN_24, "EXIT");


	//message showing that default level is Beginner
	vprint(-220, -340, GLUT_BITMAP_9_BY_15, "<<< Difficulty Level By Default Is \"BEGINNER\" >>>");

	//if 'Rules for scoring' Button is pressed
	if (Rulemsg)
	{
		vprint(90, -80, GLUT_BITMAP_HELVETICA_18, "   SCORES ");
		vprint(-15, -120, GLUT_BITMAP_9_BY_15, "If Game Ends Within 10 Sec = BRAVO...!!!");
		vprint(-35, -160, GLUT_BITMAP_9_BY_15, "If Game Ends Within 15 Sec = WELL PLAYED :-|");
		vprint(-65, -200, GLUT_BITMAP_9_BY_15, "If Doesn't End After 15 Sec = POOR PERFORMANCE :-{");
	}

	//If 'About' button is clicked display this message
	if (Aboutmsg)
	{
		vprint(50, -100, GLUT_BITMAP_9_BY_15, "   This Software is Licensed ");
		vprint(50, -120, GLUT_BITMAP_9_BY_15, "Copyrights are Reserved (c) 2018");
		vprint(50, -180, GLUT_BITMAP_9_BY_15, "       Manufactured By :");
		vprint(50, -220, GLUT_BITMAP_9_BY_15, "   Mian Muhammad Umair Amer");
		vprint(50, -240, GLUT_BITMAP_9_BY_15, "   (Future Software Engineer)");
	}

	//If 'Select Difficulty' button is clicked display message
	if (levels)
	{
		vprint(20, -10, GLUT_BITMAP_HELVETICA_18, "!! Choose The Level You Want To Play !!");
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < 3; i++) //Forms 3 equal sized boxes
		{
			glVertex2f(90, -50 - (70 * i));
			glVertex2f(260, -50 - (70 * i));
			glVertex2f(260, -100 - (70 * i));
			glVertex2f(90, -100 - (70 * i));
		}
		glEnd();
		//displays messages on the created Buttons
		vprint(130, -80, GLUT_BITMAP_HELVETICA_18, "BEGINNER");
		vprint(105, -150, GLUT_BITMAP_HELVETICA_18, "INTERMEDIATE");
		vprint(140, -220, GLUT_BITMAP_HELVETICA_18, "EXPERT");
	}

	//display a small tick in front of the level selected
	if (!Aboutmsg && !Rulemsg)
	{
		if (B) { // if Beginner is the difficulty level
			glBegin(GL_LINE_STRIP);
			glVertex2f(280, -80);
			glVertex2f(285, -90);
			glVertex2f(295, -55);
			glEnd();
		}
		else if (I) {// if Intermediated is the difficulty level
			glBegin(GL_LINE_STRIP);
			glVertex2f(280, -150);
			glVertex2f(285, -160);
			glVertex2f(295, -125);
			glEnd();
		}
		else if (E) {//Expert level & !Aboutmsg  because it should appear only when level button is selected otherwise not
			glBegin(GL_LINE_STRIP);
			glVertex2f(280, -220);
			glVertex2f(285, -230);
			glVertex2f(295, -195);
			glEnd();
		}
	}

}

//displays the game Running state
void displayRUNstate()
{
	int pos = 230; // position of first ball (length from origin - first circle)
	for (int i = 0; i < 3; i++, pos += 50) //to form balls at the path
		if (!ball[i].collision)
		{
			ball[i].cord.x = pos * cos(ball[i].angle * D2R);
			ball[i].cord.y = pos * sin(ball[i].angle * D2R);
			drawtargets(ball[i]);	 //draw a ball at x,y according to the angle
		}
	displaygun(gun);		//display gun
	fireBullet(bullet);				//display bullet

	glColor3ub(0, 200, 0);
	vprint(295, 290, GLUT_BITMAP_HELVETICA_18, "TIMER");
	vprint2(270, 250, 0.20, "%02d:%02d:%02d", mincount, seccount, timecounter % 100);

	//Display the level of the game bein played
	vprint(-367, 288, GLUT_BITMAP_TIMES_ROMAN_24, "LEVEL");
	if (I)
		vprint(-390, 250, GLUT_BITMAP_HELVETICA_18, "INTERMEDIATE");
	else if (E)
		vprint(-365, 260, GLUT_BITMAP_HELVETICA_18, "EXPERT");
	else //for Beginner/Default
		vprint(-375, 260, GLUT_BITMAP_HELVETICA_18, "BEGINNER");

	//displays message for activating or deactivating Aim
	glColor3ub(0, 255, 200);
	vprint(-380, -300, GLUT_BITMAP_9_BY_15, "Press 'A' or 'a' For");
	vprint(-385, -320, GLUT_BITMAP_9_BY_15, "Activate or Deactivate");
	vprint(-320, -340, GLUT_BITMAP_9_BY_15, "Aim");

	//When Game Ends
	if (ball[0].collision == true && ball[1].collision == true && ball[2].collision == true)
	{
		score = 5000 / seccount;
		glColor3ub(255, 255, 0);
		vprint(-75, 120, GLUT_BITMAP_TIMES_ROMAN_24, "GAME OVER");

		vprint(-215, -240, GLUT_BITMAP_TIMES_ROMAN_24, "Press <F1> To Return Back To Main Menu");
		vprint(-165, -280, GLUT_BITMAP_TIMES_ROMAN_24, "Press <ESC> To Exit Application");
		activetimer = false;
		bullet.fire = false;

		//ASk player if he/she wants to play the same level again
		glColor3ub(255, 128, 0);
		vprint(-180, -125, GLUT_BITMAP_HELVETICA_18, "Want to Play Again ?");
		glBegin(GL_LINE_LOOP);
		glVertex2f(10, -145);
		glVertex2f(100, -145);
		glVertex2f(100, -100);
		glVertex2f(10, -100);
		glEnd();
		vprint(30, -130, GLUT_BITMAP_HELVETICA_18, "YES");

		vprint(290, -260, GLUT_BITMAP_TIMES_ROMAN_24, "SCORE");
		vprint(315, -290, GLUT_BITMAP_HELVETICA_18, "%d",score);
		if(score >= 500)
			vprint(215, -320, GLUT_BITMAP_HELVETICA_18, "Excellent.. Bravo..!!!!");
		else if(score >= 300)
			vprint(280, -320, GLUT_BITMAP_HELVETICA_18, "Well Played");
		else
			vprint(180, -320, GLUT_BITMAP_HELVETICA_18, "You Need More Practice");
	}
	else
	{
		vprint(190, -300, GLUT_BITMAP_9_BY_15, "Press <F1> = Main Menu");
		vprint(195, -320, GLUT_BITMAP_9_BY_15, " Press <ESC> = EXIT");
	}
}

//displays the game's Loading State 
void displaySTARTstate()
{
	//Countdown for game to start
	Counter += 20;
	if (Counter % 1000 == 0 && backCount != 0)
		backCount--;

	//display Gun
	displaygun(gun);

	//Show message to click and start the game or wait for the count down and the game to load
	glColor3ub(0, 255, 0);
	vprint(295, 290, GLUT_BITMAP_HELVETICA_18, "TIMER");
	vprint2(270, 250, 0.20, "%02d:%02d:%02d", mincount, seccount, timecounter % 100);
	if (backCount != 0)
	{
		glColor3ub(150, 200, 100);
		vprint(-75, 140, GLUT_BITMAP_HELVETICA_18, "GAME STARTS IN");
		vprint(0, 100, GLUT_BITMAP_TIMES_ROMAN_24, "%d", backCount);
		vprint(-65, 60, GLUT_BITMAP_HELVETICA_18, "Please Be Patient");
	}
	else
	{
		glColor3ub(150, 240, 10);
		vprint(-170, 140, GLUT_BITMAP_TIMES_ROMAN_24, "GAME SUCCESSFULLY LOAD");
		vprint(-175, 60, GLUT_BITMAP_TIMES_ROMAN_24, "LEFT CLICK TO START GAME");
	}
}

//
// To display onto window using OpenGL commands
//
void display() {
	//
	// clear window to black
	//
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	if (state == BEGINDISPLAY)
		displayBEGIN_DISPLAYstate();
	else if (state == RUN || state == START)
		displayBackground();

	if (state == START)
		displaySTARTstate();
	else if (state == RUN)
		displayRUNstate();

	glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeyDown(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);
	if (key == 65 || key == 97) //'A' or 'a' for the aim to activate or deactivate
		aim = !aim;

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyDown(int key, int x, int y)
{
	// Write your codes here.
	if (state == RUN && key == GLUT_KEY_F1)
		state = BEGINDISPLAY;

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyUp(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: up = false; break;
	case GLUT_KEY_DOWN: down = false; break;
	case GLUT_KEY_LEFT: left = false; break;
	case GLUT_KEY_RIGHT: right = false; break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void onClick(int button, int stat, int x, int y)
{
	//changing cordinate system
	int x2 = x - winWidth / 2;
	int y2 = winHeight / 2 - y;
	//Displaying Main Menu Screen and Its Buttons Functions
	if (state == BEGINDISPLAY &&  button == GLUT_LEFT_BUTTON && stat == GLUT_UP)
	{
		if (x2 >= -380 && x2 <= -250 && y2 >= -50 && y2 <= 0) //if Start Button is pressed
		{
			state = START;
			backCount = 5;
			Counter = 0;
			mincount = seccount = timecounter = 0;
		}
		else if (x2 >= -380 && x2 <= -120 && y2 >= -120 && y2 <= -70) //if Difficulty level Button is pressed
		{
			levels = true;
			Aboutmsg = false; // so it's contents are not shown
			Rulemsg = false;
		}
		else if (x2 >= -380 && x2 <= -120 && y2 >= -190 && y2 <= -140) // if Rules for Score Button is pressed
		{
			Rulemsg = true;
			levels = false;
			Aboutmsg = false;
		}
		else if (x2 >= -380 && x2 <= -250 && y2 >= -260 && y2 <= -210) // if About Button is pressed
		{
			Aboutmsg = true;
			levels = false; //levels are not shown
			Rulemsg = false;
		}
		else if (x2 >= -380 && x2 <= -250 && y2 >= -330 && y2 <= -280) //if EXIT Button is pressed
			exit(0);

		// for selecting the Levels (Beginner,Interme..,Expert)
		else if (x2 >= 90 && x2 <= 260 && levels) 
		{
			if (y2 >= -100 && y2 <= -50)
			{
				speed = 0.5; //Speed Multiple for Beginner
				B = true;
				I = false;
				E = false;
			}
			else if (y2 >= -170 && y2 <= -120)
			{
				speed = 1; //Speed Multiple for Intermediate
				B = false;
				I = true;
				E = false;
			}
			else if (y2 >= -240 && y2 <= -190)
			{
				speed = 1.5;  //Speed Multiple for Expert
				B = false;
				I = false;
				E = true;
			}
			initialize(ball, speed); //intialize Targets according to the level selected
		}
	}

	//From Loading Screen To Run Screen
	if (state == START && button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && backCount == 0)
	{
		state = RUN;
		initialize(ball, speed);
		activetimer = true;
		bullet.fire = false;
	}

	//to capture the gun angle for the bullet to move in that direction when fired
	else if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && state == RUN)
	{
		if (!bullet.fire)
			bullet.angle = gun.angle;
		bullet.fire = true; //move the bullet
	}

	//to replay the game at the same difficulty level once the game finishes
	if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && state == RUN && activetimer == false)
	{
		if (x2 > 10 && x2 < 100 && y2 < -100 && y2 > -145)
		{
			state = START;
			initialize(ball, speed);
			backCount = 5;
			Counter = 0;
			mincount = seccount = timecounter = 0;
		}
	}
	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	display(); // refresh window.
}

void onMoveDown(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function   
	glutPostRedisplay();
}

// GLUT to OpenGL coordinate conversion:
//   x2 = x - winWidth / 2
//   y2 = winHeight / 2 - y
void onMove(int x, int y) {
	// To change coordinate system 
	int x2 = x - winWidth / 2;
	int y2 = winHeight / 2 - y;
	//get the mouse angle
	float angle = (atan2f(y2, x2)) * (180 / PI);
	if (angle < 0) //check for the negative angle
		angle += 360;
	gun.angle = angle; //make that mouse angle equal to gun's angle

					   // to refresh the window it calls display() function
	glutPostRedisplay();
}

//to check whether the bullet and the Balls/Targets collide
bool testCollision(bullet_t fr, target_t t) {
	float dx = t.cord.x - fr.pos.x; //(x2-x1)
	float dy = t.cord.y - fr.pos.y; //(y2-y1)
	float d = sqrt(dx*dx + dy*dy); //distance between the centres of bullet and Target
	return d <= t.radius;          // 1 is Collode ; 0 if no collison
}

#if TIMER_ON == 1
void onTimer(int v) {

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	// Write your codes here.

	if (activetimer && state == RUN)
	{
		//TIMER
		timecounter += 20;
		if (timecounter % 1000 == 0)
		{
			seccount++;
			if (seccount == 60)
			{
				mincount++;
				seccount = 0;
			}
		}

		//Movement of Balls Randomly
		for (int i = 0; i < 3; i++)
			if (ball[i].direction)
			{
				ball[i].angle += ball[i].speed; //Anti-ClockWise Movement
				if (ball[i].angle > 360)
					ball[i].angle -= 360;
			}
			else
			{
				ball[i].angle -= ball[i].speed; //Clockwise Movement
				if (ball[i].angle < 0)
					ball[i].angle += 360;
			}
	}

	//Moving Bullet
	if (bullet.fire)
	{
		bullet.pos.x += Bulletspeed * cos(bullet.angle * D2R);
		bullet.pos.y += Bulletspeed * sin(bullet.angle * D2R);

		//to check if bullet is out of the screen/Gaming Area so reset it
		if (bullet.pos.x > 400 || bullet.pos.x < -400 || bullet.pos.y < -350 || bullet.pos.y > 350)
		{
			bullet.fire = false;
			bullet.pos.x = 0;
			bullet.pos.y = 0;
		}
		//Check for collision
		for (int i = 0; i < 3; i++)
			if (testCollision(bullet, ball[i]))
				ball[i].collision = true;
	}
	// to refresh the window it calls display() function
	glutPostRedisplay(); // display()
}
#endif

void Init() {

	// Smoothing shapes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//intializing targets Steucture Array with default difficulty level
	initialize(ball, 0.5);

}

void main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	//glutInitWindowPosition(100, 100);
	glutCreateWindow("FIRE OFF - Mian Muhammad Umair Amer - HomeWork#3");

	glutDisplayFunc(display);
	glutReshapeFunc(onResize);


	//
	// keyboard registration
	//
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecialKeyDown);

	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecialKeyUp);

	//
	// mouse registration
	//
	glutMouseFunc(onClick);
	glutMotionFunc(onMoveDown);
	glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
	// timer event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif
	srand(time(NULL));
	Init();

	glutMainLoop();
}