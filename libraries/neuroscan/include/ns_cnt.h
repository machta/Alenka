/****************************************************************************
NS_CNT.H

Header file for NS_CNT.C (functions to read Neuro Scan .CNT files)
****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

//#pragma pack(1)
#include <pack.h>
#include "eegdef.h"     // some defines
#include "sethead.h"    // Neuro Scan header & footer structures
#include <unpack.h>

// definitions for types of events
#define EVT_NC      0           // not classified
#define EVT_STIM    1           // stimulus event
#define EVT_RESP    2           // response event
#define EVT_FKEY    3           // function key event
#define EVT_START   4           // start recording
#define EVT_STOP    5           // stop recording
#define EVT_DC      6           // DC correction

// Function prototypes ******************************************************
SETUP* ns_cnt_open(const char *);
EVENT2* ns_cnt_event_table();
void ns_cnt_closeall(void);
long ns_cnt_next_event(short);
float** ns_cnt_initialize_epoch(float,float,long,long);
short ns_cnt_get_epoch_flag(void);
void ns_cnt_finalize_epoch(void);
long ns_cnt_read_epoch(void);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
