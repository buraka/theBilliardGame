// Billiard ball simulator
// Created by Nelis Franken
// -----------------------------------------------------------
//  Main implementation's header file
// -----------------------------------------------------------

#ifndef _BILLIARDS_H_
#define _BILLIARDS_H_

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <OpenGL/gl.h>
#include <OpenAL/al.h>
//#include <AL/alut.h>

#include "vector3.h"
#include "mySphere.h"

// Various variable declarations.
// -------------------------------------------------------------------------------------------------

// Define ASCII values for special keystrokes.
#define ESCAPE 27
#define PAGE_UP 73
#define PAGE_DOWN 81
#define UP_ARROW 72
#define DOWN_ARROW 80
#define LEFT_ARROW 75
#define RIGHT_ARROW 77

// Define scene texture indices
// - ball textures remain numbers 0 - 15
// - mini ball icons numbers 16 - 31
// - character portraits 32 - 33
#define CUE 34
#define GREEN_CARPET 35
#define BLACK 36
#define WOOD 37
#define DARK_WOOD 38
#define CHECKER 39
#define RED_CARPET 40
#define BLUE_WALL 41
#define RED_LIGHT 42
#define BEIGE_WALL 43
#define DARKBLUE_MARBLE 44
#define RED_MARBLE 45
#define POWER_BAR 46
#define SEA_VIEW 47
#define SEA_VIEW2 48
#define PAINTING2 49
#define PAINTING1 50
#define ROOF 51

// Define sound indices
#define SHOOT 0
#define COLLIDE 1
#define SIDES 2
#define SINK 3

// Constants
const int ballCount = 16;
const int textureCount = 52;
const int playerCount = 2;

// Structures
struct Image {
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
};

struct Player {
	bool balls[ballCount];
	char* name;
	int noOfShots;
	int side;
	int id;
};

// Various lists
mySphere ballList[ballCount];
Image myTextureData[textureCount];
GLuint theTexture[textureCount];
Player players[playerCount];
GLint displayList[5];
ALuint soundClip[4];

// Camera and lighting positions
static GLdouble camera[6] = {0.0,60.0,70.0, 0.0, 0.0, 0.0};
GLfloat light0Pos[] = { 0.0f, 205.0, 0.0f, 1.0f }; // Spotlight
GLfloat light0Dir[] = { 0.0f, -1.0f, 0.0f };
GLfloat light1Pos[] = { 100.0, 205.0, 100.0, 1.0 }; // Omni light

// Various essential flags, values and objects.
ALint error;
static int mainWindow;
GLUquadricObj *pillarCylinder, *ballQuadric;
GLfloat tableWidth = 50.0, tableLength = 100.0, pocketRadius = 1.5, theta = M_PI + M_PI/2.0, power = 3.0, elevationAngle = 15.0*M_PI/180.0, zoom = 65.0, pressedTheta = 0.0, pressedZoom = zoom, pressedElevation = 0.0, maxX = tableWidth/(2.0) - 2, minX = tableWidth/(-2.0) + 2, maxZ = tableLength/(2.0) - 2, minZ = tableLength/(-2.0) + 2;
GLint parentWindowHeight = 600, parentWindowWidth = 800, prevMouseX = 0, prevMouseY = 0, currentPlayer = 0;
GLint font=(int)GLUT_BITMAP_HELVETICA_12, largeFont=(int)GLUT_BITMAP_HELVETICA_18;
bool shootMode = false, doAnimation = false, leftMouseButton = false, rightMouseButton = false, middleMouseButton = false, turnStarting = true, returnWhiteBall = false, rollingBalls = false, gameover = false, useLazer = true;
char endGame[20];

// Player names list
char* nameList[playerCount] = {"Xavier", "Gabriel"};

// Texture names list
char* textureFilenames[textureCount] = {
	"ball_white.bmp",
	"ball_yellow_solid.bmp",
	"ball_blue_solid.bmp",
	"ball_red_solid.bmp",
	"ball_purple_solid.bmp",
	"ball_orange_solid.bmp",
	"ball_green_solid.bmp",
	"ball_brown_solid.bmp",
	"ball_black.bmp",
	"ball_yellow_stripe.bmp",
	"ball_blue_stripe.bmp",
	"ball_red_stripe.bmp",
	"ball_purple_stripe.bmp",
	"ball_orange_stripe.bmp",
	"ball_green_stripe.bmp",
	"ball_brown_stripe.bmp",
	"mini_white.bmp",
	"mini_yellow_solid.bmp",
	"mini_blue_solid.bmp",
	"mini_red_solid.bmp",
	"mini_purple_solid.bmp",
	"mini_orange_solid.bmp",
	"mini_green_solid.bmp",
	"mini_brown_solid.bmp",
	"mini_black.bmp",
	"mini_yellow_stripe.bmp",
	"mini_blue_stripe.bmp",
	"mini_red_stripe.bmp",
	"mini_purple_stripe.bmp",
	"mini_orange_stripe.bmp",
	"mini_green_stripe.bmp",
	"mini_brown_stripe.bmp",
	"char1.bmp",
	"char2.bmp",
	"cuestick.bmp",
	"carpet.bmp",
	"black.bmp",
	"wood.bmp",
	"dark_wood.bmp",
	"checker.bmp",
	"marble.bmp",
	"wall_dark2.bmp",
	"red_light.bmp",
	"grey_marble.bmp",
	"darkblue_marble.bmp",
	"red_marble.bmp",
	"powerBar.bmp",
	"view.bmp",
	"view2.bmp",
	"painting2.bmp",
	"painting1.bmp",
	"roof.bmp"
};


// Function declerations
// -------------------------------------------------------------------------------------------------

// Vertex generation and drawing tools
vector3 getNormal(GLfloat point1[3], GLfloat point3[3], GLfloat point4[3]);
void renderQuad(GLfloat point1[3], GLfloat point2[3], GLfloat point3[3], GLfloat point4[3], GLfloat orientation, GLfloat texXTile, GLfloat texYTile);
void renderSurface(GLfloat width, GLfloat length, GLfloat widthSegments, GLfloat lengthSegments, GLfloat elevation, GLfloat texXTile, GLfloat texYTile, GLfloat normalX, GLfloat normalY, GLfloat normalZ, GLuint &surfaceTexture);
void renderCurve(GLfloat radius, GLfloat height, GLfloat sweep, GLuint segments, GLfloat orientation, GLuint &aTexture);
void renderCap(GLfloat inner_radius, GLfloat outer_radius, GLfloat inner_sweep, GLfloat outer_sweep, GLuint segments, GLuint &myTexture);

// Object rendering functions
void renderBalls(void);
void renderPocket(GLfloat sweep);
void renderTableLegs(void);
void renderTable(void);
void renderStairs(void);
void renderBalcony(void);
void renderRoom(void);
void renderCueStick(void);
void drawSide(void);
void drawGuideLine(void);

// Camera functions
void updateCamera(void);
void updateTarget(void);

// User interface functions
void renderBitmapString(float x, float y, void *font, char* c);
void drawUI(void);
void drawEndScreen(void);

// GLUT implementation functions and basic OpenGL setup (input/window handlers)
void changeParentWindow(GLsizei width, GLsizei height);
void renderParentWindow(void);
void switchToOrtho(void);
void switchFromOrtho(void);
void keys(unsigned char key, int x, int y);
void specialKeyPressed(int key, int x, int y);
void useMouse(int button, int state, int x, int y);
void lookMouse(int x, int y);
void initlights(void);
void initScene(int argc, char **argv);
void idle(void);

// Physics functions
void deflectBalls(int ballOne, int ballTwo);
void ballInPocket(int ballNr, int pocketNr);
void initPhysics(void);

// Setup functions (initializing display lists, loading sounds and textures, setting up players etc.)
ALvoid DisplayALError(ALbyte *szText, ALint errorcode);
int ImageLoad(const char *filename, Image *image);
void setupPlayers(void);
void setupLists(void);
void resetGame(void);
void LoadGLTextures(void);
void loadSound(void);

// Main function
int main(int argc, char **argv);

#endif
