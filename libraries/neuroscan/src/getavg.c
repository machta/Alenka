/***************************************************************************p
 *
 * getavg.c
 *    Reads the data from a Neuroscan average file, works only on a 
 *    little endian computer
 *
 *    A successful call to get_avg returns 1, and a failed call returns 0.
 *
 * (c) 1998 Robert Oostenveld
 *
 ***************************************************************************/

#define N_ELECT 64		// predefined maximum number of EEG channels in file
//#pragma pack(1)			// this is neccesary for the structures in sethead.h
#include <stdio.h>
#include <stdlib.h>
#include <pack.h>
#include "sethead.h"
#include <unpack.h>
#include "matrix2m.h"

short
get_avg (
	  char *fn,		// name of AVG file to open
	   short *nchannels,	// number of channels
	   short *pnts,		// number of points per epoch
	   float *rate,		// digitization rate (Hz)
	   float *xmin,		// pre-event interval (seconds before time zero)
	   float *xmax,		// post-event interval (seconds after time zero)
	   float ***x		// vectored, uV-scaled epoch (pointer)
)
{
  char dummy[5] = "     ";
  int i, j;
  SETUP *erp;
  FILE *fp;

  if ((erp = (SETUP *) malloc (sizeof (SETUP))) == NULL)
    {
      perror ("erp header");
      return (0);
    }

  if ((fp = fopen (fn, "rb")) == NULL)
    {
      perror (fn);
      return (0);
    }

  // read the general part of the header
  if (fread (erp, 900, 1, fp) != 1)
    {
      perror (fn);
      return (0);
    }

  // return these general parameters to the calling program
  *nchannels = erp->nchannels;
  *pnts = erp->pnts;
  *rate = erp->rate;
  *xmin = erp->xmin;
  *xmax = erp->xmax;

  // read the channel specific part of the header
  if (fread (&(erp->elect_tab[0]), 75, erp->nchannels, fp) != erp->nchannels)
    {
      perror (fn);
      return (0);
    }

  matrix_alloc (x, erp->nchannels, erp->pnts);

  for (i = 0; i < erp->nchannels; i++)
    {
      if (fread (dummy, 5, 1, fp) != 1)
	{
	  perror ("dummy");
	  return (0);
	}

      if (fread ((*x)[i], sizeof (float), erp->pnts, fp) != erp->pnts)
	{
	  perror ("data");
	  return (0);
	}

      for (j = 0; j < erp->pnts; j++)
	(*x)[i][j] *= erp->elect_tab[i].calib / erp->elect_tab[i].n;
    }

  fclose (fp);
  return (1);			// indicate everything went ok
}

