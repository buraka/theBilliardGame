// Billiard ball simulator
// Created by Nelis Franken
// -----------------------------------------------------------
// Custom vector class implementation file
// -----------------------------------------------------------

#include <iostream>
#include <math.h>
#include "vector3.h"


vector3::vector3() {

	x = 0.0;
	y = 0.0;
	z = 0.0;

}

vector3::vector3(double _x, double _y, double _z) {

	x = _x;
	y = _y;
	z = _z;

}

void vector3::normalize() {

	double power = sqrt(x*x + y*y + z*z);
	x = x / power;
	y = y / power;
	z = z / power;

}

vector3 vector3::operator*(const vector3& Rhs) {

	vector3 resultVector = vector3(0.0, 0.0, 0.0);

	resultVector.x = y*Rhs.z - z*Rhs.y;
	resultVector.y = -1.0*(x*Rhs.z - z*Rhs.x);
	resultVector.z = x*Rhs.y - y*Rhs.x;

	return resultVector;
}

double vector3::operator%(const vector3& Rhs) {

	double tempAnswer = 0.0;

	tempAnswer = (x * Rhs.x) + (y * Rhs.y) + (z * Rhs.z);

	return tempAnswer;
}

vector3 vector3::operator*(const double& Rhs) {

	vector3 resultVector = vector3(0.0, 0.0, 0.0);

	resultVector.x = x * Rhs;
	resultVector.y = y * Rhs;
	resultVector.z = z * Rhs;

	return resultVector;
}

vector3 vector3::operator-(const vector3& Rhs) {

	vector3 resultVector = vector3(0.0, 0.0, 0.0);

	resultVector.x = x - Rhs.x;
	resultVector.y = y - Rhs.y;
	resultVector.z = z - Rhs.z;

	return resultVector;
}

vector3 vector3::operator+(const vector3& Rhs) {

	vector3 resultVector = vector3(0.0, 0.0, 0.0);

	resultVector.x = x + Rhs.x;
	resultVector.y = y + Rhs.y;
	resultVector.z = z + Rhs.z;

	return resultVector;
}

vector3 vector3::operator+(const double& Rhs) {

	vector3 resultVector = vector3(0.0, 0.0, 0.0);

	resultVector.x = x + Rhs;
	resultVector.y = y + Rhs;
	resultVector.z = z + Rhs;

	return resultVector;
}

vector3 vector3::operator=(const vector3& Rhs) {

	x = Rhs.x;
	y = Rhs.y;
	z = Rhs.z;

	return *this;
}

vector3::vector3(const vector3& L) {
	x = L.x;
	y = L.y;
	z = L.z;
}
