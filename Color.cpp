#include "StdAfx.h"
#include "Color.h"

/*----------------------------------------------------------------------------*/
/* addcolor   addiert zwei Farben zusammen                                    */
/*----------------------------------------------------------------------------*/
Color Color::addcolor(Color &c)
{
	return Color(r+c.r, g+c.g, b+c.b);
} /* addcolor() */


/*----------------------------------------------------------------------------*/
/* outprodc   'filtert' eine Farbe                                            */
/*----------------------------------------------------------------------------*/

Color Color::outprodc(Color &c)
{
	return Color(r*c.r, g*c.g, b*c.b);
} /* outprodc() */


/*----------------------------------------------------------------------------*/
/* scmpy   skaliert (dimmt) eine Farbe                                        */
/*----------------------------------------------------------------------------*/

Color Color::scmpy(double s)
{
	return Color(r*s, g*s, b*s);
} /* scmpy */


