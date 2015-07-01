#include "StdAfx.h"
#include "Vector.h"
#include "math.h"

/*----------------------------------------------------------------------------*/
/* cross   berechnet das Kreuzprodukt zweier Vektoren                         */
/*----------------------------------------------------------------------------*/
Vector Vector::cross(Vector &v)
{
	Vector	result;

	result.x =   y * v.z - v.y * z;
	result.y = -(x * v.z - v.x * z);
	result.z =   x * v.y - v.x * y;
	return (result);
} /* cross() */


/*----------------------------------------------------------------------------*/
/* dot   berechnet das innere Produkt (Skalarprodukt) zweier Vektoren         */
/*----------------------------------------------------------------------------*/

double Vector::dot(Vector &v)
{
	return(x * v.x + y * v.y + z * v.z);
} /* dot() */

/*----------------------------------------------------------------------------*/
/* normalize   normalisiert den uebergebenen Vektor                           */
/*----------------------------------------------------------------------------*/
Vector Vector::normalize()
{
	return svmpy( 1/veclength() );
} /* normalize() */


/*----------------------------------------------------------------------------*/
/* svmpy   berechnet das Produkt zwischen einem Skalar und einem Vektor       */
/*----------------------------------------------------------------------------*/
Vector Vector::svmpy(double s)
{
	return Vector(s*x, s*y, s*z);
} /* svmpy() */


/*----------------------------------------------------------------------------*/
/* vadd   berechnet die Summe zweier Vektoren                                 */
/* Aufrufparameter:    die beiden aufzusummierenden Vektoren                  */
/* Rueckgabeparameter: Vektorsumme                                            */
/*----------------------------------------------------------------------------*/

Vector Vector::vadd(Vector &v)
{
	return Vector(x+v.x, y+v.y, z+v.z);
} /* vadd() */

/*----------------------------------------------------------------------------*/
/* veclength   berechnet die Laenge eines Vektors                             */
/* Aufrufparameter:    Vektor, dessen Laenge berechnet werden soll            */
/* Rueckgabeparameter: Laenge des Vektors                                     */
/*----------------------------------------------------------------------------*/
double Vector::veclength ()
{
	return (sqrt (x*x + y*y + z*z));
} /* veclength() */


/*----------------------------------------------------------------------------*/
/* vsub   berechnet die Differenz zweier Vektoren                             */
/*----------------------------------------------------------------------------*/
Vector Vector::vsub(Vector &v)
{
	return Vector(x-v.x, y-v.y, z-v.z);
} /* vsub() */


Vector Vector::mult(Vector &v)
{
	return Vector(x*v.x, y*v.y, z*v.z);
}
