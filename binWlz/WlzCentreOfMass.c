#if defined(__GNUC__)
#ident "University of Edinburgh $Id$"
#else
static char _WlzCentreOfMass_c[] = "University of Edinburgh $Id$";
#endif
/*!
* \file         binWlz/WlzCentreOfMass.c
* \author       Bill Hill
* \date         March 1999
* \version      $Id$
* \par
* Address:
*               MRC Human Genetics Unit,
*               MRC Institute of Genetics and Molecular Medicine,
*               University of Edinburgh,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \par
* Copyright (C), [2012],
* The University Court of the University of Edinburgh,
* Old College, Edinburgh, UK.
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be
* useful but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
* \brief	Calculates the mass and centre of mass of domain objects.
* \ingroup	BinWlz
*
* \par Binary
* \ref wlzcentreofmass "WlzCentreOfMass"
*/

/*!
\ingroup      BinWlz
\defgroup     wlzcentreofmass WlzCentreOfMass
\par Name
WlzCentreOfMass - calculates the mass and centre of
mass of domain objects.
\par Synopsis
\verbatim
WlzCentreOfMass [ -o<output file>] [-b] [-h] [<input object file>]

\endverbatim
\par Options
<table width="500" border="0">
  <tr>
    <td><b>-o</b></td>
    <td>output file name</td>
  </tr>
  <tr>
    <td><b>-b</b></td>
    <td>object is considered a binaryy object, i.e. just use the domain</td>
  </tr>
  <tr>
    <td><b>-h</b></td>
    <td>Help - print help message</td>
  </tr>
  <tr>
    <td><b>-v</b></td>
    <td>Verbose operation</td>
  </tr>
</table>
By default the input object is read from the standard input
and the output is written to the standard output.

\par Description
WlzCentreOfMass calculates the mass and centre of mass
of the input Woolz 2D or 3D
domain object.
The mass and centre of mass are written to the output
file in the following order:
\par
mass x y z
\par
Where x, y and z are the column, line and plane coordinates of the
centre of mass.

\par Examples
\verbatim
WlzCentreOfMass reads an object from myobj.wlz, calculates
its mass and centre of mass and then prints them to the
standard output.

WlzCentreOfMass myobj.wlz

\endverbatim

\par File
\ref WlzCentreOfMass.c "WlzCentreOfMass.c"
\par See Also
\ref BinWlz "WlzIntro(1)"
\ref wlzprinicipalangle "WlzPrinicipalAngle(1)"
\ref WlzCentreOfMass2D "WlzCentreOfMass2D(3)"
\ref WlzCentreOfMass3D "WlzCentreOfMass3D(3)"
*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/***********************************************************************
* Project:      Woolz
* Title:        WlzCentreOfMass.c
* Date:         March 1999
* Author:       Bill Hill
* Copyright:	1999 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:      Computes the mass and centre of mass of a 2 or 3D woolz
*		domain object and prints the output in the format:
*		  <mass> <c of m x> <c of m y> <c of m z>
* $Revision$
* Maintenance:	Log changes below, with most recent at top of list.
************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <Wlz.h>

extern int      getopt(int argc, char * const *argv, const char *optstring);
 
extern char     *optarg;
extern int      optind,
                opterr,
                optopt;

int             main(int argc, char **argv)
{
  int		option,
		binObjFlag = 0,
		ok = 1,
		usage = 0;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  FILE		*fP = NULL;
  WlzObject	*inObj = NULL;
  double	mass;
  WlzDVertex3	cOfMass;
  char 		*outFileStr,
  		*inObjFileStr;
  static char	optList[] = "o:bh",
		outFileStrDef[] = "-",
  		inObjFileStrDef[] = "-";

  opterr = 0;
  outFileStr = outFileStrDef;
  inObjFileStr = inObjFileStrDef;
  while(ok && ((option = getopt(argc, argv, optList)) != -1))
  {
    switch(option)
    {
      case 'o':
        outFileStr = optarg;
	break;
      case 'b':
        binObjFlag = 1;
	break;
      case 'h':
      default:
        usage = 1;
	ok = 0;
	break;
    }
  }
  if((inObjFileStr == NULL) || (*inObjFileStr == '\0') ||
     (outFileStr == NULL) || (*outFileStr == '\0'))
  {
    ok = 0;
    usage = 1;
  }
  if(ok && (optind < argc))
  {
    if((optind + 1) != argc)
    {
      usage = 1;
      ok = 0;
    }
    else
    {
      inObjFileStr = *(argv + optind);
    }
  }
  if(ok)
  {
    if((inObjFileStr == NULL) ||
       (*inObjFileStr == '\0') ||
       ((fP = (strcmp(inObjFileStr, "-")?
	      fopen(inObjFileStr, "r"): stdin)) == NULL) ||
       ((inObj= WlzReadObj(fP, &errNum)) == NULL) ||
       (errNum != WLZ_ERR_NONE))
    {
      ok = 0;
      (void )fprintf(stderr,
		     "%s: failed to read object from file %s\n",
		     *argv, inObjFileStr);
    }
    if(fP && strcmp(inObjFileStr, "-"))
    {
      fclose(fP);
    }
  }
  if(ok)
  {
    cOfMass = WlzCentreOfMass3D(inObj, binObjFlag, &mass, &errNum);
    if(errNum != WLZ_ERR_NONE)
    {
      ok = 0;
      (void )fprintf(stderr,
      		     "%s: failed to compute centre of mass of object\n",
		     *argv);
    }
  }
  if(ok)
  {
    if(((fP = (strcmp(outFileStr, "-")?
              fopen(outFileStr, "w"):
	      stdout)) == NULL))
    {
      ok = 0;
      (void )fprintf(stderr,
      		     "%s: failed to write output data\n",
		     *argv);
    }
    else
    {
      (void )fprintf(fP, "%g %g %g %g\n",
      		     mass, cOfMass.vtX, cOfMass.vtY, cOfMass.vtZ);
    }
    if(fP && strcmp(outFileStr, "-"))
    {
      fclose(fP);
    }
  }
  if(usage)
  {
    (void )fprintf(stderr,
    "Usage: %s%s%s%sExample: %s%s",
    *argv,
    " [-o<out file>] [-b] [-h] [<in object>]\n"
    "Version: ",
    WlzVersion(),
    "\n"
    "Options:\n"
    "  -o  Output file name.\n"
    "  -b  Always consider object a binary object.\n"
    "  -h  Help, prints this usage message.\n"
    "Calculates the mass and centre of mass of the input Woolz 2 or 3D\n"
    "domain object. The mass and centre of mass are written to the output\n"
    "file in the following order:\n"
    "  <mass> <x> <y> <z>\n"
    "Where x, y and z are the column, line and plane coordinates of the\n"
    "centre of mass.\n"
    "The input object is read from stdin and values are written to stdout\n"
    "unless the filenames are given.\n",
    *argv,
    " -o out.txt myobj.wlz\n"
    "The input Woolz object is read from myobj.wlz. It's mass and centre of\n"
    "mass are written to out.txt.\n");
  }
  return(!ok);
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
