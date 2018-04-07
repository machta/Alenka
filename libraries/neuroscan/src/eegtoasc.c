/****************************************************************************
EEGTOASC.C

Example program to demonstrate the get_epoch() function.

The syntax from the command line is:

EEGTOASC <EEG file> <ASCII file> <start epoch #> <stop epoch #>

where:

    <EEG file> names the Neuro Scan epoched EEG file (omit .EEG extension);
    <ASCII file> names the output ASCII file;
    <start epoch #> is the number of the first epoch to export to ASCII; and
    <stop epoch #> is the number of the last epoch to export to ASCII.

If the last two parameters are omitted, all epochs will be exported.
****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void main(int argc, char *argv[])
{
    // variables required by get_epoch() -- see GETEPOCH.C
    char flag;
    char fn[100];
    short nchannels;
    short pnts;
    float rate;
    float xmin;
    float xmax;
    short type;
    float **x;

    // other variables
    char fn_asc[100];       // name of ASCII file
    FILE *fp_asc;           // file pointer for ASCII file
    short i;                // point index
    short j;                // channel index
    short epoch_num;        // epoch counter
    short start_epoch=1;    // starting epoch number
    short stop_epoch=0;     // stop epoch number

    // read the Neuro Scan .EEG file name
    if (argc>1) strcpy(fn,argv[1]);
    else
    {
        printf("Error: Missing 1st parameter (Neuro Scan .EEG file name)");
        exit(1);
    }
    strcat(fn,".eeg");

    // read the ASCII output file name
    if (argc>2) strcpy(fn_asc,argv[2]);
    else
    {
        printf("Error: Missing 2nd parameter (ASCII file name)");
        exit(1);
    }

    // read the start_epoch & stop_epoch parameters (optional)
    if (argc>3) start_epoch=atoi(argv[3]);
    if (argc>4) stop_epoch=atoi(argv[4]);

    // open the ASCII output file
    fp_asc=fopen(fn_asc,"w");
    if (fp_asc==NULL)
    {
        printf("Error: Unable to open %s\n",fn_asc);
        exit(1);
    }

    // set flag to 0 for initial call to get_epoch()
    flag=0;
    if (!get_epoch(&flag,fn,&nchannels,&pnts,&rate,&xmin,&xmax,&type,&x))
    {
        printf("get_epoch has returned an error (#1)");
        fclose(fp_asc);
        exit(1);
    }

    // MAIN LOOP (until flag indicates all epochs have been read)
    epoch_num=1;
    while(1)
    {
        if (stop_epoch>0 && epoch_num>stop_epoch) break;

        // read epoch
        if (!get_epoch(&flag,fn,&nchannels,&pnts,&rate,&xmin,&xmax,&type,&x))
        {
            printf("get_epoch has returned an error (#2)");
            fclose(fp_asc);
            exit(1);
        }

        // end of file has been reached;
        if (flag==2) break;

        // ASCII output
        if (epoch_num>=start_epoch)
        {
            printf("%d\n",epoch_num);
            if (start_epoch!=stop_epoch)
            {
                fprintf(fp_asc,"Epoch=%5d, type=%3d\n",epoch_num,type);
            }
            for (j=0;j<nchannels;j++)
            {
                // fprintf(fp_asc,"Channel %4d: ",j+1);
                for (i=0;i<pnts;i++)
                {
                    fprintf(fp_asc,"%10.3f",x[j][i]);
                }
                fprintf(fp_asc,"\n");
            }
        }

        epoch_num++;
    }

    // cleaunp file and variables initialized by get_epoch()
    get_epoch(&flag,fn,&nchannels,&pnts,&rate,&xmin,&xmax,&type,&x);

    // close ASCII file
    fclose(fp_asc);

    exit(0);

}

