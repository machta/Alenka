/****************************************************************************
CNTTOASC.C

The primary purpose of this SCAN utility program is to demonstrate how to 
use the functions defined in NS_CNT.C to read NeuroScan continuous (.CNT)
formatted data.

Secondarily, the CNTTOASC program may be useful to some who wish to convert 
a .CNT file to an ASCII file.  However, the resulting ASCII file will usually 
be much larger than the original binary file; so, as a general rule, it is
best to work with the original binary .CNT file whenever possible.

*****************************************************************************
The following NS_CNT.C functions are demonstrated in the CNTTOASC program:

ns_cnt_open(.):
    Opens the .CNT file, reads the header, the event footer, and returns a
    pointer to the header (SETUP structure defined in sethead.h).

ns_cnt_event_table():
    Returns a pointer to the event table (EVENT2 structure defined in
    sethead.h).  ns_cnt_open() must be called first.

ns_cnt_initialize_epoch(....):
    Initializes for reading epochs (or continuous points).  Returns a pointer
    to a buffer that will contain multiplexed, uV-scaled data after calling
    the ns_cnt_read_epoch() function.  If there is no event table, or if
    there is just one point in the epoch, then continuous points are read;
    otherwise, discrete event-related epochs are read.

ns_cnt_get_epoch_flag():
    Returns 1 if discrete event-related epoched mode, 0 if continuous points
    mode.

ns_cnt_read_epoch():
    The main function for reading an epoch (or a point).  Returns the epoch
    number (or the point number) if successful; returns 0 when the file has
    been exhausted.

ns_cnt_closeall():
    Closes the .CNT file and releases all resources allocated by NS_CNT.C
    functions.

*****************************************************************************
The DOS level syntax for the CNTTOASC program is:

CNTTOASC <file_name> {<start_point> <stop_point> <epoch_min> <epoch_max>}

where:

<file_name> is the name of the continuous file to be converted, but with    
the .CNT extension omitted.  The output ASCII file will have the name   
<file_name>.ASC.

<start_point> is an optional parameter that specifies the number of the first 
data point in the continuous file to write to the ASCII file.  The default is 
0, the start of the file.

<stop_point> is an optional parameter that specifies the number of the last
data point in the continuous file to write to the ASCII file.  The default is 
-1, which codes for the end of the file.

<epoch_min> and <epoch_max> are optional parameters.  By default, all data 
points between the start and stop points will be written to the ASCII file 
continuously.  However, if <epoch_min> and <epoch_max> are specified (with 
the latter parameter greater than the former parameter) then only epochs 
surrounding stimulus events will be written to the ASCII file.  <epoch_min> 
specifies the offset from the event, in seconds, for the beginning of the 
epoch (with negative values corresponding to time prior to an event).  
Likewise, <epoch_max> specifies the offset from the event, in seconds, for 
the end of the epoch.
****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#pragma pack(1)
#include "ns_cnt.h"

void main(int argc, char *argv[])
{
    SETUP *erp;                 // pointer to Neuro Scan header structure
    EVENT2 *evt;                // pointer to Neuro Scan event table
    float **epoch;              // pointer to multiplexed, uV-scaled data
    unsigned short stim_type;   // stimulus type
    char fn_asc[200];           // name of output ASCII file
    FILE *fp_asc;               // pointer for ASCII file
    long start_point=0;         // first point to write to ASCII file
    long stop_point=-1;         // last point to write to ASCII file
    float epoch_min=0.f;        // (optional) offset in sec. for epoch start
    float epoch_max=0.f;        // (optional) offset in sec. for epoch stop
    long i,j;                   // point and channel indices
    long count=0;               // epoch or point counter
    char epoched_mode=0;        // flag for discrete epochs

    printf("CNTTOASC beta 1.0: ASCII conversion of Neuro Scan .CNT files\n");

    if (argc<=1)
    {
        printf("Please include command line parameter for file name.\n");
        goto error0;
    }

    // open file, read header and event table
    if ((erp=ns_cnt_open(argv[1]))==NULL) goto error0;

    // get pointer to event table
    evt=ns_cnt_event_table();

    // open the ASCII output file
    strcpy(fn_asc,argv[1]);
    strcat(fn_asc,".asc");
    fp_asc=fopen(fn_asc,"w");
    if (fp_asc==NULL)
    {
        printf("Unable to open ASCII output file %s\n",fn_asc);
        goto error1;
    }

    // read optional command line parameters
    if (argc>2) start_point=atoi(argv[2]);
    if (start_point<0) start_point=0;
    if (argc>3) stop_point=atoi(argv[3]);
    if (stop_point<start_point) stop_point=-1;
    if (argc>4) epoch_min=atof(argv[4]);
    if (argc>5) epoch_max=atof(argv[5]);

    // initialize epoch
    if ((epoch=ns_cnt_initialize_epoch(epoch_min,epoch_max,start_point,stop_point))==NULL)
        goto error2;

    // discrete or continuous mode
    epoched_mode=ns_cnt_get_epoch_flag();

    // write header to ASCII file
    fprintf(fp_asc,"ASCII output of file %s.CNT\n",argv[1]);
    fprintf(fp_asc,"number of channels=%d\n",erp->nchannels);
    if (epoched_mode)
        fprintf(fp_asc,"Epoched output mode: %ld points from %f sec to %f sec\n",erp->pnts,epoch_min,epoch_max);
    else
        fprintf(fp_asc,"Continuous output mode\n");
    if (start_point>0)
        fprintf(fp_asc,"Start point = %ld\n",start_point);
    if (stop_point>start_point)
        fprintf(fp_asc,"Stop  point = %ld\n",stop_point);
    for (j=0;j<erp->nchannels;j++)
        fprintf(fp_asc,"%9s",erp->elect_tab[j].lab);
    fprintf(fp_asc,"\n");

    // Main Loop: Read an epoch (or a point) at a time
    while (count=ns_cnt_read_epoch())
    {
        if (epoched_mode)
        {
            stim_type=evt[count-1].Event1.StimType;
            fprintf(fp_asc,"Epoch %ld, stimulus type = %u\n",count,stim_type);
        }
        for (i=0;i<erp->pnts;i++)
        {
            for (j=0;j<erp->nchannels;j++)
            {
                fprintf(fp_asc,"%9.2f",epoch[i][j]);
            }
            fprintf(fp_asc,"\n");
        }
    }

    fclose(fp_asc);         // close ASCII file
    ns_cnt_closeall();      // cleanup NS_CNT resources
    printf("OK.\n");
    exit(0);

    error2: fclose(fp_asc);
    error1: ns_cnt_closeall();
    error0: printf("Error.\n");
            exit(1);
}

