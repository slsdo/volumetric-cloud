#pragma once

#include <stdio.h>
#include <string>

#ifdef __APPLE__
#include <GLUT/GLUT.h>
#else
#include <GL/glut.h>
#endif

// Displays a progress bar on-screen
void renderProgress(int progress_flag, int progress_bar)
{
	// Clearing the depth buffer 
	glClear(GL_DEPTH_BUFFER_BIT);

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
			glColor4f(1.0, 1.0, 1.0, 1.0);
			char progress_txt[1024];
			
			if (progress_flag == 0) {
				glRasterPos2f(480, 450);
				sprintf(progress_txt, "Generating...%d%%", progress_bar);
				for (unsigned int ii = 0; ii < strlen(progress_txt); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, progress_txt[ii]);
			}
			else if (progress_flag == 1) {
				glRasterPos2f(480, 450);
				sprintf(progress_txt, "Saving...%d%%", progress_bar);
				for (unsigned int ii = 0; ii < strlen(progress_txt); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, progress_txt[ii]);
			}
			else if (progress_flag == 2) {
				glRasterPos2f(480, 450);
				sprintf(progress_txt, "Importing...%d%%", progress_bar);
				for (unsigned int ii = 0; ii < strlen(progress_txt); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, progress_txt[ii]);
			}
			else if (progress_flag == 3) {
				glRasterPos2f(480, 450);
				sprintf(progress_txt, "Initializing...%d%%", progress_bar);
				for (unsigned int ii = 0; ii < strlen(progress_txt); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, progress_txt[ii]);
				//cout << progress_bar << endl;
			}
			else if (progress_flag == 4) {
				glRasterPos2f(480, 450);
				sprintf(progress_txt, "Rendering...%d%%", progress_bar);
				for (unsigned int ii = 0; ii < strlen(progress_txt); ii++)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, progress_txt[ii]);
			}
			
			glColor4f(0.9,0.9,0.9,1.0);
			glBegin(GL_QUADS); 
				glVertex2i(480, 463); 
				glVertex2i(480, 467); 
				glVertex2i(480 + (int)(progress_bar*0.01*135), 467); 
				glVertex2i(480 + (int)(progress_bar*0.01*135), 463);
			glEnd();
			
			glEnable(GL_LIGHTING);
			glColor4f(0.3, 0.3, 0.3, 0.2);
			glRectf(470.0, 435.0, 630.0, 475.0);
		glPopMatrix();
		
		glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glutSwapBuffers();
}