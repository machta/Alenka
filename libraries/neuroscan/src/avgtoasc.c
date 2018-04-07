/****************************************************************************
AVGTOASC.C

Example program to demonstrate the get_avg() function.

The syntax from the command line is:

AVGTOASC <AVG file> <ASCII file>

where:

    <AVG file> names the Neuro Scan epoched AVG file (omit .AVG extension);
    <ASCII file> names the output ASCII file;

****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void main(int argc, char *argv[])
{
    // variables required by get_avg() -- see GETEPOCH.C
    char fn[100];
    short nchannels;
    short pnts;
    float rate;
    float xmin;
    float xmax;
    float **x;

    // other variables
    char fn_asc[100];       // name of ASCII file
    FILE *fp_asc;           // file pointer for ASCII file
    short i;                // point index
    short j;                // channel index

   // read the Neuro Scan .EEG file name
    if (argc>1) strcpy(fn,argv[1]);
    else
    {
        printf("Error: Missing 1st parameter (Neuro Scan .EEG file name)\n");
        exit(1);
    }
    strcat(fn,".avg");

    // read the ASCII output file name
    if (argc>2) strcpy(fn_asc,argv[2]);
    else
    {
        printf("Error: Missing 2nd parameter (ASCII file name)\n");
        exit(1);
    }

    // read the data from the Neuroscan file
    if (!get_avg(fn, &nchannels, &pnts, &rate, &xmin, &xmax, &x))
    {
        printf("Error: get_avg() returned an error\n");
        exit(1);
    }

    // open the ASCII output file
    fp_asc=fopen(fn_asc,"w");
    if (fp_asc==NULL)
    {
        printf("Error: Unable to open %s\n",fn_asc);
        exit(1);
    }

    // write the data to the ASCII file
    fprintf(fp_asc, "nchannels = %d\n", nchannels);
    fprintf(fp_asc, "pnts      = %d\n", pnts);
    fprintf(fp_asc, "rate      = %d\n", rate);
    fprintf(fp_asc, "xmin      = %f\n", xmin);
    fprintf(fp_asc, "xmax      = %f\n", xmax);
    fprintf(fp_asc, "\n");
    for (j=0;j<nchannels;j++)
    {
        // fprintf(fp_asc,"Channel %4d: ",j+1);
        for (i=0;i<pnts;i++)
        {
            fprintf(fp_asc,"%10.3f",x[j][i]);
        }
        fprintf(fp_asc,"\n");
    }

    // clean up and exit normally
    fclose(fp_asc);
    matrix_free(&x);
    exit(0);
}

