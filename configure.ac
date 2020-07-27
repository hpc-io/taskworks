# Copyright (C) 2020, Lawrence Berkeley National Laboratory.                
# All rights reserved.                                                      
#                                                                           
# This file is part of Taskworks. The full Taskworks copyright notice,      
# including terms governing use, modification, and redistribution, is       
# contained in the file COPYING at the root of the source code distribution 
# tree.      

AC_PREREQ(2.59)
AC_INIT([taskworks], [0.1], [])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/include/taskworks.h])
AC_CONFIG_HEADER([src/include/config.h])

# Init automake
AM_INIT_AUTOMAKE
AM_SILENT_RULES

AC_DEFUN([UD_MSG_DEBUG],
				 [if test "x${te_ac_debug}" = xyes ; then
						 AC_MSG_NOTICE([DEBUG: $1])
					fi
				 ]
)

# Set the prefix to an taskworks directory in the source root.
# AC_PREFIX_DEFAULT([`pwd`/taskworks])

AH_TEMPLATE([ENABLE_PARALLEL], [Enable MPI support.])
AH_TEMPLATE([ENABLE_DEBUG], [Debug build.])
AH_TEMPLATE([ENABLE_ABT], [Enable Argobots integration.])
AH_TEMPLATE([ENABLE_LIBEVENT], [Enable libevent integration.])

# Make tests
AM_EXTRA_RECURSIVE_TARGETS([tests])

## ----------------------------------------------------------------------
## Source any special files that we need.  These files normally aren't
## present but can be used by the maintainers to fine tune things like
## turning on debug or profiling flags for the compiler.  The search order
## is:
##
##    CPU-VENDOR-OS
##    VENDOR-OS
##    CPU-OS
##    CPU-VENDOR
##    OS
##    VENDOR
##    CPU
##
## If the `OS' ends with a version number then remove it. For instance,
## `freebsd3.1' would become `freebsd'
AC_CANONICAL_HOST

case $host_os in
  aix*)
    host_os_novers=aix
    ;;
  freebsd*)
    host_os_novers=freebsd
    ;;
  netbsd*)
    host_os_novers=netbsd
    ;;
  solaris*)
    host_os_novers=solaris
    ;;
  *)
    host_os_novers=$host_os
    ;;
esac

host_config="none"
for f in $host_cpu-$host_vendor-$host_os \
         $host_cpu-$host_vendor-$host_os_novers \
         $host_vendor-$host_os \
         $host_vendor-$host_os_novers \
         $host_cpu-$host_os \
         $host_cpu-$host_os_novers \
         $host_cpu-$host_vendor \
         $host_os \
         $host_os_novers \
         $host_vendor \
         $host_cpu ; do
  AC_MSG_CHECKING([for config $f])
  if test -f "$srcdir/hdf5_config_scripts/$f"; then
    host_config=$srcdir/hdf5_config_scripts/$f
    AC_MSG_RESULT([found])
    break
  fi
  AC_MSG_RESULT([no])
done
if test "X$host_config" != "Xnone"; then
  CC_BASENAME="`echo $CC | cut -f1 -d' ' | xargs basename 2>/dev/null`"
  . $host_config
fi

## Source any special site-specific file
hname="`hostname`"
while test -n "$hname"; do
  file=$srcdir/hdf5_config_scripts/site-specific/host-$hname
  AC_MSG_CHECKING([for config $file])
  if test -f "$file"; then
    . $file
    AC_MSG_RESULT([found])
    break
  fi
  AC_MSG_RESULT([no])
  hname_tmp=$hname
  hname="`echo $hname | cut -d. -f2-99`"
  test "$hname_tmp" = "$hname" && break
done

# Whether to enable parallel build with mpi
AC_ARG_ENABLE([parallel], [AS_HELP_STRING([--enable-parallel], [Search for MPI-IO and MPI support files])],[enable_parallel=${enableval}], [enable_parallel=no])

# Search for mpi compiler
if test "x$enable_parallel" = "xyes"; then
	AC_ARG_VAR(MPICC,  [MPI C compiler, @<:@default: CC@:>@])
	AC_ARG_VAR(MPICXX, [MPI C++ compiler, @<:@default: CXX@:>@])

	ac_user_MPICC=$MPICC
	ac_user_MPICXX=$MPICXX
	if test "x$MPICC"  = x && test "x$CC"  != x ; then ac_user_MPICC=$CC   ; fi
	if test "x$MPICXX" = x && test "x$CXX" != x ; then ac_user_MPICXX=$CXX ; fi

	CANDIDATE_MPICC="${MPICC} mpicc mpicc_r"
	CANDIDATE_MPICXX="${MPICXX} mpicxx mpic++ mpiCC mpcxx mpc++ mpicxx_r mpiCC_r mpcxx_r mpic++_r mpc++_r"
	dnl add GNU MPI compilers
	CANDIDATE_MPICC="$CANDIDATE_MPICC mpigcc mpgcc mpigcc_r mpgcc_r"
	CANDIDATE_MPICXX="$CANDIDATE_MPICXX mpig++ mpg++ mpig++_r mpg++_r"
	dnl add IBM MPI compilers
	CANDIDATE_MPICC="$CANDIDATE_MPICC mpcc_r mpcc mpixlc_r mpixlc"
	CANDIDATE_MPICXX="$CANDIDATE_MPICXX mpCC_r mpCC mpixlcxx_r mpixlcxx mpixlC_r mpixlC"
	dnl add IBM BGL MPI compilers
	CANDIDATE_MPICC="$CANDIDATE_MPICC blrts_xlc mpxlc_r mpxlc"
	CANDIDATE_MPICXX="$CANDIDATE_MPICXX blrts_xlC mpxlC_r mpxlC mpixlc++ mpxlcxx mpxlc++ mpxlCC mpixlc++_r mpxlcxx_r mpxlc++_r mpxlCC_r"
	dnl add Fujitsu MPI compilers
	CANDIDATE_MPICC="$CANDIDATE_MPICC mpifccpx"
	CANDIDATE_MPICXX="$CANDIDATE_MPICXX mpiFCCpx"
	dnl add Cray MPI compiler wrappers
	CANDIDATE_MPICC="$CANDIDATE_MPICC cc"
	CANDIDATE_MPICXX="$CANDIDATE_MPICXX CC"
	dnl add Intel MPI compiler wrappers
	CANDIDATE_MPICC="$CANDIDATE_MPICC mpiicc icc"
	CANDIDATE_MPICXX="$CANDIDATE_MPICXX mpiicpc mpiicxx mpiic++ mpiiCC icpc"
	dnl add PGI MPI compiler wrappers
	CANDIDATE_MPICC="$CANDIDATE_MPICC mpipgcc mppgcc"
	CANDIDATE_MPICXX="$CANDIDATE_MPICXX mpipgCC mppgCC"

	dnl find the full path of MPICC from CANDIDATE_MPICC and MPI_INSTALL
	if test "x${ac_user_MPICC}" = x ; then
		dnl if MPICC or CC has not been set by users, then search from
		dnl CANDIDATE_MPICC, and find the full path of MPICC
		UD_MPI_PATH_PROGS([MPICC], [$CANDIDATE_MPICC])
	else
		dnl check whether user specified MPICC is valid
		UD_MPI_PATH_PROG([MPICC], [$ac_user_MPICC])
	fi

	if test "x${MPICC}" = x ; then
		if test "x$ac_user_MPICC" = x ; then
			ERR_MSG="No MPI C compiler can be found"
		else
			ERR_MSG="Specified MPI C compiler \"$ac_user_MPICC\" cannot be found"
		fi
		if test "x$MPI_INSTALL" != x ; then
			ERR_MSG="$ERR_MSG under $MPI_INSTALL"
		fi
		AC_MSG_ERROR([
		-----------------------------------------------------------------------
			$ERR_MSG
			--enable-parallel is set, but no working MPI C compiler.
			Abort.
		-----------------------------------------------------------------------])
	fi
	CC=${MPICC}
fi

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Default to building static library, because opa by default builds itself
# statically without -fPIC, preventing it from working with a shared library.
AC_DISABLE_SHARED

# Initialize libtool.
LT_INIT

# Check if we can compile MPI program
ENABLE_PARALLEL=no
if test "x$enable_parallel" = "xyes"; then
	echo "enable_parallel=$enable_parallel"
	AC_MSG_CHECKING([whether a simple MPI-IO C program can be linked])
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <mpi.h>]],
									 [[ MPI_Init(0, (void *)0);
						MPI_File_open(0, (void *)0, 0, 0, (void *)0);]])],
				[AC_MSG_RESULT([yes])],
						[AC_MSG_RESULT([no])
						AC_MSG_ERROR([unable to link a simple MPI-IO C program])])


	# Require MPI 3.x
	AC_LINK_IFELSE(
			[AC_LANG_PROGRAM(
					[[
							#include <mpi.h>
					]],
					[[
							MPI_Message message;
							MPI_Init(0, (void *) 0);
							MPI_Mprobe(0, 0, 0, &message, (void *) 0);
							MPI_Imrecv((void *) 0, 0, 0, (void *) 0, (void *) 0);
					]]
			)],
			[AC_MSG_RESULT([yes])
			 PARALLEL_FILTERED_WRITES=yes],
			[AC_MSG_RESULT([no])
			 AC_MSG_WARN([A simple MPI program using the MPI_Mprobe and MPI_Imrecv functions could not be compiled and linked.
										Parallel writes of filtered data will be disabled.])])

	# We are building a parallel library
	ENABLE_PARALLEL=yes
	AC_DEFINE(ENABLE_PARALLEL, 1)
fi
AM_CONDITIONAL(ENABLE_PARALLEL, [test x$enable_parallel = xyes])

# ----------------------------------------------------------------------
# Check headers
AC_CHECK_HEADERS([pthread.h], [], [
		echo "Required headers missing!"
		exit 1
		])

# ----------------------------------------------------------------------
# Check for pthread
AC_CHECK_LIB(pthread, pthread_create, [], [
		echo "pthread not found!"
		exit 1
		])

# ----------------------------------------------------------------------

# Check for opa
AC_ARG_WITH([opa],
						[AC_HELP_STRING([--with-opa=DIR],
														[Use opa library [default=yes]])],,
						[withval=yes])

case "$withval" in
	yes)
		HAVE_OPA="yes"
		AC_CHECK_HEADERS([opa_primitives.h opa_queue.h],, [unset HAVE_OPA])
		AC_CHECK_LIB([opa], [OPA_Interprocess_lock_init],, [unset HAVE_OPA])

		if test -z "$HAVE_OPA"; then
			AC_MSG_ERROR([could not find opa library])
		fi
		;;
	no)
		AC_MSG_ERROR([opa library required to build AXE])
		;;
	*)
		HAVE_OPA="yes"
		case "$withval" in
			*,*)
				opa_inc="`echo $withval | cut -f1 -d,`"
				opa_lib="`echo $withval | cut -f2 -d, -s`"
				;;
			*)
				if test -n "$withval"; then
					opa_inc="$withval/include"
					opa_lib="$withval/lib"
				fi
				;;
		esac

		## Trying to include -I/usr/include and -L/usr/lib is redundant and
		## can mess some compilers up.
		if test "X$opa_inc" = "X/usr/include"; then
			opa_inc=""
		fi
		if test "X$opa_lib" = "X/usr/lib"; then
			opa_lib=""
		fi

		if test -n "$opa_inc"; then
			CPPFLAGS="$CPPFLAGS -I$opa_inc"
		fi

		AC_CHECK_HEADERS([opa_primitives.h opa_queue.h],, [unset HAVE_OPA])

		if test -n "$opa_lib"; then
			LDFLAGS="$LDFLAGS -L$opa_lib"
		fi

		AC_CHECK_LIB([opa], [OPA_Interprocess_lock_init],, [unset HAVE_OPA])

		if test -z "$HAVE_OPA"; then
			AC_MSG_ERROR([could not find opa library])
		fi
		;;
esac

# ----------------------------------------------------------------------
# Check for presence of sched_yield.  If not present, check for pthread_yield.
AC_CHECK_HEADER(sched.h,
		[AC_CHECK_FUNCS(sched_yield,
				,
				[AC_CHECK_FUNCS(pthread_yield)]
		)],
		[AC_CHECK_FUNCS(pthread_yield)]
)

# ----------------------------------------------------------------------
# Whether to enable debug build
AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug], [Search for MPI-IO and MPI support files])],[enable_debug=${enableval}], [enable_debug=no])
if test "x$enable_debug" = "xyes"; then
   CFLAGS=`echo $CFLAGS | ${SED} 's/-O. *//g' | ${SED} 's/-fast *//g'`
   CFLAGS+=" -O0 -g"
   AC_DEFINE(ENABLE_DEBUG, 1)
fi
# ----------------------------------------------------------------------

# Whether to enable argobots
enable_abt=no
AC_ARG_WITH([argobots],
						[AC_HELP_STRING([--with-argobots=DIR],
														[Enable argobots backend [default=yes]])],[enable_abt=yes],
						[enable_abt=no])
if test "x$enable_abt" = "xyes"; then
	case "$withval" in
		yes)
			;;
		no)
			enable_abt=no
			;;
		*)
			HAVE_ABT="yes"
			case "$withval" in
				*,*)
					abt_inc="`echo $withval | cut -f1 -d,`"
					abt_lib="`echo $withval | cut -f2 -d, -s`"
					;;
				*)
					if test -n "$withval"; then
						abt_inc="$withval/include"
						abt_lib="$withval/lib"
					fi
					;;
			esac

			## Trying to include -I/usr/include and -L/usr/lib is redundant and
			## can mess some compilers up.
			if test "X$abt_inc" = "X/usr/include"; then
				abt_inc=""
			fi
			if test "X$abt_lib" = "X/usr/lib"; then
				abt_lib=""
			fi

			if test -n "$abt_inc"; then
				CPPFLAGS="$CPPFLAGS -I$abt_inc"
			fi

			AC_CHECK_HEADERS([abt.h],, [unset ENABLE_ABT])

			if test -n "$abt_lib"; then
				LDFLAGS="$LDFLAGS -L$abt_lib"
			fi

			AC_CHECK_LIB([abt], [ABT_init],, [unset ENABLE_ABT])

			if test -z "$HAVE_ABT"; then
				AC_MSG_ERROR([could not find opa library])
			fi
			;;
	esac
fi
if test "x$enable_abt" = "xyes"; then
	ENABLE_ABT="yes"

	AC_CHECK_HEADERS([abt.h],, [unset ENABLE_ABT])
	AC_CHECK_LIB([abt], [ABT_init],, [unset ENABLE_ABT])

	if test -z "$ENABLE_ABT"; then
		AC_MSG_ERROR([could not find argobots library])
	fi

	AC_DEFINE(ENABLE_ABT, 1)
fi
AM_CONDITIONAL(ENABLE_ABT, [test x$enable_abt = xyes])

# Whether to enable argobots
enable_libevent=no
AC_ARG_WITH([libevent],
						[AC_HELP_STRING([--with-libevent=DIR],
														[Enable libevent event backend [default=yes]])],[enable_libevent=yes],
						[enable_libevent=no])
if test "x$enable_libevent" = "xyes"; then
	case "$withval" in
		yes)
			;;
		no)
			enable_libevent=no
			;;
		*)
			HAVE_LIBEVENT="yes"
			case "$withval" in
				*,*)
					libevent_inc="`echo $withval | cut -f1 -d,`"
					libevent_lib="`echo $withval | cut -f2 -d, -s`"
					;;
				*)
					if test -n "$withval"; then
						libevent_inc="$withval/include"
						libevent_lib="$withval/lib"
					fi
					;;
			esac

			## Trying to include -I/usr/include and -L/usr/lib is redundant and
			## can mess some compilers up.
			if test "X$libevent_inc" = "X/usr/include"; then
				libevent_inc=""
			fi
			if test "X$libevent_lib" = "X/usr/lib"; then
				libevent_lib=""
			fi

			if test -n "$libevent_inc"; then
				CPPFLAGS="$CPPFLAGS -I$libevent_inc"
			fi

			AC_CHECK_HEADERS([event2/event.h event.h],, [unset HAVE_LIBEVENT])

			if test -n "$libevent_lib"; then
				LDFLAGS="$LDFLAGS -L$libevent_lib"
			fi

			AC_CHECK_LIB([event], [event_init],, [unset HAVE_LIBEVENT])

			if test -z "$HAVE_LIBEVENT"; then
				AC_MSG_ERROR([could not find libevent library])
			fi
			;;
	esac
fi
if test "x$enable_libevent" = "xyes"; then
	ENABLE_LIBEVENT="yes"

	AC_CHECK_HEADERS([event.h],, [unset ENABLE_LIBEVENT])
	AC_CHECK_LIB([event], [event_init],, [unset ENABLE_LIBEVENT])

	if test -z "$ENABLE_LIBEVENT"; then
		AC_MSG_ERROR([could not find libevent library])
	fi

	AC_DEFINE(ENABLE_LIBEVENT, 1)
fi
AM_CONDITIONAL(ENABLE_LIBEVENT, [test x$enable_libevent = xyes])


echo "TW_CFLAGS=$TW_CFLAGS"

CFLAGS="$CFLAGS $TW_CFLAGS"
CXXFLAGS="$CXXFLAGS $TW_CXXFLAGS"
CPPFLAGS="$CPPFLAGS $TW_CPPFLAGS"

AC_SUBST([prefix])

AC_SUBST([CC])
AC_SUBST([CXX])

AC_SUBST([LDFLAGS])
AC_SUBST([AM_LDFLAGS])
AC_SUBST([LIBS])

AC_SUBST([CPPFLAGS])
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([CFLAGS])
AC_SUBST([CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([enable_shared])
AC_SUBST([enable_static])

AC_SUBST([ENABLE_PARALLEL])
AC_SUBST([ENABLE_ABT])
AC_SUBST([ENABLE_LIBEVENT])

AC_CONFIG_FILES([
		Makefile 
		src/Makefile 
		src/libtw.settings
		src/driver/Makefile 
		src/driver/argobots/Makefile 
		src/common/Makefile 
		src/dispatcher/Makefile 
		examples/Makefile
		test/Makefile
		test/basic/Makefile])

AC_OUTPUT

cat src/libtw.settings

