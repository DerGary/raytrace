#include "StdAfx.h"
#include "Objekt.h"


/*----------------------------------------------------------------------------*/
/* get_normal   gibt den Normalenvektor der Oberflaeche eines Objekts an dem  */
/*   in 'v' bestimmten Punkt zurueck.                                         */
/* Aufrufparameter:    Punkt-Vektor, fuer den der N.-Vektor berechnet werd. s.*/
/*                     Datenstruktur in der der Objektpointer enthalten ist   */
/* Rueckgabeparameter: Berechneter Normalenvektor                             */
/*----------------------------------------------------------------------------*/

Vector Objekt::get_normal(Vector &v)
{
	Vector normal( 
		v.dot(Vector(surface->a+surface->a, surface->b, surface->c)) + surface->d,
		v.dot(Vector(surface->b, surface->e+surface->e, surface->f)) + surface->g,
		v.dot(Vector(surface->c, surface->f, surface->h+surface->h)) + surface->j);

	return normal.normalize();
} /* get_normal() */

