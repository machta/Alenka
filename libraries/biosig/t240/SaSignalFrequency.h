/*
 * Generated by asn1c-0.9.21 (http://lionet.info/asn1c)
 * From ASN.1 module "FEF-IntermediateDraft"
 * 	found in "../annexb-snacc-122001.asn1"
 */

#ifndef	_SaSignalFrequency_H_
#define	_SaSignalFrequency_H_


#include <asn_application.h>

/* Including external dependencies */
#include "FEFFloat.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SaSignalFrequency */
typedef struct SaSignalFrequency {
	FEFFloat_t	 lowedgefreq;
	FEFFloat_t	 highedgefreq;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SaSignalFrequency_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SaSignalFrequency;

#ifdef __cplusplus
}
#endif

#endif	/* _SaSignalFrequency_H_ */
