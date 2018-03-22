/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "FEF-IntermediateDraft"
 * 	found in "../annexb-snacc-122001.asn1"
 */

#ifndef	_ObservationTime_H_
#define	_ObservationTime_H_


#include <asn_application.h>

/* Including external dependencies */
#include "AbsoluteTime.h"
#include "RelativeTime.h"
#include "HighResRelativeTime.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ObservationTime */
typedef struct ObservationTime {
	AbsoluteTime_t	*absolutetimestamp	/* OPTIONAL */;
	RelativeTime_t	*relativetimestamp	/* OPTIONAL */;
	HighResRelativeTime_t	*hirestimerelativestamp	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ObservationTime_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ObservationTime;

#ifdef __cplusplus
}
#endif

#endif	/* _ObservationTime_H_ */
