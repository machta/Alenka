#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([biosig], [1.9.0])
AC_CONFIG_SRCDIR([biosig.c])
AC_CONFIG_HEADERS([config.h])

AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([asound],  [snd_pcm_open])
AC_CHECK_LIB([b64],     [base64_encode_block], AC_SUBST(HAVE_LIBB64,     "1") )
AC_CHECK_LIB([cholmod], [cholmod_start],       AC_SUBST(HAVE_LIBCHOLMOD, "1") )
AC_CHECK_LIB([hdf5],    [H5open],              AC_SUBST(HAVE_LIBHDF5,    "1") )
AC_CHECK_LIB([matio],   [Mat_Open],            AC_SUBST(HAVE_LIBMATIO,   "1") )
AC_CHECK_LIB([z],       [gzopen],              AC_SUBST(HAVE_LIBZ,       "1") )

# FIXME: Replace `main' with a function in `-ldcmdata':
#AC_CHECK_LIB([dcmdata], [main])
# FIXME: Replace `main' with a function in `-lgdcmCommon':
#AC_CHECK_LIB([gdcmCommon], [main])
# FIXME: Replace `main' with a function in `-lgdcmDICT':
#AC_CHECK_LIB([gdcmDICT], [main])
# FIXME: Replace `main' with a function in `-lgdcmDSED':
#AC_CHECK_LIB([gdcmDSED], [main])
# FIXME: Replace `main' with a function in `-lgdcmIOD':
#AC_CHECK_LIB([gdcmIOD], [main])
# FIXME: Replace `main' with a function in `-lgdcmMEXD':
#AC_CHECK_LIB([gdcmMEXD], [main])
# FIXME: Replace `main' with a function in `-lgdcmMSFF':
#AC_CHECK_LIB([gdcmMSFF], [main])

AC_CHECK_LIB([iberty], [getlogin])
AC_CHECK_LIB([iconv], [iconv_open])
AC_CHECK_HEADER([iconv.h], ,[AC_MSG_ERROR([can not find iconv.h])])

# FIXME: Replace `main' with a function in `-llibmex':
AC_CHECK_LIB([libmex], [main])
# FIXME: Replace `main' with a function in `-llibmx':
AC_CHECK_LIB([libmx], [main])

AC_CHECK_LIB([m],       [round],               AC_SUBST(HAVE_LIBM,       "1") )

# Wolfram/Mathematica Mathlink interface
AC_CHECK_LIB([mathlink], [MLPutRealArray])
# FIXME: Replace `main' with a function in `-luuid':
AC_CHECK_LIB([uuid], [main])

#AC_CHECK_LIB([pthread], [pthread_mutex_lock])
# FIXME: Replace `main' with a function in `-lssp':
#AC_CHECK_LIB([ssp], [main])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h inttypes.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/param.h sys/socket.h sys/time.h syslog.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STRNLEN
AC_FUNC_STRTOD
AC_CHECK_FUNCS([alarm atexit bzero dup2 floor getcwd getdelim gethostbyname gethostname gettimeofday localtime_r memchr memmove memset mkdir modf pow putenv rmdir select setenv setlocale socket strcasecmp strchr strcspn strdup strerror strncasecmp strndup strpbrk strrchr strspn strstr strtol strtoul strtoumax tzset])

AC_CONFIG_FILES([Makefile
                 R/Makefile
                 examples/Makefile
                 igor/Makefile
                 java/Makefile
                 mex/Makefile
                 mma/Makefile
                 perl/Makefile
                 php/Makefile
                 python/Makefile
                 ruby/Makefile
                 t240/Makefile
                 t241/Makefile
                 tcl/Makefile])

AC_OUTPUT
