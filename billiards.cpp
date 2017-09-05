// Billiard ball simulator
// Created by Nelis Franken
// -----------------------------------------------------------
//  Main implementation file
// -----------------------------------------------------------

#include "billiards.h"

// Loads 24-bit bitmap files with 1 plane only.
// (Disclaimer: This function originally obtained from http://nehe.gamedev.net)
int ImageLoad(const char *filename, Image *image) {

    FILE *file;
    unsigned long size;
    unsigned long i;
    unsigned short int planes;
    unsigned short int bpp;
    char temp, finalName[80];

	strcpy(finalName, "textures/" );
	strcat(finalName, filename );

    if ((file = fopen(finalName, "rb"))==NULL) {
		printf("File Not Found : %s\n",finalName);
		return 0;
    }

    fseek(file, 18, SEEK_CUR);

    if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
		printf("Error reading width from %s.\n", finalName);
		return 0;
    }

    if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
		printf("Error reading height from %s.\n", finalName);
		return 0;
    }

    size = image->sizeX * image->sizeY * 3;

    if ((fread(&planes, 2, 1, file)) != 1) {
		printf("Error reading planes from %s.\n", finalName);
		return 0;
    }

    if (planes != 1) {
		printf("Planes from %s is not 1: %u\n", finalName, planes);
		return 0;
    }

    if ((i = fread(&bpp, 2, 1, file)) != 1) {
		printf("Error reading bpp from %s.\n", finalName);
		return 0;
    }

    if (bpp != 24) {
	printf("Bpp from %s is not 24: %u\n", finalName, bpp);
	return 0;
    }

    fseek(file, 24, SEEK_CUR);

    image->data = (char *) malloc(size);
    if (image->data == NULL) {
		printf("Error allocating memory for color-corrected image data");
		return 0;
    }

    if ((i = fread(image->data, size, 1, file)) != 1) {
		printf("Error reading image data from %s.\n", finalName);
		return 0;
    }

    for (i=0;i<size;i+=3) {
		temp = image->data[i];
		image->data[i] = image->data[i+2];
		image->data[i+2] = temp;
    }

    return 1;
}

// Determines the normal from any three points on a plane.
vector3 getNormal(GLfloat point1[3], GLfloat point3[3], GLfloat point4[3]) {
	vector3 theNormal = vector3(0.0,0.0,0.0);
	theNormal.x = (point1[1] - point4[1])*(point3[2] - point4[2]) - (point3[1] - point4[1])*(point1[2] - point4[2]);
	theNormal.y = (point3[0] - point4[0])*(point1[2] - point4[2]) - (point1[0] - point4[0])*(point3[2] - point4[2]);
	theNormal.z = (point1[0] - point4[0])*(point3[1] - point4[1]) - (point3[0] - point4[0])*(point1[1] - point4[1]);
	return theNormal;
}

// Renders the billiard balls to screen (with dynamic shadows)
void renderBalls() {

	GLfloat m[16];
	for (int i=0; i < 15; i++) m[i] = 0.0;
	m[0] = m[5] = m[10] = 1.0;
	m[7] = (-1.0)/(light0Pos[1] + 2.0);

	for (int p=0; p < ballCount; p++) {
		glPushMatrix();
			glTranslatef(ballList[p].position.x,ballList[p].position.y,ballList[p].position.z);

			// Determine shadows.
			glPushMatrix();
				glTranslatef(light0Pos[0], light0Pos[1]+0.65, light0Pos[2]);
				glMultMatrixf(m);
				glTranslatef(-1.0*light0Pos[0], -1.0*light0Pos[1], -1.0*light0Pos[2]);
				glColor3f(0.0,0.0,0.0);
				glBindTexture(GL_TEXTURE_2D, theTexture[BLACK]);
				gluSphere(ballQuadric, ballList[p].radius, 32, 12);
			glPopMatrix();

			vector3 tempSpeed = vector3(0.0, 0.0, 0.0);
			tempSpeed = ballList[p].speed;
			tempSpeed.normalize();
			vector3 speedNormal = vector3(0.0,0.0,0.0);
			speedNormal.x = tempSpeed.z;
			speedNormal.y = 0.0;
			speedNormal.z = (-1.0)*tempSpeed.x;

			if (ballList[p].rotation > 360.0) ballList[p].rotation =  ballList[p].rotation - 360.0;

			ballList[p].rotation += ballList[p].speedSize/ballList[p].radius*(180.0/M_PI);

			glBindTexture(GL_TEXTURE_2D, theTexture[p]);
			glRotatef(ballList[p].rotation, speedNormal.x, speedNormal.y, speedNormal.z);

			gluSphere(ballQuadric, ballList[p].radius, 32, 32);
		glPopMatrix();
	}
}

// Renders a flat surface to screen with predefined width/height, segments and texture
void renderSurface(GLfloat width, GLfloat length, GLfloat widthSegments, GLfloat lengthSegments, GLfloat elevation, GLfloat texXTile, GLfloat texYTile, GLfloat normalX, GLfloat normalY, GLfloat normalZ, GLuint &surfaceTexture) {

	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, surfaceTexture);

		for (GLfloat k=(width/(-2.0)); k < (width/(2.0)); k += (width/widthSegments)) {
			for (GLfloat r=(length/(-2.0)); r < (length/(2.0)); r += (length/lengthSegments)) {

			glBegin(GL_QUADS);
				glNormal3f(normalX, normalY, normalZ);
				glTexCoord2f(0.0, texYTile);
				glVertex3f(k, elevation, r+length/lengthSegments);
				glTexCoord2f(texXTile, texYTile);
				glVertex3f(k+width/widthSegments, elevation, r+length/lengthSegments);
				glTexCoord2f(texXTile, 0.0);
				glVertex3f(k+width/widthSegments, elevation, r);
				glTexCoord2f(0.0, 0.0);
				glVertex3f(k, elevation, r);
			glEnd();

			}
		}

	glPopMatrix();
}

// Renders a quad represented by any 4 points to screen.
void renderQuad(GLfloat point1[3], GLfloat point2[3], GLfloat point3[3], GLfloat point4[3], GLfloat orientation, GLfloat texXTile, GLfloat texYTile) {

	glBegin(GL_QUADS);
		vector3 theNormal = vector3(0.0, 0.0, 0.0);
		theNormal = getNormal(point1, point3, point4);
		theNormal = theNormal*orientation;
		glNormal3f(theNormal.x, theNormal.y, theNormal.z);
		glTexCoord2f(0.0, texYTile);
		glVertex3fv(point1);
		glNormal3f(theNormal.x, theNormal.y, theNormal.z);
		glTexCoord2f(texXTile, texYTile);
		glVertex3fv(point2);
		glNormal3f(theNormal.x, theNormal.y, theNormal.z);
		glTexCoord2f(texXTile, 0.0);
		glVertex3fv(point3);
		glNormal3f(theNormal.x, theNormal.y, theNormal.z);
		glTexCoord2f(0.0, 0.0);
		glVertex3fv(point4);
	glEnd();

}

// Renders a curve (partial cylinder of any height and sweep) to screen.
void renderCurve(GLfloat radius, GLfloat height, GLfloat sweep, GLuint segments, GLfloat orientation, GLuint &aTexture) {

	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, aTexture);

		for (float t=0.0; t <= sweep-(sweep/segments); t += sweep/segments) {
			GLfloat x = 0.0, y = 0.0, z = 0.0;
			GLfloat point1[3] = {radius*cos(t), height, radius*sin(t)};
			GLfloat point2[3] = {radius*cos(t+sweep/segments), height, radius*sin(t+sweep/segments)};
			GLfloat point3[3] = {radius*cos(t+sweep/segments), 0.0, radius*sin(t+sweep/segments)};
			GLfloat point4[3] = {radius*cos(t), 0.0, radius*sin(t)};
			renderQuad(point1, point2, point3, point4, orientation, 1.0, 1.0);
		}

	glPopMatrix();
}

// Renders a disc (cap for a cylinder) of any sweep to screen.
void renderCap(GLfloat inner_radius, GLfloat outer_radius, GLfloat inner_sweep, GLfloat outer_sweep, GLuint segments, GLuint &myTexture) {

	glPushMatrix();

		GLfloat ciX = 0.0, coX = 0.0, ciZ = 0.0, coZ = 0.0;
		GLfloat angle = -1.0*outer_sweep/segments;

		glBindTexture(GL_TEXTURE_2D, myTexture);

		for (int k=0; k < segments; k++) {

			ciX = inner_radius*cos(angle + inner_sweep/segments);
			ciZ = inner_radius*sin(angle + inner_sweep/segments);
			coX = outer_radius*cos(angle + outer_sweep/segments);
			coZ = outer_radius*sin(angle + outer_sweep/segments);
			angle += inner_sweep/segments;

			glBegin(GL_QUADS);
				glNormal3f(0.0,1.0,0.0);
				glTexCoord2f(coX/(2.0*outer_radius), coZ/(2.0*outer_radius));
				glVertex3f(coX, 0.0, coZ);
				glNormal3f(0.0,1.0,0.0);
				glTexCoord2f(outer_radius*cos(angle + outer_sweep/segments)/(2.0*outer_radius), outer_radius*sin(angle + outer_sweep/segments)/(2.0*outer_radius));
				glVertex3f(outer_radius*cos(angle + outer_sweep/segments), 0.0, outer_radius*sin(angle + outer_sweep/segments));
				glNormal3f(0.0,1.0,0.0);
				glTexCoord2f(inner_radius*cos(angle + inner_sweep/segments)/(2.0*inner_radius), inner_radius*sin(angle + inner_sweep/segments)/(2.0*inner_radius));
				glVertex3f(inner_radius*cos(angle + inner_sweep/segments), 0.0, inner_radius*sin(angle + inner_sweep/segments));
				glNormal3f(0.0,1.0,0.0);
				glTexCoord2f(ciX/(2.0*inner_radius), ciZ/(2.0*inner_radius));
				glVertex3f(ciX, 0.0, ciZ);
			glEnd();
		}

	glPopMatrix();
}

// Renders a pocket of generic alignment and predefined sweep to screen.
void renderPocket(GLfloat sweep) {

	glPushMatrix();
		glTranslatef(0.0, -1.55, 0.0);
		renderCurve(5.5, 2.55, sweep, 16, 1,  theTexture[DARK_WOOD]);
		renderCurve(3.0, 2.55, sweep, 16, -1, theTexture[GREEN_CARPET]);
		renderCap(0.0001, 5.5, 6.3, 6.3, 16, theTexture[BLACK]);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.0, 1.0, 0.0);
		renderCap(3.0, 5.5, sweep, sweep, 16, theTexture[DARK_WOOD]);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.0, -1.4, 0.0);
		if (sweep < M_PI+0.001) renderCap(0.0001, 3.0, M_PI, M_PI, 16, theTexture[BLACK]);
		else renderCap(0.0001, 3.0, 6.3, 6.3, 16, theTexture[BLACK]);
	glPopMatrix();
}

// Renders the curved table legs to screen.
void renderTableLegs() {
	glNewList(displayList[4], GL_COMPILE);

	glBindTexture(GL_TEXTURE_2D, theTexture[DARK_WOOD]);

	glPushMatrix();

		for (double k=0; k < 3.0*M_PI; k+=1.4) {
			for (double g=0; g < M_PI*2.0; g+=0.5) {
				GLfloat point1[3] = {3.0*sin((k+1.4)/3.0)*cos(g), k*2, 3.0*sin((k+1.4)/3.0)*sin(g)};
				GLfloat point2[3] = {3.0*sin((k+1.4)/3.0)*cos(g+0.5), k*2, 3.0*sin((k+1.4)/3.0)*sin(g+0.5)};
				GLfloat point3[3] = {3.0*sin(k/3.0)*cos(g+0.5), k*2 - 2.8, 3.0*sin(k/3.0)*sin(g+0.5)};
				GLfloat point4[3] = {3.0*sin(k/3.0)*cos(g), k*2 - 2.8, 3.0*sin(k/3.0)*sin(g)};
				renderQuad(point1, point2, point3, point4, 1.0, 1.0, 1.0);
			}
		}

		glRotatef(90.0, 1.0, 0.0 ,0.0);
		glutSolidTorus(1.0, 1.8, 5, 8);
		gluCylinder(pillarCylinder, 1.0, 3.0, 4.0, 8, 2);

	glPopMatrix();

	glEndList();
}

// Draws one side of the table (generically aligned)
void drawSide() {

	// Wooden side
	glPushMatrix();
		glTranslatef(0, -0.3, 0);
		glScalef(1.0, 1.0, 2.0);
		glRotatef(270.0, 0.0, 1.0, 0.0);
		glRotatef(45.0, 0.0, 0.0, 1.0);
		glBindTexture(GL_TEXTURE_2D, theTexture[WOOD]);
		gluCylinder(pillarCylinder, 1.7, 1.7, tableWidth - 6, 4, 4);
	glPopMatrix();

	// Green carpeted side
	glPushMatrix();
		glTranslatef(-1.0*tableWidth + 6, 0.0, -2.35);
		glRotatef(180.0, 0.0, 0.0, 1.0);
		glRotatef(270.0, 0.0, 1.0, 0.0);
		glScalef(0.5, 1.0, 1.0);

		glBindTexture(GL_TEXTURE_2D, theTexture[GREEN_CARPET]);
		gluCylinder(pillarCylinder, 1.7, 1.7, tableWidth - 6, 3, 3);
	glPopMatrix();
}

// Draws the entire table to screen
void renderTable() {
	glNewList(displayList[1], GL_COMPILE);
	glColor3f(1.0,1.0,1.0);

	// Table carpeted area
	renderSurface(tableWidth, tableLength, 10.0, 15.0, -1.5, 1.0, 1.0, 0.0, 1.0, 0.0, theTexture[GREEN_CARPET]);

	// Anti-aliased game-lines on carpeted area
	glPushMatrix();
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
		glColor4f(0.5, 0.5, 0.5, 0.4);
		glLineWidth(2.5f);

		// Horisontal line
		glTranslatef(0.0, -1.45, 0.0);
		glBegin(GL_LINES);
			glVertex3f(minX - pocketRadius, 0.0, minZ - minZ/3);
			glVertex3f(maxX + pocketRadius, 0.0, minZ - minZ/3);
		glEnd();

		// Half-circle
		glTranslatef(0.0, 0.0, minZ - minZ/3);
		glRotatef(90.0, 0.0, 1.0, 0.0);
		glBegin(GL_LINES);
			for (GLfloat i = 0.0; i < M_PI-(M_PI/12.0); i+=M_PI/12.0) {
				glVertex3f((maxZ/6.0)*sin(i), 0.0, (maxZ/6.0)*cos(i));
				glVertex3f((maxZ/6.0)*sin(i+M_PI/12.0), 0.0, (maxZ/6.0)*cos(i+M_PI/12.0));
			}
		glEnd();
		glLineWidth(1.0f);

		// Start-point on line
		glRotatef(90.0, 1.0, 0.0, 0.0);
		gluDisk(pillarCylinder, 0, 0.3, 9, 1);

		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
	glPopMatrix();

	// Table sides
	GLint divCount = -5, angleDirection = 1;
	GLfloat offA = 2.4, offB = 3.0, offTemp = 0.0, sideAngle = 270.0;

	for(GLint f=1; f >= -1; f--) {
		for (GLfloat r=-1.0; r <= 1.0; r+=2.0) {

			glPushMatrix();
				glTranslatef(tableWidth/(r*2.0) + (abs(divCount)/divCount)*offA, 0.0, f*tableWidth + r*offB);
				glRotatef(sideAngle, 0.0, 1.0, 0.0);
				drawSide();
			glPopMatrix();

			divCount += 2;
			if ((f == 0) && (r == -1.0)) {
				offA = 2.4; offB = 3.0; offTemp = 0.0;
				sideAngle = 90.0;
				angleDirection = 1;
			} else {
				offTemp = offA; offA = offB; offB = offTemp;
				sideAngle = sideAngle + angleDirection*90.0;
				angleDirection *= -1;
			}
		}
	}

	// Pockets
	GLfloat direction = -1.0, angle = 360.0, sweep = M_PI + M_PI/2.0;
	bool sidePocket = false;

	for (int g=-1; g <= 1; g+=2) {
		for (int d=1; d >= -1; d-=1) {
			glPushMatrix();
				glTranslatef(g*tableWidth/(2.0), 0.0, d*tableWidth);
				glRotatef(angle,0.0,1.0,0.0);
				renderPocket(sweep);
			glPopMatrix();

			if (d == 1) {
				sweep = M_PI;
				if (g == -1) {
					angle = angle + direction*90;
					sidePocket = true;
				}
			} else {
				if (sidePocket == false) angle = angle + direction*90; else sidePocket = false;
				sweep = M_PI + M_PI/2.0;
			}
		}
		direction = -1*direction;
		angle = 90;
	}

	// Base
	glBindTexture(GL_TEXTURE_2D, theTexture[WOOD]);
	glPushMatrix();
		glTranslatef(0.0, -1.4, 0.0);
		glScalef(1.5, 1.0, 2.9);
		glRotatef(45.0, 0.0, 1.0, 0.0);
		glRotatef(90.0, 1.0, 0.0, 0.0);
		gluCylinder(pillarCylinder, tableWidth/2, tableWidth/2 - 2, 5, 4, 2);
	glPopMatrix();

	// Legs
	glBindTexture(GL_TEXTURE_2D, theTexture[DARK_WOOD]);
	for (int g=-1; g <= 1; g+=2) {
		for (int d=-1; d <= 1; d+=2) {
			glPushMatrix();
				glTranslatef(tableWidth/(g*2.0) + g*(-5.0), -20.0, tableLength/(d*2.0) + d*(-5.0));
				glCallList(displayList[4]);
			glPopMatrix();
		}
	}

	glEndList();
}

// Computes the stairs using a ton of adjustable settings (will document their use maybe later)
void renderStairs(GLint orientation, GLfloat stepCount, GLfloat stepWidth, GLfloat stepWidthDecrease, GLfloat totalHeight, GLuint &myFlatTexture, GLuint &myStepTexture) {

	glPushMatrix();

	GLfloat t = 0.0, curve = 0.0, nextCurve = 0.0, groundHeight = 0, groundIncrease = totalHeight/stepCount, depth = 70.0, moveRight = 0.0;
	GLfloat spreadFactor = 0.2, minRange = -1.0*sqrt(depth/spreadFactor)+moveRight, maxRange = sqrt(depth/spreadFactor)+moveRight;
	vector3 originVector = vector3(0.0, 0.0, 0.0);

	// Half-flight of stairs climbing from right to left
	if (orientation == 0) {
		maxRange = 0.0;

	// Half-flight of stairs climbing from left to right
	} else if (orientation == 1) {
		minRange = 0.0;
		groundHeight = stepCount*groundIncrease + 2.0*groundIncrease;
		stepWidth = stepWidth - stepCount*stepWidthDecrease;
		stepWidthDecrease *= -1.0;
		groundIncrease *= -1.0;
	}

	GLfloat incr = (sqrt((maxRange - minRange)*(maxRange - minRange)))/(stepCount+1);

	if (orientation == 1) {
		maxRange = maxRange - incr;
		minRange = 0.0 - 2.0*incr;
		minRange = minRange + incr;

	} else if (orientation == 0) {

	} else if (orientation == 2) {
		groundHeight = stepCount*groundIncrease + 2.0*groundIncrease;
		stepWidth = stepWidth - stepCount*stepWidthDecrease;
		stepWidthDecrease *= -1.0;
		groundIncrease *= -1.0;
	}

	t = minRange;

	for (t; t < maxRange; t+= incr) {

		curve = spreadFactor*((-1.0*(t - moveRight)*(t-moveRight))) + depth;
		nextCurve = spreadFactor*((-1.0*(t + incr - moveRight)*(t + incr -moveRight))) + depth;

		vector3 stepNormalV = vector3(0.0,0.0,0.0);
		stepNormalV.x = curve - (spreadFactor*((-1.0*(t + incr - moveRight)*(t+incr-moveRight))) + depth);
		stepNormalV.y = 0.0;
		stepNormalV.z = incr;

		GLfloat length = sqrt(  (stepNormalV.x*stepNormalV.x) + (stepNormalV.y*stepNormalV.y) + (stepNormalV.z*stepNormalV.z) );
		stepNormalV = stepNormalV * (1/length);
		stepNormalV = stepNormalV * stepWidth;

		GLfloat stairs1[3] = {stepNormalV.x + originVector.x, groundHeight + groundIncrease, stepNormalV.z + originVector.z};
		GLfloat stairs2[3] = {originVector.x, groundHeight + groundIncrease, originVector.z};
		GLfloat stairs3[3] = {originVector.x, groundHeight, originVector.z};
		GLfloat stairs4[3] = {stepNormalV.x + originVector.x, groundHeight, stepNormalV.z + originVector.z};
		GLfloat stairs6[3] = {t+incr, groundHeight + groundIncrease, nextCurve};
		GLfloat stairs7[3] = {t+incr, groundHeight, nextCurve};

		originVector.x = stairs6[0];
		originVector.y = stairs6[1];
		originVector.z = stairs6[2];

		curve = spreadFactor*((-1.0*(t+incr - moveRight)*(t+incr-moveRight))) + depth;
		nextCurve = spreadFactor*((-1.0*(t + incr+incr - moveRight)*(t +incr+ incr -moveRight))) + depth;
		stepWidth = stepWidth - stepWidthDecrease;

		stepNormalV = vector3(0.0,0.0,0.0);
		stepNormalV.x = curve - (spreadFactor*((-1.0*(t+incr + incr - moveRight)*(t+incr+incr-moveRight))) + depth);
		stepNormalV.y = 0.0;
		stepNormalV.z = incr;
		length = sqrt(  (stepNormalV.x*stepNormalV.x) + (stepNormalV.y*stepNormalV.y) + (stepNormalV.z*stepNormalV.z) );
		stepNormalV = stepNormalV * (1/length);
		stepNormalV = stepNormalV * stepWidth;

		GLfloat stairs5[3] = {stepNormalV.x + originVector.x, groundHeight + groundIncrease, stepNormalV.z + originVector.z};

		groundHeight = groundHeight + groundIncrease;

		if (t != minRange) {

			glBindTexture(GL_TEXTURE_2D, myStepTexture);

			if (orientation == 0) {
				renderQuad(stairs1, stairs2, stairs3, stairs4, 1.0, 1.0, 1.0);
			} else {
				renderQuad(stairs1, stairs2, stairs3, stairs4, -1.0, 1.0, 1.0);
			}

			if ((orientation == 1) && (t > maxRange-incr)) { } else {
				glBindTexture(GL_TEXTURE_2D, myFlatTexture);
				renderQuad(stairs1, stairs2, stairs6, stairs5, -1.0, 1.0, 1.0);
			}
		}
	}

	glPopMatrix();
}

// Renders the balcony (stairs plus red screen) to screen
void renderBalcony() {

	glNewList(displayList[3], GL_COMPILE);

	glPushMatrix();
		glTranslatef(0.0,0.0,-100.0);
		glRotatef(145, 0.0, 1.0, 0.0);
		renderCurve(70.0, 90.0, 1.945, 16, -1, theTexture[RED_LIGHT]);
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0,0.0,-100.0);
		glRotatef(180.0, 0.0, 1.0, 0.0);

		glPushMatrix();
			glTranslatef(-45.0, -3.0, 10.0);
			renderStairs(0, 15.0, 60.0, 1.5, 60.0, theTexture[RED_CARPET], theTexture[BLACK]);
		glPopMatrix();

		glPushMatrix();
			glTranslatef(45.0, 0.0, 0.0);
			renderStairs(1, 15.0, 60.0, 1.5, 60.0, theTexture[RED_CARPET], theTexture[BLACK]);
		glPopMatrix();
	glPopMatrix();

	glEndList();
}

// Renders the rest of the room (minus balcony, floot and table) to screen
void renderRoom() {

	glNewList(displayList[2], GL_COMPILE);

	GLfloat height= 90.0;

	// Roof
	glPushMatrix();
		glTranslatef(0.0, 90.0 ,0.0);
		glRotatef(-90.0, 1.0, 0.0, 0.0);
		glRotatef(22.5, 0.0, 0.0, 1.0);
		glBindTexture(GL_TEXTURE_2D, theTexture[ROOF]);
		gluCylinder(pillarCylinder, 230.0, 0.0, 50, 8, 1);
	glPopMatrix();

	// Windows
	glPushMatrix();
		glRotatef(292.5, 0.0, 1.0, 0.0);
		renderCurve(230, height, (M_PI/4.0), 1, -1, theTexture[SEA_VIEW]);
		glRotatef(-45, 0.0, 1.0, 0.0);
		renderCurve(230, height, (M_PI/4.0), 1, -1, theTexture[SEA_VIEW2]);
	glPopMatrix();

	glPushMatrix();
		glTranslatef(165, 20.0, -25.0);
		glRotatef(22.5, 0.0, 1.0, 0.0);

		renderCurve(50, 50, (M_PI/4.0), 1, -1, theTexture[PAINTING2]);

		glTranslatef(-20, 0.0, 50.0);
		renderCurve(50, 50, (M_PI/4.0), 1, -1, theTexture[PAINTING1]);
	glPopMatrix();

	glPushMatrix();
		glRotatef(202.5, 0.0, 1.0, 0.0);

		// Walls
		renderCurve(230.0, height, ((2*M_PI) - (M_PI/4.0)), 7, -1, theTexture[BLUE_WALL]);

		// Ceiling
		glPushMatrix();
			glTranslatef(0.0,90.0,0.0);
			renderCap(135.0, 230.0, M_PI*2.0, M_PI*2.0, 8, theTexture[BLACK]);
			renderCurve(135.0, 20.0, M_PI*2.0, 8, -1, theTexture[RED_MARBLE]);
		glPopMatrix();

		glPushMatrix();
			glTranslatef(0.0,0.5,0.0);
			renderCap(135.0, 230.0, M_PI*2.0, M_PI*2.0, 8, theTexture[BLACK]);
		glPopMatrix();

		// Pillars
		for (GLfloat t=0.0; t < M_PI*2.0; t+= M_PI*2.0/8.0) {

			glBindTexture(GL_TEXTURE_2D, theTexture[DARKBLUE_MARBLE]);

			// Pillars (far)
			glPushMatrix();
				glTranslatef(223.0*cos(t), height, 223.0*sin(t));
				glRotatef(90.0, 1.0, 0.0, 0.0);
				glutSolidTorus(5.0, 6.0, 8, 12);
				gluCylinder(pillarCylinder, 7.0, 7.0, height, 16, 1);
			glPopMatrix();
			glPushMatrix();
				glTranslatef(223.0*cos(t), 1.0, 223.0*sin(t));
				glRotatef(90.0, 1.0, 0.0, 0.0);
				glutSolidTorus(5.0, 6.0, 8, 12);
			glPopMatrix();

			glBindTexture(GL_TEXTURE_2D, theTexture[BEIGE_WALL]);

			// Pillars (near)
			glPushMatrix();
				glTranslatef(145.0*cos(t), height, 145.0*sin(t));
				glRotatef(90.0, 1.0, 0.0, 0.0);
				glutSolidTorus(1.5, 2.5, 8, 12);
				gluCylinder(pillarCylinder, 3.0, 3.0, height, 16, 1);
			glPopMatrix();
			glPushMatrix();
				glTranslatef(145.0*cos(t), 1.0, 145.0*sin(t));
				glRotatef(90.0, 1.0, 0.0, 0.0);
				glutSolidTorus(1.5, 2.5, 8, 12);
			glPopMatrix();
		}

	glPopMatrix();

	glEndList();
}

// Renders the cue stick to screen
void renderCueStick() {

	glPushMatrix();
		updateTarget();
		updateCamera();

		glTranslatef(camera[3], camera[4], camera[5]);
		glRotatef(theta*180.0/M_PI*(-1.0) + 90, 0.0, 1.0 ,0.0);
		glRotatef(-11.5, 1.0, 0.0 ,0.0);
		glTranslatef(0.0, 0.0, power);

		glBindTexture(GL_TEXTURE_2D, theTexture[CUE]);
		gluCylinder(ballQuadric, 0.35, 1.0, 50.0, 8, 1);
	glPopMatrix();
}

// Draws the guiding/aiming line to screen
void drawGuideLine() {
	GLfloat guideLineLength = -1000;
	GLfloat targetX = guideLineLength*(cos(theta)) + camera[3];
	GLfloat targetZ = guideLineLength*(sin(theta)) + camera[5];

	if (targetX < minX) {
		targetX = minX;
		guideLineLength = (targetX - camera[3]) / cos(theta);
		targetZ = guideLineLength*(sin(theta)) + camera[5];
	}

	if (targetX > maxX) {
		targetX = maxX;
		guideLineLength = (targetX - camera[3]) / cos(theta);
		targetZ = guideLineLength*(sin(theta)) + camera[5];
	}

	if (targetZ < minZ) {
		targetZ = minZ;
		guideLineLength = (targetZ - camera[5]) / sin(theta);
		targetX = guideLineLength*(cos(theta)) + camera[3];
	}

	if (targetZ > maxZ) {
		targetZ = maxZ;
		guideLineLength = (targetZ - camera[5]) / sin(theta);
		targetX = guideLineLength*(cos(theta)) + camera[3];
	}

	glPushMatrix();
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.61, 0.61, 0.06, 0.75);
		glBegin(GL_LINES);
			glVertex3f(camera[3], camera[4], camera[5]);
			glVertex3f(targetX, camera[4], targetZ);
		glEnd();
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
	glPopMatrix();
}

// Draws a text string on screen
void renderBitmapString(float x, float y, void *font, char* c) {
	glRasterPos2f(x, y);
	for (c; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

// Draws the end-of-game screen
void drawEndScreen() {

	switchToOrtho();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(0.1, 0.1, 0.1, 0.7);
	glBegin(GL_POLYGON);
		glVertex2f(0, parentWindowHeight);
		glVertex2f(parentWindowWidth, parentWindowHeight);
		glVertex2f(parentWindowWidth, 0);
		glVertex2f(0, 0);
	glEnd();

	glColor4f(1.0, 1.0, 1.0, 1.0);

	renderBitmapString(parentWindowWidth / 2 - 40, parentWindowHeight/2 - 75,(void *)largeFont, endGame);
	renderBitmapString(parentWindowWidth / 2 - 5, parentWindowHeight/2 - 60,(void *)font, "Game Over");
	renderBitmapString(parentWindowWidth / 2 - 18,parentWindowHeight/2 - 45,(void *)font, "Play again? (Y/N)");

	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	switchFromOrtho();
}

// Renders the user interface to screen
void drawUI() {
	GLfloat displayOffset = parentWindowWidth - 5;
	char playerBuffer[100];
	char shotBuffer[20];
	sprintf(shotBuffer, "%d", players[currentPlayer].noOfShots);

	switchToOrtho();
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glColor3f(1.0,1.0,1.0);
	renderBitmapString(80, 15, (void *)font, "Player: ");
	renderBitmapString(80, 30, (void *)font, "Team: ");
	renderBitmapString(80, 45, (void *)font, "Shots: ");

	glColor3f(0.8,0.8,0.8);
	renderBitmapString(130, 15, (void *)font, players[currentPlayer].name);
	if (players[currentPlayer].side == 0) renderBitmapString(130, 30, (void *)font, "Solids");
	else if (players[currentPlayer].side == 1) renderBitmapString(130, 30, (void *)font, "Stripes");
	else renderBitmapString(130, 30, (void *)font, "Undetermined");
	renderBitmapString(130, 45, (void *)font, shotBuffer);

	glColor3f(1.0,1.0,1.0);
	glEnable(GL_TEXTURE_2D);

	// Render portrait
	glBindTexture(GL_TEXTURE_2D, theTexture[players[currentPlayer].id + 32]);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0, 1.0);
		glVertex2f(5, 5);
		glTexCoord2f(1.0, 1.0);
		glVertex2f(69, 5);
		glTexCoord2f(1.0, 0.0);
		glVertex2f(69, 69);
		glTexCoord2f(0.0, 0.0);
		glVertex2f(5, 69);
	glEnd();

	// Render power-indicator-bar
	if (shootMode == true) {
		glBindTexture(GL_TEXTURE_2D, theTexture[POWER_BAR]);
		glBegin(GL_POLYGON);
			glTexCoord2f(0.0, 1.0);
			glVertex2f(5, 75);
			glTexCoord2f(power/35, 1.0);
			glVertex2f(5+power*4, 75);
			glTexCoord2f(power/35, 0.88);
			glVertex2f(5+power*4, 90);
			glTexCoord2f(0.0, 0.88);
			glVertex2f(5, 90);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glLineWidth(2.5f);
		glBegin(GL_LINE_LOOP);
			glVertex2f(5,75);
			glVertex2f(145,75);
			glVertex2f(145,90);
			glVertex2f(5,90);
		glEnd();
		glLineWidth(1.0f);
		glEnable(GL_TEXTURE_2D);
	}

	// Render iconic ball images for balls already sunk.
	for (int k=0; k < ballCount; k++) {
		if (players[currentPlayer].balls[k] == true) {
			glColor3f(1.0, 1.0, 1.0);
			glBindTexture(GL_TEXTURE_2D, theTexture[k+16]);
			glBegin(GL_POLYGON);
				glTexCoord2f(0.0, 1.0);
				glVertex2f(displayOffset - 32, 5);
				glTexCoord2f(1.0, 1.0);
				glVertex2f(displayOffset, 5);
				glTexCoord2f(1.0, 0.0);
				glVertex2f(displayOffset, 37);
				glTexCoord2f(0.0, 0.0);
				glVertex2f(displayOffset - 32, 37);
			glEnd();

			displayOffset -= 36.0;
		}
	}

	glEnable(GL_LIGHTING);
	switchFromOrtho();
}

// Updates the camera eye coordinates
void updateCamera() {
	camera[0] = zoom*(cos(theta)) + camera[3];
	camera[1] = zoom*sin(elevationAngle) + camera[4];
	camera[2] = zoom*(sin(theta)) + camera[5];
	if (camera[1] < 29) camera[1] = 29;
	if (camera[1] > 90) camera[1] = 90;
	glutPostRedisplay();
}

// Updates the camera look-at position
void updateTarget() {
	camera[3] = ballList[0].position.x;
	camera[4] = ballList[0].position.y;
	camera[5] = ballList[0].position.z;
	glutPostRedisplay();
}

// Switches from perspective to orthographic projection.
void switchToOrtho() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, parentWindowWidth, 0, parentWindowHeight);
	glScalef(1, -1, 1);
	glTranslatef(0, -1.0*parentWindowHeight, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Switch back from orthographic to perspective projection.
void switchFromOrtho() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

// The main rendering function
void renderParentWindow(void) {

	double eq [] = {0.0f, 1.0f, 0.0f, 0.0f};
	double eqr[] = {0.0f,-1.0f, 0.0f, 0.0f};

	glutSetWindow(mainWindow);
	glClearColor(0.6, 0.6, 0.6, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glLoadIdentity();

	if (gameover == false) {
		drawUI();
	} else {
		shootMode = false;
		doAnimation = false;
	}

	glLoadIdentity();
	glPushMatrix();

		gluLookAt(camera[0],camera[1],camera[2],camera[3],camera[4],camera[5], 0.0, 1.0, 0.0);

		renderBalls();
		glCallList(displayList[2]);
		glCallList(displayList[3]);

		if (doAnimation == true) {
			if (power >= 3.0) {
				power -= power/2.0;
			} else {
				ballList[0].setDefaults();
				ballList[0].updateSpeedSize();
				ballList[0].accell = (ballList[0].speed * -1.0) * (ballList[0].accellSize) * (1 / ballList[0].speedSize);
				power = 3.0;
				doAnimation = false;
				shootMode = false;
				turnStarting = false;
				players[currentPlayer].noOfShots -= 1;

				alSourcePlay(soundClip[SHOOT]);
				if ((error = alGetError()) != AL_NO_ERROR)
					DisplayALError("alSourcePlay 0 : ", error);

				glutIdleFunc(idle);
			}

			glutPostRedisplay();
		}

		// The following (rather large) section of code is responsible for the reflection of
		// the table on the floor.
		glDisable(GL_DEPTH_TEST);
		glColorMask(0,0,0,0);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glDisable(GL_DEPTH_TEST);

		glPushMatrix();
			glTranslatef(0.0f, 25.0, 0.0f);
			renderSurface(500.0, 500.0, 1.0, 1.0, -24.0, 12.0, 12.0, 0.0, 1.0, 0.0,theTexture[CHECKER]);
		glPopMatrix();

		glEnable(GL_DEPTH_TEST);
		glColorMask(1,1,1,1);
		glStencilFunc(GL_EQUAL, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glEnable(GL_CLIP_PLANE0);
		glClipPlane(GL_CLIP_PLANE0, eqr);

		glPushMatrix();
			glScalef(1.0f, -1.0f, 1.0f);
			glTranslatef(0.0f, 25.0f, 0.0f);
			glCallList(displayList[1]);
		glPopMatrix();

		glDisable(GL_CLIP_PLANE0);
		glDisable(GL_STENCIL_TEST);

		glLightfv(GL_LIGHT0,GL_POSITION,light0Pos);
		glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,light0Dir);
		glLightfv(GL_LIGHT1,GL_POSITION,light1Pos);

		glEnable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glPushMatrix();
			glTranslatef(0.0f, 25.0f, 0.0f);
			glRotatef(45.0, 0.0, 1.0, 0.0);
			renderSurface(500.0, 500.0, 1.0, 1.0, -24.0, 12.0, 12.0, 0.0, 1.0, 0.0, theTexture[CHECKER]);
		glPopMatrix();

		glEnable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glEnable(GL_CLIP_PLANE0);
		glClipPlane(GL_CLIP_PLANE0, eq);

		glPushMatrix();
			glTranslatef(0.0f, 25.0f, 0.0f);
			glCallList(displayList[1]);
		glPopMatrix();

		glDisable(GL_CLIP_PLANE0);

		if (shootMode == true) {
			renderCueStick();
			if (useLazer == true) drawGuideLine();
		}

	glPopMatrix();

	if (gameover == true) {
		drawEndScreen();
	}

	glutSwapBuffers();
}

// The idle function, handles collision detection and response for the table (and various other things like ball animation)
void idle() {

	// Check for and handle collisions between balls
	for (int s=0; s < (ballCount-1); s++) {
		for (int t=(s+1); t < ballCount; t++) {
			if ((ballList[s].collides(ballList[t]) == true) && (ballList[s].collided == false) && (ballList[t].collided == false) && (ballList[s].inPlay == true) && (ballList[t].inPlay == true)) {
				deflectBalls(s, t);
			}
		}
	}

	// Check for and handle collisions between balls and table sides
	for (int t=0; t < ballCount; t++) {

		if(ballList[t].inPlay == true) {

			// Determine if the ball was sunk into a pocket
			if (ballList[t].position.z > (maxZ - pocketRadius) ) {
				if (ballList[t].position.x < (minX + pocketRadius)) ballInPocket(t, 1);
				else if (ballList[t].position.x > (maxX - pocketRadius)) ballInPocket(t, 2);
			} else if (ballList[t].position.z < (minZ + pocketRadius) )	{
				if (ballList[t].position.x < (minX + pocketRadius)) ballInPocket(t, 3);
				else if (ballList[t].position.x > (maxX - pocketRadius)) ballInPocket(t, 4);
			} else if ((ballList[t].position.z < (pocketRadius)) && (ballList[t].position.z > (-1.0*pocketRadius)))	{
				if (ballList[t].position.x < (minX + pocketRadius)) ballInPocket(t, 5);
				else if (ballList[t].position.x > (maxX - pocketRadius)) ballInPocket(t, 6);
			}

			if ((ballList[t].position.x > maxX) || (ballList[t].position.x < minX)) {
				ballList[t].position.x = ballList[t].position.x - ballList[t].speed.x*1.3;
				ballList[t].position.z = ballList[t].position.z - ballList[t].speed.z*1.3;
				ballList[t].speed.x = ballList[t].speed.x*(-0.8);
				ballList[t].speed.z = ballList[t].speed.z*(0.8);
				ballList[t].accell.x = ballList[t].accell.x*(-1.0);

				alSourcePlay(soundClip[SIDES]);
				if ((error = alGetError()) != AL_NO_ERROR)
					DisplayALError("alSourcePlay SIDES : ", error);
			}

			if ((ballList[t].position.z > maxZ) || (ballList[t].position.z < minZ)) {
				ballList[t].position.x = ballList[t].position.x - ballList[t].speed.x*1.3;
				ballList[t].position.z = ballList[t].position.z - ballList[t].speed.z*1.3;
				ballList[t].speed.z = ballList[t].speed.z*(-0.8);
				ballList[t].speed.x = ballList[t].speed.x*(0.8);
				ballList[t].accell.z = ballList[t].accell.z*(-1.0);

				alSourcePlay(soundClip[SIDES]);
				if ((error = alGetError()) != AL_NO_ERROR)
					DisplayALError("alSourcePlay SIDES : ", error);
			}
		}
	}

	rollingBalls = false;

	// Let the balls roll on
	for (int p=0; p < ballCount; p++) {
		if (ballList[p].inPlay == true) {
			rollingBalls = rollingBalls | ballList[p].roll();
		} else {
			rollingBalls = rollingBalls | false;
		}

		ballList[p].collided = false;
	}

	if (rollingBalls == false) {
		if (returnWhiteBall == true) {
			ballList[0] = mySphere(0.0, 24.9 , minZ - minZ/3);
			ballList[0].setDefaults();
			returnWhiteBall = false;
		}

		if (turnStarting == false) {
			if (players[currentPlayer].noOfShots <= 0) {
				turnStarting = true;
				players[currentPlayer].noOfShots = 0;
				currentPlayer = (currentPlayer + 1) % playerCount;
				players[currentPlayer].noOfShots += 1;
			} else {
				turnStarting = true;
			}
		}
	}

	if (turnStarting == true) {
		glutIdleFunc(NULL);
	}

	glutPostRedisplay();
}

// Handles normal (non-keypad) key input.
void keys(unsigned char key, int x, int y) {

	if (key == ESCAPE) exit(1);

	// Rotate left
	if(key == 'a') {
		theta = theta - 0.05;
		camera[0] = 65.0*(cos(theta)) + camera[3];
		camera[2] = 65.0*(sin(theta)) + camera[5];
		updateCamera();
	}

	// Rotate right
	if(key == 'd') {
		theta = theta + 0.05;
		camera[0] = 65.0*(cos(theta)) + camera[3];
	   	camera[2] = 65.0*(sin(theta)) + camera[5];
	   	updateCamera();
    }

	// Move forward
	if(key == 'w') {
		zoom -= 1.0;
		if (zoom < 10) zoom = 10;
		updateCamera();
 	}

	// Move backward
	if(key == 's') {
		zoom += 1.0;
		if (zoom > 100) zoom = 100;
		updateCamera();
	}

	// Ascend
	if(key == 'z') {
		camera[1] = camera[1] + 1.0;
		if (camera[1] > 90.0) camera[1] = 90.0;
		updateCamera();
	}

	// Descend
	if(key == 'x') {
		camera[1] = camera[1] - 1.0;
		if (camera[1] < 24.9) camera[1] = 24.9;
		updateCamera();
	}

	// Increase power of shot
	if(key == 'g') {
		power += 2.0;
		if (power >= 35.0) power = 35.0;
	}

	// Decrease power of shot
	if (key == 'f') {
		power -= 2.0;
		if (power <= 3.0) power = 3.0;
	}

	// Shoot
	if (key == 't') {
		GLfloat powX = power*(cos(theta));
	   	GLfloat powZ = power*(sin(theta));
		ballList[0].speed.x = powX*(-0.1);
		ballList[0].speed.z = powZ*(-0.1);
		doAnimation = true;
	}

	// Toggle shootmode
	if (key == ' ') {
		shootMode = !shootMode;
	}

	// Handle choice at end of game
	if (gameover == true) {
		if (key == 'y') resetGame();
		if (key == 'n') exit(0);
	}

	// Toggle lazer guided aiming
	if (key == 'l') {
		useLazer = !useLazer;
	}

	glutSetWindow(mainWindow);
	glutPostRedisplay();
}

// Handles all special (keypad) key input.
void specialKeyPressed(int key, int x, int y) {
    switch (key) {
    	case GLUT_KEY_PAGE_UP: 		keys('z',0,0); break;
    	case GLUT_KEY_PAGE_DOWN: 	keys('x',0,0); break;
	    case GLUT_KEY_UP: 			keys('w',0,0); break;
	    case GLUT_KEY_DOWN: 		keys('s',0,0); break;
	    case GLUT_KEY_LEFT:			keys('a',0,0); break;
	    case GLUT_KEY_RIGHT:		keys('d',0,0); break;
	}
}

// Handles any mouse input from the user.
void useMouse(int button, int state, int x, int y) {

	prevMouseX = x;
	prevMouseY = y;

	if (turnStarting == true) updateTarget();

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		pressedTheta = theta;
		pressedElevation = elevationAngle;
		leftMouseButton = true;
	}

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
		leftMouseButton = false;
	}

	if (rollingBalls == false) {
		if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
			pressedTheta = theta;
			shootMode = true;
			rightMouseButton = true;
		}

		if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {

			rightMouseButton = false;

			GLfloat powX = power*(cos(theta));
	   		GLfloat powZ = power*(sin(theta));

			ballList[0].speed.x = powX*(-0.1);
			ballList[0].speed.z = powZ*(-0.1);

			doAnimation = true;
		}
	}

	if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN)) {
		middleMouseButton = true;
		pressedZoom = zoom;
	}

	if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_UP)) {
		middleMouseButton = false;
	}

	glutPostRedisplay();
}

// Determines what happens when the mouse is moved.
void lookMouse(int x, int y) {

	if (leftMouseButton == true) {
		theta = pressedTheta + (x - prevMouseX)*0.005;
		elevationAngle = pressedElevation + (y - prevMouseY)*0.005;
	}

	if (rightMouseButton == true) {
		theta = pressedTheta + (x - prevMouseX)*0.005;
		power = 3.0 + (y - prevMouseY)*0.5;
		if (power >= 35.0) power = 35.0;
		if (power <= 3.0) power = 3.0;
	}

	if (middleMouseButton == true) {
		zoom = pressedZoom + (y - prevMouseY)*1.0;
		if (zoom < 10) zoom = 10;
		if (zoom > 100) zoom = 100;
	}

	updateCamera();
	glutPostRedisplay();
}

// Handles the collision of two balls on the table.
void deflectBalls(int ballOne, int ballTwo) {

	vector3 midPointConnectV = vector3(0.0, 0.0, 0.0);
	vector3 relative = vector3(0.0, 0.0, 0.0);
	float impulse = 0.0, e = 0.8;

	midPointConnectV = ballList[ballOne].position - ballList[ballTwo].position;
	relative = ballList[ballOne].speed - ballList[ballTwo].speed;
	impulse = ((-1.0)*(1.0 + e)*(relative % midPointConnectV)) / (midPointConnectV % (midPointConnectV * (2.0 / ballList[ballOne].mass)));

	ballList[ballOne].collided = true;
	ballList[ballOne].speed = ballList[ballOne].speed + ((midPointConnectV)*(impulse / ballList[ballOne].mass));
	ballList[ballOne].updateSpeedSize();
	if (ballList[ballOne].speedSize == 0) ballList[ballOne].speedSize = 0.000001;
	ballList[ballOne].accell = (ballList[ballOne].speed * -1.0) * (ballList[ballOne].accellSize) * (1 / ballList[ballOne].speedSize);

	ballList[ballTwo].collided = true;
	ballList[ballTwo].speed = ballList[ballTwo].speed - ((midPointConnectV)*(impulse / ballList[ballTwo].mass));
	ballList[ballTwo].updateSpeedSize();
	if (ballList[ballTwo].speedSize == 0) ballList[ballTwo].speedSize = 0.000001;
	ballList[ballTwo].accell = (ballList[ballTwo].speed * -1.0) * (ballList[ballTwo].accellSize) * (1 / ballList[ballTwo].speedSize);

	alSourcePlay(soundClip[COLLIDE]);
	if ((error = alGetError()) != AL_NO_ERROR)
		DisplayALError("alSourcePlay COLLIDE : ", error);
}

// Determines what happens when a ball is hit into a pocket.
void ballInPocket(int ballNr, int pocketNr) {

	ballList[ballNr].setPos(-1000.0, 2000, -1000);

	if ((ballNr != 0) && (ballList[ballNr].inPlay == true)) {

		// No team
		if (players[currentPlayer].side == -1) {
			if (ballNr > 8) {
				players[currentPlayer].side = 1;
				players[(currentPlayer+1)%playerCount].side = 0;
			} else if (ballNr < 8) {
				players[currentPlayer].side = 0;
				players[(currentPlayer+1)%playerCount].side = 1;
			}

			if (ballNr != 8) {
				players[currentPlayer].noOfShots += 1;
				players[currentPlayer].balls[ballNr] = true;
			}

		// Has team
		} else {

			// Check that the correct team ball was hit
			if (players[currentPlayer].side == 0) {
				if (ballNr < 8) {
					players[currentPlayer].noOfShots += 1;
					players[currentPlayer].balls[ballNr] = true;
				} else if (ballNr > 8) {
					players[currentPlayer].noOfShots = 0;
					players[(currentPlayer+1)%playerCount].noOfShots += 1;
					players[(currentPlayer+1)%playerCount].balls[ballNr] = true;
				}
			} else if (players[currentPlayer].side == 1) {
				if (ballNr > 8) {
					players[currentPlayer].noOfShots += 1;
					players[currentPlayer].balls[ballNr] = true;
				} else if (ballNr < 8) {
					players[currentPlayer].noOfShots = 0;
					players[(currentPlayer+1)%playerCount].noOfShots += 1;
					players[(currentPlayer+1)%playerCount].balls[ballNr] = true;
				}
			}
		}

		// Eight-ball has been sunk (end of game), determine winner (if any).
		if (ballNr == 8) {
			if (players[currentPlayer].side != -1) {
				int completeList = 0;
				for (int k=1; k < 8; k++) {
					if (players[currentPlayer].balls[players[currentPlayer].side*8 + k] == true)
						completeList++;
				}

				if (completeList == 7) {
					strcat(endGame, "Player ");
					strcat(endGame, players[currentPlayer].name);
					strcat(endGame, " wins! ");
				}
			}

			gameover = true;
			glutPostRedisplay();
		}

	} else {
		returnWhiteBall = true;
		players[(currentPlayer+1)%playerCount].noOfShots += 1;
		players[currentPlayer].noOfShots = 0;
	}

	ballList[ballNr].inPlay = false;

	alSourcePlay(soundClip[SINK]);
	if ((error = alGetError()) != AL_NO_ERROR)
		DisplayALError("alSourcePlay 0 : ", error);
}

// Instantiates the physics objects and reset them to predefined states.
void initPhysics() {

	// Setup balls' positions on table
	ballList[0] = mySphere(0.0, 24.9 , minZ - minZ/3);

	// Closely packed triangle (may cause some problems for collision detection/response)
	ballList[15] = mySphere(7.0, 24.9 , 27.0);
	ballList[1] = mySphere(3.5, 24.9 , 27.0);
	ballList[14] = mySphere(0.0, 24.9 , 27.0);
	ballList[13] = mySphere(-3.5, 24.9 , 27.0);
	ballList[2] = mySphere(-7.0, 24.9 , 27.0);

	ballList[3] = mySphere(5.25, 24.9 , 23.5);
	ballList[12] = mySphere(1.75, 24.9 , 23.5);
	ballList[4] = mySphere(-1.75, 24.9 , 23.5);
	ballList[11] = mySphere(-5.25, 24.9 , 23.5);

	ballList[10] = mySphere(3.5, 24.9 , 20.0);
	ballList[8] = mySphere(0.0, 24.9 , 20.0);
	ballList[5] = mySphere(-3.5, 24.9 , 20.0);

	ballList[6] = mySphere(1.75, 24.9 , 16.5);
	ballList[7] = mySphere(-1.75, 24.9 , 16.5);

	ballList[9] = mySphere(0.0, 24.9 , 13.0);

	// Initialize physics properties for balls
	for (int r=0; r < ballCount; r++) {
		ballList[r].setDefaults();
	}
}

// Resets the game parameters and camera
void resetGame() {
	initPhysics();
	setupPlayers();
	gameover = false;
	theta = M_PI + M_PI/2.0;
	updateTarget();
	updateCamera();
	endGame[0] = '\0';
}

// Sets up the list of players
void setupPlayers() {

	for (int p=0; p < playerCount; p++) {
		for (int i = 0; i < ballCount; i++) players[p].balls[i] = false;

		players[p].name = nameList[p];
		players[p].noOfShots = 0;
		players[p].side = -1;
		players[p].id = p;
	}

	// First player in list starts the game
	players[0].noOfShots = 1;
	currentPlayer = 0;
}

// Rendered window's reshape function (manages changes to size of the window).
void changeParentWindow(GLsizei width, GLsizei height) {

	if (height == 0) height = 1;
	glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(45.0, (GLfloat)width/height, 0.1, 5000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Initializes the OpenGL display lists
void setupLists() {

	GLint listCount = 5;
	displayList[0] = glGenLists(listCount-1);

	for (GLint k=1; k < listCount; k++) {
		displayList[k] = displayList[0] + k;
	}

	renderTableLegs();
	renderTable();
	renderRoom();
	renderBalcony();
}

// Displays OpenAL error messages
ALvoid DisplayALError(ALbyte *szText, ALint errorcode) {
	printf("%s%s", szText, alGetString(errorcode));
}

// Loads .WAV sound files using OpenAL (opensource multi-platform audio libraries)
void loadSound() {
	ALfloat listenerPos[]={0.0,0.0,0.0};
	ALfloat listenerVel[]={0.0,0.0,0.0};
	ALfloat listenerOri[]={0.0,0.0,-1.0, 0.0,1.0,0.0}; // "at", then "up"
	ALsizei size,freq;
	ALenum	format;
	ALvoid	*data;
	ALboolean loop;
	const ALuint NUM_BUFFERS = 4;
	char* audioFiles[NUM_BUFFERS] = {"audio/shoot.wav", "audio/hit.wav", "audio/side.wav", "audio/sunk.wav"};
	ALuint g_Buffers[NUM_BUFFERS];

	// Initialize OpenAL
	alutInit(0, NULL);
	alGetError();

	// Generate Buffers
	alGenBuffers(NUM_BUFFERS, g_Buffers);
	for (ALuint k=0; k < NUM_BUFFERS; k++) {
		alutLoadWAVFile(audioFiles[k],&format,&data,&size,&freq,&loop);
		alBufferData(g_Buffers[k],format,data,size,freq);
		alutUnloadWAV(format,data,size,freq);
	}

	// Generate Sources
	alGenSources(NUM_BUFFERS,soundClip);
	for (ALuint r=0; r < NUM_BUFFERS; r++) {
		alSourcei(soundClip[r], AL_BUFFER, g_Buffers[r]);
	}

	// Set Listener Position ...
	alListenerfv(AL_POSITION,listenerPos);
	if ((error = alGetError()) != AL_NO_ERROR) {
		DisplayALError("alListenerfv POSITION : ", error);
		exit(-1);
	}

	// Set Listener Velocity ...
	alListenerfv(AL_VELOCITY,listenerVel);
	if ((error = alGetError()) != AL_NO_ERROR) {
		DisplayALError("alListenerfv VELOCITY : ", error);
		exit(-1);
	}

	// Set Listener Orientation ...
	alListenerfv(AL_ORIENTATION,listenerOri);
	if ((error = alGetError()) != AL_NO_ERROR) {
		DisplayALError("alListenerfv ORIENTATION : ", error);
		exit(-1);
	}
}

// Initializes basic lighting parameters for use in the final rendered scene.
void initlights(void) {

	// Global parameters / settings
	GLfloat lmodel_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	GLfloat ambient[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f};

	// Table spotlight (0)
	glLightfv(GL_LIGHT0,GL_SPECULAR,specular);
	glLightfv(GL_LIGHT0,GL_POSITION,light0Pos);
	glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,45.0f);
	glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,100.0f);

	// Global Ambient light (1)
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, ambient);
    glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);

	// Material settings
    GLfloat mat_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 80.0 };
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	// Enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
}

// Load Bitmaps And Convert To Textures
void LoadGLTextures() {

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (int k=0; k < textureCount; k++) {
		if (!ImageLoad(textureFilenames[k], &myTextureData[k])) exit(1);

		// Various texture specific parameters, including wrapping/tiling and filtering.
		glGenTextures(1, &theTexture[k]);
	    glBindTexture(GL_TEXTURE_2D, theTexture[k]);
	   	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	   	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	   	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	   	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, myTextureData[k].sizeX, myTextureData[k].sizeY, GL_RGB, GL_UNSIGNED_BYTE, myTextureData[k].data);
	}
}

// Sets up some global OpenGL parameters..
void initScene(int argc, char **argv) {

	LoadGLTextures();
	loadSound();

	glShadeModel(GL_SMOOTH);
	glClearStencil(0);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_2D);

	// Instantiates the quadric object used to render the pillars and table sides (etc.).
	pillarCylinder=gluNewQuadric();
	gluQuadricNormals(pillarCylinder, GLU_FLAT);
	gluQuadricOrientation(pillarCylinder, GLU_OUTSIDE);
	gluQuadricTexture(pillarCylinder, GL_TRUE);

	// Instantiates the quadric object used to render the billiard balls.
	ballQuadric=gluNewQuadric();
	gluQuadricNormals(ballQuadric, GLU_SMOOTH);
	gluQuadricOrientation(ballQuadric, GLU_OUTSIDE);
	gluQuadricTexture(ballQuadric, GL_TRUE);
}

// The main function, sets up the window and links up the OpenGL GLUT functions.
int main(int argc, char **argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH  | GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(parentWindowWidth,parentWindowHeight);

	mainWindow = glutCreateWindow("The Blue Room");
	glutReshapeFunc(changeParentWindow);
	glutDisplayFunc(renderParentWindow);
	glutKeyboardFunc(keys);
	glutSpecialFunc(specialKeyPressed);
	glutMouseFunc(useMouse);
	glutMotionFunc(lookMouse);
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	glutIdleFunc(idle);
	glutFullScreen();

	initScene(argc,argv);
	initlights();
	setupLists();
	resetGame();
	glutMainLoop();

	return(0);
}
