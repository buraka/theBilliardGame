// Billiard ball simulator
// Created by Nelis Franken
// -----------------------------------------------------------
// Sphere (billiard ball) header file
// -----------------------------------------------------------

#ifndef _MYSPHERE_H_
#define _MYSPHERE_H_

#include "vector3.h"

class mySphere {

	public:
		float weight;
		float mass;
		float radius;
		float fC;
		float accellSize;
		float forceSize;
		float speedSize;
		float speedDif;
		float rotation;
		vector3 speed;
		vector3 position;
		vector3 accell;
		vector3 force;
		vector3 reflected;
		bool collided;
		bool isRolling;
		bool inPlay;

		mySphere();
		mySphere(float posX, float posY, float posZ);
		void resetValuesToZero();
		void setDefaults();
		void setPos(float posX, float posY, float posZ);
		void setSpeed(float speedX, float speedY, float speedZ);
		bool roll();
		void updateSpeedSize();
		float getSpeedSize();
		float determineDistance(mySphere testBall);
		bool collides(mySphere testBall);

};

#endif
