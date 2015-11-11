#include <iostream>
#include <stdio.h>

#include "Camera.h"
#include "VoxelGrid.h"
#include "VolumeRender.h"

#ifdef __APPLE__
#include <GLUT/GLUT.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

// Camera
Camera camera; 
int	lastMouseX = -1, lastMouseY = -1;
int theButtonState = 0;
int theModifierState = 0;
int dx, dy;

// String input
string input = "";
bool input_flag = false;
int input_type;

// Display flags
bool showInfo = true;
bool showHelp = false;
bool showRenderMenu = false;

// Colors
const float red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
const float green[] = { 0.0f, 1.0f, 0.0f, 1.0f };
const float blue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
const float text[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const float textbg[] = { 1.0f, 1.0f, 1.0f, 0.2f };

// Cache to speed up rendering in window
vector<vec3> cloudPosCache;
vector<vec3> cloudColCache;

// Useful variables
int rgb_edit = 0;
int quality = 1;
int seed;
double bg[4] = { 0.6, 0.8, 0.9, 0 };

VolumeRender *generateCloud = new VolumeRender();

void initDisplay(void); // Initialize OpenGL settings

void renderInfo(void); // Render information
void renderHelp(void); // Render help
void renderInput(void); // Render file input
void renderMenu(void); // Render menu
void renderCloud(void); // Render cloud
void renderDisplay(void); // Render display
void changeSize(int, int); // Change window size
void cacheCloud(); // Cache cloud for rendering
void processKeyboard(unsigned char, int, int); // Keyboard keys
void processKeyboardSpecial(int, int, int);	// Special keys for the arrow keys
void processMouse(int, int, int, int); // Mouse motion
void processMouseMotion(int, int); // Mouse motion action

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("Volumetric Clouds");
	
	glutDisplayFunc(renderDisplay);
	glutIdleFunc(renderDisplay);

	glutKeyboardFunc(processKeyboard);
	glutSpecialFunc(processKeyboardSpecial);
	glutMouseFunc(processMouse);
	glutMotionFunc(processMouseMotion);

	glutReshapeFunc(changeSize);

	initDisplay();

	glutMainLoop();

	return 0;
}

void initDisplay(void)
{
	float l0_dif[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float l0_amb[] = {0.2f, 0.2f, 0.2f, 0.2f};
	float l0_pos[] = {0.5f, 0.6f, 0.624f, 0.0f};

	glClearColor(0.1, 0.1, 0.1, 1.0);

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, l0_dif);
	glLightfv(GL_LIGHT0, GL_AMBIENT, l0_amb);
	glEnable(GL_COLOR_MATERIAL);
	// glEnable(GL_CULL_FACE);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, l0_pos);
}

void renderInfo()
{
	// Display information
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, 640, 0, 480);	
		glScalef(1, -1, 1); // Invert the y axis, down is positive
		glTranslatef(0, -480, 0); // Move the origin from the bottom left corner to the upper left corner
		glMatrixMode(GL_MODELVIEW);

		glPushMatrix();
			glLoadIdentity();
			glDisable(GL_LIGHTING);
			glColor4fv(text);
			char info[1024];
			
			glRasterPos2f(520, 20);
			sprintf(info, "F1 (help) / F2 (info)");
			for (unsigned int ii = 0; ii < strlen(info); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[ii]);

			if (showInfo) {
				glRasterPos2f(10, 20);
				sprintf(info, "Grid size: %2.0f x %2.0f x %2.0f", generateCloud->getGrid()->getMaxX()*generateCloud->getGrid()->getVoxelSize(),
																  generateCloud->getGrid()->getMaxY()*generateCloud->getGrid()->getVoxelSize(),
																  generateCloud->getGrid()->getMaxZ()*generateCloud->getGrid()->getVoxelSize());
				for (unsigned int ii = 0; ii < strlen(info); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[ii]);
			
				glRasterPos2f(10, 35);
				sprintf(info, "Voxel size: %.2f", generateCloud->getGrid()->getVoxelSize());
				for (unsigned int ii = 0; ii < strlen(info); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[ii]);
			
				glRasterPos2f(10, 50);
				sprintf(info, "Default density: %.2f", generateCloud->getGrid()->getDefaultDensity());
				for (unsigned int ii = 0; ii < strlen(info); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[ii]);
			
				glRasterPos2f(10, 65);
				switch (rgb_edit) {
					case 0:	{ sprintf(info, "Edit color: Red %.2f", generateCloud->getGrid()->getVoxelColor(0, 0, 0)[0]*255); break; }
					case 1:	{ sprintf(info, "Edit color: Green %.2f", generateCloud->getGrid()->getVoxelColor(0, 0, 0)[1]*255); break; }
					case 2:	{ sprintf(info, "Edit color: Blue %.2f", generateCloud->getGrid()->getVoxelColor(0, 0, 0)[2]*255); break; }
				}
				for (unsigned int ii = 0; ii < strlen(info); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[ii]);
			
				glRasterPos2f(10, 80);
				if (bg[3] == 0)	{ sprintf(info, "Background: Light blue"); }
				if (bg[3] == 1) { sprintf(info, "Background: Blue gradient"); }
				for (unsigned int ii = 0; ii < strlen(info); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[ii]);
			
				glRasterPos2f(10, 95);
				switch (quality) {
					case 0:	{ sprintf(info, "Quality: Minimum"); break; }
					case 1:	{ sprintf(info, "Quality: Low"); break; }
					case 2:	{ sprintf(info, "Quality: Medium"); break; }
					case 3:	{ sprintf(info, "Quality: High"); break; }
					case 4:	{ sprintf(info, "Quality: Maximum"); break; }
				}
				for (unsigned int ii = 0; ii < strlen(info); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[ii]);
			
				glRasterPos2f(10, 125);
				sprintf(info, "Eyep: X: %.2f Y: %.2f Z: %.2f", camera.GetEye()[0], camera.GetEye()[1], camera.GetEye()[2]);
				for (unsigned int ii = 0; ii < strlen(info); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[ii]);
			
				glRasterPos2f(10, 140);
				sprintf(info, "Vdir: X: %.2f Y: %.2f Z: %.2f", camera.GetFrontVector()[0], camera.GetFrontVector()[1], camera.GetFrontVector()[2]);
				for (unsigned int ii = 0; ii < strlen(info); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[ii]);
			
				glRasterPos2f(10, 155);
				sprintf(info, "Uvec: X: %.2f Y: %.2f Z: %.2f", camera.GetUpVector()[0], camera.GetUpVector()[1], camera.GetUpVector()[2]);
				for (unsigned int ii = 0; ii < strlen(info); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[ii]);
			
				glRasterPos2f(10, 170);
				sprintf(info, "Fovy: %.2f", camera.GetFovY());
				for (unsigned int ii = 0; ii < strlen(info); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[ii]);
			}

			glEnable(GL_LIGHTING);
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void renderHelp()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, 640, 0, 480);	
		glScalef(1, -1, 1); // Invert the y axis, down is positive
		glTranslatef(0, -480, 0); // Move the origin from the bottom left corner to the upper left corner
		glMatrixMode(GL_MODELVIEW);

		glPushMatrix();
			glLoadIdentity();
			glDisable(GL_LIGHTING);
			glColor4fv(text);
			char help_txt[1024];
			
			glRasterPos2f(220, 60);
			sprintf(help_txt, "'enter' - render scene");
			for (unsigned int ii = 0; ii < strlen(help_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, help_txt[ii]);
			
			glRasterPos2f(220, 75);
			sprintf(help_txt, "'space' - reset scene");
			for (unsigned int ii = 0; ii < strlen(help_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, help_txt[ii]);
			
			glRasterPos2f(220, 90);
			sprintf(help_txt, "'esc' - quit");
			for (unsigned int ii = 0; ii < strlen(help_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, help_txt[ii]);
			
			glRasterPos2f(220, 105);
			sprintf(help_txt, "'alt + mouse' - move/rotate/zoom camera");
			for (unsigned int ii = 0; ii < strlen(help_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, help_txt[ii]);

			glRasterPos2f(220, 120);
			sprintf(help_txt, "'r' - open render menu");
			for (unsigned int ii = 0; ii < strlen(help_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, help_txt[ii]);

			glRasterPos2f(220, 135);
			sprintf(help_txt, "'up/down' - select rgb color to edit");
			for (unsigned int ii = 0; ii < strlen(help_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, help_txt[ii]);
			
			glRasterPos2f(220, 150);
			sprintf(help_txt, "'left/right' - edit selected rgb color");
			for (unsigned int ii = 0; ii < strlen(help_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, help_txt[ii]);

			glRasterPos2f(220, 165);
			sprintf(help_txt, "'q' - change quality");
			for (unsigned int ii = 0; ii < strlen(help_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, help_txt[ii]);

			glRasterPos2f(220, 180);
			sprintf(help_txt, "'o' - output cloud data");
			for (unsigned int ii = 0; ii < strlen(help_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, help_txt[ii]);

			glRasterPos2f(220, 195);
			sprintf(help_txt, "'b' - change background color");
			for (unsigned int ii = 0; ii < strlen(help_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, help_txt[ii]);
			
			glEnable(GL_LIGHTING);
			glColor4fv(textbg);
			glRectf(210.0, 35.0, 505.0, 205.0);
		glPopMatrix();
		
		glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void renderInput()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, 640, 0, 480);	
		glScalef(1, -1, 1); // Invert the y axis, down is positive
		glTranslatef(0, -480, 0); // Move the origin from the bottom left corner to the upper left corner
		glMatrixMode(GL_MODELVIEW);

		glPushMatrix();
			glLoadIdentity();
			glDisable(GL_LIGHTING);
			glColor4fv(text);
			char input_txt[1024];

			glRasterPos2f(480, 450);
			if (input_type == 0) sprintf(input_txt, "Enter seed (integer):");
			else if (input_type > 0) sprintf(input_txt, "Enter file name:");
			for (unsigned int ii = 0; ii < strlen(input_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, input_txt[ii]);
			glRasterPos2f(480, 465);
			sprintf(input_txt, ">> %s_", input.c_str());
			for (unsigned int ii = 0; ii < strlen(input_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, input_txt[ii]);
			
			glEnable(GL_LIGHTING);
			glColor4fv(textbg);
			glRectf(470.0, 435.0, 630.0, 475.0);
		glPopMatrix();
		
		glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void renderMenu()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, 640, 0, 480);	
		glScalef(1, -1, 1); // Invert the y axis, down is positive
		glTranslatef(0, -480, 0); // Move the origin from the bottom left corner to the upper left corner
		glMatrixMode(GL_MODELVIEW);

		glPushMatrix();
			glLoadIdentity();
			glDisable(GL_LIGHTING);
			glColor4fv(text);
			char menu_txt[1024];

			glRasterPos2f(220, 90);
			sprintf(menu_txt, "'1' - create random cloud");
			for (unsigned int ii = 0; ii < strlen(menu_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, menu_txt[ii]);
				
			glRasterPos2f(220, 105);
			sprintf(menu_txt, "'2' - read from file");
			for (unsigned int ii = 0; ii < strlen(menu_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, menu_txt[ii]);
				
			glRasterPos2f(220, 120);
			sprintf(menu_txt, "'3' - read red from file");
			for (unsigned int ii = 0; ii < strlen(menu_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, menu_txt[ii]);
				
			glRasterPos2f(220, 135);
			sprintf(menu_txt, "'4' - read green from file");
			for (unsigned int ii = 0; ii < strlen(menu_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, menu_txt[ii]);
				
			glRasterPos2f(220, 150);
			sprintf(menu_txt, "'5' - read blue from file");
			for (unsigned int ii = 0; ii < strlen(menu_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, menu_txt[ii]);
				
			glRasterPos2f(220, 165);
			sprintf(menu_txt, "'enter' - render scene");
			for (unsigned int ii = 0; ii < strlen(menu_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, menu_txt[ii]);

			glRasterPos2f(220, 180);
			sprintf(menu_txt, "'esc' - exit menu");
			for (unsigned int ii = 0; ii < strlen(menu_txt); ii++)
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, menu_txt[ii]);
			
			glEnable(GL_LIGHTING);
			glColor4fv(textbg);
			glRectf(210.0, 65.0, 505.0, 195.0);
		glPopMatrix();
		
		glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void renderCloud()
{
	// Draw the XYZ axis
	glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(1.0);
		glBegin(GL_LINES);
		
		glColor4fv(red);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(1000.0, 0.0, 0.0);
		
		glColor4fv(green);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 1000.0, 0.0);
		
		glColor4fv(blue);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 1000.0);

		glEnd();
		glEnable(GL_LIGHTING);
	glPopAttrib();

	if (!cloudPosCache.empty()) {
		glPushMatrix();
			glColor4f(0.7, 0.7, 0.7, 1.0);
			glTranslatef(generateCloud->getGrid()->getMaxX()*generateCloud->getGrid()->getVoxelSize()*0.5,
						 generateCloud->getGrid()->getMaxY()*generateCloud->getGrid()->getVoxelSize()*0.5,
						 generateCloud->getGrid()->getMaxZ()*generateCloud->getGrid()->getVoxelSize()*0.5);
			glScalef(generateCloud->getGrid()->getMaxX()*generateCloud->getGrid()->getVoxelSize(),
					 generateCloud->getGrid()->getMaxY()*generateCloud->getGrid()->getVoxelSize(),
					 generateCloud->getGrid()->getMaxZ()*generateCloud->getGrid()->getVoxelSize());
			glutWireCube(1.0f);
			glColor4f(0.0, 1.0, 1.0, 1.0);
		glPopMatrix();
	}

	int inc = (cloudPosCache.size() > 100000) ? 100 : 1;
	for (int ii = 0; ii < cloudPosCache.size(); ii += inc) {
		glPushMatrix();
			glColor4f(cloudColCache[ii][0], cloudColCache[ii][1], cloudColCache[ii][2], 0.5);
			glBegin(GL_POINTS);
			glVertex3f(cloudPosCache[ii][0], cloudPosCache[ii][1], cloudPosCache[ii][2]);
			glEnd( );
		glPopMatrix();
	}
}

void renderDisplay(void)
{
	glClearColor(0.0, 0.0, 0.0, 1);
	// Clearing the depth buffer 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 1.0, 10.0, 200.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	camera.LookAt();
	
	renderCloud(); // Render cloud
	renderInfo(); // Display info
	if (input_flag) { renderInput(); } // Display file input
	if (showHelp) { renderHelp(); } // Display help
	if (showRenderMenu) { renderMenu(); } // Display menu
	
	// swapping the buffers causes the rendering above to be shown
	glutSwapBuffers();
}

void changeSize(int width, int height)
{
	// Update viewport
	glViewport(0, 0, width, height);
}

void cacheCloud()
{
	cloudPosCache.clear();
	cloudColCache.clear();

	// Cache the cloud for render
	for (int kk = 0; kk < generateCloud->getGrid()->getMaxZ(); kk++) {
		for (int jj = 0; jj < generateCloud->getGrid()->getMaxY(); jj++) {
			for (int ii = 0; ii < generateCloud->getGrid()->getMaxZ(); ii++) {
				if (generateCloud->getGrid()->getVoxelDensity(ii, jj, kk) > 0) {
					cloudPosCache.push_back(generateCloud->getGrid()->voxel2world(vec3(ii + 0.5, jj + 0.5, kk + 0.5)));
					cloudColCache.push_back(generateCloud->getGrid()->getVoxelColor(ii, jj, kk));
				}
			}
		}
	}
}

void processKeyboard(unsigned char key, int x, int y)
{
	if (input_flag && key == 13) { // Enter key
		if (input_type == 0) {
			int seed = atoi(input.c_str());
			if (seed < 1) seed = 18;
			generateCloud->generateVoxelGridRandom(seed);
			cacheCloud();
			input_flag = false;
			input = "";
		}
		else if (input_type == 1) {
			generateCloud->generateVoxelGridFromFile("data/" + input);
			cacheCloud();
			input_flag = false;
			input = "";
		}
		else if (input_type == 2) {
			generateCloud->generateVoxelGridRed("data/" + input);
			cacheCloud();
			input_flag = false;
			input = "";
		}
		else if (input_type == 3) {
			generateCloud->generateVoxelGridGreen("data/" + input);
			cacheCloud();
			input_flag = false;
			input = "";
		}
		else if (input_type == 4) {
			generateCloud->generateVoxelGridBlue("data/" + input);
			cacheCloud();
			input_flag = false;
			input = "";
		}
	}
	else if (input_flag && key == 27) { // Esc key
		input_flag = false;
		input = "";
	}
	else if (input_flag && key == 8) { // Backspace key
		input = input.substr(0, input.length() - 1);
	}
	else if (input_flag) {
		input += key;
	}
	else if (showRenderMenu) {
		switch (key) {
			case '1': { showRenderMenu = false; input_flag = true; input_type = 0; } break; // Create random cloud
			case '2': { showRenderMenu = false; input_flag = true; input_type = 1; } break; // Create cloud from config file
			case '3': { showRenderMenu = false; input_flag = true; input_type = 2; } break; // Create red color from config file
			case '4': { showRenderMenu = false; input_flag = true; input_type = 3; } break; // Create green color from config file
			case '5': { showRenderMenu = false; input_flag = true; input_type = 4; } break; // Create blue color from config file
			case 27:  { showRenderMenu = false; } break; // Esc key
			default: break;
		}
	}
	else {
		switch (key) {
			case 'r': { showRenderMenu = !showRenderMenu; showHelp = false; } break; // Show cloud menu
			case 'q': { if (quality == 4) quality = 0; else quality++; } break; // Change quality
			case 'o': { generateCloud->outputCloud(); } break; // Output cloud data
			case 'b': { if (bg[3] == 1) bg[3] = 0; else bg[3]++;
						if (bg[3] == 0) { bg[0] = 0.6; bg[1] = 0.8; bg[2] = 0.9; }
						if (bg[3] == 1) { bg[0] = 0.1; bg[1] = 0.2; bg[2] = 0.5; }
					  } break; // Change sky background
			case '1': { generateCloud->generateVoxelGridFromFile("data/lowres.txt"); cacheCloud(); } break; // Create low density test cloud
			case '2': { generateCloud->generateVoxelGridFromFile("data/highres.txt"); cacheCloud(); } break; // Create high density test cloud
			case 13:  { generateCloud->initLighting(vec3(10, 3, 3), vec3(1, 1, 1), vec3(bg[0], bg[1], bg[2]), quality, bg[3]);
					    generateCloud->initCamera(camera.GetEye(), camera.GetFrontVector(), camera.GetUpVector(), camera.GetFovY());
					    generateCloud->render();
					  } break; // Enter key
			case ' ': { generateCloud = new VolumeRender(); camera.Reset(); cloudPosCache.clear(); cloudColCache.clear();} break; // Reset everything
			case 27:  { exit(0); } break; // Esc key
			default: break;
		}
	}

	glutPostRedisplay();
}

void processKeyboardSpecial(int key, int x, int y)
{
	switch (key) {
		case GLUT_KEY_LEFT: {
			generateCloud->setCloudColor(rgb_edit, -0.05);
			cacheCloud();
			break;
		}
		case GLUT_KEY_RIGHT: {
			generateCloud->setCloudColor(rgb_edit, 0.05);
			cacheCloud();
			break;
		}
		case GLUT_KEY_UP: {
			if (rgb_edit == 2) rgb_edit = 0;
			else rgb_edit++;
			break;
		}
		case GLUT_KEY_DOWN: {
			if (rgb_edit == 0) rgb_edit = 2;
			else rgb_edit--;
			break;
		}
		case GLUT_KEY_F1: {
			if (!showRenderMenu)
				showHelp = !showHelp;
			break;
		}
		case GLUT_KEY_F2: {
			showInfo = !showInfo;
			break;
		}
		default: break;
	}
}

void processMouse(int button, int state, int x, int y)
{
	theButtonState = button;
	theModifierState = glutGetModifiers();

	if (state == GLUT_DOWN) {
		processMouseMotion(x, y);
	}
	else {
		lastMouseX = -1;
		lastMouseY = -1;
	}
}

void processMouseMotion(int x, int y)
{
	// Rotate camera
	if (theButtonState == GLUT_LEFT_BUTTON && theModifierState == GLUT_ACTIVE_ALT) {
		if (lastMouseX != -1) {
			dy = y - lastMouseY;
			dx = x - lastMouseX;
			camera.RotateCenter(-dx, -dy);
			renderDisplay();
		}
		lastMouseX = x;
		lastMouseY = y;
	}
	// Move camera
	else if (theButtonState == GLUT_MIDDLE_BUTTON && theModifierState == GLUT_ACTIVE_ALT) {
		if (lastMouseX != -1) {
			dx = (x - lastMouseX);
			dy = (y - lastMouseY);

			camera.MoveSWD(-dx, dy);
			renderDisplay();
		}
		lastMouseX = x;
		lastMouseY = y;
	}
	// Zoom camera
	else if (theButtonState == GLUT_RIGHT_BUTTON && theModifierState == GLUT_ACTIVE_ALT) {
		if (lastMouseY != -1) {			
			camera.Zoom(((y - lastMouseY) + (x - lastMouseX)) / 2.0f);
			renderDisplay();
		}
		lastMouseY = y;
		lastMouseX = x;
	}
	else {
		lastMouseX = -1;
		lastMouseY = -1;
	}
}