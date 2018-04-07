/****************************************************************************
MATRIX_M.C     Copyright (C) 1992, Neurosoft, Inc.  All rights reserved.

Matrix memory allocation and deallocation routines.  Mark E. Pflieger.

PHILOSOPHY.  Primary memory for a matrix is allocated in one contiguous
block, and pointers are assigned as "precomputed tabs" (e.g., for a 2D 
matrix, the tabs point to the beginning of rows).  This approach has two main
advantages over the method advocated in Numerical Recipes in C: (i) The
ability to perform block operations on the entire matrix (e.g., read, write,
memcpy) is retained, and (ii) for an N-dimensional matrix, exactly N memory 
allocations/deallocations are required (whereas the NRC approach requires an 
allocation for each row of a 2D matrix).

The following routines implement memory allocation and deallocation for
2, 3, 4, and 5 dimensional matrices.
****************************************************************************/

#include "matrix_m.h"

#include <malloc.h>

/****************************************************************************
matrix_alloc()

Allocates memory for a floating point 2-dimensional matrix.

CALLING PROGRAM USAGE:
    float **x;                              // declaration
    matrix_alloc(&x,n1,n2);                 // allocation
    write(fh,x[0],n1*n2*sizeof(float));     // sample block operation
    matrix_free(&x);                        // deallocation
****************************************************************************/

short matrix_alloc      // returns 1 if OK, 0 if allocation fails
(
    float ***x,         // pointer to 2-dimensional matrix (level 1)
    short n1,           // # for 1st dimension (rows)
    short n2            // # for 2nd dimension (columns)
)
{
    short i;            // index to 1st dimension
    short j;            // index to 2nd dimension

    // allocate pointers for 1st dimension (pointers to rows)
    *x=(float **)malloc(n1*sizeof(float *));
    if (*x==NULL)
    {
        //errmes("MATRIX_ALLOC: Unable to allocate memory for pointers.");
        return(0);
    }

    // primary memory allocation & pointer assignment
    (*x)[0]=(float *)malloc(n1*n2*sizeof(float));
    if ((*x)[0]==NULL)
    {
        //errmes("MATRIX_ALLOC: Unable to allocate primary memory.");
        free(*x);
        return(0);
    }
    for (i=0;i<n1;i++) (*x)[i]=&((*x)[0][i*n2]);

    // initialize matrix with zeros
    for (i=0;i<n1;i++) for (j=0;j<n2;j++) (*x)[i][j]=0.0;

    return(1);
}

/****************************************************************************
matrix_free()

Frees a floating point 2-dimensional matrix
****************************************************************************/

void matrix_free
(
    float ***x         // pointer to 2-dimensional matrix (level 1);
)
{
    free((*x)[0]);     // free primary matrix
    free(*x);          // free pointers to rows
}

/****************************************************************************
matrix_alloc_symmetric()

Allocates memory for a floating point 2-dimensional symmetric matrix, with
the convention that the second index must be less than or equal to the first
index.  If diag==1, primary memory units allocated are n*(n+1)/2 (diagonal 
plus lower offdiagonal); otherwise, primary memory units allocated are
n*(n-1)/2 (lower offdiagonal only).

CALLING PROGRAM USAGE WHEN INCLUDING THE DIAGONAL:
    float **x;                              // declaration
    matrix_alloc_symmetric(&x,n,1);         // allocation
    write(fh,x[0],sizeof(float)*n*(n+1)/2); // sample block operation
    matrix_free(&x);                        // deallocation
CALLING PROGRAM USAGE WHEN EXCLUDING THE DIAGONAL:
    float **x;                              // declaration
    matrix_alloc_symmetric(&x,n,0);         // allocation
    write(fh,x[0],sizeof(float)*n*(n-1)/2); // sample block operation
    matrix_free(&x);                        // deallocation
****************************************************************************/

short matrix_alloc_symmetric    // returns 1 if OK, 0 if allocation fails
(
    float ***x,                 // pointer to 2-dimensional matrix (level 1)
    short n,                    // dimension of square symmetric matrix
    char diag                   // 1=include, 0=exclude the diagonal
)
{
    short i;            // index to rows
    short j;            // index to columns
    short r;            // index for beginning of rows

    if (diag)           // INCLUDE THE DIAGONAL
    {
        // allocate pointers for 1st dimension (pointers to rows)
        *x=(float **)malloc(n*sizeof(float *));
        if (*x==NULL) return(0);
        
        // primary memory allocation
        (*x)[0]=(float *)malloc(sizeof(float)*n*(n+1)/2);
        if ((*x)[0]==NULL)
        {
            free(*x);
            return(0);
        }
        
        // pointer assignment to row beginnings
        for (r=0,i=0;i<n;i++)
        {
            (*x)[i]=&((*x)[0][r]);
            r+=i+1;
        }
        
        // initialize matrix with zeros
        for (i=0;i<n;i++) for (j=0;j<=i;j++) (*x)[i][j]=0.0;
    }
    else                // EXCLUDE THE DIAGONAL
    {
        // allocate pointers for 1st dimension (pointers to rows)
        *x=(float **)malloc(n*sizeof(float *));
        if (*x==NULL) return(0);
        
        // primary memory allocation
        (*x)[0]=(float *)malloc(sizeof(float)*n*(n-1)/2);
        if ((*x)[0]==NULL)
        {
            free(*x);
            return(0);
        }
        
        // pointer assignment to row beginnings
        for (r=0,i=1;i<n;i++)
        {
            (*x)[i]=&((*x)[0][r]);
            r+=i;
        }
        
        // initialize matrix with zeros (j must be less than i)
        for (i=1;i<n;i++) for (j=0;j<i;j++) (*x)[i][j]=0.0;
    }

    return(1);
}

/****************************************************************************
matrix_alloc_d()

Allocates memory for a double 2-dimensional matrix.

CALLING PROGRAM USAGE:
    double **x;                             // declaration
    matrix_alloc_d(&x,n1,n2);               // allocation
    write(fh,x[0],n1*n2*sizeof(float));     // sample block operation
    matrix_free_d(&x);                      // deallocation
****************************************************************************/

short matrix_alloc_d    // returns 1 if OK, 0 if allocation fails
(
    double ***x,        // pointer to 2-dimensional matrix (level 1)
    short n1,           // # for 1st dimension (rows)
    short n2            // # for 2nd dimension (columns)
)
{
    short i;            // index to 1st dimension
    short j;            // index to 2nd dimension

    // allocate pointers for 1st dimension (pointers to rows)
    *x=(double **)malloc(n1*sizeof(double *));
    if (*x==NULL)
    {
        // errmes("MATRIX_ALLOC_D: Unable to allocate memory for pointers.");
        return(0);
    }

    // primary memory allocation & pointer assignment
    (*x)[0]=(double *)malloc(n1*n2*sizeof(double));
    if ((*x)[0]==NULL)
    {
        // errmes("MATRIX_ALLOC_D: Unable to allocate primary memory.");
        free(*x);
        return(0);
    }
    for (i=0;i<n1;i++) (*x)[i]=&((*x)[0][i*n2]);

    // initialize matrix with zeros
    for (i=0;i<n1;i++) for (j=0;j<n2;j++) (*x)[i][j]=0.0;

    return(1);
}

/****************************************************************************
matrix_free_d()

Frees a double 2-dimensional matrix
****************************************************************************/

void matrix_free_d
(
    double ***x        // pointer to 2-dimensional matrix (level 1);
)
{
    free((*x)[0]);     // free primary matrix
    free(*x);          // free pointers to rows
}

/****************************************************************************
matrix_alloc_symmetric_d()

Allocates memory for a double 2-dimensional symmetric matrix, with the 
convention that the second index must be less than or equal to the first
index.  If diag==1, primary memory units allocated are n*(n+1)/2 (diagonal 
plus lower offdiagonal); otherwise, primary memory units allocated are
n*(n-1)/2 (offdiagonal  only).

CALLING PROGRAM USAGE WHEN INCLUDING THE DIAGONAL:
    double **x;                               // declaration
    matrix_alloc_symmetric_d(&x,n,1);         // allocation
    write(fh,x[0],sizeof(double)*n*(n+1)/2);  // sample block operation
    matrix_free_d(&x);                        // deallocation
CALLING PROGRAM USAGE WHEN EXCLUDING THE DIAGONAL:
    double **x;                               // declaration
    matrix_alloc_symmetric_d(&x,n,0);         // allocation
    write(fh,x[0],sizeof(double)*n*(n-1)/2);  // sample block operation
    matrix_free_d(&x);                        // deallocation
****************************************************************************/

short matrix_alloc_symmetric_d  // returns 1 if OK, 0 if allocation fails
(
    double ***x,                // pointer to 2-dimensional matrix (level 1)
    short n,                    // dimension of square symmetric matrix
    char diag                   // 1=include, 0=exclude diagonal
)
{
    short i;            // index to rows
    short j;            // index to columns
    short r;            // index for beginning of rows

    if (diag)           // INCLUDE THE DIAGONAL
    {
        // allocate pointers for 1st dimension (pointers to rows)
        *x=(double **)malloc(n*sizeof(double *));
        if (*x==NULL) return(0);
        
        // primary memory allocation
        (*x)[0]=(double *)malloc(sizeof(double)*n*(n+1)/2);
        if ((*x)[0]==NULL)
        {
            free(*x);
            return(0);
        }
        
        // pointer assignment to row beginnings
        for (r=0,i=0;i<n;i++)
        {
            (*x)[i]=&((*x)[0][r]);
            r+=i+1;
        }
        
        // initialize matrix with zeros
        for (i=0;i<n;i++) for (j=0;j<=i;j++) (*x)[i][j]=0.0;
    }
    else                // EXCLUDE THE DIAGONAL
    {
        // allocate pointers for 1st dimension (pointers to rows)
        *x=(double **)malloc(n*sizeof(double *));
        if (*x==NULL) return(0);
        
        // primary memory allocation
        (*x)[0]=(double *)malloc(sizeof(double)*n*(n-1)/2);
        if ((*x)[0]==NULL)
        {
            free(*x);
            return(0);
        }
        
        // pointer assignment to row beginnings
        for (r=0,i=1;i<n;i++)
        {
            (*x)[i]=&((*x)[0][r]);
            r+=i;
        }
        
        // initialize matrix with zeros (j must be less than i)
        for (i=1;i<n;i++) for (j=0;j<i;j++) (*x)[i][j]=0.0;
    }

    return(1);
}

/****************************************************************************
matrix_alloc_l()

Allocates memory for a long 2-dimensional matrix.

CALLING PROGRAM USAGE:
    long **x;                               // declaration
    matrix_alloc_l(&x,n1,n2);               // allocation
    write(fh,x[0],n1*n2*sizeof(long));      // sample block operation
    matrix_free_l(&x);                      // deallocation
****************************************************************************/

short matrix_alloc_l    // returns 1 if OK, 0 if allocation fails
(
    long ***x,          // pointer to 2-dimensional matrix (level 1)
    short n1,           // # for 1st dimension (rows)
    short n2            // # for 2nd dimension (columns)
)
{
    short i;            // index to 1st dimension
    short j;            // index to 2nd dimension

    // allocate pointers for 1st dimension (pointers to rows)
    *x=(long **)malloc(n1*sizeof(long *));
    if (*x==NULL)
    {
        // errmes("MATRIX_ALLOC_L: Unable to allocate memory for pointers.");
        return(0);
    }

    // primary memory allocation & pointer assignment
    (*x)[0]=(long *)malloc(n1*n2*sizeof(long));
    if ((*x)[0]==NULL)
    {
        // errmes("MATRIX_ALLOC_L: Unable to allocate primary memory.");
        free(*x);
        return(0);
    }
    for (i=0;i<n1;i++) (*x)[i]=&((*x)[0][i*n2]);

    // initialize matrix with zeros
    for (i=0;i<n1;i++) for (j=0;j<n2;j++) (*x)[i][j]=0;

    return(1);
}

/****************************************************************************
matrix_free_l()

Frees a long 2-dimensional matrix.
****************************************************************************/

void matrix_free_l
(
    long ***x          // pointer to 2-dimensional matrix (level 1);
)
{
    free((*x)[0]);     // free primary matrix
    free(*x);          // free pointers to rows
}

/****************************************************************************
matrix_alloc_s()

Allocates memory for a short 2-dimensional matrix.

CALLING PROGRAM USAGE:
    short **x;                              // declaration
    matrix_alloc_s(&x,n1,n2);               // allocation
    write(fh,x[0],n1*n2*sizeof(short));     // sample block operation
    matrix_free_s(&x);                      // deallocation
****************************************************************************/

short matrix_alloc_s    // returns 1 if OK, 0 if allocation fails
(
    short ***x,         // pointer to 2-dimensional matrix (level 1)
    short n1,           // # for 1st dimension (rows)
    short n2            // # for 2nd dimension (columns)
)
{
    short i;            // index to 1st dimension
    short j;            // index to 2nd dimension

    // allocate pointers for 1st dimension (pointers to rows)
    *x=(short **)malloc(n1*sizeof(short *));
    if (*x==NULL)
    {
        // errmes("MATRIX_ALLOC_S: Unable to allocate memory for pointers.");
        return(0);
    }

    // primary memory allocation & pointer assignment
    (*x)[0]=(short *)malloc(n1*n2*sizeof(short));
    if ((*x)[0]==NULL)
    {
        // errmes("MATRIX_ALLOC_S: Unable to allocate primary memory.");
        free(*x);
        return(0);
    }
    for (i=0;i<n1;i++) (*x)[i]=&((*x)[0][i*n2]);

    // initialize matrix with zeros
    for (i=0;i<n1;i++) for (j=0;j<n2;j++) (*x)[i][j]=0;

    return(1);
}

/****************************************************************************
matrix_free_s()

Frees a short 2-dimensional matrix
****************************************************************************/

void matrix_free_s
(
    short ***x          // pointer to 2-dimensional matrix (level 1);
)
{
    free((*x)[0]);     // free primary matrix
    free(*x);          // free pointers to rows
}


/****************************************************************************
matrix_alloc_c()

Allocates memory for a char 2-dimensional matrix.

CALLING PROGRAM USAGE:
    char **x;                               // declaration
    matrix_alloc_c(&x,n1,n2);               // allocation
    write(fh,x[0],n1*n2*sizeof(char));      // sample block operation
    matrix_free_c(&x);                      // deallocation
****************************************************************************/

short matrix_alloc_c    // returns 1 if OK, 0 if allocation fails
(
    char ***x,          // pointer to 2-dimensional matrix (level 1)
    short n1,           // # for 1st dimension (rows)
    short n2            // # for 2nd dimension (columns)
)
{
    short i;            // index to 1st dimension
    short j;            // index to 2nd dimension

    // allocate pointers for 1st dimension (pointers to rows)
    *x=(char **)malloc(n1*sizeof(char *));
    if (*x==NULL)
    {
        // errmes("MATRIX_ALLOC_C: Unable to allocate memory for pointers.");
        return(0);
    }

    // primary memory allocation & pointer assignment
    (*x)[0]=(char *)malloc(n1*n2*sizeof(char));
    if ((*x)[0]==NULL)
    {
        // errmes("MATRIX_ALLOC_C: Unable to allocate primary memory.");
        free(*x);
        return(0);
    }
    for (i=0;i<n1;i++) (*x)[i]=&((*x)[0][i*n2]);

    // initialize matrix with zeros
    for (i=0;i<n1;i++) for (j=0;j<n2;j++) (*x)[i][j]=0;

    return(1);
}

/****************************************************************************
matrix_free_c()

Frees a char 2-dimensional matrix
****************************************************************************/

void matrix_free_c
(
    char ***x           // pointer to 2-dimensional matrix (level 1);
)
{
    free((*x)[0]);     // free primary matrix
    free(*x);          // free pointers to rows
}


/****************************************************************************
matrix3_alloc()

Allocates memory for a floating point 3-dimensional matrix.

CALLING PROGRAM USAGE:
    float ***x;                                 // declaration
    matrix3_alloc(&x,n1,n2,n3);                 // allocation
    write(fh,x[0][0],n1*n2*n3*sizeof(float));   // sample block operation
    matrix3_free(&x);                           // deallocation
****************************************************************************/

short matrix3_alloc(    // returns 1 if OK, 0 if allocation fails
    float ****x,        // pointer to 3-dimensional matrix
    short n1,           // # for 1st dimension
    short n2,           // # for 2nd dimension
    short n3            // # for 3rd dimension
)
{
    short i;            // index for 1st dimension
    short j;            // index for 2nd dimension
    short k;            // index for 3rd dimension

    // allocate pointers for 1st dimension
    *x=(float ***)malloc(n1*sizeof(float **));
    if (*x==NULL)
    {
        // errmes("Error: Unable to allocate pointers. (matrix3_alloc:1)");
        return(0);
    }

    // allocate pointers for 2nd dimension
    (*x)[0]=(float **)malloc(n1*n2*sizeof(float *));
    if ((*x)[0]==NULL)
    {
        free(*x);
        // errmes("Error: Unable to allocate pointers. (matrix3_alloc:2)");
        return(0);
    }

    // assign pointers for 1st dimension
    for (i=0;i<n1;i++) (*x)[i]=&((*x)[0][i*n2]);

    // allocate primary memory for 3D matrix
    (*x)[0][0]=(float *)malloc(n1*n2*n3*sizeof(float));
    if ((*x)[0][0]==NULL)
    {
        free((*x)[0]);
        free(*x);
        // errmes("Error: Unable to allocate primary memory. (matrix3_alloc)");
        return(0);
    }

    // assign pointers to rows in each 2D submatrix
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            (*x)[i][j]=&((*x)[0][0][j*n3+i*(n2*n3)]);
        }
    }

    // initialize with zeros
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            for (k=0;k<n3;k++) (*x)[i][j][k]=0.0;
        }
    }

    return(1);
}

/****************************************************************************
matrix3_free()

Frees a floating point 3-dimensional matrix.
****************************************************************************/
void matrix3_free
(
    float ****x         // pointer to 3-dimensional matrix
)
{
    free((*x)[0][0]);   // free matrix at primary level (level 0)
    free((*x)[0]);      // free matrix at level 1
    free(*x);           // free matrix at level 2
}

/****************************************************************************
matrix3_alloc_symmetric()

Allocates memory for a floating point 3 dimensional matrix such that the 
first two indices are symmetric.

CALLING PROGRAM USAGE WHEN INCLUDING THE DIAGONAL:
    float ***x;                                 // declaration
    long block_size=sizeof(float)*n3*n*(n+1)/2; // size of a block
    matrix3_alloc_symmetric(&x,n,n3,1);         // allocation
    write(fh,x[0][0],block_size);               // sample block operation
    matrix3_free(&x);                           // deallocation
CALLING PROGRAM USAGE WHEN EXCLUDING THE DIAGONAL:
    float ***x;                                 // declaration
    long block_size=sizeof(float)*n3*n*(n-1)/2; // size of a block
    matrix3_alloc_symmetric(&x,n,n3,0);         // allocation
    write(fh,x[0][0],block_size);               // sample block operation
    matrix3_free(&x);                           // deallocation
****************************************************************************/

short matrix3_alloc_symmetric   // returns 1 if OK, 0 if allocation fails
(
    float ****x,                // pointer to 3-dimensional matrix
    short n,                    // dimension of square symmetric indices
    short n3,                   // # in 3rd dimension
    char diag                   // 1=include, 0=exclude the diagonal
)
{
    short i;            // index to rows
    short j;            // index to columns
    short k;            // index to 3rd dimension
    short r;            // index for beginning of rows

    if (diag)           // INCLUDE THE DIAGONAL
    {
        // allocate pointers for 1st dimension
        *x=(float ***)malloc(n*sizeof(float **));
        if (*x==NULL) return(0);
        
        // allocate pointers for 2nd dimension
        (*x)[0]=(float **)malloc(sizeof(float *)*n*(n+1)/2);
        if ((*x)[0]==NULL)
        {
            free(*x);
            return(0);
        }
        
        // assign pointers for 1st dimension
        for (r=0,i=0;i<n;i++)
        {
            (*x)[i]=&((*x)[0][r]);
            r+=i+1;
        }

        // allocate primary memory for 3D matrix
        (*x)[0][0]=(float *)malloc(sizeof(float)*n3*n*(n+1)/2);
        if ((*x)[0][0]==NULL)
        {
            free((*x)[0]);
            free(*x);
            return(0);
        }

        // assign pointers to rows in each 2D submatrix
        for (i=0;i<n;i++)
        {
            for (j=0;j<=i;j++)
            {
                (*x)[i][j]=&((*x)[0][0][n3*(j+i*(i+1)/2)]);
            }
        }
        
        // initialize matrix with zeros
        for (i=0;i<n;i++)
        {
            for (j=0;j<=i;j++)
            {
                for (k=0;k<n3;k++) (*x)[i][j][k]=0.0;
            }
        }
    }
    else                // EXCLUDE THE DIAGONAL
    {
        // allocate pointers for 1st dimension (pointers to rows)
        *x=(float ***)malloc(n*sizeof(float **));
        if (*x==NULL) return(0);
        
        // allocate pointers for 2nd dimension
        (*x)[0]=(float **)malloc(sizeof(float *)*n*(n-1)/2);
        if ((*x)[0]==NULL)
        {
            free(*x);
            return(0);
        }
        
        // assign pointers for 1st dimension
        for (r=0,i=1;i<n;i++)
        {
            (*x)[i]=&((*x)[0][r]);
            r+=i;
        }

        // allocate primary memory for 3D matrix
        (*x)[0][0]=(float *)malloc(sizeof(float)*n3*n*(n-1)/2);
        if ((*x)[0][0]==NULL)
        {
            free((*x)[0]);
            free(*x);
            return(0);
        }

        // assign pointers to rows in each 2D submatrix
        for (i=1;i<n;i++)
        {
            for (j=0;j<i;j++)
            {
                (*x)[i][j]=&((*x)[0][0][n3*(j+i*(i-1)/2)]);
            }
        }
        
        // initialize matrix with zeros
        for (i=1;i<n;i++)
        {
            for (j=0;j<i;j++)
            {
                for (k=0;k<n3;k++) (*x)[i][j][k]=0.0;
            }
        }
    }

    return(1);
}

/****************************************************************************
matrix3_alloc_d()

Allocates memory for a double 3-dimensional matrix.

CALLING PROGRAM USAGE:
    double ***x;                                // declaration
    matrix3_alloc_d(&x,n1,n2,n3);               // allocation
    write(fh,x[0][0],n1*n2*n3*sizeof(double));  // sample block operation
    matrix3_free_d(&x);                         // deallocation
****************************************************************************/

short matrix3_alloc_d(      // returns 1 if OK, 0 if allocation fails
    double ****x,           // pointer to 3-dimensional matrix
    short n1,               // # for 1st dimension
    short n2,               // # for 2nd dimension
    short n3                // # for 3rd dimension
)
{
    short i;            // index for 1st dimension
    short j;            // index for 2nd dimension
    short k;            // index for 3rd dimension

    // allocate pointers for 1st dimension
    *x=(double ***)malloc(n1*sizeof(double **));
    if (*x==NULL)
    {
        // errmes("Error: Unable to allocate pointers. (matrix3_alloc_d:1)");
        return(0);
    }

    // allocate pointers for 2nd dimension
    (*x)[0]=(double **)malloc(n1*n2*sizeof(double *));
    if ((*x)[0]==NULL)
    {
        free(*x);
        // errmes("Error: Unable to allocate pointers. (matrix3_alloc_d:2)");
        return(0);
    }

    // assign pointers for 1st dimension
    for (i=0;i<n1;i++) (*x)[i]=&((*x)[0][i*n2]);

    // allocate primary memory for 3D matrix
    (*x)[0][0]=(double *)malloc(n1*n2*n3*sizeof(double));
    if ((*x)[0][0]==NULL)
    {
        free((*x)[0]);
        free(*x);
        // errmes("Error: Unable to allocate primary memory (matrix3_alloc_d)");
        return(0);
    }

    // assign pointers to rows in each 2D submatrix
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            (*x)[i][j]=&((*x)[0][0][j*n3+i*(n2*n3)]);
        }
    }

    // initialize with zeros
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            for (k=0;k<n3;k++) (*x)[i][j][k]=0.0;
        }
    }

    return(1);
}

/****************************************************************************
matrix3_free_d

Frees a double 3-dimensional matrix.
****************************************************************************/

void matrix3_free_d
(
    double ****x        // pointer to 3-dimensional matrix
)
{
    free((*x)[0][0]);   // free matrix at primary level (level 0)
    free((*x)[0]);      // free matrix at level 1
    free(*x);           // free matrix at level 2
}

/****************************************************************************
matrix4_alloc()

Allocates memory for a floating point 4-dimensional matrix.

CALLING PROGRAM USAGE:
    float ****x;                                    // declaration
    matrix4_alloc(&x,n1,n2,n3,n4);                  // allocation
    write(fh,x[0][0][0],n1*n2*n3*n4*sizeof(float)); // sample block operation
    matrix4_free(&x);                               // deallocation
****************************************************************************/

short matrix4_alloc(    // returns 1 if OK, 0 if allocation fails
    float *****x,       // pointer to 4-dimensional matrix
    short n1,           // # for 1st dimension
    short n2,           // # for 2nd dimension
    short n3,           // # for 3rd dimension
    short n4            // # for 4th dimension
)
{
    short i;            // index for 1st dimension
    short j;            // index for 2nd dimension
    short k;            // index for 3rd dimension
    short l;            // index for 4th dimension

    // allocate pointers for 1st dimension
    *x=(float ****)malloc(n1*sizeof(float ***));
    if (*x==NULL)
    {
        // errmes("Error: Unable to allocate pointers. (matrix4_alloc:1)");
        return(0);
    }

    // allocate pointers for 2nd dimension
    (*x)[0]=(float ***)malloc(n1*n2*sizeof(float **));
    if ((*x)[0]==NULL)
    {
        free(*x);
        // errmes("Error: Unable to allocate pointers. (matrix4_alloc:2)");
        return(0);
    }

    // assign pointers for 1st dimension
    for (i=0;i<n1;i++) (*x)[i]=&((*x)[0][i*n2]);

    // allocate pointers for 3rd dimension
    (*x)[0][0]=(float **)malloc(n1*n2*n3*sizeof(float *));
    if ((*x)[0][0]==NULL)
    {
        free((*x)[0]);
        free(*x);
        // errmes("Error: Unable to allocate pointers. (matrix4_alloc:3)");
        return(0);
    }

    // assign pointers for 2nd dimension
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            (*x)[i][j]=&((*x)[0][0][j*n3+i*(n2*n3)]);
        }
    }

    // allocate primary memory
    (*x)[0][0][0]=(float *)malloc(n1*n2*n3*n4*sizeof(float));
    if ((*x)[0][0][0]==NULL)
    {
        free((*x)[0][0]);
        free((*x)[0]);
        free(*x);
        // errmes("Unable to allocate primary memory. (matrix4_alloc)");
        return(0);
    }

    // assign pointers for 3rd dimension to primary memory
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            for (k=0;k<n3;k++)
            {
                (*x)[i][j][k]=&((*x)[0][0][0][k*n4+j*(n3*n4)+i*(n2*n3*n4)]);
            }
        }
    }

    // initialize with zeros
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            for (k=0;k<n3;k++)
            {
                for (l=0;l<n4;l++) (*x)[i][j][k][l]=0.0;
            }
        }
    }

    return(1);
}

/****************************************************************************
matrix4_free()

Frees a floating point 4-dimensional matrix.
****************************************************************************/

void matrix4_free
(
    float *****x            // pointer to 4-dimensional matrix
)
{
    free((*x)[0][0][0]);    // free primary memory
    free((*x)[0][0]);       // free level 1
    free((*x)[0]);          // free level 2
    free(*x);               // free level 3
}

/****************************************************************************
matrix4_alloc_d()

Allocate memory for a double 4-dimensional matrix.

CALLING PROGRAM USAGE:
    double ****x;                                    // declaration
    matrix4_alloc_d(&x,n1,n2,n3,n4);                 // allocation
    write(fh,x[0][0][0],n1*n2*n3*n4*sizeof(double)); // a block operation
    matrix4_free(&x);                                // deallocation
****************************************************************************/

short matrix4_alloc_d(      // returns 1 if OK, 0 if allocation fails
    double *****x,          // pointer to 4-dimensional matrix
    short n1,               // # for 1st dimension
    short n2,               // # for 2nd dimension
    short n3,               // # for 3rd dimension
    short n4                // # for 4th dimension
)
{
    short i;            // index for 1st dimension
    short j;            // index for 2nd dimension
    short k;            // index for 3rd dimension
    short l;            // index for 4th dimension

    // allocate pointers for 1st dimension
    *x=(double ****)malloc(n1*sizeof(double ***));
    if (*x==NULL)
    {
        // errmes("Unable to allocate pointers. (matrix4_alloc_d:1)");
        return(0);
    }

    // allocate pointers for 2nd dimension
    (*x)[0]=(double ***)malloc(n1*n2*sizeof(double **));
    if ((*x)[0]==NULL)
    {
        free(*x);
        // errmes("Unable to allocate pointers. (matrix4_alloc_d:2)");
        return(0);
    }

    // assign pointers for 1st dimension
    for (i=0;i<n1;i++) (*x)[i]=&((*x)[0][i*n2]);

    // allocate pointers for 3rd dimension
    (*x)[0][0]=(double **)malloc(n1*n2*n3*sizeof(double *));
    if ((*x)[0][0]==NULL)
    {
        free((*x)[0]);
        free(*x);
        // errmes("Unable to allocate pointers. (matrix4_alloc_d:3)");
        return(0);
    }

    // assign pointers for 2nd dimension
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            (*x)[i][j]=&((*x)[0][0][j*n3+i*(n2*n3)]);
        }
    }

    // allocate primary memory
    (*x)[0][0][0]=(double *)malloc(n1*n2*n3*n4*sizeof(double));
    if ((*x)[0][0][0]==NULL)
    {
        free((*x)[0][0]);
        free((*x)[0]);
        free(*x);
        // errmes("Unable to allocate primary memory. (matrix4_alloc_d)");
        return(0);
    }

    // assign pointers for 3rd dimension to primary memory
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            for (k=0;k<n3;k++)
            {
                (*x)[i][j][k]=&((*x)[0][0][0][k*n4+j*(n3*n4)+i*(n2*n3*n4)]);
            }
        }
    }

    // initialize with zeros
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            for (k=0;k<n3;k++)
            {
                for (l=0;l<n4;l++) (*x)[i][j][k][l]=0.0;
            }
        }
    }

    return(1);
}

/****************************************************************************
matrix4_free_d()

Frees a double 4-dimensional matrix.
****************************************************************************/

void matrix4_free_d
(
    double *****x           // pointer to 4-dimensional matrix
)
{
    free((*x)[0][0][0]);    // free primary memory
    free((*x)[0][0]);       // free level 1
    free((*x)[0]);          // free level 2
    free(*x);               // free level 3
}

/****************************************************************************
matrix5_alloc_d()

Allocates memory for a double 5-dimensional matrix.

CALLING PROGRAM USAGE:
    double *****x;                                   // declaration
    matrix5_alloc_d(&x,n1,n2,n3,n4,n5);              // allocation
    write(fh,x[0][0][0][0],n1*n2*n3*n4*n5*sizeof(double)); // block op
    matrix5_free(&x);                                // deallocation
****************************************************************************/

short matrix5_alloc_d(      // returns 1 if OK, 0 if allocation fails
    double ******x,         // pointer to 5-dimensional matrix
    short n1,               // # for 1st dimension
    short n2,               // # for 2nd dimension
    short n3,               // # for 3rd dimension
    short n4,               // # for 4th dimension
    short n5                // # for 5th dimension
)
{
    short i;            // index for 1st dimension
    short j;            // index for 2nd dimension
    short k;            // index for 3rd dimension
    short l;            // index for 4th dimension
    short m;            // index for 5th dimension

    // allocate pointers for 1st dimension
    *x=(double *****)malloc(n1*sizeof(double ****));
    if (*x==NULL)
    {
        // errmes("Unable to allocate pointers. (matrix5_alloc_d:1)");
        return(0);
    }

    // allocate pointers for 2nd dimension
    (*x)[0]=(double ****)malloc(n1*n2*sizeof(double ***));
    if ((*x)[0]==NULL)
    {
        free(*x);
        // errmes("Unable to allocate pointers. (matrix5_alloc_d:2)");
        return(0);
    }

    // assign pointers for 1st dimension
    for (i=0;i<n1;i++) (*x)[i]=&((*x)[0][i*n2]);

    // allocate pointers for 3rd dimension
    (*x)[0][0]=(double ***)malloc(n1*n2*n3*sizeof(double **));
    if ((*x)[0][0]==NULL)
    {
        free((*x)[0]);
        free(*x);
        // errmes("Unable to allocate pointers. (matrix5_alloc_d:3)");
        return(0);
    }

    // assign pointers for 2nd dimension
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            (*x)[i][j]=&((*x)[0][0][j*n3+i*(n2*n3)]);
        }
    }

    // allocate pointers for 4th dimension
    (*x)[0][0][0]=(double **)malloc(n1*n2*n3*n4*sizeof(double *));
    if ((*x)[0][0][0]==NULL)
    {
        free((*x)[0][0]);
        free((*x)[0]);
        free(*x);
        // errmes("Unable to allocate pointers. (matrix5_alloc_d:4)");
        return(0);
    }

    // assign pointers for 3rd dimension
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            for (k=0;k<n3;k++)
            {
                (*x)[i][j][k]=&((*x)[0][0][0][k*n4+j*(n3*n4)+i*(n2*n3*n4)]);
            }
        }
    }

    // allocate primary memory
    (*x)[0][0][0][0]=(double *)malloc(n1*n2*n3*n4*n5*sizeof(double));
    if ((*x)[0][0][0][0]==NULL)
    {
        free((*x)[0][0][0]);
        free((*x)[0][0]);
        free((*x)[0]);
        free(*x);
        // errmes("Unable to allocate primary memory. (matrix5_alloc_d)");
        return(0);
    }

    // assign pointers for 4th dimension to primary memory
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            for (k=0;k<n3;k++)
            {
                for (l=0;l<n4;l++)
                {
                    (*x)[i][j][k][l]=
                        &(
                            (*x)[0][0][0][0]
                            [
                                l*n5 +
                                k*(n4*n5) + 
                                j*(n3*n4*n5) +
                                i*(n2*n3*n4*n5)
                            ]
                        );
                }
            }
        }
    }

    // initialize with zeros
    for (i=0;i<n1;i++)
    {
        for (j=0;j<n2;j++)
        {
            for (k=0;k<n3;k++)
            {
                for (l=0;l<n4;l++)
                {
                    for (m=0;m<n5;m++) (*x)[i][j][k][l][m]=0.0;
                }
            }
        }
    }

    return(1);
}

/****************************************************************************
matrix5_free_d()

Frees a double 5-dimensional matrix.
****************************************************************************/

void matrix5_free_d
(
    double ******x              // pointer to 5-dimensional matrix
)
{
    free((*x)[0][0][0][0]);     // free primary memory
    free((*x)[0][0][0]);        // free level 1
    free((*x)[0][0]);           // free level 2
    free((*x)[0]);              // free level 3
    free(*x);                   // free level 4
}

