/****************************************************************************
NS_CNT.C

Stand-alone functions to read a Neuro Scan continuous (.CNT) format file.
****************************************************************************/

#define FIX_TEEG_TYPE		// fixes bug in Neuroscan sethead.h header

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <assert.h>

#include "matrix2m.h"           // matrix memory allocation
#include "ns_cnt.h"

extern void errmes(char *);

// INTERNAL STATIC VARIABLES ************************************************
static FILE  *ns_cnt_fp;            // pointer to continuous file
static SETUP *ns_cnt_erp;           // header structure
static long ns_cnt_nc;              // number of channels
static long ns_cnt_header_offset;   // size of header in bytes
static short ns_cnt_type;           // type of continuous file
static long ns_cnt_event_type;      // type of event table
static long ns_cnt_nevents;         // number of events
static EVENT2 *ns_cnt_evt;          // type 2 event table
static long ns_cnt_offset;          // current offset in file
static long ns_cnt_point_number;    // number of current data point
static long ns_cnt_current_event;   // current event number
static long ns_cnt_point_size;      // size in bytes of an acquired point
static short ns_cnt_opened=0;       // has ns_cnt_open been called?
static short **ns_cnt_adbuff;       // multiplexed, raw A/D   epoch buffer
static long ns_cnt_adbuff_size;     // size in bytes for adbuff
static float **ns_cnt_epoch;        // multiplexed, uV-scaled epoch buffer
static short ns_cnt_epoch_flag;     // 1=discrete epochs, 0=continuous
static long ns_cnt_epoch_pnts;      // number of points in the epoch
static long ns_cnt_epoch_offset;    // point offset from the event
static long ns_cnt_read_count;      // number of successful reads
static long ns_cnt_start_point;     // starting point for read
static long ns_cnt_stop_point;      // stopping point for read
static long ns_cnt_SynAmp_pnts;     // # points in a SymAmp block
static long ns_cnt_SynAmp_size;     // size of a SymAmp block (in bytes)
static short **ns_cnt_SynAmp_blk;   // vectored block of A/D SynAmp data
static short ns_cnt_epoch_initialized=0;    // has epoch been initialized?

/****************************************************************************
ns_cnt_open()

Opens a .CNT file and returns a pointer to the header structure.
****************************************************************************/

SETUP* ns_cnt_open
(
    const char *fn            // name of continuous file (assumes .CNT extension)
)
{
    char fn_cnt[1000];   // complete file name
    char mes[400];      // error message
    TEEG teeg;          // event table identifier
    long i;

    if (ns_cnt_opened)   ns_cnt_closeall();
    ns_cnt_epoch_initialized=0;

    // open the file <=======================================================
    strcpy(fn_cnt,fn);
//    strcat(fn_cnt,".cnt");
    ns_cnt_fp=fopen(fn_cnt,"rb");
    if (ns_cnt_fp==NULL)
    {
        strcpy(mes,"Unable to open file ");
        strcat(mes,fn_cnt);
        errmes(mes);
        goto error0;
    }

    // determine number of channels <========================================
    /// allocate main part of erp
    ns_cnt_erp=(SETUP *)malloc(900);
    if (ns_cnt_erp==NULL)
    {
        errmes("Out of memory");
        goto error1;
    }

    /// read general part of the erp header
    if (fread(ns_cnt_erp,900,1,ns_cnt_fp)!=1)
    {
        strcpy(mes,"Unable to read header of file ");
        strcat(mes,fn_cnt);
        errmes(mes);
        goto error2;
    }
    ns_cnt_nc=ns_cnt_erp->nchannels;
    ns_cnt_header_offset=900+75*ns_cnt_nc;
    free(ns_cnt_erp);

    // allocate memory for entire erp header <===============================
    ns_cnt_erp=(SETUP *)malloc(ns_cnt_header_offset);
//    assert(sizeof(SETUP) == ns_cnt_header_offset);
    if (ns_cnt_erp==NULL)
    {
        errmes("Out of memory");
        goto error1;
    }

    // read the entire header <==============================================
    fseek(ns_cnt_fp,0L,SEEK_SET);
    if (fread(ns_cnt_erp,ns_cnt_header_offset,1,ns_cnt_fp)!=1)
    {
        strcpy(mes,"Unable to read header of file ");
        strcat(mes,fn_cnt);
        errmes(mes);
        goto error2;
    }

    // validity check <======================================================
    if (ns_cnt_erp->savemode!=3)
    {
        strcpy(mes,fn_cnt);
        strcat(mes," is not a valid continuous file");
        errmes(mes);
        goto error2;
    }

    // type of continuous file; read event table <===========================
    ns_cnt_type=ns_cnt_erp->ContinousType;
    if (ns_cnt_type==1 || ns_cnt_type==3)
    {
        // read the TEEG structure
        if (fseek(ns_cnt_fp,ns_cnt_erp->EventTablePos,SEEK_SET)!=0)
        {
            strcpy(mes,"Unable to find event table of file ");
            strcat(mes,fn_cnt);
            errmes(mes);
            goto error2;
        }
        if (fread(&teeg,sizeof(TEEG),1,ns_cnt_fp)!=1)
        {
            strcpy(mes,"Unable to read event table of file ");
            strcat(mes,fn_cnt);
            errmes(mes);
            goto error2;
        }

        // type of event table
        ns_cnt_event_type=teeg.Teeg;
        if (ns_cnt_event_type<1 || ns_cnt_event_type>2)
        {
            sprintf(mes,"Invalid event table type (%i) for file ", ns_cnt_event_type);
            strcat(mes,fn_cnt);
            errmes(mes);
#ifdef FIX_TEEG_TYPE
	    ns_cnt_event_type = (ns_cnt_event_type & 0x00FF);
	    sprintf(mes, "assuming lower byte contains valid event table type (%i)", ns_cnt_event_type);
            errmes(mes);
#else
            goto error2;
#endif
        }

        // number of events
        if (ns_cnt_event_type==1) ns_cnt_nevents=teeg.Size/sizeof(EVENT1);
        else                      ns_cnt_nevents=teeg.Size/sizeof(EVENT2);

        if (ns_cnt_nevents>0)
        {
            // allocate memory for type 2 event table
            ns_cnt_evt=(EVENT2 *)malloc(ns_cnt_nevents*sizeof(EVENT2));
            if (ns_cnt_evt==NULL)
            {
                errmes("Out of memory");
                goto error2;
            }
            
            // read event table
            if (ns_cnt_event_type==1)
            {
                // read type 1 events, and convert to type 2
                for (i=0;i<ns_cnt_nevents;i++)
                {
                    if (fread(&ns_cnt_evt[i].Event1,sizeof(EVENT1),1,ns_cnt_fp)!=1)
                    {
                        strcat(mes,"Unable to read event table for file ");
                        strcpy(mes,fn_cnt);
                        errmes(mes);
                        goto error3;
                    }
                    // set other fields to default values
                    ns_cnt_evt[i].Type=0;
                    ns_cnt_evt[i].Code=0;
                    ns_cnt_evt[i].Latency=0.f;
                    ns_cnt_evt[i].EpochEvent=0;
                    ns_cnt_evt[i].Accept=1;
                    ns_cnt_evt[i].Accuracy=0;
                }
            }
            else
            {
                // read type2 events
                if (fread(ns_cnt_evt,ns_cnt_nevents*sizeof(EVENT2),1,ns_cnt_fp)!=1)
                {
                        strcat(mes,"Unable to read event table for file ");
                        strcpy(mes,fn_cnt);
                        errmes(mes);
                        goto error3;
                }
            }
        }
    }
    else if (ns_cnt_type==0)
    {
	printf("ns_cnt_type==0\n");
        ns_cnt_event_type=0;
    }
    else
    {
        errmes("Type of continuous file is not recognized");
        goto error2;
    }

    // size of a single acquired point of data
    if (ns_cnt_type==0) ns_cnt_point_size=2*(ns_cnt_nc+2);
    else                ns_cnt_point_size=2*ns_cnt_nc;

    ns_cnt_current_event=-1;
    // position to beginning of data
    fseek(ns_cnt_fp,ns_cnt_offset,SEEK_SET);

    // successful open and read of header and event table
    ns_cnt_opened=1;
    return(ns_cnt_erp);

    // error
    error3: if (ns_cnt_nevents>0) free(ns_cnt_evt);
    error2: free(ns_cnt_erp);
    error1: fclose(ns_cnt_fp);
    error0: ns_cnt_opened=0;
            return(NULL);
}

/****************************************************************************
ns_cnt_event_table

Passes pointer to event table.
****************************************************************************/

EVENT2* ns_cnt_event_table()
{
    return(ns_cnt_evt);
}

/****************************************************************************
ns_cnt_closeall

Free all static memory and close the .CNT file.
****************************************************************************/

void ns_cnt_closeall()
{
    ns_cnt_finalize_epoch();
    if (ns_cnt_opened)
    {
        if (ns_cnt_nevents>0) free(ns_cnt_evt);
        free(ns_cnt_erp);
        fclose(ns_cnt_fp);
        ns_cnt_opened=0;
    }
}

/****************************************************************************
ns_cnt_next_event()

Sets current event number for next event of a given type.
****************************************************************************/

long ns_cnt_next_event
(
    short event_type
)
{
    long i;
    long ret;

    ns_cnt_current_event++;

    for (i=ns_cnt_current_event;i<ns_cnt_nevents;i++)
    {
        if (event_type==EVT_STIM)
        {
            if (ret=ns_cnt_evt[i].Event1.StimType) break;
        }
        else if (event_type==EVT_FKEY)
        {
            if (ret=ns_cnt_evt[i].Event1.KeyBoard) break;
        }
    }

    if (i>=ns_cnt_nevents) return(0);       // no more events of given type
    ns_cnt_current_event=i;
    ns_cnt_offset=ns_cnt_evt[i].Event1.Offset;
    ns_cnt_point_number=(ns_cnt_offset-ns_cnt_header_offset)/ns_cnt_point_size;
    return(ret);
}

/****************************************************************************
ns_cnt_initialize_epoch()

Initializes variables for epoching the continuous file.  If successful, 
returns a pointer to a multiplexed, microvolt-scaled epoch buffer;
returns NULL otherwise.
****************************************************************************/

float** ns_cnt_initialize_epoch
(
    float xmin,         // lower time bound for epoch (seconds)
    float xmax,         // upper time bound for epoch (seconds)
    long start_point,   // starting point for read
    long stop_point     // stopping point for read
)
{
    long ltemp;

    if (!ns_cnt_opened)
    {
        errmes("Error: ns_cnt_initialize_epoch called without an open file");
        goto error0;
    }
    if (ns_cnt_epoch_initialized) ns_cnt_finalize_epoch();

    if (xmin>xmax) xmin=xmax=0.f;

    ns_cnt_erp->xmin=xmin;
    ns_cnt_erp->xmax=xmax;

    // (re)compute number of points in the epoch
    ns_cnt_epoch_pnts=ns_cnt_erp->pnts=1+(unsigned)((xmax-xmin)*ns_cnt_erp->rate);

    // compute offset for start of epoch relative to an event
    ns_cnt_epoch_offset=(long)(xmin*ns_cnt_erp->rate);

    // allocate memory for microvolt-scaled epoch buffer
    if (!matrix_alloc(&ns_cnt_epoch,ns_cnt_epoch_pnts,ns_cnt_nc))
    {
        errmes("Out of memory");
        goto error0;
    }

    // allocate memory for raw a/d buffer
    if (ns_cnt_type==0) ltemp=ns_cnt_nc+2;
    else                ltemp=ns_cnt_nc;
    if (!matrix_alloc_s(&ns_cnt_adbuff,ns_cnt_epoch_pnts,ltemp))
    {
        errmes("Out of memory");
        goto error1;
    }
    ns_cnt_adbuff_size=2*ns_cnt_epoch_pnts*ltemp;

    // if SynAmp, allocate memory for block buffer
    if (ns_cnt_type==3)
    {
        // compute number of time points in a block
        ns_cnt_SynAmp_pnts=ns_cnt_erp->ChannelOffset/2;
        // compute block size
        ns_cnt_SynAmp_size=2*ns_cnt_nc*ns_cnt_SynAmp_pnts;
        // allocate vectored block
        if (!matrix_alloc_s(&ns_cnt_SynAmp_blk,ns_cnt_nc,ns_cnt_SynAmp_pnts))
        {
            errmes("Out of memory");
            goto error2;
        }
    }

    // set the epoch_flag
    if (ns_cnt_epoch_pnts>1 && ns_cnt_nevents>0) ns_cnt_epoch_flag=1;
    else                                         ns_cnt_epoch_flag=0;

    // initiaze some variables for reading
    ns_cnt_read_count=0;
    ns_cnt_start_point=start_point;
    ns_cnt_stop_point=stop_point;

    // success
    ns_cnt_epoch_initialized=1;
    return(ns_cnt_epoch);

    // error
    error2: matrix_free_s(&ns_cnt_adbuff);
    error1: matrix_free(&ns_cnt_epoch);
    error0: ns_cnt_finalize_epoch();
            return(NULL);
}

/****************************************************************************
ns_cnt_finalize_epoch

Releases memory for epoch buffer.
****************************************************************************/

void ns_cnt_finalize_epoch()
{
    if (ns_cnt_epoch_initialized)
    {
        if (ns_cnt_type==3) matrix_free_s(&ns_cnt_SynAmp_blk);
        matrix_free_s(&ns_cnt_adbuff);
        matrix_free(&ns_cnt_epoch);
        ns_cnt_epoch_initialized=0;
    }
}

/****************************************************************************
ns_cnt_read_epoch

Reads next epoch (or point).  Returns ns_cnt_read_count on success, and 0
when the file is exhausted.
****************************************************************************/

long ns_cnt_read_epoch()
{
    long point_number;
    long offset1;       // offset for start of epoch
    long offset2;       // offset for end of epoch
    long blk1;          // SynAmp block number containing first point
    long blk2;          // SynAmp block number containing last  point
    long pnt1;          // SynAmp: point number within first block
    long pnt2;          // SynAmp: point number within last  block
    long i;             // point index
    long j;             // channel index
    long k;             // block index
    long l;             // point index within a block
    long ltemp;         // temporary variable
    short dc;           // dc calibration offset
    float sf;           // scale factor to microvolts

    if (ns_cnt_epoch_flag)
    {
        // discrete case: event-related epochs
        while(1)
        {
            // go to the next stimulus event
            if (!ns_cnt_next_event(EVT_STIM)) return(0);

            // start/stop bounds
            if (ns_cnt_point_number<ns_cnt_start_point) continue;
            if
            (
                ns_cnt_stop_point>ns_cnt_start_point &&
                ns_cnt_point_number>ns_cnt_stop_point
            )
                return(0);

            // point number corresponding to beginning of epoch
            point_number=ns_cnt_point_number+ns_cnt_epoch_offset;
            if (point_number<0) continue;
            // offset boundaries for epoch
            offset1=ns_cnt_header_offset+point_number*ns_cnt_point_size;
            offset2=offset1+(ns_cnt_epoch_pnts-1)*ns_cnt_point_size;
            /// ** here is the place to implement check for rejected block **
            break;
        }
    }
    else
    {
        // continuous case
        if (ns_cnt_read_count==0) ns_cnt_point_number=ns_cnt_start_point;
        else                      ns_cnt_point_number++;
        point_number=ns_cnt_point_number;
        ns_cnt_offset=ns_cnt_header_offset+point_number*ns_cnt_point_size;
        offset1=offset2=ns_cnt_offset;
        if
        (
            ns_cnt_stop_point>ns_cnt_start_point &&
            ns_cnt_point_number>ns_cnt_stop_point
        )
            return(0);
    }

    // is point or end of epoch past the end?
    if (offset2>=ns_cnt_erp->EventTablePos) return(0);

    // SynAmp file
    if (ns_cnt_type==3)
    {
        // compute block number that contains the first point
        blk1=(offset1-ns_cnt_header_offset)/ns_cnt_SynAmp_size;
        // compute point number within the first block
        pnt1=point_number-(blk1*ns_cnt_SynAmp_pnts);
        // compute block number that contains the last point
        blk2=(offset2-ns_cnt_header_offset)/ns_cnt_SynAmp_size;
        // compute point number within the first block
        pnt2=(point_number+ns_cnt_epoch_pnts-1)-(blk2*ns_cnt_SynAmp_pnts);

        //printf("blk1=%ld, pnt1=%ld, blk2=%ld, pnt2=%ld\n",blk1,pnt1,blk2,pnt2);

        // Transfer data to ns_cnt_adbuff
        /// 1. Read the first block
        offset1=ns_cnt_header_offset+blk1*ns_cnt_SynAmp_size;
        fseek(ns_cnt_fp,offset1,SEEK_SET);
        if (fread(ns_cnt_SynAmp_blk[0],ns_cnt_SynAmp_size,1,ns_cnt_fp)!=1)
        {
            errmes("Error attempting to read .CNT file");
            return(0);
        }
        i=0;
        //// handle special case that first and last blocks are the same
        if (blk1==blk2) ltemp=pnt2;                 
        else            ltemp=ns_cnt_SynAmp_pnts-1;
        for (l=pnt1;l<=ltemp;l++)
        {
            for (j=0;j<ns_cnt_nc;j++)
            {
                ns_cnt_adbuff[i][j]=ns_cnt_SynAmp_blk[j][l];
            }
            i++;
        }
        /// 2. Read the blocks properly between first and last
        for (k=blk1+1;k<blk2;k++)
        {
            if (fread(ns_cnt_SynAmp_blk[0],ns_cnt_SynAmp_size,1,ns_cnt_fp)!=1)
            {
                errmes("Error attempting to read .CNT file");
                return(0);
            }
            for (l=0;l<ns_cnt_SynAmp_pnts;l++)
            {
                for (j=0;j<ns_cnt_nc;j++)
                {
                    ns_cnt_adbuff[i][j]=ns_cnt_SynAmp_blk[j][l];
                }
                i++;
            }
        }
        /// 3. Read the last block (if different from the first)
        if (blk2>blk1)
        {
            if (fread(ns_cnt_SynAmp_blk[0],ns_cnt_SynAmp_size,1,ns_cnt_fp)!=1)
            {
                errmes("Error attempting to read .CNT file");
                return(0);
            }
            for (l=0;l<=pnt2;l++)
            {
                for (j=0;j<ns_cnt_nc;j++)
                {
                    ns_cnt_adbuff[i][j]=ns_cnt_SynAmp_blk[j][l];
                }
                i++;
            }
        }
    }
    // Type 1 and Type 0 continuous files
    else if (ns_cnt_type==1 || ns_cnt_type==0)
    {
        fseek(ns_cnt_fp,offset1,SEEK_SET);
        if (fread(ns_cnt_adbuff[0],ns_cnt_adbuff_size,1,ns_cnt_fp)!=1)
        {
            errmes("Error attempting to read .CNT file");
            return(0);
        }
    }
    else
    {
        errmes("This file type is not implemented");
        return(0);
    }

    // scale to microvolts
    for (j=0;j<ns_cnt_nc;j++)
    {
        dc=ns_cnt_erp->elect_tab[j].baseline;
        sf=ns_cnt_erp->elect_tab[j].sensitivity*
           ns_cnt_erp->elect_tab[j].calib/204.8f;
        for (i=0;i<ns_cnt_epoch_pnts;i++)
        {
            ns_cnt_epoch[i][j]=sf*(ns_cnt_adbuff[i][j]-dc);
        }
    }

    return(++ns_cnt_read_count);
}

/****************************************************************************
ns_cnt_get_epoch_flag

Returns the epoch_flag
****************************************************************************/

short ns_cnt_get_epoch_flag()
{
    return(ns_cnt_epoch_flag);
}


