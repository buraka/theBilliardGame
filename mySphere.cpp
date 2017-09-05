// Billiard ball simulator
// Created by Nelis Franken
// -----------------------------------------------------------
// Sphere (billiard ball) implementation file
// -----------------------------------------------------------

#include <iostream>
#include <math.h>
#include "mySphere.h"

mySphere::mySphere() {
	resetValuesToZero();
}

mySphere::mySphere(float posX, float posY, float posZ) {
	resetValuesToZero();
	position = vector3(posX, posY, posZ);
}

void mySphere::resetValuesToZero() {
	weight = 0.0;
	mass = 0.0;
	radius = 0.0;
	fC = 0.0;
	accellSize = 0.0;
	forceSize = 0.0;
	speedSize = 0.0;
	speedDif = 0.0;
	rotation = 0.0;
	speed = vector3(0.0,0.0,0.0);
	position = vector3(0.0,24.9,0.0);
	accell = vector3(0.0,0.0,0.0);
	force = vector3(0.0,0.0,0.0);
	reflected = vector3(0.0,0.0,0.0);
	collided = false;
	inPlay = true;
}

void mySphere::setDefaults() {

	speedSize = 0.0;
	mass = 0.0001;
	weight = mass * 9.81;
	fC = 0.00004;
	speedDif = 10000.0;
	radius = 1.5;

	collided = false;
	inPlay = true;
	force = speed;
	forceSize = sqrt((force.x)*(force.x) + (force.y)*(force.y) + (force.z)*(force.z));

	if (forceSize != 0) {
		force = force * ((1.0 / forceSize) * (fC * weight));
  	}

	accell = (force * (1.0/mass)) * (-1.0);
	accellSize = fC * 170.0;
	accell = accell * accellSize;
}

void mySphere::setPos(float posX, float posY, float posZ) {
	position = vector3(posX, posY, posZ);
}

void mySphere::setSpeed(float speedX, float speedY, float speedZ) {
	speed = vector3(speedX, speedY, speedZ);
}

bool mySphere::roll() {

	updateSpeedSize();
	speed = speed + accell;
	speedDif = speedSize - sqrt((speed.x)*(speed.x) + (speed.y)*(speed.y) + (speed.z)*(speed.z));

	if (speedDif >= 0.0000001) {
		position = position + speed;
		return true;
	} else {
		accell = accell*0.0;
		speedSize = 0.0;
		return false;
	}
}

void mySphere::updateSpeedSize() {
	speedSize = sqrt((speed.x)*(speed.x) + (speed.y)*(speed.y) + (speed.z)*(speed.z));
}

float mySphere::getSpeedSize() {
	speedSize = (sqrt((speed.x)*(speed.x) + (speed.y)*(speed.y) + (speed.z)*(speed.z)));
	return speedSize;
}

float mySphere::determineDistance(mySphere testBall) {
	return (sqrt((position.x - testBall.position.x)*(position.x - testBall.position.x) + (position.y - testBall.position.y)*(position.y - testBall.position.y) + (position.z - testBall.position.z)*(position.z - testBall.position.z) ));
}

bool mySphere::collides(mySphere testBall) {

	float distance = determineDistance(testBall);

	if (distance >= (2.0*radius)) {
		return false;
	} else {
		//position = position + (position - testBall.position)*0.1;
		position = position - (testBall.position - position)*(2.0*radius - distance);
		//position = position - speed*1.4;

		return true;
	}

}
