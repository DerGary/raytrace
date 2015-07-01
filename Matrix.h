#pragma once
#include "Vector.h"
class Matrix
{
public:
	~Matrix(){};

	float M11, M12, M13;
	float	M21, M22, M23;
	float	M31, M32, M33;
	Matrix(float m11, float m12, float m13,
		float m21, float m22, float m23,
		float m31, float m32, float m33) :
		M11(m11), M12(m12), M13(m13),
		M21(m21), M22(m22), M23(m23),
		M31(m31), M32(m32), M33(m33){}
	Matrix(){};

};

bool operator==(const Matrix& left, const Matrix& right){
	return left.M11 == right.M11
		&& left.M12 == right.M12
		&& left.M13 == right.M13

		&& left.M21 == right.M21
		&& left.M22 == right.M22
		&& left.M23 == right.M23

		&& left.M31 == right.M31
		&& left.M32 == right.M32
		&& left.M33 == right.M33;

}
bool operator!=(const Matrix& left, const Matrix& right) {
	return !(left == right);
}

const Matrix IdentityMatrix = Matrix(1, 0, 0,
	0, 1, 0,
	0, 0, 1);

Matrix MatrixMultiply(Matrix a, Matrix b){
	return Matrix(a.M11 * b.M11 + a.M12 * b.M21 + a.M13 * b.M31,
		a.M11 * b.M12 + a.M12 * b.M22 + a.M13 * b.M32,
		a.M11 * b.M13 + a.M12 * b.M23 + a.M13 * b.M33,

		a.M21 * b.M11 + a.M22 * b.M21 + a.M23 * b.M31,
		a.M21 * b.M12 + a.M22 * b.M22 + a.M23 * b.M32,
		a.M21 * b.M13 + a.M22 * b.M23 + a.M23 * b.M33,

		a.M31 * b.M11 + a.M32 * b.M21 + a.M33 * b.M31,
		a.M31 * b.M12 + a.M32 * b.M22 + a.M33 * b.M32,
		a.M31 * b.M13 + a.M32 * b.M23 + a.M33 * b.M33);
}
Matrix operator*(const Matrix left, const Matrix right) {
	return MatrixMultiply(left, right);
}

Vector operator*(const Matrix left, const Vector right) {
	return Vector(left.M11*right.x + left.M12*right.y + left.M13*right.z,
		left.M21*right.x + left.M22*right.y + left.M23*right.z,
		left.M31*right.x + left.M32*right.y + left.M33*right.z);
}

