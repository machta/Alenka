#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include "matrix2m.h"           // matrix memory allocation

#pragma pack(1)
#define  N_ELECT  64            // maximum number of electrodes
#include "sethead.h"            // Neuro Scan header structure

// structure for epoch-specific header
typedef struct
{
    char accept;
    short ttype;
    short correct;
    float rt;
    short response;
    short reserved;
} SWEEP_HEAD;

/****************************************************************************
get_epoch()

Function to read an epoch from a Neuro Scan epoched EEG file.  On the first
call to the function, the user sets *flag to 0; the function then opens the 
file, reads the data header, and allocates memory for the epoch data buffer 
(x).  The function returns the number of channels, number of points per 
epoch, the digitization rate, & the number of seconds before and after time 
zero.  After successful initialization, *flag is automatically set to 1.  
Subsequent calls to get_epoch() will return the type code for the next epoch 
and the microvolt scaled epoch buffer (x). When the last epoch has been 
reached, the function sets the *flag to 2.  The last call of get_epoch() 
will close the EEG file and free allocated memory.

A successful call to get_epoch returns 1, and a failed call returns 0.
****************************************************************************/

short get_epoch
(
    char *flag,         // 0=initial, 1=middle, 2=final
    char *fn,           // name of EEG file to open
    short *nchannels,   // number of channels
    short *pnts,        // number of points per epoch
    float *rate,        // digitization rate (Hz)
    float *xmin,        // pre-event interval (seconds before time zero)
    float *xmax,        // post-event interval (seconds after time zero)
    short *type,        // type code associated with epoch
    float ***x          // vectored, uV-scaled epoch (pointer)
)
{
    // static local variables
    static FILE *fp;            // pointer for EEG file
    static SETUP *erp;          // header of file
    static short epoch_count;   // epoch counter
    static short **buff;        // buffer for raw multiplexed data
    static long buff_size;      // size of a raw epoch of data
    static long x_size;         // size of scaled output data

    // nonstatic local variables
    short ret;                  // generic function return value
    SWEEP_HEAD sh;              // sweep header
    float factor;               // channel-specific microvolt scaling factor
    short dc;                   // channel-specific dc calibration value
    short i,j;                  // point and channel indices

    // INITIALIZATION (first call) <=========================================
    if (*flag==0)
    {
        // open file
        fp=fopen(fn,"rb");
        if (fp==NULL) return(0);

        // allocate erp
        erp=(SETUP *)malloc(sizeof(SETUP));
        if (erp==NULL)
        {
            fclose(fp);
            return(0);
        }
        
        // read general part of the erp header
        ret=fread(erp,900,1,fp);
        if (ret!=1)
        {
            free(erp);
            fclose(fp);
            return(0);
        }

        // return these general parameters to the calling program
        *nchannels=(*erp).nchannels;
        *pnts=(*erp).pnts;
        *rate=(*erp).rate;
        *xmin=(*erp).xmin;
        *xmax=(*erp).xmax;

        // read the channel-specific part of the erp header
        ret=fread((*erp).elect_tab,75*(*nchannels),1,fp);
        if (ret!=1)
        {
            free(erp);
            fclose(fp);
            return(0);
        }
        
        // allocate memory for the vectored output epoch, x
        if (!matrix_alloc(x,*nchannels,*pnts))
        {
            free(erp);
            fclose(fp);
            return(0);
        }
        x_size=(*nchannels)*(*pnts)*sizeof(float); 
        
        // allocate memory for multiplexed input epoch, buff
        if (!matrix_alloc_s(&buff,*pnts,*nchannels))
        {
            matrix_free(x);
            free(erp);
            fclose(fp);
            return(0);
        }
        buff_size=(*nchannels)*(*pnts)*sizeof(short); 
        
        epoch_count=0;
        *flag=1;
        return(1);  
    }

    // CLEANUP (last call) <=================================================
    if (*flag==2)
    {
        matrix_free_s(&buff);
        matrix_free(x);
        free(erp);
        fclose(fp);
        return(1);      
    }

    // GET AN EPOCH (typical call) <=========================================
    while(1)
    {
        if (epoch_count>=(*erp).compsweeps)
        {
            // if the last epoch has already been read, set flag to 2
            *flag=2;
            return(1);
        }

        // read the sweep header
        ret=fread(&sh,sizeof(SWEEP_HEAD),1,fp);
        if (ret!=1) return(0);
        
        // read the data buffer
        ret=fread(buff[0],buff_size,1,fp);
        if (ret!=1) return(0);

        // increment the epoch counter
        epoch_count++;

        // return only sweeps that have been accepted
        if (sh.accept)
        {
            // return the epoch type code to the calling program
            *type=sh.ttype;

            // demultiplex the data buffer and convert to microvolts
            for (j=0;j<(*nchannels);j++)
            {
                factor=(*erp).elect_tab[j].calib *
                       (*erp).elect_tab[j].sensitivity/204.8;
                dc=(*erp).elect_tab[j].baseline;
                for (i=0;i<(*pnts);i++)
                {
                    (*x)[j][i]=factor*(buff[i][j]-dc);
                }
            }

            return(1);
        }
    }
    
}
