// function prototypes for matrix allocation and deallocation (matrix_m.c)
short matrix_alloc(float ***,short,short);
void  matrix_free(float ***);
short matrix_alloc_symmetric(float ***,short,char);
short matrix_alloc_d(double ***,short,short);
void  matrix_free_d(double ***);
short matrix_alloc_symmetric_d(double ***,short,char);
short matrix_alloc_l(long ***,short,short);
void  matrix_free_l(long ***);
short matrix_alloc_s(short ***,short,short);
void  matrix_free_s(short ***);
short matrix_alloc_c(char ***,short,short);
void  matrix_free_c(char ***);
short matrix3_alloc(float ****,short,short,short);
void  matrix3_free(float ****);
short matrix3_alloc_symmetric(float ****,short,short,char);
short matrix3_alloc_d(double ****,short,short,short);
void  matrix3_free_d(double ****);
short matrix4_alloc(float *****,short,short,short,short);
void  matrix4_free(float *****);
short matrix4_alloc_d(double *****,short,short,short,short);
void  matrix4_free_d(double *****);
short matrix5_alloc_d(double ******,short,short,short,short,short);
void  matrix5_free_d(double ******);

