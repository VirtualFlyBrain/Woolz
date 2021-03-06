#if defined(__GNUC__)
#ident "University of Edinburgh $Id$"
#else
static char _WlzDomainFill_c[] = "University of Edinburgh $Id$";
#endif
/*!
* \file         binWlz/WlzDomainFill.c
* \author       Richard Baldock
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
* \brief	Fills the domain of a  woolz object.
* \ingroup	BinWlz
*
* \par Binary
* \ref wlzdomainfill "WlzDomainFill"
*/

/*!
\ingroup BinWlz
\defgroup wlzdomainfill WlzDomainFill
\par Name
WlzDomainFill - Fills the domain of a  woolz object.
\par Synopsis
\verbatim
WlzDomainFill [-h] [<input file>]
\endverbatim
\par Options
<table width="500" border="0">
  <tr> 
    <td><b>-h</b></td>
    <td>Help, prints usage message.</td>
  </tr>
  <tr> 
    <td><b>-T</b></td>
    <td>Report elapsed time.</td>
  </tr>
</table>
\par Description
Fills any holes in the domain of the object.
\par Examples
\verbatim
WlzDomainFill in.wlz >out.wlz
\endverbatim
Creates a new object out.wlz from in.wlz,
in which the domain has no holes.
\par File
\ref WlzDomainFill.c "WlzDomainFill.c"
\par See Also
\ref BinWlz "WlzIntro(1)"
\ref wlzdilation "WlzDilation(1)"
\ref wlzerosion "WlzErosion(1)"
\ref WlzDomainFill "WlzDomainFill(3)"
*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <Wlz.h>

/* externals required by getopt  - not in ANSI C standard */
#ifdef __STDC__ /* [ */
extern int      getopt(int argc, char * const *argv, const char *optstring);
 
extern int 	optind, opterr, optopt;
extern char     *optarg;
#endif /* __STDC__ ] */

static void usage(char *proc_str)
{
  fprintf(stderr,
	  "Usage:\t%s [-h] [-v] [<input file>]\n"
	  "\tFill the domain of a  woolz object, this only\n"
	  "\taffects 2D and 3D domain objects and will automatically\n"
	  "\tremove the valuetable if present. The new object is\n"
	  "\twritten to standard output."
	  "Version: %s\n"
	  "Options:\n"
	  "\t  -h        help - prints this usage message\n"
	  "\t  -T        Report elapsed time.\n",
	  proc_str,
	  WlzVersion());
  return;
}
 
int main(int	argc,
	 char	**argv)
{

  WlzObject	*obj = NULL,
  		*tmpObj = NULL;
  FILE		*inFile;
  char 		optList[] = "hT";
  int		option,
  		timeFlg = 0;
  struct timeval times[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  const char	*errMsg;
    
  /* read the argument list and check for an input file */
  opterr = 0;
  
  while( (option = getopt(argc, argv, optList)) != EOF ){
    switch( option ){
    case 'T':
      timeFlg = 1;
      break;
    case 'h':
    default:
      usage(argv[0]);
      return 1;
    }
  }

  inFile = stdin;
  if( optind < argc ){
    if( (inFile = fopen(*(argv+optind), "r")) == NULL ){
      fprintf(stderr, "%s: can't open file %s\n", argv[0], *(argv+optind));
      usage(argv[0]);
      return 1;
    }
  }

  /* read objects and threshold if possible */
  while(((obj = WlzAssignObject(WlzReadObj(inFile, NULL), NULL)) != NULL) &&
        (errNum == WLZ_ERR_NONE))
  {
    switch( obj->type )
    {
    case WLZ_2D_DOMAINOBJ:
    case WLZ_3D_DOMAINOBJ:
      if(timeFlg) {
        gettimeofday(times + 0, NULL);
      }
      tmpObj = WlzAssignObject(WlzDomainFill(obj, &errNum), NULL);
      if(timeFlg) {
        gettimeofday(times + 1, NULL);
	ALC_TIMERSUB(times + 1, times + 0, times + 2);
	(void )fprintf(stderr,
		       "%s: Elapsed time for WlzDomainFill()  %gus\n",
		       *argv,
		       (1000000.0 * times[2].tv_sec) + times[2].tv_usec);
      }
      if(tmpObj) {
	if((errNum = WlzWriteObj(stdout, tmpObj)) != WLZ_ERR_NONE) {
	  (void )WlzStringFromErrorNum(errNum, &errMsg);
	  (void )fprintf(stderr,
			 "%s: failed to write object (%s).\n",
			 argv[0], errMsg);
	  return(1);
	}
	WlzFreeObj(tmpObj);
      }
      break;

    default:
      if((errNum = WlzWriteObj(stdout, obj)) != WLZ_ERR_NONE) {
	(void )WlzStringFromErrorNum(errNum, &errMsg);
	(void )fprintf(stderr,
		       "%s: failed to write object (%s).\n",
		       argv[0], errMsg);
	return(1);
      }
      break;
    }
    WlzFreeObj(obj);
  }

  return 0;
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
