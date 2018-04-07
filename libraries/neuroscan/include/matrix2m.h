/****************************************************************************
MATRIX2M.H

Header file for MATRIX2M.C (Derived from matrix_m.h)
****************************************************************************/

// function prototypes for matrix allocation and deallocation (matrix2m.c)
short matrix_alloc(float ***,long,long);
void  matrix_free(float ***);
short matrix_alloc_symmetric(float ***,long,char);
short matrix_alloc_d(double ***,long,long);
void  matrix_free_d(double ***);
short matrix_alloc_symmetric_d(double ***,long,char);
short matrix_alloc_l(long ***,long,long);
void  matrix_free_l(long ***);
short matrix_alloc_s(short ***,long,long);
void  matrix_free_s(short ***);
short matrix_alloc_c(char ***,long,long);
void  matrix_free_c(char ***);
short matrix3_alloc(float ****,long,long,long);
void  matrix3_free(float ****);
short matrix3_alloc_symmetric(float ****,long,long,char);
short matrix3_alloc_d(double ****,long,long,long);
void  matrix3_free_d(double ****);
short matrix3_alloc_s(short ****,long,long,long);
void  matrix3_free_s(short ****);
short matrix3_alloc_us(unsigned short ****,long,long,long);
void  matrix3_free_us(unsigned short ****);
short matrix3_alloc_uc(unsigned char ****,long,long,long);
void  matrix3_free_uc(unsigned char ****);
short matrix4_alloc(float *****,long,long,long,long);
void  matrix4_free(float *****);
short matrix4_alloc_d(double *****,long,long,long,long);
void  matrix4_free_d(double *****);
short matrix5_alloc_d(double ******,long,long,long,long,long);
void  matrix5_free_d(double ******);

