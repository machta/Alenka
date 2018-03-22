/*
    Example how to generate GDF files as C/C++ code. 
    This is only a rough outline, for details, please check biosig.h. 
    

    Copyright (C) 2016,2018 Alois Schloegl <alois.schloegl@ist.ac.at>
    This file is part of the "BioSig for C/C++" repository
    (biosig4c++) at http://biosig.sf.net/


    BioSig is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#include <biosig.h>
#include <assert.h>
#include <stdlib.h>

extern int VERBOSE_LEVEL;

int main(int argc, char* argv[]) {
	VERBOSE_LEVEL = 8;

	int number_of_channels=4;
	int number_of_events=0;
	double sampling_rate = 5000; // Hz
	size_t total_number_of_samples=100000;
	size_t number_of_epochs=5;

	HDRTYPE *hdr=constructHDR(number_of_channels, number_of_events);
	
	/* alternatively, number of channels, and number of events can be set this way. 
	HDRTYPE *hdr=constructHDR(0, 0);
	biosig_set_number_of_channels(hdr, number_of_channels);
	biosig_set_number_of_events(hdr, number_of_events);
	*/

	// set sampling rate 
	biosig_set_samplerate(hdr, sampling_rate);

	/*
	the data layout can be controlled in this way.
	Here it is assumed, that all channels have the same sampling rate
	
	It's also possible, the define different sampling rates for different channels 
	through  
	    hdr->CHANNEL[ch].SPR
	and
	    biosig_channel_set_samples_per_record(hdr->CHANNEL[ch], spr);
	
	*/
	size_t nrec,spr; 
	// multiplexed data
	spr=1; nrec=total_number_of_samples;
	
	// channel-based data
	spr=total_number_of_samples; nrec=1;
	
	// sweep-based data
	spr=total_number_of_samples/number_of_epochs; nrec=number_of_epochs;
	
	// any other combination is fine as long as 
	assert( spr*nrec == total_number_of_samples);
	biosig_set_number_of_samples(hdr, nrec, spr);

	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);

	/*************************************************************
	            define channel properties 
	 *************************************************************/
	double PhysMax=10;	
	double PhysMin=0;	
	double DigMax= 32767;	
	double DigMin=-32768;	
	double LowPass=sampling_rate/4;
	double HighPass=0.0;
	double Notch=50;	// 0: notch off, NaN: unkown
	int chan;
	
	chan=0;
	CHANNEL_TYPE *CH = biosig_get_channel(hdr, chan);
	biosig_channel_set_label(CH, "Channel 1");
	biosig_channel_set_unit(CH,"mV");
	biosig_channel_set_scaling(CH, PhysMax, PhysMin, DigMax, DigMin);
	biosig_channel_set_datatype_to_int16(CH);
	biosig_channel_set_filter(CH, LowPass, HighPass, Notch);
	biosig_channel_set_transducer(CH, "EEG");
	biosig_channel_set_timing_offset(CH, 0);
		
	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);

	// do the same for all other channels 
	for (chan++; chan < number_of_channels; chan++) {
		CHANNEL_TYPE *CH = biosig_get_channel(hdr, chan);
		biosig_channel_set_label(CH, "Channel 2");
		biosig_channel_set_unit(CH,"mV");
		biosig_channel_set_scaling(CH, PhysMax, PhysMin, DigMax, DigMin);
		biosig_channel_set_datatype_to_int16(CH);
		biosig_channel_set_filter(CH, LowPass, HighPass, Notch);
		biosig_channel_set_timing_offset(CH, chan*1e-6);	// e.g. 1us delay in ADC multiplexer, 0.0 when simultaneous sampling
	}
	CHANNEL_TYPE *ch1 = biosig_get_channel(hdr, 0);
	CHANNEL_TYPE *ch2 = biosig_get_channel(hdr, 1);
	CHANNEL_TYPE *ch3 = biosig_get_channel(hdr, 2);
	CHANNEL_TYPE *ch4 = biosig_get_channel(hdr, 3);
	biosig_channel_set_samples_per_record(ch2, spr/2);
	biosig_channel_set_samples_per_record(ch3, spr/4);
	biosig_channel_set_samples_per_record(ch4, spr/5);

	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);


	/*************************************************************
	            demographic data 
	 *************************************************************/
	time_t timer;	
	timer=time(NULL);
	struct tm *StartTime = localtime(&timer);
	biosig_set_startdatetime(hdr, *StartTime);
	struct tm birthday;
	birthday.tm_year=2005-1900;
	birthday.tm_mon=1;
	birthday.tm_mday=1;
	biosig_set_birthdate(hdr, birthday);

	biosig_set_patient_name(hdr, "Anonymous", "A.", NULL);
	biosig_set_patient_id(hdr, "pseudonym_0123456789");
	biosig_set_recording_id(hdr, "recording 0000");
	// biosig_set_technician(hdr, "Ms.Expert"); // by login name is used
	biosig_set_manufacturer_name(hdr, "Vendor XYZ");
	biosig_set_manufacturer_model(hdr, "SuperDuper");
	biosig_set_manufacturer_version(hdr, "alpha");
	biosig_set_manufacturer_serial_number(hdr, "000000");
	biosig_set_application_specific_information(hdr, "anything");

	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);


	/*
	set markers in event table, this can be called at any time 
	file is closed with SCLOSE. 
	*/ 
#if 0
	// add 3 markers indicating "start-of-new-segment" 	
	spr = 5000;
	biosig_set_nth_event(hdr, 0, 0x7ffe, spr, 0, 0, NULL, NULL);
	biosig_set_nth_event(hdr, 0, 0x7ffe, 2*spr, 0, 0, NULL, NULL);
	biosig_set_nth_event(hdr, 0, 0x7ffe, 3*spr, 0, 0, NULL, NULL);

	// add free text annoations, typ must be between 1 and 255 (0xff)
	biosig_set_nth_event(hdr, 0, 0x01, 7000, 0, 0, NULL, "Marker1");
	biosig_set_nth_event(hdr, 0, 0, 0, 0, 5000, NULL, "Seg1chan1");
	biosig_set_nth_event(hdr, 0, 0, 5000, 3, 5000, NULL, "Seg2chan4");
	biosig_set_nth_event(hdr, 0, 0, 10000, 2, 5000, NULL, "Seg3chan3");
#endif	

	/*************************************************************
	            open file and write header
	 *************************************************************/
	 
	const char *f="filename.gdf";
	if (argc>1) f=argv[1]; 
	
	hdr->TYPE=GDF;
	hdr->VERSION=3.0;
	hdr = sopen(f,"w",hdr);
	
	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);

	
	/*************************************************************
	  write data block: 
		there are high level and low level approaches. 
		high level approaches assume data samples are available
		in a huge matrix, and swrite will deal with the details,
		like endian conversion, downsampling of the data, etc. 		

	  	the low level approach, assumes that the user, knows how 
	  	to generate the correct binary layout of the data samples,  
	  	and will use fwrite(..,..,..,hdr->FILE.FID). This might 
	  	cause ABI incompatibilities on Windows. Most likely, 
	  	another API need to be provided by biosig. 
	 *************************************************************/
	size_t i,j,k;

	/* highlevel approach: needs to define data matrix, and its properties 
		BIOSIG_FLAG_UCAL : do not apply scaling factors to convert from physical to digital values
				if this flag is not set, the scaling factors are applied in SWRITE
		BIOSIG_FLAG_ROW_BASED_CHANNELS: consecutive samples are from neighboring channel 
				if not defined, all samples of first channel, then all samples form next
				channel must be available in data. 
	*/
	biosig_set_flag(hdr, BIOSIG_FLAG_UCAL | BIOSIG_FLAG_ROW_BASED_CHANNELS);

	/* lets generate all data
		1,2,3,4 Hz sinusoid, for each channel, resp.
	 */
	biosig_data_type* data=(biosig_data_type*)malloc(number_of_channels * total_number_of_samples * sizeof(biosig_data_type)); 
	for (k=0; k < number_of_channels * total_number_of_samples; k++)
		data[k] = 33000 * sin( 1.0*(k%number_of_channels +1 )*(k%total_number_of_samples)*2*M_PI/sampling_rate );

	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);

#if defined(HL1)
	// option 1: define all data
	size_t count = swrite(data, nrec, hdr);
	if (nrec != count) fprintf(stderr,"%s (line %i) error\n",__FILE__,__LINE__);

	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);

#elif defined(HL2)
	// option 2: define data of a single block
	data = realloc(data, number_of_channels * spr * sizeof(biosig_data_type)); 
	for (k=0; k<nrec; k++) {	
		for (k=0; k < number_of_channels * spr; k++) data[k] = (k % 2^15);
		
		size_t count = swrite(data, 1, hdr);
		if (count != 1) fprintf(stderr,"%s (line %i) error\n",__FILE__,__LINE__);
	}

	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);

#elif 0 
	// this 
	spr=total_number_of_samples/number_of_epochs; 
	for (k=0; k < number_of_epochs; k++) {
	for (chan=0; chan < number_of_channels; chan++) {
		for (chan=0; chan < number_of_channels; chan++) {
			CHANNEL_TYPE *ch = biosig_get_channel(hdr, chan);
			for (j=0; j < spr; j++)	{
				data[j + chan*spr + k*spr*number_of_channels] = 3;

				// option 1: write sample j channel chan of sweep k
				// fwrite(&sample[k][chan][j],sizeof(int16_t),1,hdr->FILE.FID);
			}
			// option 2: avoid inner loop
			// fwrite(&sample_block[k][chan],sizeof(int16_t),spr,hdr->FILE.FID);
		}
	}
	
	 
	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);


	/* low level approaches using fwrite - chances are it will 
	   not work in Windows because of ABI incompatibilities */ 

#elif defined(LL1)
 	/* option 1: all the data is prepared and written at once. */	     
	uint16_t *sample_block=(uint16_t*)malloc(hdr->AS.bpb*nrec); 
	for (k=0; k < hdr->AS.bpb * nrec / sizeof(uint16_t); k++)
		sample_block[k] = 33000 * sin( 1.0*(k%number_of_channels +1 )*(k%total_number_of_samples)*2*M_PI/sampling_rate );

	fwrite(sample_block,hdr->AS.bpb,nrec,hdr->FILE.FID);
	free(sample_block);

	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);

#elif defined(LL2)
	uint16_t *sample_block=(uint16_t*)malloc(hdr->AS.bpb); 
	for (k=0; k < hdr->AS.bpb / sizeof(uint16_t); k++)
		sample_block[k] = 33000 * sin( 1.0*(k%number_of_channels +1 )*(k%total_number_of_samples)*2*M_PI/sampling_rate );
		
	for (k=0; k < nrec; k++) {
		/* option 3: the size of a single block is hdr->AS.bpb, 
	           a single data block is define, and written at once. 
	           change content of sample_block within loop
		*/	     
		fwrite(sample_block,hdr->AS.bpb,1,hdr->FILE.FID);
	}
	free(sample_block);
	
	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);

#endif	
	/*************************************************************
	            write event table and close file
	 *************************************************************/

	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);

	sclose(hdr);
	/* cleanup, free any allocated memory to avoid memory leaks */
	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);

	destructHDR(hdr);

	if (VERBOSE_LEVEL>7) fprintf(stdout,"%s (line %i)\n",__FILE__,__LINE__);

	if (data!=NULL) free(data);

}

