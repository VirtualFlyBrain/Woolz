#pragma ident "MRC HGU $Id$"
/************************************************************************
*   Copyright  :   1994 Medical Research Council, UK.                   *
*                  All rights reserved.                                 *
*************************************************************************
*   Address    :   MRC Human Genetics Unit,                             *
*                  Western General Hospital,                            *
*                  Edinburgh, EH4 2XU, UK.                              *
*************************************************************************
*   Project    :   Woolz Library					*
*   File       :   WlzExtFFWlzToXYZ.c					*
*************************************************************************
* This module has been copied from the original woolz library and       *
* modified for the public domain distribution. The original authors of  *
* the code and the original file headers and comments are in the        *
* HISTORY file.                                                         *
*************************************************************************
*   Author Name :  Richard Baldock					*
*   Author Login:  richard@hgu.mrc.ac.uk				*
*   Date        :  Mon Oct 23 14:43:18 2000				*
*   $Revision$								*
*   $Name$								*
*   Synopsis    : Converts a set of woolz domain files (3D binary) with	*
*		associated ID file to GW XYZ format. The ID file	*
*		holds a set of ids unique for each domain.		*
*************************************************************************
*   Maintenance :  date - name - comments (Last changes at the top)	*
************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Wlz.h>
#include <HGUDlpList.h>

/* externals required by getopt  - not in ANSI C standard */
#ifdef __STDC__ /* [ */
extern int      getopt(int argc, char * const *argv, const char *optstring);
 
extern int 	optind, opterr, optopt;
extern char     *optarg;
#endif /* __STDC__ ] */

typedef struct _domainItem {
  char		*filename;
  WlzObject	*obj;
  char		*uid;
} DomainItem;

typedef struct _contourItem {
  WlzPolygonDomain	*poly;
  char			*uid;
  struct _contourItem	*up;
  int			index;
  int			intern;
} ContourItem;

static void usage(char *proc_str)
{
  fprintf(stderr,
	  "Usage:\t%s "
	  "<input file-list>\n"
	  "\tRead the file list and convert the woolz domains to\n"
	  "\tGlaxo-Wellcome XYZ format contour files.\n"
	  "\tOptions are:\n"
	  "\t  -h        Help - prints this usage message\n"
	  "\t  -v        Verbose operation\n"
	  "",
	  proc_str);
  return;
}

void XYZWriteComment(
  FILE	*fp,
  char	*str)
{
  fprintf(fp, ";%s\n", str);
  return;
}

void XYZWriteHeader(
  FILE *fp)
{
  XYZWriteComment(fp, "Description of file format to read XYZ files");
  XYZWriteComment(fp, "");
  XYZWriteComment(fp, "");
  XYZWriteComment(fp, "the first field indicates if the ColorMap is GrayScale of Color");
  XYZWriteComment(fp, "the second field indicates the image dimension (Width Height)");
  XYZWriteComment(fp, "the third field indicates the number of contours");
  XYZWriteComment(fp, "For each CONTOUR: Auto Id, Marker Id, index, nb points XY, intern or extern contours");
  XYZWriteComment(fp, "");
  XYZWriteComment(fp, "RAB note: index seemed to be missing out of the description, now inserted, and");
  XYZWriteComment(fp, "\tafter discussion with Frank Dorkeld the last number is interpreted as the");
  XYZWriteComment(fp, "\tindex of the nearest enclosing contour");
  XYZWriteComment(fp, "");
  XYZWriteComment(fp, "X: x coordinate");
  XYZWriteComment(fp, "Y: y coordinate");
  XYZWriteComment(fp, "Remark: Identifier (automatically determined by image analysis) of the contour type may be:");
  XYZWriteComment(fp, "UNKNOWN         -> -1");
  XYZWriteComment(fp, "EXTERN          ->  0");
  XYZWriteComment(fp, "NUCLEAR MARKER  ->  1");
  XYZWriteComment(fp, "VENTRICLE       ->  2");
  XYZWriteComment(fp, "GREY MATTER     ->  3");
  XYZWriteComment(fp, "WHITE MATTER    ->  4");
  XYZWriteComment(fp, "WHITE OTHER     ->  5");
  XYZWriteComment(fp, "");
  XYZWriteComment(fp, "");
  XYZWriteComment(fp, "RAB note: This is a contour file generated by WlzExtFFWlzToXYZ which");
  XYZWriteComment(fp, "\tsets all autoIds to -1");
  XYZWriteComment(fp, "\tThe marker id is set to the uid of the respective anatomical component");
  XYZWriteComment(fp, "\tin the Mouse Atlas anatomy database. The link between uid and anatomy");
  XYZWriteComment(fp, "\tis input as a simple list which is given below");
  XYZWriteComment(fp, "\tIf the width and height are (0 0) then none were set when the domains converted.");
  XYZWriteComment(fp, "\tThe input offsets determine the shift required to bring the lower bound");
  XYZWriteComment(fp, "\tof the woolz object bounding box to (0, 0) and is needed if the contours");
  XYZWriteComment(fp, "\tare to be converted back to woolz objects");
  XYZWriteComment(fp, "");
  XYZWriteComment(fp, "UID\t\tAnatomy Domain");
  XYZWriteComment(fp, "");
  return;
}

void freeDomainItem(
  void		*item)
{
  DomainItem	*dmnItem = (DomainItem *) item;

  if( dmnItem->obj ){
    WlzFreeObj(dmnItem->obj);
  }
  if( dmnItem->uid ){
    free(dmnItem->uid);
  }
  if( dmnItem->filename ){
    free(dmnItem->filename);
  }
  AlcFree(item);

  return;
}

void freeContourItem(
  void		*item)
{
  ContourItem	*cntrItem = (ContourItem *) item;

  if( cntrItem->poly ){
    WlzFreePolyDmn(cntrItem->poly);
  }
  if( cntrItem->uid ){
    free(cntrItem->uid);
  }
  AlcFree(item);

  return;
}

void insertPolygonDomain(
  HGUDlpList		*cntrList,
  WlzPolygonDomain	*poly,
  char			*uid)
{
  ContourItem	*cntrItem;

  cntrItem = (ContourItem *) AlcCalloc(1, sizeof(ContourItem));
  cntrItem->poly = WlzAssignPolygonDomain(poly, NULL);
  cntrItem->uid = strdup(uid);
  (void) HGUDlpListInsert(cntrList, NULL, cntrItem, freeContourItem);

  return;
}

void insertBoundaryList(
  HGUDlpList	*cntrList,
  WlzBoundList	*bound,
  char		*uid)
{
  if( bound ){
    insertBoundaryList(cntrList, bound->next, uid);
    insertBoundaryList(cntrList, bound->down, uid);
    if(bound->type == WLZ_BOUNDLIST_PIECE){
      insertPolygonDomain(cntrList, bound->poly, uid);
    }
  }

  return;
}

void insertObject(
HGUDlpList	*cntrList,
  WlzObject	*obj,
  char		*uid)
{
  WlzObject	*boundObj;
  WlzErrorNum	errNum=WLZ_ERR_NONE;
  
  /* should check object, type etc., assume 2D domain */
  boundObj = WlzObjToBoundary(obj, 1, &errNum);
  insertBoundaryList(cntrList, boundObj->domain.b, uid);
  WlzFreeObj(boundObj);

  return;
}

int compContourItem(
  void	*item1,
  void	*item2)
{
  ContourItem	*cItem1 = (ContourItem *) item1;
  ContourItem	*cItem2 = (ContourItem *) item2;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  if( WlzPolyCrossings(cItem2->poly->vtx[0], cItem1->poly, &errNum) ){
    return 1;
  }
  if( WlzPolyCrossings(cItem1->poly->vtx[0], cItem2->poly, &errNum) ){
    return -1;
  }
  return 0;
}

int main(int	argc,
	 char	**argv)
{
  char 		optList[] = "hvm:p:o:s:";
  int		option;
  int		verboseFlg=0;
  WlzErrorNum	errNum=WLZ_ERR_NONE;
  HGUDlpList	*dmnList;
  DomainItem	*dmnItem;
  HGUDlpList	*cntrList;
  ContourItem	*cntrItem, *upCntrItem;
  HGUDlpListItem	*item, *upItem;
  FILE		*fp;
  WlzObject	*obj;
  WlzValues	values;
  char		strBuf[1024];
  int		plane=0, xOff=0, yOff=0;
  int		width=0, height=0;
  int		colorFlg = 0;
  int		minL, maxL, minK, maxK;
  WlzIVertex2	vtxs[5];
  int		index, i;
  int		minSize=4;

  /* read the argument list and check for an input file */
  opterr = 0;
  while( (option = getopt(argc, argv, optList)) != EOF ){
    switch( option ){

    case 'c':
      colorFlg = 1;
      break;

    case 'm':
      if( sscanf(optarg, "%d", &minSize) < 1 ){
	usage(argv[0]);
	return 1;
      }
      break;

    case 'o':
      if( sscanf(optarg, "%d,%d", &xOff, &yOff) < 2 ){
	usage(argv[0]);
	return 1;
      }
      break;

    case 'p':
      if( sscanf(optarg, "%d", &plane) < 1 ){
	usage(argv[0]);
	return 1;
      }
      break;

    case 's':
      if( sscanf(optarg, "%d,%d", &width, &height) < 2 ){
	usage(argv[0]);
	return 1;
      }
      break;

    case 'v':
      verboseFlg = 1;
      break;

    case 'h':
      usage(argv[0]);
      return WLZ_ERR_NONE;

    default:
      usage(argv[0]);
      return WLZ_ERR_UNSPECIFIED;
    }
  }

  /* read the domain file list plus ids */
  /* The file list has the id and file name space separated,
     one pair per line */
  /* start writing the XYZ file here */
  XYZWriteHeader(stdout);
  fp = stdin;
  if( optind < argc ){
    if( (fp = fopen(*(argv+optind), "r")) == NULL ){
      fprintf(stderr, "%s: can't open file %s\n", argv[0], *(argv+optind));
      usage(argv[0]);
      return 1;
    }
  }
  dmnList = HGUDlpListCreate(NULL);
  while ( fgets(strBuf, 1024, fp) ){
    char uidBuf[64], filenameBuf[1024];
    FILE *objFp;

    if(sscanf(strBuf, "%s %s", uidBuf, filenameBuf) < 2){
      fprintf(stderr, "%s: something wrong with the domain list\n", argv[0]);
      usage(argv[0]);
      return WLZ_ERR_UNSPECIFIED;
    }

    /* check we can read the file and it includes a woolz 3D object */
    if( objFp = fopen(filenameBuf, "r") ){
      if( obj = WlzReadObj(objFp, &errNum) ){
	if( obj->type != WLZ_3D_DOMAINOBJ ){
	  fprintf(stderr, "%s: not a 3D object in %s\n", argv[0], filenameBuf);
	  WlzFreeObj(obj);
	  continue;
	}
      }
      else {
	fprintf(stderr, "%s: could not read object in %s\n", argv[0],
		filenameBuf);
	continue;
      }
      fclose(objFp);
    }
    else {
      fprintf(stderr, "%s: failed to open %s\n", argv[0], filenameBuf);
      continue;
    }

    /* can write this line to XYZ file */
    sprintf(strBuf, "%s\t\t%s", uidBuf, filenameBuf);
    XYZWriteComment(stdout, strBuf);

    /* make a new domain item */
    dmnItem = (DomainItem *) AlcCalloc(1, sizeof(DomainItem));
    dmnItem->filename = strdup(filenameBuf);
    dmnItem->uid = strdup(uidBuf);
    dmnItem->obj = WlzAssignObject(obj, &errNum);
    (void) HGUDlpListInsert(dmnList, NULL, dmnItem, freeDomainItem);
  }
  if( fp != stdin ){
    fclose(fp);
  }
  XYZWriteComment(stdout, "");

  /* add a dummy enclosing contour */
  if( item = HGUDlpListHead(dmnList) ){
    dmnItem = (DomainItem *) HGUDlpListEntryGet(dmnList, item);
    minL = dmnItem->obj->domain.p->line1;
    maxL = dmnItem->obj->domain.p->lastln;
    minK = dmnItem->obj->domain.p->kol1;
    maxK = dmnItem->obj->domain.p->lastkl;
    item = HGUDlpListNext(dmnList, item);
  }
  while( item ){
    dmnItem = (DomainItem *) HGUDlpListEntryGet(dmnList, item);
    if(minL > dmnItem->obj->domain.p->line1){
      minL = dmnItem->obj->domain.p->line1;
    }
    if(maxL < dmnItem->obj->domain.p->lastln){
      maxL = dmnItem->obj->domain.p->lastln;
    }
    if( minK > dmnItem->obj->domain.p->kol1){
      minK = dmnItem->obj->domain.p->kol1;
    }
    if(maxK < dmnItem->obj->domain.p->lastkl){
      maxK = dmnItem->obj->domain.p->lastkl;
    }

    item = HGUDlpListNext(dmnList, item);
  }
  minL -= 1;
  maxL += 1;
  minK -= 1;
  maxK += 1;
  vtxs[0].vtX = minK; vtxs[0].vtY = minL;
  vtxs[1].vtX = minK; vtxs[1].vtY = maxL;
  vtxs[2].vtX = maxK; vtxs[2].vtY = maxL;
  vtxs[3].vtX = maxK; vtxs[3].vtY = minL;
  vtxs[4] = vtxs[0];
  cntrItem = (ContourItem *) AlcCalloc(1, sizeof(ContourItem));
  cntrItem->poly = WlzAssignPolygonDomain(
    WlzMakePolyDmn(WLZ_POLYGON_INT, vtxs, 5, 5, 1, &errNum), NULL);
  cntrItem->index = -1;
  cntrList = HGUDlpListCreate(NULL);
  (void) HGUDlpListInsert(cntrList, NULL, cntrItem, freeContourItem);

  /* generate the contour list for the required plane */
  item = HGUDlpListHead(dmnList);
  while( item ){
    dmnItem = (DomainItem *) HGUDlpListEntryGet(dmnList, item);
    if((dmnItem->obj->domain.p->plane1 <= plane) &&
       (dmnItem->obj->domain.p->lastpl >= plane)){
      switch( dmnItem->obj->domain.p->type ){
      case WLZ_PLANEDOMAIN_DOMAIN:
	values.core = NULL;
	obj = 
	  WlzMakeMain(WLZ_2D_DOMAINOBJ,
		      dmnItem->obj->domain.p->domains
		      [plane - dmnItem->obj->domain.p->plane1],
		      values, NULL, NULL, NULL);
	insertObject(cntrList, obj, dmnItem->uid);
	WlzFreeObj(obj);
	break;

      case WLZ_PLANEDOMAIN_POLYGON:
	insertPolygonDomain(
	  cntrList, dmnItem->obj->domain.p->domains
	  [plane - dmnItem->obj->domain.p->plane1].poly,
	  dmnItem->uid);
	break;

      case WLZ_PLANEDOMAIN_BOUNDLIST:
	insertBoundaryList(
	  cntrList, dmnItem->obj->domain.p->domains
	  [plane - dmnItem->obj->domain.p->plane1].b,
	  dmnItem->uid);
	break;

      default:
	break;
      }
    }

    item = HGUDlpListNext(dmnList, item);
  }

  /* sort the contours and find nearest enclosing contour */
  HGUDlpListSort(cntrList, compContourItem);

  /* write the XYZ file, put in offsets for back conversion
     assume all autoIds = -1 (unknown) */
  sprintf(strBuf, "Offset = (%d, %d)", xOff, yOff);
  XYZWriteComment(stdout, strBuf);
  XYZWriteComment(stdout, "");

  /* now write the contours, remember the top contour
     is a dummy to be discarded */
  if( colorFlg ){
    fprintf(stdout, "COLOR\n");
  }
  else {
    fprintf(stdout, "GRAYSCALE\n");
  }
  fprintf(stdout, "%d %d\n", width, height);
  fprintf(stdout, "%d\n", HGUDlpListCount(cntrList) - 1);
  index = 0;
  item = HGUDlpListHead(cntrList);
  cntrItem = (ContourItem *) HGUDlpListEntryGet(cntrList, item);
  cntrItem->index = -1;
  item = HGUDlpListNext(cntrList, item);
  while( item ){
    cntrItem = (ContourItem *) HGUDlpListEntryGet(cntrList, item);
    cntrItem->index = index;

    /* find nearest enclosing */
    upItem = HGUDlpListPrev(cntrList, item);
    while( upItem ){
      upCntrItem = (ContourItem *) HGUDlpListEntryGet(cntrList, upItem);
      if( compContourItem(upCntrItem, cntrItem) == 1 ){
	cntrItem->up = upCntrItem;
	break;
      }

      upItem = HGUDlpListPrev(cntrList, upItem);
    }

    obj = WlzPolyTo8Polygon(cntrItem->poly, 1, &errNum);
    if( (obj->domain.poly->nvertices - 1) > minSize ){
      fprintf(stdout, "CONTOUR\n");
      fprintf(stdout, "-1 %s %d %d %d\n", cntrItem->uid,
	      cntrItem->index, obj->domain.poly->nvertices - 1,
	      cntrItem->up->index);

      for(i=0; i < (obj->domain.poly->nvertices - 1); i++){
	fprintf(stdout, "%6d %6d\n",
		obj->domain.poly->vtx[i].vtX - xOff,
		obj->domain.poly->vtx[i].vtY - yOff);
      }
      index++;
    }
    item = HGUDlpListNext(cntrList, item);
  }
    

  return WLZ_ERR_NONE;
}
