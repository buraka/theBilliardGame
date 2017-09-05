// Billiard ball simulator
// Created by Nelis Franken
// -----------------------------------------------------------
//  Header file for basic vector class
// -----------------------------------------------------------

#ifndef _VECTOR3_H_
#define _VECTOR3_H_

class vector3 {

	public:
		double x, y, z;
		vector3();
		vector3(double _x, double _y, double _z);
		vector3(const vector3& L);
		void normalize();

		vector3 operator*(const vector3& Rhs);
		vector3 operator*(const double& Rhs);
		vector3 operator-(const vector3& Rhs);
		vector3 operator+(const vector3& Rhs);
		vector3 operator+(const double& Rhs);
		double operator%(const vector3& Rhs);
		vector3 operator=(const vector3& Rhs);

};

#endif
