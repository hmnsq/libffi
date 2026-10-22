/*
  This is a version (aka dlmalloc) of malloc/free/realloc written by
  Doug Lea and released to the public domain, as explained at
  http://creativecommons.org/publicdomain/zero/1.0/ Send questions,
  comments, complaints, performance data, etc to dl@cs.oswego.edu

* Version 2.8.6 Wed Aug 29 06:57:58 2012  Doug Lea
   Note: There may be an updated version of this malloc obtainable at
           ftp://gee.cs.oswego.edu/pub/misc/malloc.c
         Check before installing!

* Quickstart

  This library is all in one file to simplify the most common usage:
  ftp it, compile it (-O3), and link it into another program. All of
  the compile-time options default to reasonable values for use on
  most platforms.  You might later want to step through various
  compile-time and dynamic tuning options.

  For convenience, an include file for code using this malloc is at:
     ftp://gee.cs.oswego.edu/pub/misc/malloc-2.8.6.h
  You don't really need this .h file unless you call functions not
  defined in your system include files.  The .h file contains only the
  excerpts from this file needed for using this malloc on ANSI C/C++
  systems, so long as you haven't changed compile-time options about
  naming and tuning parameters.  If you do, then you can create your
  own malloc.h that does include all settings by cutting at the point
  indicated below. Note that you may already by default be using a C
  library containing a malloc that is based on some version of this
  malloc (for example in linux). You might still want to use the one
  in this file to customize settings or to avoid overheads associated
  with library versions.

* Vital statistics:

  Supported pointer/size_t representation:       4 or 8 bytes
       size_t MUST be an unsigned type of the same width as
       pointers. (If you are using an ancient system that declares
       size_t as a signed type, or need it to be a different width
       than pointers, you can use a previous release of this malloc
       (e.g. 2.7.2) supporting these.)

  Alignment:                                     8 bytes (minimum)
       This suffices for nearly all current machines and C compilers.
       However, you can define MALLOC_ALIGNMENT to be wider than this
       if necessary (up to 128bytes), at the expense of using more space.

  Minimum overhead per allocated chunk:   4 or  8 bytes (if 4byte sizes)
                                          8 or 16 bytes (if 8byte sizes)
       Each malloced chunk has a hidden word of overhead holding size
       and status information, and additional cross-check word
       if FOOTERS is defined.

  Minimum allocated size: 4-byte ptrs:  16 bytes    (including overhead)
                          8-byte ptrs:  32 bytes    (including overhead)

       Even a request for zero bytes (i.e., malloc(0)) returns a
       pointer to something of the minimum allocatable size.
       The maximum overhead wastage (i.e., number of extra bytes
       allocated than were requested in malloc) is less than or equal
       to the minimum size, except for requests >= mmap_threshold that
       are serviced via mmap(), where the worst case wastage is about
       32 bytes plus the remainder from a system page (the minimal
       mmap unit); typically 4096 or 8192 bytes.

  Security: static-safe; optionally more or less
       The "security" of malloc refers to the ability of malicious
       code to accentuate the effects of errors (for example, freeing
       space that is not currently malloc'ed or overwriting past the
       ends of chunks) in code that calls malloc.  This malloc
       guarantees not to modify any memory locations below the base of
       heap, i.e., static variables, even in the presence of usage
       errors.  The routines additionally detect most improper frees
       and reallocs.  All this holds as long as the static bookkeeping
       for malloc itself is not corrupted by some other means.  This
       is only one aspect of security -- these checks do not, and
       cannot, detect all possible programming errors.

       If FOOTERS is defined nonzero, then each allocated chunk
       carries an additional check word to verify that it was malloced
       from its space.  These check words are the same within each
       execution of a program using malloc, but differ across
       executions, so externally crafted fake chunks cannot be
       freed. This improves security by rejecting frees/reallocs that
       could corrupt heap memory, in addition to the checks preventing
       writes to statics that are always on.  This may further improve
       security at the expense of time and space overhead.  (Note that
       FOOTERS may also be worth using with MSPACES.)

       By default detected errors cause the program to abort (calling
       "abort()"). You can override this to instead proceed past
       errors by defining PROCEED_ON_ERROR.  In this case, a bad free
       has no effect, and a malloc that encounters a bad address
       caused by user overwrites will ignore the bad address by
       dropping pointers and indices to all known memory. This may
       be appropriate for programs that should continue if at all
       possible in the face of programming errors, although they may
       run out of memory because dropped memory is never reclaimed.

       If you don't like either of these options, you can define
       CORRUPTION_ERROR_ACTION and USAGE_ERROR_ACTION to do anything
       else. And if if you are sure that your program using malloc has
       no errors or vulnerabilities, you can define INSECURE to 1,
       which might (or might not) provide a small performance improvement.

       It is also possible to limit the maximum total allocatable
       space, using malloc_set_footprint_limit. This is not
       designed as a security feature in itself (calls to set limits
       are not screened or privileged), but may be useful as one
       aspect of a secure implementation.

  Thread-safety: NOT thread-safe unless USE_LOCKS defined non-zero
       When USE_LOCKS is defined, each public call to malloc, free,
       etc is surrounded with a lock. By default, this uses a plain
       pthread mutex, win32 critical section, or a spin-lock if if
       available for the platform and not disabled by setting
       USE_SPIN_LOCKS=0.  However, if USE_RECURSIVE_LOCKS is defined,
       recursive versions are used instead (which are not required for
       base functionality but may be needed in layered extensions).
       Using a global lock is not especially fast, and can be a major
       bottleneck.  It is designed only to provide minimal protection
       in concurrent environments, and to provide a basis for
       extensions.  If you are using malloc in a concurrent program,
       consider instead using nedmalloc
       (http://www.nedprod.com/programs/portable/nedmalloc/) or
       ptmalloc (See http://www.malloc.de), which are derived from
       versions of this malloc.

  System requirements: Any combination of MORECORE and/or MMAP/MUNMAP
       This malloc can use unix sbrk or any emulation (invoked using
       the CALL_MORECORE macro) and/or mmap/munmap or any emulation
       (invoked using CALL_MMAP/CALL_MUNMAP) to get and release system
       memory.  On most unix systems, it tends to work best if both
       MORECORE and MMAP are enabled.  On Win32, it uses emulations
       based on VirtualAlloc. It also uses common C library functions
       like memset.

  Compliance: I believe it is compliant with the Single Unix Specification
       (See http://www.unix.org). Also SVID/XPG, ANSI C, and probably
       others as well.

* Overview of algorithms

  This is not the fastest, most space-conserving, most portable, or
  most tunable malloc ever written. However it is among the fastest
  while also being among the most space-conserving, portable and
  tunable.  Consistent balance across these factors results in a good
  general-purpose allocator for malloc-intensive programs.

  In most ways, this malloc is a best-fit allocator. Generally, it
  chooses the best-fitting existing chunk for a request, with ties
  broken in approximately least-recently-used order. (This strategy
  normally maintains low fragmentation.) However, for requests less
  than 256bytes, it deviates from best-fit when there is not an
  exactly fitting available chunk by preferring to use space adjacent
  to that used for the previous small request, as well as by breaking
  ties in approximately most-recently-used order. (These enhance
  locality of series of small allocations.)  And for very large requests
  (>= 256Kb by default), it relies on system memory mapping
  facilities, if supported.  (This helps avoid carrying around and
  possibly fragmenting memory used only for large chunks.)

  All operations (except malloc_stats and mallinfo) have execution
  times that are bounded by a constant factor of the number of bits in
  a size_t, not counting any clearing in calloc or copying in realloc,
  or actions surrounding MORECORE and MMAP that have times
  proportional to the number of non-contiguous regions returned by
  system allocation routines, which is often just 1. In real-time
  applications, you can optionally suppress segment traversals using
  NO_SEGMENT_TRAVERSAL, which assures bounded execution even when
  system allocators return non-contiguous spaces, at the typical
  expense of carrying around more memory and increased fragmentation.

  The implementation is not very modular and seriously overuses
  macros. Perhaps someday all C compilers will do as good a job
  inlining modular code as can now be done by brute-force expansion,
  but now, enough of them seem not to.

  Some compilers issue a lot of warnings about code that is
  dead/unreachable only on some platforms, and also about intentional
  uses of negation on unsigned types. All known cases of each can be
  ignored.

  For a longer but out of date high-level description, see
     http://gee.cs.oswego.edu/dl/html/malloc.html

* MSPACES
  If MSPACES is defined, then in addition to malloc, free, etc.,
  this file also defines mspace_malloc, mspace_free, etc. These
  are versions of malloc routines that take an "mspace" argument
  obtained using create_mspace, to control all internal bookkeeping.
  If ONLY_MSPACES is defined, only these versions are compiled.
  So if you would like to use this allocator for only some allocations,
  and your system malloc for others, you can compile with
  ONLY_MSPACES and then do something like...
    static mspace mymspace = create_mspace(0,0); // for example
    #define mymalloc(bytes)  mspace_malloc(mymspace, bytes)

  (Note: If you only need one instance of an mspace, you can instead
  use "USE_DL_PREFIX" to relabel the global malloc.)

  You can similarly create thread-local allocators by storing
  mspaces as thread-locals. For example:
    static __thread mspace tlms = 0;
    void*  tlmalloc(size_t bytes) {
      if (tlms == 0) tlms = create_mspace(0, 0);
      return mspace_malloc(tlms, bytes);
    }
    void  tlfree(void* mem) { mspace_free(tlms, mem); }

  Unless FOOTERS is defined, each mspace is completely independent.
  You cannot allocate from one and free to another (although
  conformance is only weakly checked, so usage errors are not always
  caught). If FOOTERS is defined, then each chunk carries around a tag
  indicating its originating mspace, and frees are directed to their
  originating spaces. Normally, this requires use of locks.

 -------------------------  Compile-time options ---------------------------

Be careful in setting #define values for numerical constants of type
size_t. On some systems, literal values are not automatically extended
to size_t precision unless they are explicitly casted. You can also
use the symbolic values MAX_SIZE_T, SIZE_T_ONE, etc below.

WIN32                    default: defined if _WIN32 defined
  Defining WIN32 sets up defaults for MS environment and compilers.
  Otherwise defaults are for unix. Beware that there seem to be some
  cases where this malloc might not be a pure drop-in replacement for
  Win32 malloc: Random-looking failures from Win32 GDI API's (eg;
  SetDIBits()) may be due to bugs in some video driver implementations
  when pixel buffers are malloc()ed, and the region spans more than
  one VirtualAlloc()ed region. Because dlmalloc uses a small (64Kb)
  default granularity, pixel buffers may straddle virtual allocation
  regions more often than when using the Microsoft allocator.  You can
  avoid this by using VirtualAlloc() and VirtualFree() for all pixel
  buffers rather than using malloc().  If this is not possible,
  recompile this malloc with a larger DEFAULT_GRANULARITY. Note:
  in cases where MSC and gcc (cygwin) are known to differ on WIN32,
  conditions use _MSC_VER to distinguish them.

DLMALLOC_EXPORT       default: extern
  Defines how public APIs are declared. If you want to export via a
  Windows DLL, you might define this as
    #define DLMALLOC_EXPORT extern  __declspec(dllexport)
  If you want a POSIX ELF shared object, you might use
    #define DLMALLOC_EXPORT extern __attribute__((visibility("default")))

MALLOC_ALIGNMENT         default: (size_t)(2 * sizeof(void *))
  Controls the minimum alignment for malloc'ed chunks.  It must be a
  power of two and at least 8, even on machines for which smaller
  alignments would suffice. It may be defined as larger than this
  though. Note however that code and data structures are optimized for
  the case of 8-byte alignment.

MSPACES                  default: 0 (false)
  If true, compile in support for independent allocation spaces.
  This is only supported if HAVE_MMAP is true.

ONLY_MSPACES             default: 0 (false)
  If true, only compile in mspace versions, not regular versions.

USE_LOCKS                default: 0 (false)
  Causes each call to each public routine to be surrounded with
  pthread or WIN32 mutex lock/unlock. (If set true, this can be
  overridden on a per-mspace basis for mspace versions.) If set to a
  non-zero value other than 1, locks are used, but their
  implementation is left out, so lock functions must be supplied manually,
  as described below.

USE_SPIN_LOCKS           default: 1 iff USE_LOCKS and spin locks available
  If true, uses custom spin locks for locking. This is currently
  supported only gcc >= 4.1, older gccs on x86 platforms, and recent
  MS compilers.  Otherwise, posix locks or win32 critical sections are
  used.

USE_RECURSIVE_LOCKS      default: not defined
  If defined nonzero, uses recursive (aka reentrant) locks, otherwise
  uses plain mutexes. This is not required for malloc proper, but may
  be needed for layered allocators such as nedmalloc.

LOCK_AT_FORK            default: not defined
  If defined nonzero, performs pthread_atfork upon initialization
  to initialize child lock while holding parent lock. The implementation
  assumes that pthread locks (not custom locks) are being used. In other
  cases, you may need to customize the implementation.

FOOTERS                  default: 0
  If true, provide extra checking and dispatching by placing
  information in the footers of allocated chunks. This adds
  space and time overhead.

INSECURE                 default: 0
  If true, omit checks for usage errors and heap space overwrites.

USE_DL_PREFIX            default: NOT defined
  Causes compiler to prefix all public routines with the string 'dl'.
  This can be useful when you only want to use this malloc in one part
  of a program, using your regular system malloc elsewhere.

MALLOC_INSPECT_ALL       default: NOT defined
  If defined, compiles malloc_inspect_all and mspace_inspect_all, that
  perform traversal of all heap space.  Unless access to these
  functions is otherwise restricted, you probably do not want to
  include them in secure implementations.

ABORT                    default: defined as abort()
  Defines how to abort on failed checks.  On most systems, a failed
  check cannot die with an "assert" or even print an informative
  message, because the underlying print routines in turn call malloc,
  which will fail again.  Generally, the best policy is to simply call
  abort(). It's not very useful to do more than this because many
  errors due to overwriting will show up as address faults (null, odd
  addresses etc) rather than malloc-triggered checks, so will also
  abort.  Also, most compilers know that abort() does not return, so
  can better optimize code conditionally calling it.

PROCEED_ON_ERROR           default: defined as 0 (false)
  Controls whether detected bad addresses cause them to bypassed
  rather than aborting. If set, detected bad arguments to free and
  realloc are ignored. And all bookkeeping information is zeroed out
  upon a detected overwrite of freed heap space, thus losing the
  ability to ever return it from malloc again, but enabling the
  application to proceed. If PROCEED_ON_ERROR is defined, the
  static variable malloc_corruption_error_count is compiled in
  and can be examined to see if errors have occurred. This option
  generates slower code than the default abort policy.

DEBUG                    default: NOT defined
  The DEBUG setting is mainly intended for people trying to modify
  this code or diagnose problems when porting to new platforms.
  However, it may also be able to better isolate user errors than just
  using runtime checks.  The assertions in the check routines spell
  out in more detail the assumptions and invariants underlying the
  algorithms.  The checking is fairly extensive, and will slow down
  execution noticeably. Calling malloc_stats or mallinfo with DEBUG
  set will attempt to check every non-mmapped allocated and free chunk
  in the course of computing the summaries.

ABORT_ON_ASSERT_FAILURE   default: defined as 1 (true)
  Debugging assertion failures can be nearly impossible if your
  version of the assert macro causes malloc to be called, which will
  lead to a cascade of further failures, blowing the runtime stack.
  ABORT_ON_ASSERT_FAILURE cause assertions failures to call abort(),
  which will usually make debugging easier.

MALLOC_FAILURE_ACTION     default: sets errno to ENOMEM, or no-op on win32
  The action to take before "return 0" when malloc fails to be able to
  return memory because there is none available.

HAVE_MORECORE             default: 1 (true) unless win32 or ONLY_MSPACES
  True if this system supports sbrk or an emulation of it.

MORECORE                  default: sbrk
  The name of the sbrk-style system routine to call to obtain more
  memory.  See below for guidance on writing custom MORECORE
  functions. The type of the argument to sbrk/MORECORE varies across
  systems.  It cannot be size_t, because it supports negative
  arguments, so it is normally the signed type of the same width as
  size_t (sometimes declared as "intptr_t").  It doesn't much matter
  though. Internally, we only call it with arguments less than half
  the max value of a size_t, which should work across all reasonable
  possibilities, although sometimes generating compiler warnings.

MORECORE_CONTIGUOUS       default: 1 (true) if HAVE_MORECORE
  If true, take advantage of fact that consecutive calls to MORECORE
  with positive arguments always return contiguous increasing
  addresses.  This is true of unix sbrk. It does not hurt too much to
  set it true anyway, since malloc copes with non-contiguities.
  Setting it false when definitely non-contiguous saves time
  and possibly wasted space it would take to discover this though.

MORECORE_CANNOT_TRIM      default: NOT defined
  True if MORECORE cannot release space back to the system when given
  negative arguments. This is generally necessary only if you are
  using a hand-crafted MORECORE function that cannot handle negative
  arguments.

NO_SEGMENT_TRAVERSAL       default: 0
  If non-zero, suppresses traversals of memory segments
  returned by either MORECORE or CALL_MMAP. This disables
  merging of segments that are contiguous, and selectively
  releasing them to the OS if unused, but bounds execution times.

HAVE_MMAP                 default: 1 (true)
  True if this system supports mmap or an emulation of it.  If so, and
  HAVE_MORECORE is not true, MMAP is used for all system
  allocation. If set and HAVE_MORECORE is true as well, MMAP is
  primarily used to directly allocate very large blocks. It is also
  used as a backup strategy in cases where MORECORE fails to provide
  space from system. Note: A single call to MUNMAP is assumed to be
  able to unmap memory that may have be allocated using multiple calls
  to MMAP, so long as they are adjacent.

HAVE_MREMAP               default: 1 on linux, else 0
  If true realloc() uses mremap() to re-allocate large blocks and
  extend or shrink allocation spaces.

MMAP_CLEARS               default: 1 except on WINCE.
  True if mmap clears memory so calloc doesn't need to. This is true
  for standard unix mmap using /dev/zero and on WIN32 except for WINCE.

USE_BUILTIN_FFS            default: 0 (i.e., not used)
  Causes malloc to use the builtin ffs() function to compute indices.
  Some compilers may recognize and intrinsify ffs to be faster than the
  supplied C version. Also, the case of x86 using gcc is special-cased
  to an asm instruction, so is already as fast as it can be, and so
  this setting has no effect. Similarly for Win32 under recent MS compilers.
  (On most x86s, the asm version is only slightly faster than the C version.)

malloc_getpagesize         default: derive from system includes, or 4096.
  The system page size. To the extent possible, this malloc manages
  memory from the system in page-size units.  This may be (and
  usually is) a function rather than a constant. This is ignored
  if WIN32, where page size is determined using getSystemInfo during
  initialization.

USE_DEV_RANDOM             default: 0 (i.e., not used)
  Causes malloc to use /dev/random to initialize secure magic seed for
  stamping footers. Otherwise, the current time is used.

NO_MALLINFO                default: 0
  If defined, don't compile "mallinfo". This can be a simple way
  of dealing with mismatches between system declarations and
  those in this file.

MALLINFO_FIELD_TYPE        default: size_t
  The type of the fields in the mallinfo struct. This was originally
  defined as "int" in SVID etc, but is more usefully defined as
  size_t. The value is used only if  HAVE_USR_INCLUDE_MALLOC_H is not set

NO_MALLOC_STATS            default: 0
  If defined, don't compile "malloc_stats". This avoids calls to
  fprintf and bringing in stdio dependencies you might not want.

REALLOC_ZERO_BYTES_FREES    default: not defined
  This should be set if a call to realloc with zero bytes should
  be the same as a call to free. Some people think it should. Otherwise,
  since this malloc returns a unique pointer for malloc(0), so does
  realloc(p, 0).

LACKS_UNISTD_H, LACKS_FCNTL_H, LACKS_SYS_PARAM_H, LACKS_SYS_MMAN_H
LACKS_STRINGS_H, LACKS_STRING_H, LACKS_SYS_TYPES_H,  LACKS_ERRNO_H
LACKS_STDLIB_H LACKS_SCHED_H LACKS_TIME_H  default: NOT defined unless on WIN32
  Define these if your system does not have these header files.
  You might need to manually insert some of the declarations they provide.

DEFAULT_GRANULARITY        default: page size if MORECORE_CONTIGUOUS,
                                system_info.dwAllocationGranularity in WIN32,
                                otherwise 64K.
      Also settable using mallopt(M_GRANULARITY, x)
  The unit for allocating and deallocating memory from the system.  On
  most systems with contiguous MORECORE, there is no reason to
  make this more than a page. However, systems with MMAP tend to
  either require or encourage larger granularities.  You can increase
  this value to prevent system allocation functions to be called so
  often, especially if they are slow.  The value must be at least one
  page and must be a power of two.  Setting to 0 causes initialization
  to either page size or win32 region size.  (Note: In previous
  versions of malloc, the equivalent of this option was called
  "TOP_PAD")

DEFAULT_TRIM_THRESHOLD    default: 2MB
      Also settable using mallopt(M_TRIM_THRESHOLD, x)
  The maximum amount of unused top-most memory to keep before
  releasing via malloc_trim in free().  Automatic trimming is mainly
  useful in long-lived programs using contiguous MORECORE.  Because
  trimming via sbrk can be slow on some systems, and can sometimes be
  wasteful (in cases where programs immediately afterward allocate
  more large chunks) the value should be high enough so that your
  overall system performance would improve by releasing this much
  memory.  As a rough guide, you might set to a value close to the
  average size of a process (program) running on your system.
  Releasing this much memory would allow such a process to run in
  memory.  Generally, it is worth tuning trim thresholds when a
  program undergoes phases where several large chunks are allocated
  and released in ways that can reuse each other's storage, perhaps
  mixed with phases where there are no such chunks at all. The trim
  value must be greater than page size to have any useful effect.  To
  disable trimming completely, you can set to MAX_SIZE_T. Note that the trick
  some people use of mallocing a huge space and then freeing it at
  program startup, in an attempt to reserve system memory, doesn't
  have the intended effect under automatic trimming, since that memory
  will immediately be returned to the system.

DEFAULT_MMAP_THRESHOLD       default: 256K
      Also settable using mallopt(M_MMAP_THRESHOLD, x)
  The request size threshold for using MMAP to directly service a
  request. Requests of at least this size that cannot be allocated
  using already-existing space will be serviced via mmap.  (If enough
  normal freed space already exists it is used instead.)  Using mmap
  segregates relatively large chunks of memory so that they can be
  individually obtained and released from the host system. A request
  serviced through mmap is never reused by any other request (at least
  not directly; the system may just so happen to remap successive
  requests to the same locations).  Segregating space in this way has
  the benefits that: Mmapped space can always be individually released
  back to the system, which helps keep the system level memory demands
  of a long-lived program low.  Also, mapped memory doesn't become
  `locked' between other chunks, as can happen with normally allocated
  chunks, which means that even trimming via malloc_trim would not
  release them.  However, it has the disadvantage that the space
  cannot be reclaimed, consolidated, and then used to service later
  requests, as happens with normal chunks.  The advantages of mmap
  nearly always outweigh disadvantages for "large" chunks, but the
  value of "large" may vary across systems.  The default is an
  empirically derived value that works well in most systems. You can
  disable mmap by setting to MAX_SIZE_T.

MAX_RELEASE_CHECK_RATE   default: 4095 unless not HAVE_MMAP
  The number of consolidated frees between checks to release
  unused segments when freeing. When using non-contiguous segments,
  especially with multiple mspaces, checking only for topmost space
  doesn't always suffice to trigger trimming. To compensate for this,
  free() will, with a period of MAX_RELEASE_CHECK_RATE (or the
  current number of segments, if greater) try to release unused
  segments to the OS when freeing chunks that result in
  consolidation. The best value for this parameter is a compromise
  between slowing down frees with relatively costly checks that
  rarely trigger versus holding on to unused memory. To effectively
  disable, set to MAX_SIZE_T. This may lead to a very slight speed
  improvement at the expense of carrying around more memory.
*/

/* Version identifier to allow people to support multiple versions */
#यदि_अपरिभाषितम् DLMALLOC_संस्करणम्
#परिभाषय DLMALLOC_संस्करणम् 20806
#यदि_अन्तः /* DLMALLOC_VERSION */

#यदि_अपरिभाषितम् DLMALLOC_EXPORT
#परिभाषय DLMALLOC_EXPORT बाह्य
#यदि_अन्तः

#यदि defined __linux__ && !defined _GNU_SOURCE
/* mremap() on Linux requires this via sys/mman.h */
#परिभाषय _GNU_SOURCE 1
#यदि_अन्तः

#यदि_अपरिभाषितम् WIN32
#यदि_परिभाषितम् _WIN32
#परिभाषय WIN32 1
#यदि_अन्तः  /* _WIN32 */
#यदि_परिभाषितम् _WIN32_WCE
#परिभाषय LACKS_FCNTL_H
#परिभाषय WIN32 1
#यदि_अन्तः /* _WIN32_WCE */
#यदि_अन्तः  /* WIN32 */
#यदि_परिभाषितम् WIN32
#परिभाषय WIN32_LEAN_AND_माध्यम्
#अन्तर्भाव <windows.h>
#अन्तर्भाव <tchar.h>
#परिभाषय HAVE_MMAP 1
#परिभाषय HAVE_MORECORE 0
#परिभाषय LACKS_UNISTD_H
#परिभाषय LACKS_SYS_PARAM_H
#परिभाषय LACKS_SYS_MMAN_H
#परिभाषय LACKS_STRING_H
#परिभाषय LACKS_वाक्यानि_H
#परिभाषय LACKS_SYS_TYPES_H
#परिभाषय LACKS_ERRNO_H
#परिभाषय LACKS_SCHED_H
#यदि_अपरिभाषितम् MALLOC_FAILURE_ACTION
#परिभाषय MALLOC_FAILURE_ACTION
#यदि_अन्तः /* MALLOC_FAILURE_ACTION */
#यदि_अपरिभाषितम् MMAP_CLEARS
#यदि_परिभाषितम् _WIN32_WCE /* WINCE reportedly does not clear */
#परिभाषय MMAP_CLEARS 0
#अन्यथा
#परिभाषय MMAP_CLEARS 1
#यदि_अन्तः /* _WIN32_WCE */
#यदि_अन्तः /*MMAP_CLEARS */
#यदि_अन्तः  /* WIN32 */

#यदि_परिभाषितम् __OS2__
#परिभाषय INCL_DOS
#अन्तर्भाव <os2.h>
#परिभाषय HAVE_MMAP 1
#परिभाषय HAVE_MORECORE 0
#परिभाषय LACKS_SYS_MMAN_H
#यदि_अन्तः  /* __OS2__ */

#यदि defined(DARWIN) || defined(_DARWIN)
/* Mac OSX docs advise not to use sbrk; it seems better to use mmap */
#यदि_अपरिभाषितम् HAVE_MORECORE
#परिभाषय HAVE_MORECORE 0
#परिभाषय HAVE_MMAP 1
/* OSX allocators provide 16 byte alignment */
#यदि_अपरिभाषितम् MALLOC_ALIGNMENT
#परिभाषय MALLOC_ALIGNMENT ((size_t)16U)
#यदि_अन्तः
#यदि_अन्तः  /* HAVE_MORECORE */
#यदि_अन्तः  /* DARWIN */

#यदि_अपरिभाषितम् LACKS_SYS_TYPES_H
#अन्तर्भाव <sys/types.h>  /* For size_t */
#यदि_अन्तः  /* LACKS_SYS_TYPES_H */

/* The maximum possible size_t value has all bits set */
#परिभाषय MAX_आकार_T           (~(size_t)0)

#यदि_अपरिभाषितम् USE_LOCKS /* ensure true if spin or recursive locks set */
#परिभाषय USE_LOCKS  ((defined(USE_SPIN_LOCKS) && USE_SPIN_LOCKS != 0) || \
                    (defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0))
#यदि_अन्तः /* USE_LOCKS */

#यदि USE_LOCKS /* Spin locks for gcc >= 4.1, older gcc on x86, MSC >= 1310 */
#यदि ((defined(__GNUC__) &&                                              \
      ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) ||      \
       defined(__i386__) || defined(__x86_64__))) ||                    \
     (defined(_MSC_VER) && _MSC_VER>=1310))
#यदि_अपरिभाषितम् USE_SPIN_LOCKS
#परिभाषय USE_SPIN_LOCKS 1
#यदि_अन्तः /* USE_SPIN_LOCKS */
#नोचेद्यदि USE_SPIN_LOCKS
#दोषः "USE_SPIN_LOCKS defined without implementation"
#यदि_अन्तः /* ... locks available... */
#नोचेद्यदि !defined(USE_SPIN_LOCKS)
#परिभाषय USE_SPIN_LOCKS 0
#यदि_अन्तः /* USE_LOCKS */

#यदि_अपरिभाषितम् ONLY_MSPACES
#परिभाषय ONLY_MSPACES 0
#यदि_अन्तः  /* ONLY_MSPACES */
#यदि_अपरिभाषितम् MSPACES
#यदि ONLY_MSPACES
#परिभाषय MSPACES 1
#अन्यथा   /* ONLY_MSPACES */
#परिभाषय MSPACES 0
#यदि_अन्तः  /* ONLY_MSPACES */
#यदि_अन्तः  /* MSPACES */
#यदि_अपरिभाषितम् MALLOC_ALIGNMENT
#परिभाषय MALLOC_ALIGNMENT ((size_t)(2 * परिमाण(शून्य *)))
#यदि_अन्तः  /* MALLOC_ALIGNMENT */
#यदि_अपरिभाषितम् FOOTERS
#परिभाषय FOOTERS 0
#यदि_अन्तः  /* FOOTERS */
#यदि_अपरिभाषितम् ABORT
#परिभाषय ABORT  abort()
#यदि_अन्तः  /* ABORT */
#यदि_अपरिभाषितम् ABORT_ON_ASSERT_FAILURE
#परिभाषय ABORT_ON_ASSERT_FAILURE 1
#यदि_अन्तः  /* ABORT_ON_ASSERT_FAILURE */
#यदि_अपरिभाषितम् PROCEED_ON_ERROR
#परिभाषय PROCEED_ON_ERROR 0
#यदि_अन्तः  /* PROCEED_ON_ERROR */

#यदि_अपरिभाषितम् INSECURE
#परिभाषय INSECURE 0
#यदि_अन्तः  /* INSECURE */
#यदि_अपरिभाषितम् MALLOC_INSPECT_ALL
#परिभाषय MALLOC_INSPECT_ALL 0
#यदि_अन्तः  /* MALLOC_INSPECT_ALL */
#यदि_अपरिभाषितम् HAVE_MMAP
#परिभाषय HAVE_MMAP 1
#यदि_अन्तः  /* HAVE_MMAP */
#यदि_अपरिभाषितम् MMAP_CLEARS
#परिभाषय MMAP_CLEARS 1
#यदि_अन्तः  /* MMAP_CLEARS */
#यदि_अपरिभाषितम् HAVE_MREMAP
#यदि_परिभाषितम् linux
#परिभाषय HAVE_MREMAP 1
#परिभाषय _GNU_SOURCE /* Turns on mremap() definition */
#अन्यथा   /* linux */
#परिभाषय HAVE_MREMAP 0
#यदि_अन्तः  /* linux */
#यदि_अन्तः  /* HAVE_MREMAP */
#यदि_अपरिभाषितम् MALLOC_FAILURE_ACTION
#परिभाषय MALLOC_FAILURE_ACTION  errno = ENOMEM;
#यदि_अन्तः  /* MALLOC_FAILURE_ACTION */
#यदि_अपरिभाषितम् HAVE_MORECORE
#यदि ONLY_MSPACES
#परिभाषय HAVE_MORECORE 0
#अन्यथा   /* ONLY_MSPACES */
#परिभाषय HAVE_MORECORE 1
#यदि_अन्तः  /* ONLY_MSPACES */
#यदि_अन्तः  /* HAVE_MORECORE */
#यदि !HAVE_MORECORE
#परिभाषय MORECORE_CONTIGUOUS 0
#अन्यथा   /* !HAVE_MORECORE */
#परिभाषय MORECORE_DEFAULT sbrk
#यदि_अपरिभाषितम् MORECORE_CONTIGUOUS
#परिभाषय MORECORE_CONTIGUOUS 1
#यदि_अन्तः  /* MORECORE_CONTIGUOUS */
#यदि_अन्तः  /* HAVE_MORECORE */
#यदि_अपरिभाषितम् DEFAULT_GRANULARITY
#यदि (MORECORE_CONTIGUOUS || defined(WIN32))
#परिभाषय DEFAULT_GRANULARITY (0)  /* 0 means to compute in init_mparams */
#अन्यथा   /* MORECORE_CONTIGUOUS */
#परिभाषय DEFAULT_GRANULARITY ((size_t)64U * (size_t)1024U)
#यदि_अन्तः  /* MORECORE_CONTIGUOUS */
#यदि_अन्तः  /* DEFAULT_GRANULARITY */
#यदि_अपरिभाषितम् DEFAULT_TRIM_देहली
#यदि_अपरिभाषितम् MORECORE_CANNOT_TRIM
#परिभाषय DEFAULT_TRIM_देहली ((size_t)2U * (size_t)1024U * (size_t)1024U)
#अन्यथा   /* MORECORE_CANNOT_TRIM */
#परिभाषय DEFAULT_TRIM_देहली MAX_आकार_T
#यदि_अन्तः  /* MORECORE_CANNOT_TRIM */
#यदि_अन्तः  /* DEFAULT_TRIM_THRESHOLD */
#यदि_अपरिभाषितम् DEFAULT_MMAP_देहली
#यदि HAVE_MMAP
#परिभाषय DEFAULT_MMAP_देहली ((size_t)256U * (size_t)1024U)
#अन्यथा   /* HAVE_MMAP */
#परिभाषय DEFAULT_MMAP_देहली MAX_आकार_T
#यदि_अन्तः  /* HAVE_MMAP */
#यदि_अन्तः  /* DEFAULT_MMAP_THRESHOLD */
#यदि_अपरिभाषितम् MAX_RELEASE_CHECK_दरः
#यदि HAVE_MMAP
#परिभाषय MAX_RELEASE_CHECK_दरः 4095
#अन्यथा
#परिभाषय MAX_RELEASE_CHECK_दरः MAX_आकार_T
#यदि_अन्तः /* HAVE_MMAP */
#यदि_अन्तः /* MAX_RELEASE_CHECK_RATE */
#यदि_अपरिभाषितम् USE_BUILTIN_FFS
#परिभाषय USE_BUILTIN_FFS 0
#यदि_अन्तः  /* USE_BUILTIN_FFS */
#यदि_अपरिभाषितम् USE_DEV_RANDOM
#परिभाषय USE_DEV_RANDOM 0
#यदि_अन्तः  /* USE_DEV_RANDOM */
#यदि_अपरिभाषितम् NO_MALLINFO
#परिभाषय NO_MALLINFO 0
#यदि_अन्तः  /* NO_MALLINFO */
#यदि_अपरिभाषितम् MALLINFO_क्षेत्रम्_TYPE
#परिभाषय MALLINFO_क्षेत्रम्_TYPE size_t
#यदि_अन्तः  /* MALLINFO_FIELD_TYPE */
#यदि_अपरिभाषितम् NO_MALLOC_STATS
#परिभाषय NO_MALLOC_STATS 0
#यदि_अन्तः  /* NO_MALLOC_STATS */
#यदि_अपरिभाषितम् NO_SEGMENT_TRAVERSAL
#परिभाषय NO_SEGMENT_TRAVERSAL 0
#यदि_अन्तः /* NO_SEGMENT_TRAVERSAL */

/*
  mallopt tuning options.  SVID/XPG defines four standard parameter
  numbers for mallopt, normally defined in malloc.h.  None of these
  are used in this malloc, so setting them has no effect. But this
  malloc does support the following options.
*/

/* The system's malloc.h may have conflicting defines. */
#अपरिभाषय M_TRIM_देहली
#अपरिभाषय M_GRANULARITY
#अपरिभाषय M_MMAP_देहली

#परिभाषय M_TRIM_देहली     (-1)
#परिभाषय M_GRANULARITY        (-2)
#परिभाषय M_MMAP_देहली     (-3)

/* ------------------------ Mallinfo declarations ------------------------ */

#यदि !NO_MALLINFO
/*
  This version of malloc supports the standard SVID/XPG mallinfo
  routine that returns a struct containing usage properties and
  statistics. It should work on any system that has a
  /usr/include/malloc.h defining struct mallinfo.  The main
  declaration needed is the mallinfo struct that is returned (by-copy)
  by mallinfo().  The malloinfo struct contains a bunch of fields that
  are not even meaningful in this version of malloc.  These fields are
  are instead filled by mallinfo() with other numbers that might be of
  interest.

  HAVE_USR_INCLUDE_MALLOC_H should be set if you have a
  /usr/include/malloc.h file that includes a declaration of struct
  mallinfo.  If so, it is included; else a compliant version is
  declared below.  These must be precisely the same for mallinfo() to
  work.  The original SVID version of this struct, defined on most
  systems with mallinfo, declares all fields as ints. But some others
  define as unsigned long. If your system defines the fields using a
  type of different width than listed here, you MUST #include your
  system version and #define HAVE_USR_INCLUDE_MALLOC_H.
*/

/* #define HAVE_USR_INCLUDE_MALLOC_H */

#यदि_परिभाषितम् HAVE_USR_INCLUDE_MALLOC_H
#अन्तर्भाव "/usr/include/malloc.h"
#अन्यथा /* HAVE_USR_INCLUDE_MALLOC_H */
#यदि_अपरिभाषितम् STRUCT_MALLINFO_DECLARED
/* HP-UX (and others?) redefines mallinfo unless _STRUCT_MALLINFO is defined */
#परिभाषय _STRUCT_MALLINFO
#परिभाषय STRUCT_MALLINFO_DECLARED 1
संरचना mallinfo {
  MALLINFO_क्षेत्रम्_TYPE arena;    /* non-mmapped space allocated from system */
  MALLINFO_क्षेत्रम्_TYPE ordblks;  /* number of free chunks */
  MALLINFO_क्षेत्रम्_TYPE smblks;   /* always 0 */
  MALLINFO_क्षेत्रम्_TYPE hblks;    /* always 0 */
  MALLINFO_क्षेत्रम्_TYPE hblkhd;   /* space in mmapped regions */
  MALLINFO_क्षेत्रम्_TYPE usmblks;  /* maximum total allocated space */
  MALLINFO_क्षेत्रम्_TYPE fsmblks;  /* always 0 */
  MALLINFO_क्षेत्रम्_TYPE uordblks; /* total allocated space */
  MALLINFO_क्षेत्रम्_TYPE fordblks; /* total free space */
  MALLINFO_क्षेत्रम्_TYPE keepcost; /* releasable (via malloc_trim) space */
};
#यदि_अन्तः /* STRUCT_MALLINFO_DECLARED */
#यदि_अन्तः /* HAVE_USR_INCLUDE_MALLOC_H */
#यदि_अन्तः /* NO_MALLINFO */

/*
  Try to persuade compilers to inline. The most critical functions for
  inlining are defined as macros, so these aren't used for them.
*/

#यदि_अपरिभाषितम् FORCEINLINE
  #यदि defined(__GNUC__)
#परिभाषय FORCEINLINE __inline __attribute__ ((always_inline))
  #नोचेद्यदि defined(_MSC_VER)
    #परिभाषय FORCEINLINE __forceinline
  #यदि_अन्तः
#यदि_अन्तः
#यदि_अपरिभाषितम् NOINLINE
  #यदि defined(__GNUC__)
    #परिभाषय NOINLINE __attribute__ ((noinline))
  #नोचेद्यदि defined(_MSC_VER)
    #परिभाषय NOINLINE __declspec(noinline)
  #अन्यथा
    #परिभाषय NOINLINE
  #यदि_अन्तः
#यदि_अन्तः

#यदि_परिभाषितम् __cplusplus
बाह्य "C" {
#यदि_अपरिभाषितम् FORCEINLINE
 #परिभाषय FORCEINLINE अन्तःस्थ
#यदि_अन्तः
#यदि_अन्तः /* __cplusplus */
#यदि_अपरिभाषितम् FORCEINLINE
 #परिभाषय FORCEINLINE
#यदि_अन्तः

#यदि !ONLY_MSPACES

/* ------------------- Declarations of public routines ------------------- */

#यदि_अपरिभाषितम् USE_DL_उपसर्गः
#परिभाषय dlcalloc               शून्यस्मृतिं_देहि
#परिभाषय dlfree                 मुक्त
#परिभाषय dlmalloc               स्मृतिं_देहि
#परिभाषय dlmemalign             memalign
#परिभाषय dlposix_memalign       posix_memalign
#परिभाषय dlrealloc              realloc
#परिभाषय dlrealloc_अन्तर्_place     realloc_अन्तर्_place
#परिभाषय dlvalloc               valloc
#परिभाषय dlpvalloc              pvalloc
#परिभाषय dlmallinfo             mallinfo
#परिभाषय dlmallopt              mallopt
#परिभाषय dlmalloc_trim          malloc_trim
#परिभाषय dlmalloc_stats         malloc_stats
#परिभाषय dlmalloc_usable_आकार   malloc_usable_आकार
#परिभाषय dlmalloc_footprint     malloc_footprint
#परिभाषय dlmalloc_max_footprint malloc_max_footprint
#परिभाषय dlmalloc_footprint_सीमा malloc_footprint_सीमा
#परिभाषय dlmalloc_समुच्चयः_footprint_सीमा malloc_समुच्चयः_footprint_सीमा
#परिभाषय dlmalloc_inspect_all   malloc_inspect_all
#परिभाषय dlindependent_calloc   independent_calloc
#परिभाषय dlindependent_comalloc independent_comalloc
#परिभाषय dlbulk_free            bulk_free
#यदि_अन्तः /* USE_DL_PREFIX */

/*
  malloc(size_t n)
  Returns a pointer to a newly allocated chunk of at least n bytes, or
  null if no space is available, in which case errno is set to ENOMEM
  on ANSI C systems.

  If n is zero, malloc returns a minimum-sized chunk. (The minimum
  size is 16 bytes on most 32bit systems, and 32 bytes on 64bit
  systems.)  Note that size_t is an unsigned type, so calls with
  arguments that would be negative if signed are interpreted as
  requests for huge amounts of space, which will often fail. The
  maximum supported value of n differs across systems, but is in all
  cases less than the maximum representable value of a size_t.
*/
DLMALLOC_EXPORT शून्य* dlmalloc(size_t);

/*
  free(void* p)
  Releases the chunk of memory pointed to by p, that had been previously
  allocated using malloc or a related routine such as realloc.
  It has no effect if p is null. If p was not malloced or already
  freed, free(p) will by default cause the current program to abort.
*/
DLMALLOC_EXPORT शून्य  dlfree(शून्य*);

/*
  calloc(size_t n_elements, size_t element_size);
  Returns a pointer to n_elements * element_size bytes, with all locations
  set to zero.
*/
DLMALLOC_EXPORT शून्य* dlcalloc(size_t, size_t);

/*
  realloc(void* p, size_t n)
  Returns a pointer to a chunk of size n that contains the same data
  as does chunk p up to the minimum of (n, p's size) bytes, or null
  if no space is available.

  The returned pointer may or may not be the same as p. The algorithm
  prefers extending p in most cases when possible, otherwise it
  employs the equivalent of a malloc-copy-free sequence.

  If p is null, realloc is equivalent to malloc.

  If space is not available, realloc returns null, errno is set (if on
  ANSI) and p is NOT freed.

  if n is for fewer bytes than already held by p, the newly unused
  space is lopped off and freed if possible.  realloc with a size
  argument of zero (re)allocates a minimum-sized chunk.

  The old unix realloc convention of allowing the last-free'd chunk
  to be used as an argument to realloc is not supported.
*/
DLMALLOC_EXPORT शून्य* dlrealloc(शून्य*, size_t);

/*
  realloc_in_place(void* p, size_t n)
  Resizes the space allocated for p to size n, only if this can be
  done without moving p (i.e., only if there is adjacent space
  available if n is greater than p's current allocated size, or n is
  less than or equal to p's size). This may be used instead of plain
  realloc if an alternative allocation strategy is needed upon failure
  to expand space; for example, reallocation of a buffer that must be
  memory-aligned or cleared. You can use realloc_in_place to trigger
  these alternatives only when needed.

  Returns p if successful; otherwise null.
*/
DLMALLOC_EXPORT शून्य* dlrealloc_अन्तर्_place(शून्य*, size_t);

/*
  memalign(size_t alignment, size_t n);
  Returns a pointer to a newly allocated chunk of n bytes, aligned
  in accord with the alignment argument.

  The alignment argument should be a power of two. If the argument is
  not a power of two, the nearest greater power is used.
  8-byte alignment is guaranteed by normal malloc calls, so don't
  bother calling memalign with an argument of 8 or less.

  Overreliance on memalign is a sure way to fragment space.
*/
DLMALLOC_EXPORT शून्य* dlmemalign(size_t, size_t);

/*
  int posix_memalign(void** pp, size_t alignment, size_t n);
  Allocates a chunk of n bytes, aligned in accord with the alignment
  argument. Differs from memalign only in that it (1) assigns the
  allocated memory to *pp rather than returning it, (2) fails and
  returns EINVAL if the alignment is not a power of two (3) fails and
  returns ENOMEM if memory cannot be allocated.
*/
DLMALLOC_EXPORT पूर्णाङ्क dlposix_memalign(शून्य**, size_t, size_t);

/*
  valloc(size_t n);
  Equivalent to memalign(pagesize, n), where pagesize is the page
  size of the system. If the pagesize is unknown, 4096 is used.
*/
DLMALLOC_EXPORT शून्य* dlvalloc(size_t);

/*
  mallopt(int parameter_number, int parameter_value)
  Sets tunable parameters The format is to provide a
  (parameter-number, parameter-value) pair.  mallopt then sets the
  corresponding parameter to the argument value if it can (i.e., so
  long as the value is meaningful), and returns 1 if successful else
  0.  To workaround the fact that mallopt is specified to use int,
  not size_t parameters, the value -1 is specially treated as the
  maximum unsigned size_t value.

  SVID/XPG/ANSI defines four standard param numbers for mallopt,
  normally defined in malloc.h.  None of these are use in this malloc,
  so setting them has no effect. But this malloc also supports other
  options in mallopt. See below for details.  Briefly, supported
  parameters are as follows (listed defaults are for "typical"
  configurations).

  Symbol            param #  default    allowed param values
  M_TRIM_THRESHOLD     -1   2*1024*1024   any   (-1 disables)
  M_GRANULARITY        -2     page size   any power of 2 >= page size
  M_MMAP_THRESHOLD     -3      256*1024   any   (or 0 if no MMAP support)
*/
DLMALLOC_EXPORT पूर्णाङ्क dlmallopt(पूर्णाङ्क, पूर्णाङ्क);

/*
  malloc_footprint();
  Returns the number of bytes obtained from the system.  The total
  number of bytes allocated by malloc, realloc etc., is less than this
  value. Unlike mallinfo, this function returns only a precomputed
  result, so can be called frequently to monitor memory consumption.
  Even if locks are otherwise defined, this function does not use them,
  so results might not be up to date.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint(शून्य);

/*
  malloc_max_footprint();
  Returns the maximum number of bytes obtained from the system. This
  value will be greater than current footprint if deallocated space
  has been reclaimed by the system. The peak number of bytes allocated
  by malloc, realloc etc., is less than this value. Unlike mallinfo,
  this function returns only a precomputed result, so can be called
  frequently to monitor memory consumption.  Even if locks are
  otherwise defined, this function does not use them, so results might
  not be up to date.
*/
DLMALLOC_EXPORT size_t dlmalloc_max_footprint(शून्य);

/*
  malloc_footprint_limit();
  Returns the number of bytes that the heap is allowed to obtain from
  the system, returning the last value returned by
  malloc_set_footprint_limit, or the maximum size_t value if
  never set. The returned value reflects a permission. There is no
  guarantee that this number of bytes can actually be obtained from
  the system.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint_सीमा();

/*
  malloc_set_footprint_limit();
  Sets the maximum number of bytes to obtain from the system, causing
  failure returns from malloc and related functions upon attempts to
  exceed this value. The argument value may be subject to page
  rounding to an enforceable limit; this actual value is returned.
  Using an argument of the maximum possible size_t effectively
  disables checks. If the argument is less than or equal to the
  current malloc_footprint, then all future allocations that require
  additional system memory will fail. However, invocation cannot
  retroactively deallocate existing used memory.
*/
DLMALLOC_EXPORT size_t dlmalloc_समुच्चयः_footprint_सीमा(size_t अष्टकानि);

#यदि MALLOC_INSPECT_ALL
/*
  malloc_inspect_all(void(*handler)(void *start,
                                    void *end,
                                    size_t used_bytes,
                                    void* callback_arg),
                      void* arg);
  Traverses the heap and calls the given handler for each managed
  region, skipping all bytes that are (or may be) used for bookkeeping
  purposes.  Traversal does not include include chunks that have been
  directly memory mapped. Each reported region begins at the start
  address, and continues up to but not including the end address.  The
  first used_bytes of the region contain allocated data. If
  used_bytes is zero, the region is unallocated. The handler is
  invoked with the given callback argument. If locks are defined, they
  are held during the entire traversal. It is a bad idea to invoke
  other malloc functions from within the handler.

  For example, to count the number of in-use chunks with size greater
  than 1000, you could write:
  static int count = 0;
  void count_chunks(void* start, void* end, size_t used, void* arg) {
    if (used >= 1000) ++count;
  }
  then:
    malloc_inspect_all(count_chunks, NULL);

  malloc_inspect_all is compiled only if MALLOC_INSPECT_ALL is defined.
*/
DLMALLOC_EXPORT शून्य dlmalloc_inspect_all(शून्य(*handler)(शून्य*, शून्य *, size_t, शून्य*),
                           शून्य* arg);

#यदि_अन्तः /* MALLOC_INSPECT_ALL */

#यदि !NO_MALLINFO
/*
  mallinfo()
  Returns (by copy) a struct containing various summary statistics:

  arena:     current total non-mmapped bytes allocated from system
  ordblks:   the number of free chunks
  smblks:    always zero.
  hblks:     current number of mmapped regions
  hblkhd:    total bytes held in mmapped regions
  usmblks:   the maximum total allocated space. This will be greater
                than current total if trimming has occurred.
  fsmblks:   always zero
  uordblks:  current total allocated space (normal or mmapped)
  fordblks:  total free space
  keepcost:  the maximum number of bytes that could ideally be released
               back to system via malloc_trim. ("ideally" means that
               it ignores page restrictions etc.)

  Because these fields are ints, but internal bookkeeping may
  be kept as longs, the reported values may wrap around zero and
  thus be inaccurate.
*/
DLMALLOC_EXPORT संरचना mallinfo dlmallinfo(शून्य);
#यदि_अन्तः /* NO_MALLINFO */

/*
  independent_calloc(size_t n_elements, size_t element_size, void* chunks[]);

  independent_calloc is similar to calloc, but instead of returning a
  single cleared space, it returns an array of pointers to n_elements
  independent elements that can hold contents of size elem_size, each
  of which starts out cleared, and can be independently freed,
  realloc'ed etc. The elements are guaranteed to be adjacently
  allocated (this is not guaranteed to occur with multiple callocs or
  mallocs), which may also improve cache locality in some
  applications.

  The "chunks" argument is optional (i.e., may be null, which is
  probably the most typical usage). If it is null, the returned array
  is itself dynamically allocated and should also be freed when it is
  no longer needed. Otherwise, the chunks array must be of at least
  n_elements in length. It is filled in with the pointers to the
  chunks.

  In either case, independent_calloc returns this pointer array, or
  null if the allocation failed.  If n_elements is zero and "chunks"
  is null, it returns a chunk representing an array with zero elements
  (which should be freed if not wanted).

  Each element must be freed when it is no longer needed. This can be
  done all at once using bulk_free.

  independent_calloc simplifies and speeds up implementations of many
  kinds of pools.  It may also be useful when constructing large data
  structures that initially have a fixed number of fixed-sized nodes,
  but the number is not known at compile time, and some of the nodes
  may later need to be freed. For example:

  struct Node { int item; struct Node* next; };

  struct Node* build_list() {
    struct Node** pool;
    int n = read_number_of_nodes_needed();
    if (n <= 0) return 0;
    pool = (struct Node**)(independent_calloc(n, sizeof(struct Node), 0);
    if (pool == 0) die();
    // organize into a linked list...
    struct Node* first = pool[0];
    for (i = 0; i < n-1; ++i)
      pool[i]->next = pool[i+1];
    free(pool);     // Can now free the array (or not, if it is needed later)
    return first;
  }
*/
DLMALLOC_EXPORT शून्य** dlindependent_calloc(size_t, size_t, शून्य**);

/*
  independent_comalloc(size_t n_elements, size_t sizes[], void* chunks[]);

  independent_comalloc allocates, all at once, a set of n_elements
  chunks with sizes indicated in the "sizes" array.    It returns
  an array of pointers to these elements, each of which can be
  independently freed, realloc'ed etc. The elements are guaranteed to
  be adjacently allocated (this is not guaranteed to occur with
  multiple callocs or mallocs), which may also improve cache locality
  in some applications.

  The "chunks" argument is optional (i.e., may be null). If it is null
  the returned array is itself dynamically allocated and should also
  be freed when it is no longer needed. Otherwise, the chunks array
  must be of at least n_elements in length. It is filled in with the
  pointers to the chunks.

  In either case, independent_comalloc returns this pointer array, or
  null if the allocation failed.  If n_elements is zero and chunks is
  null, it returns a chunk representing an array with zero elements
  (which should be freed if not wanted).

  Each element must be freed when it is no longer needed. This can be
  done all at once using bulk_free.

  independent_comallac differs from independent_calloc in that each
  element may have a different size, and also that it does not
  automatically clear elements.

  independent_comalloc can be used to speed up allocation in cases
  where several structs or objects must always be allocated at the
  same time.  For example:

  struct Head { ... }
  struct Foot { ... }

  void send_message(char* msg) {
    int msglen = strlen(msg);
    size_t sizes[3] = { sizeof(struct Head), msglen, sizeof(struct Foot) };
    void* chunks[3];
    if (independent_comalloc(3, sizes, chunks) == 0)
      die();
    struct Head* head = (struct Head*)(chunks[0]);
    char*        body = (char*)(chunks[1]);
    struct Foot* foot = (struct Foot*)(chunks[2]);
    // ...
  }

  In general though, independent_comalloc is worth using only for
  larger values of n_elements. For small values, you probably won't
  detect enough difference from series of malloc calls to bother.

  Overuse of independent_comalloc can increase overall memory usage,
  since it cannot reuse existing noncontiguous small chunks that
  might be available for some of the elements.
*/
DLMALLOC_EXPORT शून्य** dlindependent_comalloc(size_t, size_t*, शून्य**);

/*
  bulk_free(void* array[], size_t n_elements)
  Frees and clears (sets to null) each non-null pointer in the given
  array.  This is likely to be faster than freeing them one-by-one.
  If footers are used, pointers that have been allocated in different
  mspaces are not freed or cleared, and the count of all such pointers
  is returned.  For large arrays of pointers with poor locality, it
  may be worthwhile to sort this array before calling bulk_free.
*/
DLMALLOC_EXPORT size_t  dlbulk_free(शून्य**, size_t n_elements);

/*
  pvalloc(size_t n);
  Equivalent to valloc(minimum-page-that-holds(n)), that is,
  round up n to nearest pagesize.
 */
DLMALLOC_EXPORT शून्य*  dlpvalloc(size_t);

/*
  malloc_trim(size_t pad);

  If possible, gives memory back to the system (via negative arguments
  to sbrk) if there is unused memory at the `high' end of the malloc
  pool or in unused MMAP segments. You can call this after freeing
  large blocks of memory to potentially reduce the system-level memory
  requirements of a program. However, it cannot guarantee to reduce
  memory. Under some allocation patterns, some large free blocks of
  memory will be locked between two used chunks, so they cannot be
  given back to the system.

  The `pad' argument to malloc_trim represents the amount of free
  trailing space to leave untrimmed. If this argument is zero, only
  the minimum amount of memory to maintain internal data structures
  will be left. Non-zero arguments can be supplied to maintain enough
  trailing space to service future expected allocations without having
  to re-obtain memory from the system.

  Malloc_trim returns 1 if it actually released any memory, else 0.
*/
DLMALLOC_EXPORT पूर्णाङ्क  dlmalloc_trim(size_t);

/*
  malloc_stats();
  Prints on stderr the amount of space obtained from the system (both
  via sbrk and mmap), the maximum amount (which may be more than
  current if malloc_trim and/or munmap got called), and the current
  number of bytes allocated via malloc (or realloc, etc) but not yet
  freed. Note that this is the number of bytes allocated, not the
  number requested. It will be larger than the number requested
  because of alignment and bookkeeping overhead. Because it includes
  alignment wastage as being in use, this figure may be greater than
  zero even when no user-level chunks are allocated.

  The reported current and maximum system memory can be inaccurate if
  a program makes other calls to system memory allocation functions
  (normally sbrk) outside of malloc.

  malloc_stats prints only the most commonly interesting statistics.
  More information can be obtained by calling mallinfo.
*/
DLMALLOC_EXPORT शून्य  dlmalloc_stats(शून्य);

/*
  malloc_usable_size(void* p);

  Returns the number of bytes you can actually use in
  an allocated chunk, which may be more than you requested (although
  often not) due to alignment and minimum size constraints.
  You can use this many bytes without worrying about
  overwriting other allocated objects. This is not a particularly great
  programming practice. malloc_usable_size can be more useful in
  debugging and assertions, for example:

  p = malloc(n);
  assert(malloc_usable_size(p) >= 256);
*/
size_t dlmalloc_usable_आकार(शून्य*);

#यदि_अन्तः /* ONLY_MSPACES */

#यदि MSPACES

/*
  mspace is an opaque type representing an independent
  region of space that supports mspace_malloc, etc.
*/
प्रकारनिर्देश शून्य* mspace;

/*
  create_mspace creates and returns a new independent space with the
  given initial capacity, or, if 0, the default granularity size.  It
  returns null if there is no system memory available to create the
  space.  If argument locked is non-zero, the space uses a separate
  lock to control access. The capacity of the space will grow
  dynamically as needed to service mspace_malloc requests.  You can
  control the sizes of incremental increases of this space by
  compiling with a different DEFAULT_GRANULARITY or dynamically
  setting with mallopt(M_GRANULARITY, value).
*/
DLMALLOC_EXPORT mspace create_mspace(size_t क्षमता, पूर्णाङ्क locked);

/*
  destroy_mspace destroys the given space, and attempts to return all
  of its memory back to the system, returning the total number of
  bytes freed. After destruction, the results of access to all memory
  used by the space become undefined.
*/
DLMALLOC_EXPORT size_t destroy_mspace(mspace msp);

/*
  create_mspace_with_base uses the memory supplied as the initial base
  of a new mspace. Part (less than 128*sizeof(size_t) bytes) of this
  space is used for bookkeeping, so the capacity must be at least this
  large. (Otherwise 0 is returned.) When this initial space is
  exhausted, additional memory will be obtained from the system.
  Destroying this space will deallocate all additionally allocated
  space (if possible) but not the initial base.
*/
DLMALLOC_EXPORT mspace create_mspace_with_आधारः(शून्य* आधारः, size_t क्षमता, पूर्णाङ्क locked);

/*
  mspace_track_large_chunks controls whether requests for large chunks
  are allocated in their own untracked mmapped regions, separate from
  others in this mspace. By default large chunks are not tracked,
  which reduces fragmentation. However, such chunks are not
  necessarily released to the system upon destroy_mspace.  Enabling
  tracking by setting to true may increase fragmentation, but avoids
  leakage when relying on destroy_mspace to release all memory
  allocated using this space.  The function returns the previous
  setting.
*/
DLMALLOC_EXPORT पूर्णाङ्क mspace_track_large_chunks(mspace msp, पूर्णाङ्क enable);


/*
  mspace_malloc behaves as malloc, but operates within
  the given space.
*/
DLMALLOC_EXPORT शून्य* mspace_malloc(mspace msp, size_t अष्टकानि);

/*
  mspace_free behaves as free, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_free is not actually needed.
  free may be called instead of mspace_free because freed chunks from
  any space are handled by their originating spaces.
*/
DLMALLOC_EXPORT शून्य mspace_free(mspace msp, शून्य* mem);

/*
  mspace_realloc behaves as realloc, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_realloc is not actually
  needed.  realloc may be called instead of mspace_realloc because
  realloced chunks from any space are handled by their originating
  spaces.
*/
DLMALLOC_EXPORT शून्य* mspace_realloc(mspace msp, शून्य* mem, size_t newsize);

/*
  mspace_calloc behaves as calloc, but operates within
  the given space.
*/
DLMALLOC_EXPORT शून्य* mspace_calloc(mspace msp, size_t n_elements, size_t elem_आकार);

/*
  mspace_memalign behaves as memalign, but operates within
  the given space.
*/
DLMALLOC_EXPORT शून्य* mspace_memalign(mspace msp, size_t alignment, size_t अष्टकानि);

/*
  mspace_independent_calloc behaves as independent_calloc, but
  operates within the given space.
*/
DLMALLOC_EXPORT शून्य** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_आकार, शून्य* chunks[]);

/*
  mspace_independent_comalloc behaves as independent_comalloc, but
  operates within the given space.
*/
DLMALLOC_EXPORT शून्य** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], शून्य* chunks[]);

/*
  mspace_footprint() returns the number of bytes obtained from the
  system for this space.
*/
DLMALLOC_EXPORT size_t mspace_footprint(mspace msp);

/*
  mspace_max_footprint() returns the peak number of bytes obtained from the
  system for this space.
*/
DLMALLOC_EXPORT size_t mspace_max_footprint(mspace msp);


#यदि !NO_MALLINFO
/*
  mspace_mallinfo behaves as mallinfo, but reports properties of
  the given space.
*/
DLMALLOC_EXPORT संरचना mallinfo mspace_mallinfo(mspace msp);
#यदि_अन्तः /* NO_MALLINFO */

/*
  malloc_usable_size(void* p) behaves the same as malloc_usable_size;
*/
DLMALLOC_EXPORT size_t mspace_usable_आकार(ध्रुव शून्य* mem);

/*
  mspace_malloc_stats behaves as malloc_stats, but reports
  properties of the given space.
*/
DLMALLOC_EXPORT शून्य mspace_malloc_stats(mspace msp);

/*
  mspace_trim behaves as malloc_trim, but
  operates within the given space.
*/
DLMALLOC_EXPORT पूर्णाङ्क mspace_trim(mspace msp, size_t pad);

/*
  An alias for mallopt.
*/
DLMALLOC_EXPORT पूर्णाङ्क mspace_mallopt(पूर्णाङ्क, पूर्णाङ्क);

#यदि_अन्तः /* MSPACES */

#यदि_परिभाषितम् __cplusplus
}  /* end of extern "C" */
#यदि_अन्तः /* __cplusplus */

/*
  ========================================================================
  To make a fully customizable malloc.h header file, cut everything
  above this line, put into file malloc.h, edit to suit, and #include it
  on the next line, as well as in programs that use this malloc.
  ========================================================================
*/

/* #include "malloc.h" */

/*------------------------------ internal #includes ---------------------- */

#यदि_परिभाषितम् _MSC_VER
#निर्देशः warning( disable : 4146 ) /* no "unsigned" warnings */
#यदि_अन्तः /* _MSC_VER */
#यदि !NO_MALLOC_STATS
#अन्तर्भाव <stdio.h>       /* for printing in malloc_stats */
#यदि_अन्तः /* NO_MALLOC_STATS */
#यदि_अपरिभाषितम् LACKS_ERRNO_H
#अन्तर्भाव <errno.h>       /* for MALLOC_FAILURE_ACTION */
#यदि_अन्तः /* LACKS_ERRNO_H */
#यदि_परिभाषितम् DEBUG
#यदि ABORT_ON_ASSERT_FAILURE
#अपरिभाषय प्रतिज्ञा
#परिभाषय प्रतिज्ञा(x) यदि(!(x)) ABORT
#अन्यथा /* ABORT_ON_ASSERT_FAILURE */
#अन्तर्भाव <assert.h>
#यदि_अन्तः /* ABORT_ON_ASSERT_FAILURE */
#अन्यथा  /* DEBUG */
#यदि_अपरिभाषितम् प्रतिज्ञा
#परिभाषय प्रतिज्ञा(x)
#यदि_अन्तः
#परिभाषय DEBUG 0
#यदि_अन्तः /* DEBUG */
#यदि !defined(WIN32) && !defined(LACKS_TIME_H)
#अन्तर्भाव <time.h>        /* for magic initialization */
#यदि_अन्तः /* WIN32 */
#यदि_अपरिभाषितम् LACKS_STDLIB_H
#अन्तर्भाव <stdlib.h>      /* for abort() */
#यदि_अन्तः /* LACKS_STDLIB_H */
#यदि_अपरिभाषितम् LACKS_STRING_H
#अन्तर्भाव <string.h>      /* for memset etc */
#यदि_अन्तः  /* LACKS_STRING_H */
#यदि USE_BUILTIN_FFS
#यदि_अपरिभाषितम् LACKS_वाक्यानि_H
#अन्तर्भाव <strings.h>     /* for ffs */
#यदि_अन्तः /* LACKS_STRINGS_H */
#यदि_अन्तः /* USE_BUILTIN_FFS */
#यदि HAVE_MMAP
#यदि_अपरिभाषितम् LACKS_SYS_MMAN_H
/* On some versions of linux, mremap decl in mman.h needs __USE_GNU set */
#यदि (defined(linux) && !defined(__USE_GNU))
#परिभाषय __USE_GNU 1
#अन्तर्भाव <sys/mman.h>    /* for mmap */
#अपरिभाषय __USE_GNU
#अन्यथा
#अन्तर्भाव <sys/mman.h>    /* for mmap */
#यदि_अन्तः /* linux */
#यदि_अन्तः /* LACKS_SYS_MMAN_H */
#यदि_अपरिभाषितम् LACKS_FCNTL_H
#अन्तर्भाव <fcntl.h>
#यदि_अन्तः /* LACKS_FCNTL_H */
#यदि_अन्तः /* HAVE_MMAP */
#यदि_अपरिभाषितम् LACKS_UNISTD_H
#अन्तर्भाव <unistd.h>     /* for sbrk, sysconf */
#अन्यथा /* LACKS_UNISTD_H */
#यदि !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
बाह्य शून्य*     sbrk(ptrdiff_t);
#यदि_अन्तः /* FreeBSD etc */
#यदि_अन्तः /* LACKS_UNISTD_H */

/* Declarations for locking */
#यदि USE_LOCKS
#यदि_अपरिभाषितम् WIN32
#यदि defined (__SVR4) && defined (__sun)  /* solaris */
#अन्तर्भाव <thread.h>
#नोचेद्यदि !defined(LACKS_SCHED_H)
#अन्तर्भाव <sched.h>
#यदि_अन्तः /* solaris or LACKS_SCHED_H */
#यदि (defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0) || !USE_SPIN_LOCKS
#अन्तर्भाव <pthread.h>
#यदि_अन्तः /* USE_RECURSIVE_LOCKS ... */
#नोचेद्यदि defined(_MSC_VER)
#यदि_अपरिभाषितम् _M_AMD64
/* These are already defined on AMD64 builds */
#यदि_परिभाषितम् __cplusplus
बाह्य "C" {
#यदि_अन्तः /* __cplusplus */
LONG __cdecl _InterlockedCompareExchange(LONG चञ्चल *Dest, LONG Exchange, LONG Comp);
LONG __cdecl _InterlockedExchange(LONG चञ्चल *Target, LONG मानम्_2);
#यदि_परिभाषितम् __cplusplus
}
#यदि_अन्तः /* __cplusplus */
#यदि_अन्तः /* _M_AMD64 */
#निर्देशः intrinsic (_InterlockedCompareExchange)
#निर्देशः intrinsic (_InterlockedExchange)
#परिभाषय interlockedcompareexchange _InterlockedCompareExchange
#परिभाषय interlockedexchange _InterlockedExchange
#नोचेद्यदि defined(WIN32) && defined(__GNUC__)
#परिभाषय interlockedcompareexchange(a, b, c) __sync_val_compare_and_swap(a, c, b)
#परिभाषय interlockedexchange __sync_lock_test_and_set
#यदि_अन्तः /* Win32 */
#अन्यथा /* USE_LOCKS */
#यदि_अन्तः /* USE_LOCKS */

#यदि_अपरिभाषितम् तालकम्_AT_FORK
#परिभाषय तालकम्_AT_FORK 0
#यदि_अन्तः

/* Declarations for bit scanning on win32 */
#यदि defined(_MSC_VER) && _MSC_VER>=1300
#यदि_अपरिभाषितम् Bit_अवलोकनम्_Forward /* Try to avoid pulling in WinNT.h */
#यदि_परिभाषितम् __cplusplus
बाह्य "C" {
#यदि_अन्तः /* __cplusplus */
अचिह्नित वर्ण Bit_अवलोकनम्_Forward_2(अचिह्नित दीर्घ *सूचकांक, अचिह्नित दीर्घ आवरणम्);
अचिह्नित वर्ण Bit_अवलोकनम्_Reverse(अचिह्नित दीर्घ *सूचकांक, अचिह्नित दीर्घ आवरणम्);
#यदि_परिभाषितम् __cplusplus
}
#यदि_अन्तः /* __cplusplus */

#परिभाषय Bit_अवलोकनम्_Forward Bit_अवलोकनम्_Forward_2
#परिभाषय Bit_अवलोकनम्_Reverse_2 Bit_अवलोकनम्_Reverse
#निर्देशः intrinsic(Bit_अवलोकनम्_Forward_2)
#निर्देशः intrinsic(Bit_अवलोकनम्_Reverse)
#यदि_अन्तः /* BitScanForward */
#यदि_अन्तः /* defined(_MSC_VER) && _MSC_VER>=1300 */

#यदि_अपरिभाषितम् WIN32
#यदि_अपरिभाषितम् malloc_getpagesize
#  ifdef _SC_PAGESIZE         /* some SVR4 systems omit an underscore */
#    ifndef SC_PAGE_आकार
#      define SC_PAGE_आकार _SC_PAGESIZE
#    endif
#  endif
#  ifdef SC_PAGE_आकार
#    define malloc_getpagesize sysconf(SC_PAGE_आकार)
#  अन्यथा
#    यदि defined(BSD) || defined(DGUX) || defined(HAVE_GETPAGESIZE)
       बाह्य size_t getpagesize();
#      define malloc_getpagesize getpagesize()
#    अन्यथा
#      ifdef WIN32 /* use supplied emulation of getpagesize */
#        define malloc_getpagesize getpagesize()
#      अन्यथा
#        ifndef LACKS_SYS_PARAM_H
#          include <sys/param.h>
#        endif
#        ifdef EXEC_PAGESIZE
#          define malloc_getpagesize EXEC_PAGESIZE
#        अन्यथा
#          ifdef NBPG
#            ifndef CLSIZE
#              define malloc_getpagesize NBPG
#            अन्यथा
#              define malloc_getpagesize (NBPG * CLSIZE)
#            endif
#          अन्यथा
#            ifdef NBPC
#              define malloc_getpagesize NBPC
#            अन्यथा
#              ifdef PAGESIZE
#                define malloc_getpagesize PAGESIZE
#              अन्यथा /* just guess */
#                define malloc_getpagesize ((size_t)4096U)
#              endif
#            endif
#          endif
#        endif
#      endif
#    endif
#  endif
#यदि_अन्तः
#यदि_अन्तः

/* libffi: the upstream malloc_getpagesize definition above is gated on
   !WIN32 (WIN32 builds use GetSystemInfo in init_mparams instead).  Guarantee
   the symbol is always defined so the !WIN32 init_mparams branch still compiles
   on toolchains where it is not preprocessed out as expected (e.g. MSVC).  */
#यदि_अपरिभाषितम् malloc_getpagesize
#परिभाषय malloc_getpagesize ((size_t)4096U)
#यदि_अन्तः

/* ------------------- size_t and alignment properties -------------------- */

/* The byte and bit size of a size_t */
#परिभाषय आकार_T_आकार         (परिमाण(size_t))
#परिभाषय आकार_T_BITSIZE      (परिमाण(size_t) << 3)

/* Some constants coerced to size_t */
/* Annoying but necessary to avoid errors on some platforms */
#परिभाषय आकार_T_ZERO         ((size_t)0)
#परिभाषय आकार_T_ONE          ((size_t)1)
#परिभाषय आकार_T_TWO          ((size_t)2)
#परिभाषय आकार_T_FOUR         ((size_t)4)
#परिभाषय TWO_आकार_T_SIZES    (आकार_T_आकार<<1)
#परिभाषय FOUR_आकार_T_SIZES   (आकार_T_आकार<<2)
#परिभाषय SIX_आकार_T_SIZES    (FOUR_आकार_T_SIZES+TWO_आकार_T_SIZES)
#परिभाषय HALF_MAX_आकार_T     (MAX_आकार_T / 2U)

/* The bit mask value corresponding to MALLOC_ALIGNMENT */
#परिभाषय CHUNK_ALIGN_आवरणम्    (MALLOC_ALIGNMENT - आकार_T_ONE)

/* True if address a has acceptable alignment */
#परिभाषय is_aligned(A)       (((size_t)((A)) & (CHUNK_ALIGN_आवरणम्)) == 0)

/* the number of bytes to offset an address to align it */
#परिभाषय align_विस्थापनम्(A)\
 ((((size_t)(A) & CHUNK_ALIGN_आवरणम्) == 0)? 0 :\
  ((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_आवरणम्)) & CHUNK_ALIGN_आवरणम्))

/* -------------------------- MMAP preliminaries ------------------------- */

/*
   If HAVE_MORECORE or HAVE_MMAP are false, we just define calls and
   checks to fail so compiler optimizer can delete code rather than
   using so many "#if"s.
*/


/* MORECORE and MMAP must return MFAIL on failure */
#परिभाषय MFAIL                ((शून्य*)(MAX_आकार_T))
#परिभाषय CMFAIL               ((वर्ण*)(MFAIL)) /* defined for convenience */

#यदि HAVE_MMAP

#यदि !defined(WIN32) && !defined (__OS2__)
#परिभाषय MUNMAP_DEFAULT(a, s)  munmap((a), (s))
#परिभाषय MMAP_PROT            (PROT_READ|PROT_WRITE)
#यदि !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#परिभाषय MAP_ANONYMOUS        MAP_ANON
#यदि_अन्तः /* MAP_ANON */
#यदि_परिभाषितम् MAP_ANONYMOUS
#परिभाषय MMAP_FLAGS           (MAP_वैयक्तिकम्|MAP_ANONYMOUS)
#परिभाषय MMAP_DEFAULT(s)       mmap(0, (s), MMAP_PROT, MMAP_FLAGS, -1, 0)
#अन्यथा /* MAP_ANONYMOUS */
/*
   Nearly all versions of mmap support MAP_ANONYMOUS, so the following
   is unlikely to be needed, but is supplied just in case.
*/
#परिभाषय MMAP_FLAGS           (MAP_वैयक्तिकम्)
स्थिर पूर्णाङ्क dev_zero_fd = -1; /* Cached file descriptor for /dev/zero. */
#परिभाषय MMAP_DEFAULT(s) ((dev_zero_fd < 0) ? \
           (dev_zero_fd = उद्घाटय("/dev/zero", O_RDWR), \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0)) : \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0))
#यदि_अन्तः /* MAP_ANONYMOUS */

#परिभाषय प्रत्यक्षम्_MMAP_DEFAULT(s) MMAP_DEFAULT(s)

#नोचेद्यदि defined(__OS2__)

/* OS/2 MMAP via DosAllocMem */
स्थिर शून्य* os2mmap(size_t आकार) {
  शून्य* ptr;
  यदि (DosAllocMem(&ptr, आकार, OBJ_ANY|PAG_COMMIT|PAG_READ|PAG_WRITE) &&
      DosAllocMem(&ptr, आकार, PAG_COMMIT|PAG_READ|PAG_WRITE))
    प्रत्यागम MFAIL;
  प्रत्यागम ptr;
}

#परिभाषय os2direct_mmap(n)     os2mmap(n)

/* This function supports releasing coalesed segments */
स्थिर पूर्णाङ्क os2munmap(शून्य* ptr, size_t आकार) {
  यावत् (आकार) {
    ULONG ul_आकार = आकार;
    ULONG ulFlags = 0;
    यदि (DosQueryMem(ptr, &ul_आकार, &ulFlags) != 0)
      प्रत्यागम -1;
    यदि ((ulFlags & PAG_आधारः) == 0 ||(ulFlags & PAG_COMMIT) == 0 ||
        ul_आकार > आकार)
      प्रत्यागम -1;
    यदि (DosFreeMem(ptr) != 0)
      प्रत्यागम -1;
    ptr = ( शून्य * ) ( ( वर्ण * ) ptr + ul_आकार );
    आकार -= ul_आकार;
  }
  प्रत्यागम 0;
}

#परिभाषय MMAP_DEFAULT(s)         os2mmap(s)
#परिभाषय MUNMAP_DEFAULT(a, s)    os2munmap((a), (s))
#परिभाषय प्रत्यक्षम्_MMAP_DEFAULT(s)  os2direct_mmap(s)

#अन्यथा /* WIN32 */

/* Win32 MMAP via VirtualAlloc */
/* libffi: plain static (not FORCEINLINE) -- mingw's <windows.h> defines
   FORCEINLINE with an 'extern' storage class, which clashes with 'static'.  */
स्थिर शून्य* win32mmap(size_t आकार) {
  शून्य* ptr = VirtualAlloc(0, आकार, MEM_RESERVE|MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  प्रत्यागम (ptr != 0)? ptr: MFAIL;
}

/* For direct MMAP, use MEM_TOP_DOWN to minimize interference */
स्थिर शून्य* win32direct_mmap(size_t आकार) {
  शून्य* ptr = VirtualAlloc(0, आकार, MEM_RESERVE|MEM_COMMIT|MEM_शिखरम्_अधस्तात्,
                           PAGE_EXECUTE_READWRITE);
  प्रत्यागम (ptr != 0)? ptr: MFAIL;
}

/* This function supports releasing coalesed segments */
स्थिर पूर्णाङ्क win32munmap(शून्य* ptr, size_t आकार) {
  स्मृतिः_BASIC_INFORMATION minfo;
  वर्ण* cptr = (वर्ण*)ptr;
  यावत् (आकार) {
    यदि (VirtualQuery(cptr, &minfo, परिमाण(minfo)) == 0)
      प्रत्यागम -1;
    यदि (minfo.आधारः_स्थानाङ्कः != cptr || minfo.आवण्टनम्_आधारः != cptr ||
        minfo.State != MEM_COMMIT || minfo.Region_आकार > आकार)
      प्रत्यागम -1;
    यदि (VirtualFree(cptr, 0, MEM_RELEASE) == 0)
      प्रत्यागम -1;
    cptr += minfo.Region_आकार;
    आकार -= minfo.Region_आकार;
  }
  प्रत्यागम 0;
}

#परिभाषय MMAP_DEFAULT(s)             win32mmap(s)
#परिभाषय MUNMAP_DEFAULT(a, s)        win32munmap((a), (s))
#परिभाषय प्रत्यक्षम्_MMAP_DEFAULT(s)      win32direct_mmap(s)
#यदि_अन्तः /* WIN32 */
#यदि_अन्तः /* HAVE_MMAP */

#यदि HAVE_MREMAP
#यदि_अपरिभाषितम् WIN32
#परिभाषय MREMAP_DEFAULT(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv))
#यदि_अन्तः /* WIN32 */
#यदि_अन्तः /* HAVE_MREMAP */

/**
 * Define CALL_MORECORE
 */
#यदि HAVE_MORECORE
    #यदि_परिभाषितम् MORECORE
        #परिभाषय CALL_MORECORE(S)    MORECORE(S)
    #अन्यथा  /* MORECORE */
        #परिभाषय CALL_MORECORE(S)    MORECORE_DEFAULT(S)
    #यदि_अन्तः /* MORECORE */
#अन्यथा  /* HAVE_MORECORE */
    #परिभाषय CALL_MORECORE(S)        MFAIL
#यदि_अन्तः /* HAVE_MORECORE */

/**
 * Define CALL_MMAP/CALL_MUNMAP/CALL_DIRECT_MMAP
 */
#यदि HAVE_MMAP
    #परिभाषय USE_MMAP_BIT            (आकार_T_ONE)

    #यदि_परिभाषितम् MMAP
        #परिभाषय CALL_MMAP(s)        MMAP(s)
    #अन्यथा /* MMAP */
        #परिभाषय CALL_MMAP(s)        MMAP_DEFAULT(s)
    #यदि_अन्तः /* MMAP */
    #यदि_परिभाषितम् MUNMAP
        #परिभाषय CALL_MUNMAP(a, s)   MUNMAP((a), (s))
    #अन्यथा /* MUNMAP */
        #परिभाषय CALL_MUNMAP(a, s)   MUNMAP_DEFAULT((a), (s))
    #यदि_अन्तः /* MUNMAP */
    #यदि_परिभाषितम् प्रत्यक्षम्_MMAP
        #परिभाषय CALL_प्रत्यक्षम्_MMAP(s) प्रत्यक्षम्_MMAP(s)
    #अन्यथा /* DIRECT_MMAP */
        #परिभाषय CALL_प्रत्यक्षम्_MMAP(s) प्रत्यक्षम्_MMAP_DEFAULT(s)
    #यदि_अन्तः /* DIRECT_MMAP */
#अन्यथा  /* HAVE_MMAP */
    #परिभाषय USE_MMAP_BIT            (आकार_T_ZERO)

    #परिभाषय MMAP(s)                 MFAIL
    #परिभाषय MUNMAP(a, s)            (-1)
    #परिभाषय प्रत्यक्षम्_MMAP(s)          MFAIL
    #परिभाषय CALL_प्रत्यक्षम्_MMAP(s)     प्रत्यक्षम्_MMAP(s)
    #परिभाषय CALL_MMAP(s)            MMAP(s)
    #परिभाषय CALL_MUNMAP(a, s)       MUNMAP((a), (s))
#यदि_अन्तः /* HAVE_MMAP */

/**
 * Define CALL_MREMAP
 */
#यदि HAVE_MMAP && HAVE_MREMAP
    #यदि_परिभाषितम् MREMAP
        #परिभाषय CALL_MREMAP(addr, osz, nsz, mv) MREMAP((addr), (osz), (nsz), (mv))
    #अन्यथा /* MREMAP */
        #परिभाषय CALL_MREMAP(addr, osz, nsz, mv) MREMAP_DEFAULT((addr), (osz), (nsz), (mv))
    #यदि_अन्तः /* MREMAP */
#अन्यथा  /* HAVE_MMAP && HAVE_MREMAP */
    #परिभाषय CALL_MREMAP(addr, osz, nsz, mv)     MFAIL
#यदि_अन्तः /* HAVE_MMAP && HAVE_MREMAP */

/* mstate bit set if contiguous morecore disabled or failed */
#परिभाषय USE_NONCONTIGUOUS_BIT (4U)

/* segment bit set in create_mspace_with_base */
#परिभाषय EXTERN_BIT            (8U)


/* --------------------------- Lock preliminaries ------------------------ */

/*
  When locks are defined, there is one global lock, plus
  one per-mspace lock.

  The global lock_ensures that mparams.magic and other unique
  mparams values are initialized only once. It also protects
  sequences of calls to MORECORE.  In many cases sys_alloc requires
  two calls, that should not be interleaved with calls by other
  threads.  This does not protect against direct calls to MORECORE
  by other threads not using this lock, so there is still code to
  cope the best we can on interference.

  Per-mspace locks surround calls to malloc, free, etc.
  By default, locks are simple non-reentrant mutexes.

  Because lock-protected regions generally have bounded times, it is
  OK to use the supplied simple spinlocks. Spinlocks are likely to
  improve performance for lightly contended applications, but worsen
  performance under heavy contention.

  If USE_LOCKS is > 1, the definitions of lock routines here are
  bypassed, in which case you will need to define the type MLOCK_T,
  and at least INITIAL_LOCK, DESTROY_LOCK, ACQUIRE_LOCK, RELEASE_LOCK
  and TRY_LOCK.  You must also declare a
    static MLOCK_T malloc_global_mutex = { initialization values };.

*/

#यदि !USE_LOCKS
#परिभाषय USE_तालकम्_BIT               (0U)
#परिभाषय INITIAL_तालकम्(l)            (0)
#परिभाषय DESTROY_तालकम्(l)            (0)
#परिभाषय ACQUIRE_MALLOC_GLOBAL_तालकम्()
#परिभाषय RELEASE_MALLOC_GLOBAL_तालकम्()

#अन्यथा
#यदि USE_LOCKS > 1
/* -----------------------  User-defined locks ------------------------ */
/* Define your own lock implementation here */
/* #define INITIAL_LOCK(lk)  ... */
/* #define DESTROY_LOCK(lk)  ... */
/* #define ACQUIRE_LOCK(lk)  ... */
/* #define RELEASE_LOCK(lk)  ... */
/* #define TRY_LOCK(lk) ... */
/* static MLOCK_T malloc_global_mutex = ... */

#नोचेद्यदि USE_SPIN_LOCKS

/* First, define CAS_LOCK and CLEAR_LOCK on ints */
/* Note CAS_LOCK defined to return 0 on success */

#यदि defined(__GNUC__)&& (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#परिभाषय CAS_तालकम्(sl)     __sync_lock_test_and_set(sl, 1)
#परिभाषय CLEAR_तालकम्(sl)   __sync_lock_release(sl)

#नोचेद्यदि (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
/* Custom spin locks for older gcc on x86 */
स्थिर FORCEINLINE पूर्णाङ्क x86_cas_तालकम्(पूर्णाङ्क *sl) {
  पूर्णाङ्क ret;
  पूर्णाङ्क val = 1;
  पूर्णाङ्क cmp = 0;
  __asm__ __volatile__  ("lock; cmpxchgl %1, %2"
                         : "=a" (ret)
                         : "r" (val), "m" (*(sl)), "0"(cmp)
                         : "memory", "cc");
  प्रत्यागम ret;
}

स्थिर FORCEINLINE शून्य x86_clear_तालकम्(पूर्णाङ्क* sl) {
  प्रतिज्ञा(*sl != 0);
  पूर्णाङ्क पूर्व = 0;
  पूर्णाङ्क ret;
  __asm__ __volatile__ ("lock; xchgl %0, %1"
                        : "=r" (ret)
                        : "m" (*(sl)), "0"(पूर्व)
                        : "memory");
}

#परिभाषय CAS_तालकम्(sl)     x86_cas_तालकम्(sl)
#परिभाषय CLEAR_तालकम्(sl)   x86_clear_तालकम्(sl)

#अन्यथा /* Win32 MSC */
#परिभाषय CAS_तालकम्(sl)     interlockedexchange((LONG चञ्चल *)(sl), (LONG)1)
#परिभाषय CLEAR_तालकम्(sl)   interlockedexchange ((LONG चञ्चल *)(sl), (LONG)0)

#यदि_अन्तः /* ... gcc spins locks ... */

/* How to yield for a spin lock */
#परिभाषय SPINS_प्रत्येकम्_YIELD       63
#यदि defined(_MSC_VER)
#परिभाषय SLEEP_EX_DURATION     50 /* delay for yield/sleep */
#परिभाषय SPIN_तालकम्_YIELD  SleepEx(SLEEP_EX_DURATION, FALSE)
#नोचेद्यदि defined (__SVR4) && defined (__sun) /* solaris */
#परिभाषय SPIN_तालकम्_YIELD   thr_yield();
#नोचेद्यदि !defined(LACKS_SCHED_H)
#परिभाषय SPIN_तालकम्_YIELD   sched_yield();
#अन्यथा
#परिभाषय SPIN_तालकम्_YIELD
#यदि_अन्तः /* ... yield ... */

/* libffi: read the spin-lock word with a relaxed atomic load for the lock-free
   "is it held?" peek, so ThreadSanitizer does not flag it as racing with the
   atomic CLEAR_LOCK release on unlock.  Relaxed ordering is sufficient: the
   peek is only a hint to avoid a costly CAS, and CAS_LOCK still provides the
   acquire ordering on success.  Falls back to the original volatile read when
   atomic builtins are unavailable.  */
#यदि defined(__ATOMIC_RELAXED)
# define ffi_spin_peek(sl) __atomic_load_n((sl), __ATOMIC_RELAXED)
#नोचेद्यदि defined(_MSC_VER)
# define ffi_spin_peek(sl) \
  ((पूर्णाङ्क)_InterlockedCompareExchange((LONG चञ्चल *)(sl), (LONG)0, (LONG)0))
#अन्यथा
# define ffi_spin_peek(sl) (*(चञ्चल पूर्णाङ्क *)(sl))
#यदि_अन्तः

#यदि !defined(USE_RECURSIVE_LOCKS) || USE_RECURSIVE_LOCKS == 0
/* Plain spin locks use single word (embedded in malloc_states) */
स्थिर पूर्णाङ्क spin_acquire_तालकम्(पूर्णाङ्क *sl) {
  पूर्णाङ्क spins = 0;
  यावत् (ffi_spin_peek(sl) != 0 || CAS_तालकम्(sl)) {
    यदि ((++spins & SPINS_प्रत्येकम्_YIELD) == 0) {
      SPIN_तालकम्_YIELD;
    }
  }
  प्रत्यागम 0;
}

#परिभाषय MLOCK_T               पूर्णाङ्क
#परिभाषय TRY_तालकम्(sl)          !CAS_तालकम्(sl)
#परिभाषय RELEASE_तालकम्(sl)      CLEAR_तालकम्(sl)
#परिभाषय ACQUIRE_तालकम्(sl)      (CAS_तालकम्(sl)? spin_acquire_तालकम्(sl) : 0)
#परिभाषय INITIAL_तालकम्(sl)      (*sl = 0)
#परिभाषय DESTROY_तालकम्(sl)      (0)
स्थिर MLOCK_T malloc_global_mutex = 0;

#अन्यथा /* USE_RECURSIVE_LOCKS */
/* types for lock owners */
#यदि_परिभाषितम् WIN32
#परिभाषय THREAD_ID_T           DWORD
#परिभाषय CURRENT_THREAD        GetCurrentThreadId()
#परिभाषय EQ_OWNER(X,Y)         ((X) == (Y))
#अन्यथा
/*
  Note: the following assume that pthread_t is a type that can be
  initialized to (casted) zero. If this is not the case, you will need to
  somehow redefine these or not use spin locks.
*/
#परिभाषय THREAD_ID_T           pthread_t
#परिभाषय CURRENT_THREAD        pthread_self()
#परिभाषय EQ_OWNER(X,Y)         pthread_equal(X, Y)
#यदि_अन्तः

संरचना malloc_recursive_तालकम् {
  पूर्णाङ्क sl;
  अचिह्नित पूर्णाङ्क c;
  THREAD_ID_T threadid;
};

#परिभाषय MLOCK_T  संरचना malloc_recursive_तालकम्
स्थिर MLOCK_T malloc_global_mutex = { 0, 0, (THREAD_ID_T)0};

स्थिर FORCEINLINE शून्य recursive_release_तालकम्(MLOCK_T *lk) {
  प्रतिज्ञा(lk->sl != 0);
  यदि (--lk->c == 0) {
    CLEAR_तालकम्(&lk->sl);
  }
}

स्थिर FORCEINLINE पूर्णाङ्क recursive_acquire_तालकम्(MLOCK_T *lk) {
  THREAD_ID_T mythreadid = CURRENT_THREAD;
  पूर्णाङ्क spins = 0;
  प्रति (;;) {
    यदि (ffi_spin_peek(&lk->sl) == 0) {
      यदि (!CAS_तालकम्(&lk->sl)) {
        lk->threadid = mythreadid;
        lk->c = 1;
        प्रत्यागम 0;
      }
    }
    अन्यथा यदि (EQ_OWNER(lk->threadid, mythreadid)) {
      ++lk->c;
      प्रत्यागम 0;
    }
    यदि ((++spins & SPINS_प्रत्येकम्_YIELD) == 0) {
      SPIN_तालकम्_YIELD;
    }
  }
}

स्थिर FORCEINLINE पूर्णाङ्क recursive_try_तालकम्(MLOCK_T *lk) {
  THREAD_ID_T mythreadid = CURRENT_THREAD;
  यदि (ffi_spin_peek(&lk->sl) == 0) {
    यदि (!CAS_तालकम्(&lk->sl)) {
      lk->threadid = mythreadid;
      lk->c = 1;
      प्रत्यागम 1;
    }
  }
  अन्यथा यदि (EQ_OWNER(lk->threadid, mythreadid)) {
    ++lk->c;
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
}

#परिभाषय RELEASE_तालकम्(lk)      recursive_release_तालकम्(lk)
#परिभाषय TRY_तालकम्(lk)          recursive_try_तालकम्(lk)
#परिभाषय ACQUIRE_तालकम्(lk)      recursive_acquire_तालकम्(lk)
#परिभाषय INITIAL_तालकम्(lk)      ((lk)->threadid = (THREAD_ID_T)0, (lk)->sl = 0, (lk)->c = 0)
#परिभाषय DESTROY_तालकम्(lk)      (0)
#यदि_अन्तः /* USE_RECURSIVE_LOCKS */

#नोचेद्यदि defined(WIN32) /* Win32 critical sections */
#परिभाषय MLOCK_T               CRITICAL_SECTION
#परिभाषय ACQUIRE_तालकम्(lk)      (EnterCriticalSection(lk), 0)
#परिभाषय RELEASE_तालकम्(lk)      LeaveCriticalSection(lk)
#परिभाषय TRY_तालकम्(lk)          TryEnterCriticalSection(lk)
#परिभाषय INITIAL_तालकम्(lk)      (!Initialize_Critical_Section_And_Spin_गणनम्((lk), 0x80000000|4000))
#परिभाषय DESTROY_तालकम्(lk)      (DeleteCriticalSection(lk), 0)
#परिभाषय NEED_GLOBAL_तालकम्_INIT

स्थिर MLOCK_T malloc_global_mutex;
स्थिर चञ्चल LONG malloc_global_mutex_स्थितिः;

/* Use spin loop to initialize global lock */
स्थिर शून्य init_malloc_global_mutex() {
  प्रति (;;) {
    दीर्घ stat = malloc_global_mutex_स्थितिः;
    यदि (stat > 0)
      प्रत्यागम;
    /* transition to < 0 while initializing, then to > 0) */
    यदि (stat == 0 &&
        interlockedcompareexchange(&malloc_global_mutex_स्थितिः, (LONG)-1, (LONG)0) == 0) {
      InitializeCriticalSection(&malloc_global_mutex);
      interlockedexchange(&malloc_global_mutex_स्थितिः, (LONG)1);
      प्रत्यागम;
    }
    SleepEx(0, FALSE);
  }
}

#अन्यथा /* pthreads-based locks */
#परिभाषय MLOCK_T               pthread_mutex_t
#परिभाषय ACQUIRE_तालकम्(lk)      pthread_mutex_तालकम्(lk)
#परिभाषय RELEASE_तालकम्(lk)      pthread_mutex_unlock(lk)
#परिभाषय TRY_तालकम्(lk)          (!pthread_mutex_trylock(lk))
#परिभाषय INITIAL_तालकम्(lk)      pthread_init_तालकम्(lk)
#परिभाषय DESTROY_तालकम्(lk)      pthread_mutex_destroy(lk)

#यदि defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0 && defined(linux) && !defined(PTHREAD_MUTEX_RECURSIVE)
/* Cope with old-style linux recursive lock initialization by adding */
/* skipped internal declaration from pthread.h */
बाह्य पूर्णाङ्क pthread_mutexattr_setkind_np __P ((pthread_mutexattr_t *__attr,
                                              पूर्णाङ्क __kind));
#परिभाषय PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#परिभाषय pthread_mutexattr_settype(x,y) pthread_mutexattr_setkind_np(x,y)
#यदि_अन्तः /* USE_RECURSIVE_LOCKS ... */

स्थिर MLOCK_T malloc_global_mutex = PTHREAD_MUTEX_INITIALIZER;

स्थिर पूर्णाङ्क pthread_init_तालकम् (MLOCK_T *lk) {
  pthread_mutexattr_t attr;
  यदि (pthread_mutexattr_init(&attr)) प्रत्यागम 1;
#यदि defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0
  यदि (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) प्रत्यागम 1;
#यदि_अन्तः
  यदि (pthread_mutex_init(lk, &attr)) प्रत्यागम 1;
  यदि (pthread_mutexattr_destroy(&attr)) प्रत्यागम 1;
  प्रत्यागम 0;
}

#यदि_अन्तः /* ... lock types ... */

/* Common code for all lock types */
#परिभाषय USE_तालकम्_BIT               (2U)

#यदि_अपरिभाषितम् ACQUIRE_MALLOC_GLOBAL_तालकम्
#परिभाषय ACQUIRE_MALLOC_GLOBAL_तालकम्()  ACQUIRE_तालकम्(&malloc_global_mutex);
#यदि_अन्तः

#यदि_अपरिभाषितम् RELEASE_MALLOC_GLOBAL_तालकम्
#परिभाषय RELEASE_MALLOC_GLOBAL_तालकम्()  RELEASE_तालकम्(&malloc_global_mutex);
#यदि_अन्तः

#यदि_अन्तः /* USE_LOCKS */

/* -----------------------  Chunk representations ------------------------ */

/*
  (The following includes lightly edited explanations by Colin Plumb.)

  The malloc_chunk declaration below is misleading (but accurate and
  necessary).  It declares a "view" into memory allowing access to
  necessary fields at known offsets from a given base.

  Chunks of memory are maintained using a `boundary tag' method as
  originally described by Knuth.  (See the paper by Paul Wilson
  ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps for a survey of such
  techniques.)  Sizes of free chunks are stored both in the front of
  each chunk and at the end.  This makes consolidating fragmented
  chunks into bigger chunks fast.  The head fields also hold bits
  representing whether chunks are free or in use.

  Here are some pictures to make it clearer.  They are "exploded" to
  show that the state of a chunk can be thought of as extending from
  the high 31 bits of the head field of its header through the
  prev_foot and PINUSE_BIT bit of the following chunk header.

  A chunk that's in use looks like:

   chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
           | Size of previous chunk (if P = 0)                             |
           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |P|
         | Size of this chunk                                         1| +-+
   mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         |                                                               |
         +-                                                             -+
         |                                                               |
         +-                                                             -+
         |                                                               :
         +-      size - sizeof(size_t) available payload bytes          -+
         :                                                               |
 chunk-> +-                                                             -+
         |                                                               |
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |1|
       | Size of next chunk (may or may not be in use)               | +-+
 mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    And if it's free, it looks like this:

   chunk-> +-                                                             -+
           | User payload (must be in use, or we would have merged!)       |
           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |P|
         | Size of this chunk                                         0| +-+
   mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         | Next pointer                                                  |
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         | Prev pointer                                                  |
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         |                                                               :
         +-      size - sizeof(struct chunk) unused bytes               -+
         :                                                               |
 chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         | Size of this chunk                                            |
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |0|
       | Size of next chunk (must be in use, or we would have merged)| +-+
 mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                                                               :
       +- User payload                                                -+
       :                                                               |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                                                     |0|
                                                                     +-+
  Note that since we always merge adjacent free chunks, the chunks
  adjacent to a free chunk must be in use.

  Given a pointer to a chunk (which can be derived trivially from the
  payload pointer) we can, in O(1) time, find out whether the adjacent
  chunks are free, and if so, unlink them from the lists that they
  are on and merge them with the current chunk.

  Chunks always begin on even word boundaries, so the mem portion
  (which is returned to the user) is also on an even word boundary, and
  thus at least double-word aligned.

  The P (PINUSE_BIT) bit, stored in the unused low-order bit of the
  chunk size (which is always a multiple of two words), is an in-use
  bit for the *previous* chunk.  If that bit is *clear*, then the
  word before the current chunk size contains the previous chunk
  size, and can be used to find the front of the previous chunk.
  The very first chunk allocated always has this bit set, preventing
  access to non-existent (or non-owned) memory. If pinuse is set for
  any given chunk, then you CANNOT determine the size of the
  previous chunk, and might even get a memory addressing fault when
  trying to do so.

  The C (CINUSE_BIT) bit, stored in the unused second-lowest bit of
  the chunk size redundantly records whether the current chunk is
  inuse (unless the chunk is mmapped). This redundancy enables usage
  checks within free and realloc, and reduces indirection when freeing
  and consolidating chunks.

  Each freshly allocated chunk must have both cinuse and pinuse set.
  That is, each allocated chunk borders either a previously allocated
  and still in-use chunk, or the base of its memory arena. This is
  ensured by making all allocations from the `lowest' part of any
  found chunk.  Further, no free chunk physically borders another one,
  so each free chunk is known to be preceded and followed by either
  inuse chunks or the ends of memory.

  Note that the `foot' of the current chunk is actually represented
  as the prev_foot of the NEXT chunk. This makes it easier to
  deal with alignments etc but can be very confusing when trying
  to extend or adapt this code.

  The exceptions to all this are

     1. The special chunk `top' is the top-most available chunk (i.e.,
        the one bordering the end of available memory). It is treated
        specially.  Top is never included in any bin, is used only if
        no other chunk is available, and is released back to the
        system if it is very large (see M_TRIM_THRESHOLD).  In effect,
        the top chunk is treated as larger (and thus less well
        fitting) than any other available chunk.  The top chunk
        doesn't update its trailing size field since there is no next
        contiguous chunk that would have to index off it. However,
        space is still allocated for it (TOP_FOOT_SIZE) to enable
        separation or merging when space is extended.

     3. Chunks allocated via mmap, have both cinuse and pinuse bits
        cleared in their head fields.  Because they are allocated
        one-by-one, each must carry its own prev_foot field, which is
        also used to hold the offset this chunk has within its mmapped
        region, which is needed to preserve alignment. Each mmapped
        chunk is trailed by the first two fields of a fake next-chunk
        for sake of usage checks.

*/

संरचना malloc_chunk {
  size_t               पूर्व_foot;  /* Size of previous chunk (if free).  */
  size_t               शीर्ष;       /* Size and inuse bits. */
  संरचना malloc_chunk* fd;         /* double links -- used only if free. */
  संरचना malloc_chunk* bk;
};

प्रकारनिर्देश संरचना malloc_chunk  mchunk;
प्रकारनिर्देश संरचना malloc_chunk* mchunkptr;
प्रकारनिर्देश संरचना malloc_chunk* sbinptr;  /* The type of bins of chunks */
प्रकारनिर्देश अचिह्नित पूर्णाङ्क bindex_t;         /* Described below */
प्रकारनिर्देश अचिह्नित पूर्णाङ्क binmap_t;         /* Described below */
प्रकारनिर्देश अचिह्नित पूर्णाङ्क flag_t;           /* The type of various bit flag sets */

/* ------------------- Chunks sizes and alignments ----------------------- */

#परिभाषय MCHUNK_आकार         (परिमाण(mchunk))

#यदि FOOTERS
#परिभाषय CHUNK_OVERHEAD      (TWO_आकार_T_SIZES)
#अन्यथा /* FOOTERS */
#परिभाषय CHUNK_OVERHEAD      (आकार_T_आकार)
#यदि_अन्तः /* FOOTERS */

/* MMapped chunks need a second word of overhead ... */
#परिभाषय MMAP_CHUNK_OVERHEAD (TWO_आकार_T_SIZES)
/* ... and additional padding for fake next-chunk at foot */
#परिभाषय MMAP_FOOT_PAD       (FOUR_आकार_T_SIZES)

/* The smallest size we can malloc is an aligned minimal chunk */
#परिभाषय MIN_CHUNK_आकार\
  ((MCHUNK_आकार + CHUNK_ALIGN_आवरणम्) & ~CHUNK_ALIGN_आवरणम्)

/* conversion from malloc headers to user pointers, and back */
#परिभाषय chunk2mem(p)        ((शून्य*)((वर्ण*)(p)       + TWO_आकार_T_SIZES))
#परिभाषय mem2chunk(mem)      ((mchunkptr)((वर्ण*)(mem) - TWO_आकार_T_SIZES))
/* chunk associated with aligned address A */
#परिभाषय align_as_chunk(A)   (mchunkptr)((A) + align_विस्थापनम्(chunk2mem(A)))

/* Bounds on request (not chunk) sizes. */
#परिभाषय MAX_REQUEST         ((-MIN_CHUNK_आकार) << 2)
#परिभाषय MIN_REQUEST         (MIN_CHUNK_आकार - CHUNK_OVERHEAD - आकार_T_ONE)

/* pad request bytes into a usable size */
#परिभाषय pad_request(req) \
   (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_आवरणम्) & ~CHUNK_ALIGN_आवरणम्)

/* pad request, checking for minimum (but not maximum) */
#परिभाषय request2size(req) \
  (((req) < MIN_REQUEST)? MIN_CHUNK_आकार : pad_request(req))


/* ------------------ Operations on head and foot fields ----------------- */

/*
  The head field of a chunk is or'ed with PINUSE_BIT when previous
  adjacent chunk in use, and or'ed with CINUSE_BIT if this chunk is in
  use, unless mmapped, in which case both bits are cleared.

  FLAG4_BIT is not used by this malloc, but might be useful in extensions.
*/

#परिभाषय PINUSE_BIT          (आकार_T_ONE)
#परिभाषय CINUSE_BIT          (आकार_T_TWO)
#परिभाषय FLAG4_BIT           (आकार_T_FOUR)
#परिभाषय INUSE_द्वयङ्काः          (PINUSE_BIT|CINUSE_BIT)
#परिभाषय FLAG_द्वयङ्काः           (PINUSE_BIT|CINUSE_BIT|FLAG4_BIT)

/* Head value for fenceposts */
#परिभाषय FENCEPOST_शीर्ष      (INUSE_द्वयङ्काः|आकार_T_आकार)

/* extraction of fields from head words */
#परिभाषय cinuse(p)           ((p)->शीर्ष & CINUSE_BIT)
#परिभाषय pinuse(p)           ((p)->शीर्ष & PINUSE_BIT)
#परिभाषय flag4inuse(p)       ((p)->शीर्ष & FLAG4_BIT)
#परिभाषय is_inuse(p)         (((p)->शीर्ष & INUSE_द्वयङ्काः) != PINUSE_BIT)
#परिभाषय is_mmapped(p)       (((p)->शीर्ष & INUSE_द्वयङ्काः) == 0)

#परिभाषय chunksize(p)        ((p)->शीर्ष & ~(FLAG_द्वयङ्काः))

#परिभाषय clear_pinuse(p)     ((p)->शीर्ष &= ~PINUSE_BIT)
#परिभाषय समुच्चयः_flag4(p)        ((p)->शीर्ष |= FLAG4_BIT)
#परिभाषय clear_flag4(p)      ((p)->शीर्ष &= ~FLAG4_BIT)

/* Treat space at ptr +/- offset as a chunk */
#परिभाषय chunk_plus_विस्थापनम्(p, s)  ((mchunkptr)(((वर्ण*)(p)) + (s)))
#परिभाषय chunk_minus_विस्थापनम्(p, s) ((mchunkptr)(((वर्ण*)(p)) - (s)))

/* Ptr to next or previous physical malloc_chunk. */
#परिभाषय अग्र_chunk(p) ((mchunkptr)( ((वर्ण*)(p)) + ((p)->शीर्ष & ~FLAG_द्वयङ्काः)))
#परिभाषय पूर्व_chunk(p) ((mchunkptr)( ((वर्ण*)(p)) - ((p)->पूर्व_foot) ))

/* extract next chunk's pinuse bit */
#परिभाषय अग्र_pinuse(p)  ((अग्र_chunk(p)->शीर्ष) & PINUSE_BIT)

/* Get/set size at footer */
#परिभाषय get_foot(p, s)  (((mchunkptr)((वर्ण*)(p) + (s)))->पूर्व_foot)
#परिभाषय समुच्चयः_foot(p, s)  (((mchunkptr)((वर्ण*)(p) + (s)))->पूर्व_foot = (s))

/* Set size, pinuse bit, and foot */
#परिभाषय समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(p, s)\
  ((p)->शीर्ष = (s|PINUSE_BIT), समुच्चयः_foot(p, s))

/* Set size, pinuse bit, foot, and clear next pinuse */
#परिभाषय समुच्चयः_free_with_pinuse(p, s, n)\
  (clear_pinuse(n), समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(p, s))

/* Get the internal overhead associated with chunk p */
#परिभाषय overhead_for(p)\
 (is_mmapped(p)? MMAP_CHUNK_OVERHEAD : CHUNK_OVERHEAD)

/* Return true if malloced space is not necessarily cleared */
#यदि MMAP_CLEARS
#परिभाषय calloc_must_clear(p) (!is_mmapped(p))
#अन्यथा /* MMAP_CLEARS */
#परिभाषय calloc_must_clear(p) (1)
#यदि_अन्तः /* MMAP_CLEARS */

/* ---------------------- Overlaid data structures ----------------------- */

/*
  When chunks are not in use, they are treated as nodes of either
  lists or trees.

  "Small"  chunks are stored in circular doubly-linked lists, and look
  like this:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of previous chunk                            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Forward pointer to next chunk in list             |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Back pointer to previous chunk in list            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Unused space (may be 0 bytes long)                .
            .                                                               .
            .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  Larger chunks are kept in a form of bitwise digital trees (aka
  tries) keyed on chunksizes.  Because malloc_tree_chunks are only for
  free chunks greater than 256 bytes, their size doesn't impose any
  constraints on user chunk sizes.  Each node looks like:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of previous chunk                            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Forward pointer to next chunk of same size        |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Back pointer to previous chunk of same size       |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Pointer to left child (child[0])                  |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Pointer to right child (child[1])                 |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Pointer to parent                                 |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             bin index of this chunk                           |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Unused space                                      .
            .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  Each tree holding treenodes is a tree of unique chunk sizes.  Chunks
  of the same size are arranged in a circularly-linked list, with only
  the oldest chunk (the next to be used, in our FIFO ordering)
  actually in the tree.  (Tree members are distinguished by a non-null
  parent pointer.)  If a chunk with the same size an an existing node
  is inserted, it is linked off the existing node using pointers that
  work in the same way as fd/bk pointers of small chunks.

  Each tree contains a power of 2 sized range of chunk sizes (the
  smallest is 0x100 <= x < 0x180), which is is divided in half at each
  tree level, with the chunks in the smaller half of the range (0x100
  <= x < 0x140 for the top nose) in the left subtree and the larger
  half (0x140 <= x < 0x180) in the right subtree.  This is, of course,
  done by inspecting individual bits.

  Using these rules, each node's left subtree contains all smaller
  sizes than its right subtree.  However, the node at the root of each
  subtree has no particular ordering relationship to either.  (The
  dividing line between the subtree sizes is based on trie relation.)
  If we remove the last chunk of a given size from the interior of the
  tree, we need to replace it with a leaf node.  The tree ordering
  rules permit a node to be replaced by any leaf below it.

  The smallest chunk in a tree (a common operation in a best-fit
  allocator) can be found by walking a path to the leftmost leaf in
  the tree.  Unlike a usual binary tree, where we follow left child
  pointers until we reach a null, here we follow the right child
  pointer any time the left one is null, until we reach a leaf with
  both child pointers null. The smallest chunk in the tree will be
  somewhere along that path.

  The worst case number of steps to add, find, or remove a node is
  bounded by the number of bits differentiating chunks within
  bins. Under current bin calculations, this ranges from 6 up to 21
  (for 32 bit sizes) or up to 53 (for 64 bit sizes). The typical case
  is of course much better.
*/

संरचना malloc_वृक्षः_chunk {
  /* The first four fields must be compatible with malloc_chunk */
  size_t                    पूर्व_foot;
  size_t                    शीर्ष;
  संरचना malloc_वृक्षः_chunk* fd;
  संरचना malloc_वृक्षः_chunk* bk;

  संरचना malloc_वृक्षः_chunk* child[2];
  संरचना malloc_वृक्षः_chunk* parent;
  bindex_t                  सूचकांक;
};

प्रकारनिर्देश संरचना malloc_वृक्षः_chunk  tchunk;
प्रकारनिर्देश संरचना malloc_वृक्षः_chunk* tchunkptr;
प्रकारनिर्देश संरचना malloc_वृक्षः_chunk* tbinptr; /* The type of bins of trees */

/* A little helper macro for trees */
#परिभाषय leftmost_child(t) ((t)->child[0] != 0? (t)->child[0] : (t)->child[1])

/* ----------------------------- Segments -------------------------------- */

/*
  Each malloc space may include non-contiguous segments, held in a
  list headed by an embedded malloc_segment record representing the
  top-most space. Segments also include flags holding properties of
  the space. Large chunks that are directly allocated by mmap are not
  included in this list. They are instead independently created and
  destroyed without otherwise keeping track of them.

  Segment management mainly comes into play for spaces allocated by
  MMAP.  Any call to MMAP might or might not return memory that is
  adjacent to an existing segment.  MORECORE normally contiguously
  extends the current space, so this space is almost always adjacent,
  which is simpler and faster to deal with. (This is why MORECORE is
  used preferentially to MMAP when both are available -- see
  sys_alloc.)  When allocating using MMAP, we don't use any of the
  hinting mechanisms (inconsistently) supported in various
  implementations of unix mmap, or distinguish reserving from
  committing memory. Instead, we just ask for space, and exploit
  contiguity when we get it.  It is probably possible to do
  better than this on some systems, but no general scheme seems
  to be significantly better.

  Management entails a simpler variant of the consolidation scheme
  used for chunks to reduce fragmentation -- new adjacent memory is
  normally prepended or appended to an existing segment. However,
  there are limitations compared to chunk consolidation that mostly
  reflect the fact that segment processing is relatively infrequent
  (occurring only when getting memory from system) and that we
  don't expect to have huge numbers of segments:

  * Segments are not indexed, so traversal requires linear scans.  (It
    would be possible to index these, but is not worth the extra
    overhead and complexity for most programs on most platforms.)
  * New segments are only appended to old ones when holding top-most
    memory; if they cannot be prepended to others, they are held in
    different segments.

  Except for the top-most segment of an mstate, each segment record
  is kept at the tail of its segment. Segments are added by pushing
  segment records onto the list headed by &mstate.seg for the
  containing mstate.

  Segment flags control allocation/merge/deallocation policies:
  * If EXTERN_BIT set, then we did not allocate this segment,
    and so should not try to deallocate or merge with others.
    (This currently holds only for the initial segment passed
    into create_mspace_with_base.)
  * If USE_MMAP_BIT set, the segment may be merged with
    other surrounding mmapped segments and trimmed/de-allocated
    using munmap.
  * If neither bit is set, then the segment was obtained using
    MORECORE so can be merged with surrounding MORECORE'd segments
    and deallocated/trimmed using MORECORE with negative arguments.
*/

संरचना malloc_segment {
  वर्ण*        आधारः;             /* base address */
  size_t       आकार;             /* allocated size */
  संरचना malloc_segment* अग्र;   /* ptr to next segment */
#यदि FFI_MMAP_EXEC_WRIT
  /* The mmap magic is supposed to store the address of the executable
     segment at the very end of the requested block.  */

# define mmap_exec_विस्थापनम्(b,s) (*(ptrdiff_t*)((b)+(s)-परिमाण(ptrdiff_t)))

  /* We can only merge segments if their corresponding executable
     segments are at identical offsets.  */
# define check_segment_merge(S,b,s) \
  (mmap_exec_विस्थापनम्((b),(s)) == (S)->exec_विस्थापनम्)

# define सङ्कलय_segment_exec_विस्थापनम्(p,S) ((वर्ण*)(p) + (S)->exec_विस्थापनम्)
# define sub_segment_exec_विस्थापनम्(p,S) ((वर्ण*)(p) - (S)->exec_विस्थापनम्)

  /* The removal of sflags only works with HAVE_MORECORE == 0.  */

# define get_segment_flags(S)   (USE_MMAP_BIT)
# define समुच्चयः_segment_flags(S,v) \
  (((v) != USE_MMAP_BIT) ? (ABORT, (v)) :				\
   (((S)->exec_विस्थापनम् =							\
     mmap_exec_विस्थापनम्((S)->आधारः, (S)->आकार)),				\
    (mmap_exec_विस्थापनम्((S)->आधारः + (S)->exec_विस्थापनम्, (S)->आकार) !=	\
     (S)->exec_विस्थापनम्) ? (ABORT, (v)) :					\
   (mmap_exec_विस्थापनम्((S)->आधारः, (S)->आकार) = 0), (v)))

  /* We use an offset here, instead of a pointer, because then, when
     base changes, we don't have to modify this.  On architectures
     with segmented addresses, this might not work.  */
  ptrdiff_t    exec_विस्थापनम्;
#अन्यथा

# define get_segment_flags(S)   ((S)->sflags)
# define समुच्चयः_segment_flags(S,v) ((S)->sflags = (v))
# define check_segment_merge(S,b,s) (1)

  flag_t       sflags;           /* mmap and extern flag */
#यदि_अन्तः
};

#परिभाषय is_mmapped_segment(S)  (get_segment_flags(S) & USE_MMAP_BIT)
#परिभाषय is_extern_segment(S)   (get_segment_flags(S) & EXTERN_BIT)

प्रकारनिर्देश संरचना malloc_segment  msegment;
प्रकारनिर्देश संरचना malloc_segment* msegmentptr;

/* ---------------------------- malloc_state ----------------------------- */

/*
   A malloc_state holds all of the bookkeeping for a space.
   The main fields are:

शिखरम्
    The topmost chunk of the currently active segment. Its size is
    cached in topsize.  The actual size of topmost space is
    topsize+TOP_FOOT_SIZE, which includes space reserved for adding
    fenceposts and segment records if necessary when getting more
    space from the system.  The size at which to autotrim top is
    cached from mparams in trim_check, except that it is disabled if
    an autotrim fails.

  Designated victim (dv)
    This is the preferred chunk for servicing small requests that
    don't have exact fits.  It is normally the chunk split off most
    recently to service another small request.  Its size is cached in
    dvsize. The link fields of this chunk are not maintained since it
    is not kept in a bin.

  SmallBins
    An array of bin headers for free chunks.  These bins hold chunks
    with sizes less than MIN_LARGE_SIZE bytes. Each bin contains
    chunks of all the same size, spaced 8 bytes apart.  To simplify
    use in double-linked lists, each bin header acts as a malloc_chunk
    pointing to the real first node, if it exists (else pointing to
    itself).  This avoids special-casing for headers.  But to avoid
    waste, we allocate only the fd/bk pointers of bins, and then use
    repositioning tricks to treat these as the fields of a chunk.

  TreeBins
    Treebins are pointers to the roots of trees holding a range of
    sizes. There are 2 equally spaced treebins for each power of two
    from TREE_SHIFT to TREE_SHIFT+16. The last bin holds anything
    larger.

  Bin maps
    There is one bit map for small bins ("smallmap") and one for
    treebins ("treemap).  Each bin sets its bit when non-empty, and
    clears the bit when empty.  Bit operations are then used to avoid
    bin-by-bin searching -- nearly all "search" is done without ever
    looking at bins that won't be selected.  The bit maps
    conservatively use 32 bits per map word, even if on 64bit system.
    For a good description of some of the bit-based techniques used
    here, see Henry S. Warren Jr's book "Hacker's Delight" (and
    supplement at http://hackersdelight.org/). Many of these are
    intended to reduce the branchiness of paths through malloc etc, as
    well as to reduce the number of memory locations read or written.

  Segments
    A list of segments headed by an embedded malloc_segment record
    representing the initial space.

  Address check support
    The least_addr field is the least address ever obtained from
    MORECORE or MMAP. Attempted frees and reallocs of any address less
    than this are trapped (unless INSECURE is defined).

  Magic tag
    A cross-check field that should always hold same value as mparams.magic.

  Max allowed footprint
    The maximum allowed bytes to allocate from system (zero means no limit)

  Flags
    Bits recording whether to use MMAP, locks, or contiguous MORECORE

  Statistics
    Each space keeps track of current and maximum system memory
    obtained via MORECORE or MMAP.

  Trim support
    Fields holding the amount of unused topmost memory that should trigger
    trimming, and a counter to force periodic scanning to release unused
    non-topmost segments.

  Locking
    If USE_LOCKS is defined, the "mutex" lock is acquired and released
    around every public call using this mspace.

  Extension support
    A void* pointer and a size_t field that can be used to help implement
    extensions to this malloc.
*/

/* Bin types, widths and sizes */
#परिभाषय NSMALLBINS        (32U)
#परिभाषय NTREEBINS         (32U)
#परिभाषय SMALLBIN_सरणम्    (3U)
#परिभाषय SMALLBIN_विस्तारः    (आकार_T_ONE << SMALLBIN_सरणम्)
#परिभाषय TREEBIN_सरणम्     (8U)
#परिभाषय MIN_LARGE_आकार    (आकार_T_ONE << TREEBIN_सरणम्)
#परिभाषय MAX_SMALL_आकार    (MIN_LARGE_आकार - आकार_T_ONE)
#परिभाषय MAX_SMALL_REQUEST (MAX_SMALL_आकार - CHUNK_ALIGN_आवरणम् - CHUNK_OVERHEAD)

संरचना malloc_state {
  binmap_t   smallmap;
  binmap_t   treemap;
  size_t     dvsize;
  size_t     topsize;
  वर्ण*      न्यूनतमम्_addr;
  mchunkptr  dv;
  mchunkptr  शिखरम्;
  size_t     trim_check;
  size_t     release_checks;
  size_t     magic;
  mchunkptr  smallbins[(NSMALLBINS+1)*2];
  tbinptr    treebins[NTREEBINS];
  size_t     footprint;
  size_t     max_footprint;
  size_t     footprint_सीमा; /* zero means no limit */
  flag_t     mflags;
#यदि USE_LOCKS
  MLOCK_T    mutex;     /* locate lock among fields that rarely change */
#यदि_अन्तः /* USE_LOCKS */
  msegment   seg;
  शून्य*      extp;      /* Unused but available for extensions */
  size_t     exts;
};

प्रकारनिर्देश संरचना malloc_state*    mstate;

/* ------------- Global malloc_state and malloc_params ------------------- */

/*
  malloc_params holds global properties, including those that can be
  dynamically set using mallopt. There is a single instance, mparams,
  initialized in init_mparams. Note that the non-zeroness of "magic"
  also serves as an initialization flag.
*/

संरचना malloc_params {
  size_t magic;
  size_t page_आकार;
  size_t granularity;
  size_t mmap_देहली;
  size_t trim_देहली;
  flag_t default_mflags;
};

स्थिर संरचना malloc_params mparams;

/* libffi: mparams is initialized exactly once (under the malloc global lock),
   but mparams.magic is read on every allocation's fast path WITHOUT a lock to
   decide whether initialization already happened.  Upstream writes magic
   through a `volatile` cast, which is not a synchronizing operation, so
   ThreadSanitizer reports a data race (libffi issue #873) between that one-time
   write and the lock-free readers.  Access magic with acquire/release ordering
   instead: a thread that observes magic != 0 via the acquire load is then
   guaranteed to see all the other initialization writes (page_size, mflags,
   ...) performed before the release store.  Fall back to the original volatile
   access when no supported compiler atomic operations are available.  */
#यदि USE_LOCKS && defined(__ATOMIC_ACQUIRE)
# define ffi_mparams_magic_आरोपय()   __atomic_load_n(&mparams.magic, __ATOMIC_ACQUIRE)
# define ffi_mparams_magic_store(v) __atomic_store_n(&mparams.magic, (size_t)(v), __ATOMIC_RELEASE)
#नोचेद्यदि USE_LOCKS && defined(_MSC_VER)
# यदि defined(_WIN64)
#  define ffi_mparams_magic_आरोपय() \
  ((size_t)_InterlockedCompareExchange64((__int64 चञ्चल *)&mparams.magic, \
                                         (__int64)0, (__int64)0))
#  define ffi_mparams_magic_store(v) \
  ((शून्य)_InterlockedExchange64((__int64 चञ्चल *)&mparams.magic, \
                                (__int64)(v)))
# अन्यथा
#  define ffi_mparams_magic_आरोपय() \
  ((size_t)_InterlockedCompareExchange((LONG चञ्चल *)&mparams.magic, \
                                       (LONG)0, (LONG)0))
#  define ffi_mparams_magic_store(v) \
  ((शून्य)_InterlockedExchange((LONG चञ्चल *)&mparams.magic, (LONG)(v)))
# endif
#अन्यथा
# define ffi_mparams_magic_आरोपय()   (mparams.magic)
# define ffi_mparams_magic_store(v) (*(चञ्चल size_t *)(&(mparams.magic)) = (size_t)(v))
#यदि_अन्तः

/* Ensure mparams initialized */
#परिभाषय ensure_initialization() (शून्य)(ffi_mparams_magic_आरोपय() != 0 || init_mparams())

#यदि !ONLY_MSPACES

/* The global malloc_state used for all non-"mspace" calls */
स्थिर संरचना malloc_state _gm_;
#परिभाषय gm                 (&_gm_)
#परिभाषय is_global(M)       ((M) == &_gm_)

#यदि_अन्तः /* !ONLY_MSPACES */

#परिभाषय is_initialized(M)  ((M)->शिखरम् != 0)

/* -------------------------- system alloc setup ------------------------- */

/* Operations on mflags */

#परिभाषय use_तालकम्(M)           ((M)->mflags &   USE_तालकम्_BIT)
#परिभाषय enable_तालकम्(M)        ((M)->mflags |=  USE_तालकम्_BIT)
#यदि USE_LOCKS
#परिभाषय disable_तालकम्(M)       ((M)->mflags &= ~USE_तालकम्_BIT)
#अन्यथा
#परिभाषय disable_तालकम्(M)
#यदि_अन्तः

#परिभाषय use_mmap(M)           ((M)->mflags &   USE_MMAP_BIT)
#परिभाषय enable_mmap(M)        ((M)->mflags |=  USE_MMAP_BIT)
#यदि HAVE_MMAP
#परिभाषय disable_mmap(M)       ((M)->mflags &= ~USE_MMAP_BIT)
#अन्यथा
#परिभाषय disable_mmap(M)
#यदि_अन्तः

#परिभाषय use_noncontiguous(M)  ((M)->mflags &   USE_NONCONTIGUOUS_BIT)
#परिभाषय disable_contiguous(M) ((M)->mflags |=  USE_NONCONTIGUOUS_BIT)

#परिभाषय समुच्चयः_तालकम्(M,L)\
 ((M)->mflags = (L)?\
  ((M)->mflags | USE_तालकम्_BIT) :\
  ((M)->mflags & ~USE_तालकम्_BIT))

/* page-align a size */
#परिभाषय page_align(S)\
 (((S) + (mparams.page_आकार - आकार_T_ONE)) & ~(mparams.page_आकार - आकार_T_ONE))

/* granularity-align a size */
#परिभाषय granularity_align(S)\
  (((S) + (mparams.granularity - आकार_T_ONE))\
   & ~(mparams.granularity - आकार_T_ONE))


/* For mmap, use granularity alignment on windows, else page-align */
#यदि_परिभाषितम् WIN32
#परिभाषय mmap_align(S) granularity_align(S)
#अन्यथा
#परिभाषय mmap_align(S) page_align(S)
#यदि_अन्तः

/* For sys_alloc, enough padding to ensure can malloc request on success */
#परिभाषय SYS_ALLOC_PADDING (शिखरम्_FOOT_आकार + MALLOC_ALIGNMENT)

#परिभाषय is_page_aligned(S)\
   (((size_t)(S) & (mparams.page_आकार - आकार_T_ONE)) == 0)
#परिभाषय is_granularity_aligned(S)\
   (((size_t)(S) & (mparams.granularity - आकार_T_ONE)) == 0)

/*  True if segment S holds address A */
#परिभाषय segment_holds(S, A)\
  ((वर्ण*)(A) >= S->आधारः && (वर्ण*)(A) < S->आधारः + S->आकार)

/* Return segment holding given address */
स्थिर msegmentptr segment_holding(mstate m, वर्ण* addr) {
  msegmentptr sp = &m->seg;
  प्रति (;;) {
    यदि (addr >= sp->आधारः && addr < sp->आधारः + sp->आकार)
      प्रत्यागम sp;
    यदि ((sp = sp->अग्र) == 0)
      प्रत्यागम 0;
  }
}

/* Return true if segment contains a segment link */
स्थिर पूर्णाङ्क has_segment_link(mstate m, msegmentptr ss) {
  msegmentptr sp = &m->seg;
  प्रति (;;) {
    यदि ((वर्ण*)sp >= ss->आधारः && (वर्ण*)sp < ss->आधारः + ss->आकार)
      प्रत्यागम 1;
    यदि ((sp = sp->अग्र) == 0)
      प्रत्यागम 0;
  }
}

#यदि_अपरिभाषितम् MORECORE_CANNOT_TRIM
#परिभाषय should_trim(M,s)  ((s) > (M)->trim_check)
#अन्यथा  /* MORECORE_CANNOT_TRIM */
#परिभाषय should_trim(M,s)  (0)
#यदि_अन्तः /* MORECORE_CANNOT_TRIM */

/*
  TOP_FOOT_SIZE is padding at the end of a segment, including space
  that may be needed to place segment records and fenceposts when new
  noncontiguous segments are added.
*/
#परिभाषय शिखरम्_FOOT_आकार\
  (align_विस्थापनम्(chunk2mem(0))+pad_request(परिमाण(संरचना malloc_segment))+MIN_CHUNK_आकार)


/* -------------------------------  Hooks -------------------------------- */

/*
  PREACTION should be defined to return 0 on success, and nonzero on
  failure. If you are not using locking, you can redefine these to do
  anything you like.
*/

#यदि USE_LOCKS
#परिभाषय PREACTION(M)  ((use_तालकम्(M))? ACQUIRE_तालकम्(&(M)->mutex) : 0)
#परिभाषय POSTACTION(M) { यदि (use_तालकम्(M)) RELEASE_तालकम्(&(M)->mutex); }
#अन्यथा /* USE_LOCKS */

#यदि_अपरिभाषितम् PREACTION
#परिभाषय PREACTION(M) (0)
#यदि_अन्तः  /* PREACTION */

#यदि_अपरिभाषितम् POSTACTION
#परिभाषय POSTACTION(M)
#यदि_अन्तः  /* POSTACTION */

#यदि_अन्तः /* USE_LOCKS */

/*
  CORRUPTION_ERROR_ACTION is triggered upon detected bad addresses.
  USAGE_ERROR_ACTION is triggered on detected bad frees and
  reallocs. The argument p is an address that might have triggered the
  fault. It is ignored by the two predefined actions, but might be
  useful in custom actions that try to help diagnose errors.
*/

#यदि PROCEED_ON_ERROR

/* A count of the number of corruption errors causing resets */
पूर्णाङ्क malloc_corruption_error_गणनम्;

/* default corruption action */
स्थिर शून्य reset_on_error(mstate m);

#परिभाषय CORRUPTION_ERROR_ACTION(m)  reset_on_error(m)
#परिभाषय USAGE_ERROR_ACTION(m, p)

#अन्यथा /* PROCEED_ON_ERROR */

#यदि_अपरिभाषितम् CORRUPTION_ERROR_ACTION
#परिभाषय CORRUPTION_ERROR_ACTION(m) ABORT
#यदि_अन्तः /* CORRUPTION_ERROR_ACTION */

#यदि_अपरिभाषितम् USAGE_ERROR_ACTION
#परिभाषय USAGE_ERROR_ACTION(m,p) ABORT
#यदि_अन्तः /* USAGE_ERROR_ACTION */

#यदि_अन्तः /* PROCEED_ON_ERROR */


/* -------------------------- Debugging setup ---------------------------- */

#यदि ! DEBUG

#परिभाषय check_free_chunk(M,P)
#परिभाषय check_inuse_chunk(M,P)
#परिभाषय check_malloced_chunk(M,P,N)
#परिभाषय check_mmapped_chunk(M,P)
#परिभाषय check_malloc_state(M)
#परिभाषय check_शिखरम्_chunk(M,P)

#अन्यथा /* DEBUG */
#परिभाषय check_free_chunk(M,P)       do_check_free_chunk(M,P)
#परिभाषय check_inuse_chunk(M,P)      do_check_inuse_chunk(M,P)
#परिभाषय check_शिखरम्_chunk(M,P)        do_check_शिखरम्_chunk(M,P)
#परिभाषय check_malloced_chunk(M,P,N) do_check_malloced_chunk(M,P,N)
#परिभाषय check_mmapped_chunk(M,P)    do_check_mmapped_chunk(M,P)
#परिभाषय check_malloc_state(M)       do_check_malloc_state(M)

स्थिर शून्य   do_check_any_chunk(mstate m, mchunkptr p);
स्थिर शून्य   do_check_शिखरम्_chunk(mstate m, mchunkptr p);
स्थिर शून्य   do_check_mmapped_chunk(mstate m, mchunkptr p);
स्थिर शून्य   do_check_inuse_chunk(mstate m, mchunkptr p);
स्थिर शून्य   do_check_free_chunk(mstate m, mchunkptr p);
स्थिर शून्य   do_check_malloced_chunk(mstate m, शून्य* mem, size_t s);
स्थिर शून्य   do_check_वृक्षः(mstate m, tchunkptr t);
स्थिर शून्य   do_check_treebin(mstate m, bindex_t i);
स्थिर शून्य   do_check_smallbin(mstate m, bindex_t i);
स्थिर शून्य   do_check_malloc_state(mstate m);
स्थिर पूर्णाङ्क    bin_find(mstate m, mchunkptr x);
स्थिर size_t traverse_and_check(mstate m);
#यदि_अन्तः /* DEBUG */

/* ---------------------------- Indexing Bins ---------------------------- */

#परिभाषय is_small(s)         (((s) >> SMALLBIN_सरणम्) < NSMALLBINS)
#परिभाषय small_सूचकांक(s)      (bindex_t)((s)  >> SMALLBIN_सरणम्)
#परिभाषय small_index2size(i) ((i)  << SMALLBIN_सरणम्)
#परिभाषय MIN_SMALL_सूचकांक     (small_सूचकांक(MIN_CHUNK_आकार))

/* addressing by index. See above about smallbin repositioning */
#परिभाषय smallbin_at(M, i)   ((sbinptr)((वर्ण*)&((M)->smallbins[(i)<<1])))
#परिभाषय treebin_at(M,i)     (&((M)->treebins[i]))

/* assign tree index for size S to variable I.
   libffi: enable the __builtin_clz path on any GNU-compatible compiler, not
   only x86, so the log2 is an intrinsic rather than the generic C idiom that
   the compiler can't always recognize (libffi #754).  */
#यदि defined(__GNUC__)
#परिभाषय compute_वृक्षः_सूचकांक(S, I)\
{\
  अचिह्नित पूर्णाङ्क X = S >> TREEBIN_सरणम्;\
  यदि (X == 0)\
    I = 0;\
  अन्यथा यदि (X > 0xFFFF)\
    I = NTREEBINS-1;\
  अन्यथा {\
    अचिह्नित पूर्णाङ्क K = (अचिह्नित) परिमाण(X)*__CHAR_BIT__ - 1 - (अचिह्नित) __builtin_clz(X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_सरणम्-1)) & 1)));\
  }\
}

#नोचेद्यदि defined (__INTEL_COMPILER)
#परिभाषय compute_वृक्षः_सूचकांक(S, I)\
{\
  size_t X = S >> TREEBIN_सरणम्;\
  यदि (X == 0)\
    I = 0;\
  अन्यथा यदि (X > 0xFFFF)\
    I = NTREEBINS-1;\
  अन्यथा {\
    अचिह्नित पूर्णाङ्क K = bit_अवलोकनम्_reverse (X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_सरणम्-1)) & 1)));\
  }\
}

#नोचेद्यदि defined(_MSC_VER) && _MSC_VER>=1300
#परिभाषय compute_वृक्षः_सूचकांक(S, I)\
{\
  size_t X = S >> TREEBIN_सरणम्;\
  यदि (X == 0)\
    I = 0;\
  अन्यथा यदि (X > 0xFFFF)\
    I = NTREEBINS-1;\
  अन्यथा {\
    अचिह्नित पूर्णाङ्क K;\
    Bit_अवलोकनम्_Reverse((DWORD *) &K, (DWORD) X);\
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_सरणम्-1)) & 1)));\
  }\
}

#अन्यथा /* GNUC */
#परिभाषय compute_वृक्षः_सूचकांक(S, I)\
{\
  size_t X = S >> TREEBIN_सरणम्;\
  यदि (X == 0)\
    I = 0;\
  अन्यथा यदि (X > 0xFFFF)\
    I = NTREEBINS-1;\
  अन्यथा {\
    अचिह्नित पूर्णाङ्क Y = (अचिह्नित पूर्णाङ्क)X;\
    अचिह्नित पूर्णाङ्क N = ((Y - 0x100) >> 16) & 8;\
    अचिह्नित पूर्णाङ्क K = (((Y <<= N) - 0x1000) >> 16) & 4;\
    N += K;\
    N += K = (((Y <<= K) - 0x4000) >> 16) & 2;\
    K = 14 - N + ((Y <<= K) >> 15);\
    I = (K << 1) + ((S >> (K + (TREEBIN_सरणम्-1)) & 1));\
  }\
}
#यदि_अन्तः /* GNUC */

/* Bit representing maximum resolved size in a treebin at i */
#परिभाषय bit_for_वृक्षः_सूचकांक(i) \
   (i == NTREEBINS-1)? (आकार_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_सरणम् - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit */
#परिभाषय leftshift_for_वृक्षः_सूचकांक(i) \
   ((i == NTREEBINS-1)? 0 : \
    ((आकार_T_BITSIZE-आकार_T_ONE) - (((i) >> 1) + TREEBIN_सरणम् - 2)))

/* The size of the smallest chunk held in bin with index i */
#परिभाषय minsize_for_वृक्षः_सूचकांक(i) \
   ((आकार_T_ONE << (((i) >> 1) + TREEBIN_सरणम्)) |  \
   (((size_t)((i) & आकार_T_ONE)) << (((i) >> 1) + TREEBIN_सरणम् - 1)))


/* ------------------------ Operations on bin maps ----------------------- */

/* bit corresponding to given index */
#परिभाषय idx2bit(i)              ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#परिभाषय mark_smallmap(M,i)      ((M)->smallmap |=  idx2bit(i))
#परिभाषय clear_smallmap(M,i)     ((M)->smallmap &= ~idx2bit(i))
#परिभाषय smallmap_is_marked(M,i) ((M)->smallmap &   idx2bit(i))

#परिभाषय mark_treemap(M,i)       ((M)->treemap  |=  idx2bit(i))
#परिभाषय clear_treemap(M,i)      ((M)->treemap  &= ~idx2bit(i))
#परिभाषय treemap_is_marked(M,i)  ((M)->treemap  &   idx2bit(i))

/* isolate the least set bit of a bitmap */
#परिभाषय न्यूनतमम्_bit(x)         ((x) & -(x))

/* mask with all bits to left of least bit of x on */
#परिभाषय वामः_द्वयङ्काः(x)         ((x<<1) | -(x<<1))

/* mask with all bits to left of or equal to least bit of x on */
#परिभाषय same_or_वामः_द्वयङ्काः(x) ((x) | -(x))

/* index corresponding to given bit.
   libffi: use __builtin_ctz on any GNU-compatible compiler, not only x86
   (libffi #754).  */

#यदि defined(__GNUC__)
#परिभाषय compute_bit2idx(X, I)\
{\
  अचिह्नित पूर्णाङ्क J;\
  J = __builtin_ctz(X); \
  I = (bindex_t)J;\
}

#नोचेद्यदि defined (__INTEL_COMPILER)
#परिभाषय compute_bit2idx(X, I)\
{\
  अचिह्नित पूर्णाङ्क J;\
  J = bit_अवलोकनम्_forward (X); \
  I = (bindex_t)J;\
}

#नोचेद्यदि defined(_MSC_VER) && _MSC_VER>=1300
#परिभाषय compute_bit2idx(X, I)\
{\
  अचिह्नित पूर्णाङ्क J;\
  Bit_अवलोकनम्_Forward_2((DWORD *) &J, X);\
  I = (bindex_t)J;\
}

#नोचेद्यदि USE_BUILTIN_FFS
#परिभाषय compute_bit2idx(X, I) I = ffs(X)-1

#अन्यथा
#परिभाषय compute_bit2idx(X, I)\
{\
  अचिह्नित पूर्णाङ्क Y = X - 1;\
  अचिह्नित पूर्णाङ्क K = Y >> (16-4) & 16;\
  अचिह्नित पूर्णाङ्क N = K;        Y >>= K;\
  N += K = Y >> (8-3) &  8;  Y >>= K;\
  N += K = Y >> (4-2) &  4;  Y >>= K;\
  N += K = Y >> (2-1) &  2;  Y >>= K;\
  N += K = Y >> (1-0) &  1;  Y >>= K;\
  I = (bindex_t)(N + Y);\
}
#यदि_अन्तः /* GNUC */


/* ----------------------- Runtime Check Support ------------------------- */

/*
  For security, the main invariant is that malloc/free/etc never
  writes to a static address other than malloc_state, unless static
  malloc_state itself has been corrupted, which cannot occur via
  malloc (because of these checks). In essence this means that we
  believe all pointers, sizes, maps etc held in malloc_state, but
  check all of those linked or offsetted from other embedded data
  structures.  These checks are interspersed with main code in a way
  that tends to minimize their run-time cost.

  When FOOTERS is defined, in addition to range checking, we also
  verify footer fields of inuse chunks, which can be used guarantee
  that the mstate controlling malloc/free is intact.  This is a
  streamlined version of the approach described by William Robertson
  et al in "Run-time Detection of Heap-based Overflows" LISA'03
  http://www.usenix.org/events/lisa03/tech/robertson.html The footer
  of an inuse chunk holds the xor of its mstate and a random seed,
  that is checked upon calls to free() and realloc().  This is
  (probabalistically) unguessable from outside the program, but can be
  computed by any code successfully malloc'ing any chunk, so does not
  itself provide protection against code that has already broken
  security through some other means.  Unlike Robertson et al, we
  always dynamically check addresses of all offset chunks (previous,
  next, etc). This turns out to be cheaper than relying on hashes.
*/

#यदि !INSECURE
/* Check if address a is at least as high as any from MORECORE or MMAP */
#परिभाषय ok_स्थानाङ्कः(M, a) ((वर्ण*)(a) >= (M)->न्यूनतमम्_addr)
/* Check if address of next chunk n is higher than base chunk p */
#परिभाषय ok_अग्र(p, n)    ((वर्ण*)(p) < (वर्ण*)(n))
/* Check if p has inuse status */
#परिभाषय ok_inuse(p)     is_inuse(p)
/* Check if p has its pinuse bit on */
#परिभाषय ok_pinuse(p)     pinuse(p)

#अन्यथा /* !INSECURE */
#परिभाषय ok_स्थानाङ्कः(M, a) (1)
#परिभाषय ok_अग्र(b, n)    (1)
#परिभाषय ok_inuse(p)      (1)
#परिभाषय ok_pinuse(p)     (1)
#यदि_अन्तः /* !INSECURE */

#यदि (FOOTERS && !INSECURE)
/* Check if (alleged) mstate m has expected magic field */
#परिभाषय ok_magic(M)      ((M)->magic == mparams.magic)
#अन्यथा  /* (FOOTERS && !INSECURE) */
#परिभाषय ok_magic(M)      (1)
#यदि_अन्तः /* (FOOTERS && !INSECURE) */

/* In gcc, use __builtin_expect to minimize impact of checks */
#यदि !INSECURE
#यदि defined(__GNUC__) && __GNUC__ >= 3
#परिभाषय RTCHECK(e)  __builtin_expect(e, 1)
#अन्यथा /* GNUC */
#परिभाषय RTCHECK(e)  (e)
#यदि_अन्तः /* GNUC */
#अन्यथा /* !INSECURE */
#परिभाषय RTCHECK(e)  (1)
#यदि_अन्तः /* !INSECURE */

/* macros to set up inuse chunks with or without footers */

#यदि !FOOTERS

#परिभाषय mark_inuse_foot(M,p,s)

/* Macros for setting head/foot of non-mmapped chunks */

/* Set cinuse bit and pinuse bit of next chunk */
#परिभाषय समुच्चयः_inuse(M,p,s)\
  ((p)->शीर्ष = (((p)->शीर्ष & PINUSE_BIT)|s|CINUSE_BIT),\
  ((mchunkptr)(((वर्ण*)(p)) + (s)))->शीर्ष |= PINUSE_BIT)

/* Set cinuse and pinuse of this chunk and pinuse of next chunk */
#परिभाषय समुच्चयः_inuse_and_pinuse(M,p,s)\
  ((p)->शीर्ष = (s|PINUSE_BIT|CINUSE_BIT),\
  ((mchunkptr)(((वर्ण*)(p)) + (s)))->शीर्ष |= PINUSE_BIT)

/* Set size, cinuse and pinuse bit of this chunk */
#परिभाषय समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(M, p, s)\
  ((p)->शीर्ष = (s|PINUSE_BIT|CINUSE_BIT))

#अन्यथा /* FOOTERS */

/* Set foot of inuse chunk to be xor of mstate and seed */
#परिभाषय mark_inuse_foot(M,p,s)\
  (((mchunkptr)((वर्ण*)(p) + (s)))->पूर्व_foot = ((size_t)(M) ^ mparams.magic))

#परिभाषय get_mstate_for(p)\
  ((mstate)(((mchunkptr)((वर्ण*)(p) +\
    (chunksize(p))))->पूर्व_foot ^ mparams.magic))

#परिभाषय समुच्चयः_inuse(M,p,s)\
  ((p)->शीर्ष = (((p)->शीर्ष & PINUSE_BIT)|s|CINUSE_BIT),\
  (((mchunkptr)(((वर्ण*)(p)) + (s)))->शीर्ष |= PINUSE_BIT), \
  mark_inuse_foot(M,p,s))

#परिभाषय समुच्चयः_inuse_and_pinuse(M,p,s)\
  ((p)->शीर्ष = (s|PINUSE_BIT|CINUSE_BIT),\
  (((mchunkptr)(((वर्ण*)(p)) + (s)))->शीर्ष |= PINUSE_BIT),\
 mark_inuse_foot(M,p,s))

#परिभाषय समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(M, p, s)\
  ((p)->शीर्ष = (s|PINUSE_BIT|CINUSE_BIT),\
  mark_inuse_foot(M, p, s))

#यदि_अन्तः /* !FOOTERS */

/* ---------------------------- setting mparams -------------------------- */

#यदि तालकम्_AT_FORK
स्थिर शून्य pre_fork(शून्य)         { ACQUIRE_तालकम्(&(gm)->mutex); }
स्थिर शून्य post_fork_parent(शून्य) { RELEASE_तालकम्(&(gm)->mutex); }
स्थिर शून्य post_fork_child(शून्य)  { INITIAL_तालकम्(&(gm)->mutex); }
#यदि_अन्तः /* LOCK_AT_FORK */

/* Initialize mparams */
स्थिर पूर्णाङ्क init_mparams(शून्य) {
#यदि_परिभाषितम् NEED_GLOBAL_तालकम्_INIT
  यदि (malloc_global_mutex_स्थितिः <= 0)
    init_malloc_global_mutex();
#यदि_अन्तः

  ACQUIRE_MALLOC_GLOBAL_तालकम्();
  यदि (mparams.magic == 0) {
    size_t magic;
    size_t psize;
    size_t gsize;

#यदि !defined(WIN32) && !defined(__OS2__)
    psize = malloc_getpagesize;
    gsize = ((DEFAULT_GRANULARITY != 0)? DEFAULT_GRANULARITY : psize);
#नोचेद्यदि defined (__OS2__)
    /* if low-memory is used, os2munmap() would break
       if it were anything other than 64k */
    psize = 4096u;
    gsize = 65536u;
#अन्यथा /* WIN32 */
    {
      SYSTEM_INFO system_info;
      GetSystemInfo(&system_info);
      psize = system_info.dw_Page_आकार;
      gsize = ((DEFAULT_GRANULARITY != 0)?
               DEFAULT_GRANULARITY : system_info.dw_आवण्टनम्_Granularity);
    }
#यदि_अन्तः /* WIN32 */

    /* Sanity-check configuration:
       size_t must be unsigned and as wide as pointer type.
       ints must be at least 4 bytes.
       alignment must be at least 8.
       Alignment, min chunk size, and page size must all be powers of 2.
    */
    यदि ((परिमाण(size_t) != परिमाण(वर्ण*)) ||
        (MAX_आकार_T < MIN_CHUNK_आकार)  ||
        (परिमाण(पूर्णाङ्क) < 4)  ||
        (MALLOC_ALIGNMENT < (size_t)8U) ||
        ((MALLOC_ALIGNMENT & (MALLOC_ALIGNMENT-आकार_T_ONE)) != 0) ||
        ((MCHUNK_आकार      & (MCHUNK_आकार-आकार_T_ONE))      != 0) ||
        ((gsize            & (gsize-आकार_T_ONE))            != 0) ||
        ((psize            & (psize-आकार_T_ONE))            != 0))
      ABORT;
    mparams.granularity = gsize;
    mparams.page_आकार = psize;
    mparams.mmap_देहली = DEFAULT_MMAP_देहली;
    mparams.trim_देहली = DEFAULT_TRIM_देहली;
#यदि MORECORE_CONTIGUOUS
    mparams.default_mflags = USE_तालकम्_BIT|USE_MMAP_BIT;
#अन्यथा  /* MORECORE_CONTIGUOUS */
    mparams.default_mflags = USE_तालकम्_BIT|USE_MMAP_BIT|USE_NONCONTIGUOUS_BIT;
#यदि_अन्तः /* MORECORE_CONTIGUOUS */

#यदि !ONLY_MSPACES
    /* Set up lock for main malloc area */
    gm->mflags = mparams.default_mflags;
    (शून्य)INITIAL_तालकम्(&gm->mutex);
#यदि_अन्तः
#यदि तालकम्_AT_FORK
    pthread_atfork(&pre_fork, &post_fork_parent, &post_fork_child);
#यदि_अन्तः

    {
#यदि USE_DEV_RANDOM
      पूर्णाङ्क fd;
      अचिह्नित वर्ण buf[परिमाण(size_t)];
      /* Try to use /dev/urandom, else fall back on using time */
      यदि ((fd = उद्घाटय("/dev/urandom", O_RDONLY)) >= 0 &&
          पठ(fd, buf, परिमाण(buf)) == परिमाण(buf)) {
        magic = *((size_t *) buf);
        close(fd);
      }
      अन्यथा
#यदि_अन्तः /* USE_DEV_RANDOM */
#यदि_परिभाषितम् WIN32
      magic = (size_t)(Get_Tick_गणनम्() ^ (size_t)0x55555555U);
#नोचेद्यदि defined(LACKS_TIME_H)
      magic = (size_t)&magic ^ (size_t)0x55555555U;
#अन्यथा
      magic = (size_t)(time(0) ^ (size_t)0x55555555U);
#यदि_अन्तः
      magic |= (size_t)8U;    /* ensure nonzero */
      magic &= ~(size_t)7U;   /* improve chances of fault for bad values */
      /* libffi: release-store so any thread that observes magic via the
         acquire load in ensure_initialization() sees the writes above (#873).  */
      ffi_mparams_magic_store(magic);
    }
  }

  RELEASE_MALLOC_GLOBAL_तालकम्();
  प्रत्यागम 1;
}

/* support for mallopt */
स्थिर पूर्णाङ्क change_mparam(पूर्णाङ्क param_सङ्ख्या, पूर्णाङ्क मानम्) {
  size_t val;
  ensure_initialization();
  val = (मानम् == -1)? MAX_आकार_T : (size_t)मानम्;
  विकल्प(param_सङ्ख्या) {
  पक्ष M_TRIM_देहली:
    mparams.trim_देहली = val;
    प्रत्यागम 1;
  पक्ष M_GRANULARITY:
    यदि (val >= mparams.page_आकार && ((val & (val-1)) == 0)) {
      mparams.granularity = val;
      प्रत्यागम 1;
    }
    अन्यथा
      प्रत्यागम 0;
  पक्ष M_MMAP_देहली:
    mparams.mmap_देहली = val;
    प्रत्यागम 1;
  स्वभाव:
    प्रत्यागम 0;
  }
}

#यदि DEBUG
/* ------------------------- Debugging Support --------------------------- */

/* Check properties of any chunk, whether free, inuse, mmapped etc  */
स्थिर शून्य do_check_any_chunk(mstate m, mchunkptr p) {
  प्रतिज्ञा((is_aligned(chunk2mem(p))) || (p->शीर्ष == FENCEPOST_शीर्ष));
  प्रतिज्ञा(ok_स्थानाङ्कः(m, p));
}

/* Check properties of top chunk */
स्थिर शून्य do_check_शिखरम्_chunk(mstate m, mchunkptr p) {
  msegmentptr sp = segment_holding(m, (वर्ण*)p);
  size_t  sz = p->शीर्ष & ~INUSE_द्वयङ्काः; /* third-lowest bit can be set! */
  प्रतिज्ञा(sp != 0);
  प्रतिज्ञा((is_aligned(chunk2mem(p))) || (p->शीर्ष == FENCEPOST_शीर्ष));
  प्रतिज्ञा(ok_स्थानाङ्कः(m, p));
  प्रतिज्ञा(sz == m->topsize);
  प्रतिज्ञा(sz > 0);
  प्रतिज्ञा(sz == ((sp->आधारः + sp->आकार) - (वर्ण*)p) - शिखरम्_FOOT_आकार);
  प्रतिज्ञा(pinuse(p));
  प्रतिज्ञा(!pinuse(chunk_plus_विस्थापनम्(p, sz)));
}

/* Check properties of (inuse) mmapped chunks */
स्थिर शून्य do_check_mmapped_chunk(mstate m, mchunkptr p) {
  size_t  sz = chunksize(p);
  size_t len = (sz + (p->पूर्व_foot) + MMAP_FOOT_PAD);
  प्रतिज्ञा(is_mmapped(p));
  प्रतिज्ञा(use_mmap(m));
  प्रतिज्ञा((is_aligned(chunk2mem(p))) || (p->शीर्ष == FENCEPOST_शीर्ष));
  प्रतिज्ञा(ok_स्थानाङ्कः(m, p));
  प्रतिज्ञा(!is_small(sz));
  प्रतिज्ञा((len & (mparams.page_आकार-आकार_T_ONE)) == 0);
  प्रतिज्ञा(chunk_plus_विस्थापनम्(p, sz)->शीर्ष == FENCEPOST_शीर्ष);
  प्रतिज्ञा(chunk_plus_विस्थापनम्(p, sz+आकार_T_आकार)->शीर्ष == 0);
}

/* Check properties of inuse chunks */
स्थिर शून्य do_check_inuse_chunk(mstate m, mchunkptr p) {
  do_check_any_chunk(m, p);
  प्रतिज्ञा(is_inuse(p));
  प्रतिज्ञा(अग्र_pinuse(p));
  /* If not pinuse and not mmapped, previous chunk has OK offset */
  प्रतिज्ञा(is_mmapped(p) || pinuse(p) || अग्र_chunk(पूर्व_chunk(p)) == p);
  यदि (is_mmapped(p))
    do_check_mmapped_chunk(m, p);
}

/* Check properties of free chunks */
स्थिर शून्य do_check_free_chunk(mstate m, mchunkptr p) {
  size_t sz = chunksize(p);
  mchunkptr अग्र = chunk_plus_विस्थापनम्(p, sz);
  do_check_any_chunk(m, p);
  प्रतिज्ञा(!is_inuse(p));
  प्रतिज्ञा(!अग्र_pinuse(p));
  प्रतिज्ञा (!is_mmapped(p));
  यदि (p != m->dv && p != m->शिखरम्) {
    यदि (sz >= MIN_CHUNK_आकार) {
      प्रतिज्ञा((sz & CHUNK_ALIGN_आवरणम्) == 0);
      प्रतिज्ञा(is_aligned(chunk2mem(p)));
      प्रतिज्ञा(अग्र->पूर्व_foot == sz);
      प्रतिज्ञा(pinuse(p));
      प्रतिज्ञा (अग्र == m->शिखरम् || is_inuse(अग्र));
      प्रतिज्ञा(p->fd->bk == p);
      प्रतिज्ञा(p->bk->fd == p);
    }
    अन्यथा  /* markers are always of size SIZE_T_SIZE */
      प्रतिज्ञा(sz == आकार_T_आकार);
  }
}

/* Check properties of malloced chunks at the point they are malloced */
स्थिर शून्य do_check_malloced_chunk(mstate m, शून्य* mem, size_t s) {
  यदि (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    size_t sz = p->शीर्ष & ~INUSE_द्वयङ्काः;
    do_check_inuse_chunk(m, p);
    प्रतिज्ञा((sz & CHUNK_ALIGN_आवरणम्) == 0);
    प्रतिज्ञा(sz >= MIN_CHUNK_आकार);
    प्रतिज्ञा(sz >= s);
    /* unless mmapped, size is less than MIN_CHUNK_SIZE more than request */
    प्रतिज्ञा(is_mmapped(p) || sz < (s + MIN_CHUNK_आकार));
  }
}

/* Check a tree and its subtrees.  */
स्थिर शून्य do_check_वृक्षः(mstate m, tchunkptr t) {
  tchunkptr शीर्ष = 0;
  tchunkptr u = t;
  bindex_t tindex = t->सूचकांक;
  size_t tsize = chunksize(t);
  bindex_t idx;
  compute_वृक्षः_सूचकांक(tsize, idx);
  प्रतिज्ञा(tindex == idx);
  प्रतिज्ञा(tsize >= MIN_LARGE_आकार);
  प्रतिज्ञा(tsize >= minsize_for_वृक्षः_सूचकांक(idx));
  प्रतिज्ञा((idx == NTREEBINS-1) || (tsize < minsize_for_वृक्षः_सूचकांक((idx+1))));

  कुरु { /* traverse through chain of same-sized nodes */
    do_check_any_chunk(m, ((mchunkptr)u));
    प्रतिज्ञा(u->सूचकांक == tindex);
    प्रतिज्ञा(chunksize(u) == tsize);
    प्रतिज्ञा(!is_inuse(u));
    प्रतिज्ञा(!अग्र_pinuse(u));
    प्रतिज्ञा(u->fd->bk == u);
    प्रतिज्ञा(u->bk->fd == u);
    यदि (u->parent == 0) {
      प्रतिज्ञा(u->child[0] == 0);
      प्रतिज्ञा(u->child[1] == 0);
    }
    अन्यथा {
      प्रतिज्ञा(शीर्ष == 0); /* only one node on chain has parent */
      शीर्ष = u;
      प्रतिज्ञा(u->parent != u);
      प्रतिज्ञा (u->parent->child[0] == u ||
              u->parent->child[1] == u ||
              *((tbinptr*)(u->parent)) == u);
      यदि (u->child[0] != 0) {
        प्रतिज्ञा(u->child[0]->parent == u);
        प्रतिज्ञा(u->child[0] != u);
        do_check_वृक्षः(m, u->child[0]);
      }
      यदि (u->child[1] != 0) {
        प्रतिज्ञा(u->child[1]->parent == u);
        प्रतिज्ञा(u->child[1] != u);
        do_check_वृक्षः(m, u->child[1]);
      }
      यदि (u->child[0] != 0 && u->child[1] != 0) {
        प्रतिज्ञा(chunksize(u->child[0]) < chunksize(u->child[1]));
      }
    }
    u = u->fd;
  } यावत् (u != t);
  प्रतिज्ञा(शीर्ष != 0);
}

/*  Check all the chunks in a treebin.  */
स्थिर शून्य do_check_treebin(mstate m, bindex_t i) {
  tbinptr* tb = treebin_at(m, i);
  tchunkptr t = *tb;
  पूर्णाङ्क रिक्तानि = (m->treemap & (1U << i)) == 0;
  यदि (t == 0)
    प्रतिज्ञा(रिक्तानि);
  यदि (!रिक्तानि)
    do_check_वृक्षः(m, t);
}

/*  Check all the chunks in a smallbin.  */
स्थिर शून्य do_check_smallbin(mstate m, bindex_t i) {
  sbinptr b = smallbin_at(m, i);
  mchunkptr p = b->bk;
  अचिह्नित पूर्णाङ्क रिक्तानि = (m->smallmap & (1U << i)) == 0;
  यदि (p == b)
    प्रतिज्ञा(रिक्तानि);
  यदि (!रिक्तानि) {
    प्रति (; p != b; p = p->bk) {
      size_t आकार = chunksize(p);
      mchunkptr q;
      /* each chunk claims to be free */
      do_check_free_chunk(m, p);
      /* chunk belongs in bin */
      प्रतिज्ञा(small_सूचकांक(आकार) == i);
      प्रतिज्ञा(p->bk == b || chunksize(p->bk) == chunksize(p));
      /* chunk is followed by an inuse chunk */
      q = अग्र_chunk(p);
      यदि (q->शीर्ष != FENCEPOST_शीर्ष)
        do_check_inuse_chunk(m, q);
    }
  }
}

/* Find x in a bin. Used in other check functions. */
स्थिर पूर्णाङ्क bin_find(mstate m, mchunkptr x) {
  size_t आकार = chunksize(x);
  यदि (is_small(आकार)) {
    bindex_t sidx = small_सूचकांक(आकार);
    sbinptr b = smallbin_at(m, sidx);
    यदि (smallmap_is_marked(m, sidx)) {
      mchunkptr p = b;
      कुरु {
        यदि (p == x)
          प्रत्यागम 1;
      } यावत् ((p = p->fd) != b);
    }
  }
  अन्यथा {
    bindex_t tidx;
    compute_वृक्षः_सूचकांक(आकार, tidx);
    यदि (treemap_is_marked(m, tidx)) {
      tchunkptr t = *treebin_at(m, tidx);
      size_t sizebits = आकार << leftshift_for_वृक्षः_सूचकांक(tidx);
      यावत् (t != 0 && chunksize(t) != आकार) {
        t = t->child[(sizebits >> (आकार_T_BITSIZE-आकार_T_ONE)) & 1];
        sizebits <<= 1;
      }
      यदि (t != 0) {
        tchunkptr u = t;
        कुरु {
          यदि (u == (tchunkptr)x)
            प्रत्यागम 1;
        } यावत् ((u = u->fd) != t);
      }
    }
  }
  प्रत्यागम 0;
}

/* Traverse each chunk and check it; return total */
स्थिर size_t traverse_and_check(mstate m) {
  size_t योग = 0;
  यदि (is_initialized(m)) {
    msegmentptr s = &m->seg;
    योग += m->topsize + शिखरम्_FOOT_आकार;
    यावत् (s != 0) {
      mchunkptr q = align_as_chunk(s->आधारः);
      mchunkptr lastq = 0;
      प्रतिज्ञा(pinuse(q));
      यावत् (segment_holds(s, q) &&
             q != m->शिखरम् && q->शीर्ष != FENCEPOST_शीर्ष) {
        योग += chunksize(q);
        यदि (is_inuse(q)) {
          प्रतिज्ञा(!bin_find(m, q));
          do_check_inuse_chunk(m, q);
        }
        अन्यथा {
          प्रतिज्ञा(q == m->dv || bin_find(m, q));
          प्रतिज्ञा(lastq == 0 || is_inuse(lastq)); /* Not 2 consecutive free */
          do_check_free_chunk(m, q);
        }
        lastq = q;
        q = अग्र_chunk(q);
      }
      s = s->अग्र;
    }
  }
  प्रत्यागम योग;
}


/* Check all properties of malloc_state. */
स्थिर शून्य do_check_malloc_state(mstate m) {
  bindex_t i;
  size_t योग_2;
  /* check bins */
  प्रति (i = 0; i < NSMALLBINS; ++i)
    do_check_smallbin(m, i);
  प्रति (i = 0; i < NTREEBINS; ++i)
    do_check_treebin(m, i);

  यदि (m->dvsize != 0) { /* check dv chunk */
    do_check_any_chunk(m, m->dv);
    प्रतिज्ञा(m->dvsize == chunksize(m->dv));
    प्रतिज्ञा(m->dvsize >= MIN_CHUNK_आकार);
    प्रतिज्ञा(bin_find(m, m->dv) == 0);
  }

  यदि (m->शिखरम् != 0) {   /* check top chunk */
    do_check_शिखरम्_chunk(m, m->शिखरम्);
    /*assert(m->topsize == chunksize(m->top)); redundant */
    प्रतिज्ञा(m->topsize > 0);
    प्रतिज्ञा(bin_find(m, m->शिखरम्) == 0);
  }

  योग_2 = traverse_and_check(m);
  प्रतिज्ञा(योग_2 <= m->footprint);
  प्रतिज्ञा(m->footprint <= m->max_footprint);
}
#यदि_अन्तः /* DEBUG */

/* ----------------------------- statistics ------------------------------ */

#यदि !NO_MALLINFO
स्थिर संरचना mallinfo आन्तरिकम्_mallinfo(mstate m) {
  संरचना mallinfo nm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  ensure_initialization();
  यदि (!PREACTION(m)) {
    check_malloc_state(m);
    यदि (is_initialized(m)) {
      size_t nfree = आकार_T_ONE; /* top always free */
      size_t mfree = m->topsize + शिखरम्_FOOT_आकार;
      size_t योग = mfree;
      msegmentptr s = &m->seg;
      यावत् (s != 0) {
        mchunkptr q = align_as_chunk(s->आधारः);
        यावत् (segment_holds(s, q) &&
               q != m->शिखरम् && q->शीर्ष != FENCEPOST_शीर्ष) {
          size_t sz = chunksize(q);
          योग += sz;
          यदि (!is_inuse(q)) {
            mfree += sz;
            ++nfree;
          }
          q = अग्र_chunk(q);
        }
        s = s->अग्र;
      }

      nm.arena    = योग;
      nm.ordblks  = nfree;
      nm.hblkhd   = m->footprint - योग;
      nm.usmblks  = m->max_footprint;
      nm.uordblks = m->footprint - mfree;
      nm.fordblks = mfree;
      nm.keepcost = m->topsize;
    }

    POSTACTION(m);
  }
  प्रत्यागम nm;
}
#यदि_अन्तः /* !NO_MALLINFO */

#यदि !NO_MALLOC_STATS
स्थिर शून्य आन्तरिकम्_malloc_stats(mstate m) {
  ensure_initialization();
  यदि (!PREACTION(m)) {
    size_t maxfp = 0;
    size_t fp = 0;
    size_t प्रयुक्तम् = 0;
    check_malloc_state(m);
    यदि (is_initialized(m)) {
      msegmentptr s = &m->seg;
      maxfp = m->max_footprint;
      fp = m->footprint;
      प्रयुक्तम् = fp - (m->topsize + शिखरम्_FOOT_आकार);

      यावत् (s != 0) {
        mchunkptr q = align_as_chunk(s->आधारः);
        यावत् (segment_holds(s, q) &&
               q != m->शिखरम् && q->शीर्ष != FENCEPOST_शीर्ष) {
          यदि (!is_inuse(q))
            प्रयुक्तम् -= chunksize(q);
          q = अग्र_chunk(q);
        }
        s = s->अग्र;
      }
    }
    POSTACTION(m); /* drop lock */
    fprintf(stderr, "max system bytes = %10lu\n", (अचिह्नित दीर्घ)(maxfp));
    fprintf(stderr, "system bytes     = %10lu\n", (अचिह्नित दीर्घ)(fp));
    fprintf(stderr, "in use bytes     = %10lu\n", (अचिह्नित दीर्घ)(प्रयुक्तम्));
  }
}
#यदि_अन्तः /* NO_MALLOC_STATS */

/* ----------------------- Operations on smallbins ----------------------- */

/*
  Various forms of linking and unlinking are defined as macros.  Even
  the ones for trees, which are very long but have very short typical
  paths.  This is ugly but reduces reliance on inlining support of
  compilers.
*/

/* Link a free chunk into a smallbin  */
#परिभाषय निवेशय_small_chunk(M, P, S) {\
  bindex_t I  = small_सूचकांक(S);\
  mchunkptr B = smallbin_at(M, I);\
  mchunkptr F = B;\
  प्रतिज्ञा(S >= MIN_CHUNK_आकार);\
  यदि (!smallmap_is_marked(M, I))\
    mark_smallmap(M, I);\
  अन्यथा यदि (RTCHECK(ok_स्थानाङ्कः(M, B->fd)))\
    F = B->fd;\
  अन्यथा {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
  B->fd = P;\
  F->bk = P;\
  P->fd = F;\
  P->bk = B;\
}

/* Unlink a chunk from a smallbin  */
#परिभाषय unlink_small_chunk(M, P, S) {\
  mchunkptr F = P->fd;\
  mchunkptr B = P->bk;\
  bindex_t I = small_सूचकांक(S);\
  प्रतिज्ञा(P != B);\
  प्रतिज्ञा(P != F);\
  प्रतिज्ञा(chunksize(P) == small_index2size(I));\
  यदि (RTCHECK(F == smallbin_at(M,I) || (ok_स्थानाङ्कः(M, F) && F->bk == P))) { \
    यदि (B == F) {\
      clear_smallmap(M, I);\
    }\
    अन्यथा यदि (RTCHECK(B == smallbin_at(M,I) ||\
                     (ok_स्थानाङ्कः(M, B) && B->fd == P))) {\
      F->bk = B;\
      B->fd = F;\
    }\
    अन्यथा {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  अन्यथा {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Unlink the first chunk from a smallbin */
#परिभाषय unlink_first_small_chunk(M, B, P, I) {\
  mchunkptr F = P->fd;\
  प्रतिज्ञा(P != B);\
  प्रतिज्ञा(P != F);\
  प्रतिज्ञा(chunksize(P) == small_index2size(I));\
  यदि (B == F) {\
    clear_smallmap(M, I);\
  }\
  अन्यथा यदि (RTCHECK(ok_स्थानाङ्कः(M, F) && F->bk == P)) {\
    F->bk = B;\
    B->fd = F;\
  }\
  अन्यथा {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Replace dv node, binning the old one */
/* Used only when dvsize known to be small */
#परिभाषय replace_dv(M, P, S) {\
  size_t DVS = M->dvsize;\
  प्रतिज्ञा(is_small(DVS));\
  यदि (DVS != 0) {\
    mchunkptr DV = M->dv;\
    निवेशय_small_chunk(M, DV, DVS);\
  }\
  M->dvsize = S;\
  M->dv = P;\
}

/* ------------------------- Operations on trees ------------------------- */

/* Insert chunk into tree */
#परिभाषय निवेशय_large_chunk(M, X, S) {\
  tbinptr* H;\
  bindex_t I;\
  compute_वृक्षः_सूचकांक(S, I);\
  H = treebin_at(M, I);\
  X->सूचकांक = I;\
  X->child[0] = X->child[1] = 0;\
  यदि (!treemap_is_marked(M, I)) {\
    mark_treemap(M, I);\
    *H = X;\
    X->parent = (tchunkptr)H;\
    X->fd = X->bk = X;\
  }\
  अन्यथा {\
    tchunkptr T = *H;\
    size_t K = S << leftshift_for_वृक्षः_सूचकांक(I);\
    प्रति (;;) {\
      यदि (chunksize(T) != S) {\
        tchunkptr* C = &(T->child[(K >> (आकार_T_BITSIZE-आकार_T_ONE)) & 1]);\
        K <<= 1;\
        यदि (*C != 0)\
          T = *C;\
        अन्यथा यदि (RTCHECK(ok_स्थानाङ्कः(M, C))) {\
          *C = X;\
          X->parent = T;\
          X->fd = X->bk = X;\
          भङ्ग;\
        }\
        अन्यथा {\
          CORRUPTION_ERROR_ACTION(M);\
          भङ्ग;\
        }\
      }\
      अन्यथा {\
        tchunkptr F = T->fd;\
        यदि (RTCHECK(ok_स्थानाङ्कः(M, T) && ok_स्थानाङ्कः(M, F))) {\
          T->fd = F->bk = X;\
          X->fd = F;\
          X->bk = T;\
          X->parent = 0;\
          भङ्ग;\
        }\
        अन्यथा {\
          CORRUPTION_ERROR_ACTION(M);\
          भङ्ग;\
        }\
      }\
    }\
  }\
}

/*
  Unlink steps:

  1. If x is a chained node, unlink it from its same-sized fd/bk links
     and choose its bk node as its replacement.
  2. If x was the last node of its size, but not a leaf node, it must
     be replaced with a leaf node (not merely one with an open left or
     right), to make sure that lefts and rights of descendants
     correspond properly to bit masks.  We use the rightmost descendant
     of x.  We could use any other leaf, but this is easy to locate and
     tends to counteract removal of leftmosts elsewhere, and so keeps
     paths shorter than minimally guaranteed.  This doesn't loop much
     because on average a node in a tree is near the bottom.
  3. If x is the base of a chain (i.e., has parent links) relink
     x's parent and children to x's replacement (or null if none).
*/

#परिभाषय unlink_large_chunk(M, X) {\
  tchunkptr XP = X->parent;\
  tchunkptr R;\
  यदि (X->bk != X) {\
    tchunkptr F = X->fd;\
    R = X->bk;\
    यदि (RTCHECK(ok_स्थानाङ्कः(M, F) && F->bk == X && R->fd == X)) {\
      F->bk = R;\
      R->fd = F;\
    }\
    अन्यथा {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  अन्यथा {\
    tchunkptr* RP;\
    यदि (((R = *(RP = &(X->child[1]))) != 0) ||\
        ((R = *(RP = &(X->child[0]))) != 0)) {\
      tchunkptr* CP;\
      यावत् ((*(CP = &(R->child[1])) != 0) ||\
             (*(CP = &(R->child[0])) != 0)) {\
        R = *(RP = CP);\
      }\
      यदि (RTCHECK(ok_स्थानाङ्कः(M, RP)))\
        *RP = 0;\
      अन्यथा {\
        CORRUPTION_ERROR_ACTION(M);\
      }\
    }\
  }\
  यदि (XP != 0) {\
    tbinptr* H = treebin_at(M, X->सूचकांक);\
    यदि (X == *H) {\
      यदि ((*H = R) == 0) \
        clear_treemap(M, X->सूचकांक);\
    }\
    अन्यथा यदि (RTCHECK(ok_स्थानाङ्कः(M, XP))) {\
      यदि (XP->child[0] == X) \
        XP->child[0] = R;\
      अन्यथा \
        XP->child[1] = R;\
    }\
    अन्यथा\
      CORRUPTION_ERROR_ACTION(M);\
    यदि (R != 0) {\
      यदि (RTCHECK(ok_स्थानाङ्कः(M, R))) {\
        tchunkptr C0, C1;\
        R->parent = XP;\
        यदि ((C0 = X->child[0]) != 0) {\
          यदि (RTCHECK(ok_स्थानाङ्कः(M, C0))) {\
            R->child[0] = C0;\
            C0->parent = R;\
          }\
          अन्यथा\
            CORRUPTION_ERROR_ACTION(M);\
        }\
        यदि ((C1 = X->child[1]) != 0) {\
          यदि (RTCHECK(ok_स्थानाङ्कः(M, C1))) {\
            R->child[1] = C1;\
            C1->parent = R;\
          }\
          अन्यथा\
            CORRUPTION_ERROR_ACTION(M);\
        }\
      }\
      अन्यथा\
        CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
}

/* Relays to large vs small bin operations */

#परिभाषय निवेशय_chunk(M, P, S)\
  यदि (is_small(S)) निवेशय_small_chunk(M, P, S)\
  अन्यथा { tchunkptr TP = (tchunkptr)(P); निवेशय_large_chunk(M, TP, S); }

#परिभाषय unlink_chunk(M, P, S)\
  यदि (is_small(S)) unlink_small_chunk(M, P, S)\
  अन्यथा { tchunkptr TP = (tchunkptr)(P); unlink_large_chunk(M, TP); }


/* Relays to internal calls to malloc/free from realloc, memalign etc */

#यदि ONLY_MSPACES
#परिभाषय आन्तरिकम्_malloc(m, b) mspace_malloc(m, b)
#परिभाषय आन्तरिकम्_free(m, mem) mspace_free(m,mem);
#अन्यथा /* ONLY_MSPACES */
#यदि MSPACES
#परिभाषय आन्तरिकम्_malloc(m, b)\
  ((m == gm)? dlmalloc(b) : mspace_malloc(m, b))
#परिभाषय आन्तरिकम्_free(m, mem)\
   यदि (m == gm) dlfree(mem); अन्यथा mspace_free(m,mem);
#अन्यथा /* MSPACES */
#परिभाषय आन्तरिकम्_malloc(m, b) dlmalloc(b)
#परिभाषय आन्तरिकम्_free(m, mem) dlfree(mem)
#यदि_अन्तः /* MSPACES */
#यदि_अन्तः /* ONLY_MSPACES */

/* -----------------------  Direct-mmapping chunks ----------------------- */

/*
  Directly mmapped chunks are set up with an offset to the start of
  the mmapped region stored in the prev_foot field of the chunk. This
  allows reconstruction of the required argument to MUNMAP when freed,
  and also allows adjustment of the returned chunk to meet alignment
  requirements (especially in memalign).
*/

/* Malloc using mmap */
स्थिर शून्य* mmap_alloc(mstate m, size_t nb) {
  size_t mmsize = mmap_align(nb + SIX_आकार_T_SIZES + CHUNK_ALIGN_आवरणम्);
  यदि (m->footprint_सीमा != 0) {
    size_t fp = m->footprint + mmsize;
    यदि (fp <= m->footprint || fp > m->footprint_सीमा)
      प्रत्यागम 0;
  }
  यदि (mmsize > nb) {     /* Check for wrap around 0 */
    वर्ण* mm = (वर्ण*)(CALL_प्रत्यक्षम्_MMAP(mmsize));
    यदि (mm != CMFAIL) {
      size_t offset = align_विस्थापनम्(chunk2mem(mm));
      size_t psize = mmsize - offset - MMAP_FOOT_PAD;
      mchunkptr p = (mchunkptr)(mm + offset);
      p->पूर्व_foot = offset;
      p->शीर्ष = psize;
      mark_inuse_foot(m, p, psize);
      chunk_plus_विस्थापनम्(p, psize)->शीर्ष = FENCEPOST_शीर्ष;
      chunk_plus_विस्थापनम्(p, psize+आकार_T_आकार)->शीर्ष = 0;

      यदि (m->न्यूनतमम्_addr == 0 || mm < m->न्यूनतमम्_addr)
        m->न्यूनतमम्_addr = mm;
      यदि ((m->footprint += mmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      प्रतिज्ञा(is_aligned(chunk2mem(p)));
      check_mmapped_chunk(m, p);
      प्रत्यागम chunk2mem(p);
    }
  }
  प्रत्यागम 0;
}

/* Realloc using mmap */
स्थिर mchunkptr mmap_resize(mstate m, mchunkptr oldp, size_t nb, पूर्णाङ्क flags) {
  size_t oldsize = chunksize(oldp);
  (शून्य)flags; /* placate people compiling -Wunused */
  यदि (is_small(nb)) /* Can't shrink mmap regions below small size */
    प्रत्यागम 0;
  /* Keep old chunk if big enough but not too big */
  यदि (oldsize >= nb + आकार_T_आकार &&
      (oldsize - nb) <= (mparams.granularity << 1))
    प्रत्यागम oldp;
  अन्यथा {
    size_t offset = oldp->पूर्व_foot;
    size_t oldmmsize = oldsize + offset + MMAP_FOOT_PAD;
    size_t newmmsize = mmap_align(nb + SIX_आकार_T_SIZES + CHUNK_ALIGN_आवरणम्);
    वर्ण* cp = (वर्ण*)CALL_MREMAP((वर्ण*)oldp - offset,
                                  oldmmsize, newmmsize, flags);
    यदि (cp != CMFAIL) {
      mchunkptr newp = (mchunkptr)(cp + offset);
      size_t psize = newmmsize - offset - MMAP_FOOT_PAD;
      newp->शीर्ष = psize;
      mark_inuse_foot(m, newp, psize);
      chunk_plus_विस्थापनम्(newp, psize)->शीर्ष = FENCEPOST_शीर्ष;
      chunk_plus_विस्थापनम्(newp, psize+आकार_T_आकार)->शीर्ष = 0;

      यदि (cp < m->न्यूनतमम्_addr)
        m->न्यूनतमम्_addr = cp;
      यदि ((m->footprint += newmmsize - oldmmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      check_mmapped_chunk(m, newp);
      प्रत्यागम newp;
    }
  }
  प्रत्यागम 0;
}


/* -------------------------- mspace management -------------------------- */

/* Initialize top chunk and its size */
स्थिर शून्य init_शिखरम्(mstate m, mchunkptr p, size_t psize) {
  /* Ensure alignment */
  size_t offset = align_विस्थापनम्(chunk2mem(p));
  p = (mchunkptr)((वर्ण*)p + offset);
  psize -= offset;

  m->शिखरम् = p;
  m->topsize = psize;
  p->शीर्ष = psize | PINUSE_BIT;
  /* set size of fake trailing chunk holding overhead space only once */
  chunk_plus_विस्थापनम्(p, psize)->शीर्ष = शिखरम्_FOOT_आकार;
  m->trim_check = mparams.trim_देहली; /* reset on each update */
}

/* Initialize bins for a new mstate that is otherwise zeroed out */
स्थिर शून्य init_bins(mstate m) {
  /* Establish circular links for smallbins */
  bindex_t i;
  प्रति (i = 0; i < NSMALLBINS; ++i) {
    sbinptr bin = smallbin_at(m,i);
    bin->fd = bin->bk = bin;
  }
}

#यदि PROCEED_ON_ERROR

/* default corruption action */
स्थिर शून्य reset_on_error(mstate m) {
  पूर्णाङ्क i;
  ++malloc_corruption_error_गणनम्;
  /* Reinitialize fields to forget about all memory */
  m->smallmap = m->treemap = 0;
  m->dvsize = m->topsize = 0;
  m->seg.आधारः = 0;
  m->seg.आकार = 0;
  m->seg.अग्र = 0;
  m->शिखरम् = m->dv = 0;
  प्रति (i = 0; i < NTREEBINS; ++i)
    *treebin_at(m, i) = 0;
  init_bins(m);
}
#यदि_अन्तः /* PROCEED_ON_ERROR */

/* Allocate chunk and prepend remainder with chunk in successor base. */
स्थिर शून्य* prepend_alloc(mstate m, वर्ण* newbase, वर्ण* oldbase,
                           size_t nb) {
  mchunkptr p = align_as_chunk(newbase);
  mchunkptr oldfirst = align_as_chunk(oldbase);
  size_t psize = (वर्ण*)oldfirst - (वर्ण*)p;
  mchunkptr q = chunk_plus_विस्थापनम्(p, nb);
  size_t qsize = psize - nb;
  समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(m, p, nb);

  प्रतिज्ञा((वर्ण*)oldfirst > (वर्ण*)q);
  प्रतिज्ञा(pinuse(oldfirst));
  प्रतिज्ञा(qsize >= MIN_CHUNK_आकार);

  /* consolidate remainder with first chunk of old base */
  यदि (oldfirst == m->शिखरम्) {
    size_t tsize = m->topsize += qsize;
    m->शिखरम् = q;
    q->शीर्ष = tsize | PINUSE_BIT;
    check_शिखरम्_chunk(m, q);
  }
  अन्यथा यदि (oldfirst == m->dv) {
    size_t dsize = m->dvsize += qsize;
    m->dv = q;
    समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(q, dsize);
  }
  अन्यथा {
    यदि (!is_inuse(oldfirst)) {
      size_t nsize = chunksize(oldfirst);
      unlink_chunk(m, oldfirst, nsize);
      oldfirst = chunk_plus_विस्थापनम्(oldfirst, nsize);
      qsize += nsize;
    }
    समुच्चयः_free_with_pinuse(q, qsize, oldfirst);
    निवेशय_chunk(m, q, qsize);
    check_free_chunk(m, q);
  }

  check_malloced_chunk(m, chunk2mem(p), nb);
  प्रत्यागम chunk2mem(p);
}

/* Add a segment to hold a new noncontiguous region */
स्थिर शून्य सङ्कलय_segment(mstate m, वर्ण* tbase, size_t tsize, flag_t mmapped) {
  /* Determine locations and sizes of segment, fenceposts, old top */
  वर्ण* old_शिखरम् = (वर्ण*)m->शिखरम्;
  msegmentptr oldsp = segment_holding(m, old_शिखरम्);
  वर्ण* old_अन्तः = oldsp->आधारः + oldsp->आकार;
  size_t ssize = pad_request(परिमाण(संरचना malloc_segment));
  वर्ण* rawsp = old_अन्तः - (ssize + FOUR_आकार_T_SIZES + CHUNK_ALIGN_आवरणम्);
  size_t offset = align_विस्थापनम्(chunk2mem(rawsp));
  वर्ण* asp = rawsp + offset;
  वर्ण* csp = (asp < (old_शिखरम् + MIN_CHUNK_आकार))? old_शिखरम् : asp;
  mchunkptr sp = (mchunkptr)csp;
  msegmentptr ss = (msegmentptr)(chunk2mem(sp));
  mchunkptr tnext = chunk_plus_विस्थापनम्(sp, ssize);
  mchunkptr p = tnext;
  पूर्णाङ्क nfences = 0;
  (शून्य)nfences; /* Suppress unused variable warning */

  /* reset top to new space */
  init_शिखरम्(m, (mchunkptr)tbase, tsize - शिखरम्_FOOT_आकार);

  /* Set up segment record */
  प्रतिज्ञा(is_aligned(ss));
  समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(m, sp, ssize);
  *ss = m->seg; /* Push current record */
  m->seg.आधारः = tbase;
  m->seg.आकार = tsize;
  (शून्य)समुच्चयः_segment_flags(&m->seg, mmapped);
  m->seg.अग्र = ss;

  /* Insert trailing fenceposts */
  प्रति (;;) {
    mchunkptr nextp = chunk_plus_विस्थापनम्(p, आकार_T_आकार);
    p->शीर्ष = FENCEPOST_शीर्ष;
    ++nfences;
    यदि ((वर्ण*)(&(nextp->शीर्ष)) < old_अन्तः)
      p = nextp;
    अन्यथा
      भङ्ग;
  }
  प्रतिज्ञा(nfences >= 2);

  /* Insert the rest of old top into a bin as an ordinary free chunk */
  यदि (csp != old_शिखरम्) {
    mchunkptr q = (mchunkptr)old_शिखरम्;
    size_t psize = csp - old_शिखरम्;
    mchunkptr tn = chunk_plus_विस्थापनम्(q, psize);
    समुच्चयः_free_with_pinuse(q, psize, tn);
    निवेशय_chunk(m, q, psize);
  }

  check_शिखरम्_chunk(m, m->शिखरम्);
}

/* -------------------------- System allocation -------------------------- */

/* Get memory from system using MORECORE or MMAP */
स्थिर शून्य* sys_alloc(mstate m, size_t nb) {
  वर्ण* tbase = CMFAIL;
  size_t tsize = 0;
  flag_t mmap_flag = 0;
  size_t asize; /* allocation size */

  ensure_initialization();

  /* Directly map large chunks, but only if already initialized */
  यदि (use_mmap(m) && nb >= mparams.mmap_देहली && m->topsize != 0) {
    शून्य* mem = mmap_alloc(m, nb);
    यदि (mem != 0)
      प्रत्यागम mem;
  }

  asize = granularity_align(nb + SYS_ALLOC_PADDING);
  यदि (asize <= nb)
    प्रत्यागम 0; /* wraparound */
  यदि (m->footprint_सीमा != 0) {
    size_t fp = m->footprint + asize;
    यदि (fp <= m->footprint || fp > m->footprint_सीमा)
      प्रत्यागम 0;
  }

  /*
    Try getting memory in any of three ways (in most-preferred to
    least-preferred order):
    1. A call to MORECORE that can normally contiguously extend memory.
       (disabled if not MORECORE_CONTIGUOUS or not HAVE_MORECORE or
       or main space is mmapped or a previous contiguous call failed)
    2. A call to MMAP new space (disabled if not HAVE_MMAP).
       Note that under the default settings, if MORECORE is unable to
       fulfill a request, and HAVE_MMAP is true, then mmap is
       used as a noncontiguous system allocator. This is a useful backup
       strategy for systems with holes in address spaces -- in this case
       sbrk cannot contiguously expand the heap, but mmap may be able to
       find space.
    3. A call to MORECORE that cannot usually contiguously extend memory.
       (disabled if not HAVE_MORECORE)

   In all cases, we need to request enough bytes from system to ensure
   we can malloc nb bytes upon success, so pad with enough space for
   top_foot, plus alignment-pad to make sure we don't lose bytes if
   not on boundary, and round this up to a granularity unit.
  */

  यदि (MORECORE_CONTIGUOUS && !use_noncontiguous(m)) {
    वर्ण* br = CMFAIL;
    size_t ssize = asize; /* sbrk call size */
    msegmentptr ss = (m->शिखरम् == 0)? 0 : segment_holding(m, (वर्ण*)m->शिखरम्);
    ACQUIRE_MALLOC_GLOBAL_तालकम्();

    यदि (ss == 0) {  /* First time through or recovery */
      वर्ण* आधारः = (वर्ण*)CALL_MORECORE(0);
      यदि (आधारः != CMFAIL) {
        size_t fp;
        /* Adjust to end on a page boundary */
        यदि (!is_page_aligned(आधारः))
          ssize += (page_align((size_t)आधारः) - (size_t)आधारः);
        fp = m->footprint + ssize; /* recheck limits */
        यदि (ssize > nb && ssize < HALF_MAX_आकार_T &&
            (m->footprint_सीमा == 0 ||
             (fp > m->footprint && fp <= m->footprint_सीमा)) &&
            (br = (वर्ण*)(CALL_MORECORE(ssize))) == आधारः) {
          tbase = आधारः;
          tsize = ssize;
        }
      }
    }
    अन्यथा {
      /* Subtract out existing available top space from MORECORE request. */
      ssize = granularity_align(nb - m->topsize + SYS_ALLOC_PADDING);
      /* Use mem here only if it did continuously extend old space */
      यदि (ssize < HALF_MAX_आकार_T &&
          (br = (वर्ण*)(CALL_MORECORE(ssize))) == ss->आधारः+ss->आकार) {
        tbase = br;
        tsize = ssize;
      }
    }

    यदि (tbase == CMFAIL) {    /* Cope with partial failure */
      यदि (br != CMFAIL) {    /* Try to use/extend the space we did get */
        यदि (ssize < HALF_MAX_आकार_T &&
            ssize < nb + SYS_ALLOC_PADDING) {
          size_t esize = granularity_align(nb + SYS_ALLOC_PADDING - ssize);
          यदि (esize < HALF_MAX_आकार_T) {
            वर्ण* अन्तः = (वर्ण*)CALL_MORECORE(esize);
            यदि (अन्तः != CMFAIL)
              ssize += esize;
            अन्यथा {            /* Can't use; try to release */
              (शून्य) CALL_MORECORE(-ssize);
              br = CMFAIL;
            }
          }
        }
      }
      यदि (br != CMFAIL) {    /* Use the space we did get */
        tbase = br;
        tsize = ssize;
      }
      अन्यथा
        disable_contiguous(m); /* Don't try contiguous path in the future */
    }

    RELEASE_MALLOC_GLOBAL_तालकम्();
  }

  यदि (HAVE_MMAP && tbase == CMFAIL) {  /* Try MMAP */
    वर्ण* mp = (वर्ण*)(CALL_MMAP(asize));
    यदि (mp != CMFAIL) {
      tbase = mp;
      tsize = asize;
      mmap_flag = USE_MMAP_BIT;
    }
  }

  यदि (HAVE_MORECORE && tbase == CMFAIL) { /* Try noncontiguous MORECORE */
    यदि (asize < HALF_MAX_आकार_T) {
      वर्ण* br = CMFAIL;
      वर्ण* अन्तः = CMFAIL;
      ACQUIRE_MALLOC_GLOBAL_तालकम्();
      br = (वर्ण*)(CALL_MORECORE(asize));
      अन्तः = (वर्ण*)(CALL_MORECORE(0));
      RELEASE_MALLOC_GLOBAL_तालकम्();
      यदि (br != CMFAIL && अन्तः != CMFAIL && br < अन्तः) {
        size_t ssize = अन्तः - br;
        यदि (ssize > nb + शिखरम्_FOOT_आकार) {
          tbase = br;
          tsize = ssize;
        }
      }
    }
  }

  यदि (tbase != CMFAIL) {

    यदि ((m->footprint += tsize) > m->max_footprint)
      m->max_footprint = m->footprint;

    यदि (!is_initialized(m)) { /* first-time initialization */
      यदि (m->न्यूनतमम्_addr == 0 || tbase < m->न्यूनतमम्_addr)
        m->न्यूनतमम्_addr = tbase;
      m->seg.आधारः = tbase;
      m->seg.आकार = tsize;
      (शून्य)समुच्चयः_segment_flags(&m->seg, mmap_flag);
      m->magic = mparams.magic;
      m->release_checks = MAX_RELEASE_CHECK_दरः;
      init_bins(m);
#यदि !ONLY_MSPACES
      यदि (is_global(m))
        init_शिखरम्(m, (mchunkptr)tbase, tsize - शिखरम्_FOOT_आकार);
      अन्यथा
#यदि_अन्तः
      {
        /* Offset top by embedded malloc_state */
        mchunkptr mn = अग्र_chunk(mem2chunk(m));
        init_शिखरम्(m, mn, (size_t)((tbase + tsize) - (वर्ण*)mn) -शिखरम्_FOOT_आकार);
      }
    }

    अन्यथा {
      /* Try to merge with an existing segment */
      msegmentptr sp = &m->seg;
      /* Only consider most recent segment if traversal suppressed */
      यावत् (sp != 0 && tbase != sp->आधारः + sp->आकार)
        sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->अग्र;
      यदि (sp != 0 &&
          !is_extern_segment(sp) &&
          check_segment_merge(sp, tbase, tsize) &&
          (get_segment_flags(sp) & USE_MMAP_BIT) == mmap_flag &&
          segment_holds(sp, m->शिखरम्)) { /* append */
        sp->आकार += tsize;
        init_शिखरम्(m, m->शिखरम्, m->topsize + tsize);
      }
      अन्यथा {
        यदि (tbase < m->न्यूनतमम्_addr)
          m->न्यूनतमम्_addr = tbase;
        sp = &m->seg;
        यावत् (sp != 0 && sp->आधारः != tbase + tsize)
          sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->अग्र;
        यदि (sp != 0 &&
            !is_extern_segment(sp) &&
            check_segment_merge(sp, tbase, tsize) &&
            (get_segment_flags(sp) & USE_MMAP_BIT) == mmap_flag) {
          वर्ण* oldbase = sp->आधारः;
          sp->आधारः = tbase;
          sp->आकार += tsize;
          प्रत्यागम prepend_alloc(m, tbase, oldbase, nb);
        }
        अन्यथा
          सङ्कलय_segment(m, tbase, tsize, mmap_flag);
      }
    }

    यदि (nb < m->topsize) { /* Allocate from new or extended top space */
      size_t rsize = m->topsize -= nb;
      mchunkptr p = m->शिखरम्;
      mchunkptr r = m->शिखरम् = chunk_plus_विस्थापनम्(p, nb);
      r->शीर्ष = rsize | PINUSE_BIT;
      समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(m, p, nb);
      check_शिखरम्_chunk(m, m->शिखरम्);
      check_malloced_chunk(m, chunk2mem(p), nb);
      प्रत्यागम chunk2mem(p);
    }
  }

  MALLOC_FAILURE_ACTION;
  प्रत्यागम 0;
}

/* -----------------------  system deallocation -------------------------- */

/* Unmap and unlink any mmapped segments that don't contain used chunks */
स्थिर size_t release_अप्रयुक्तम्_segments(mstate m) {
  size_t released = 0;
  पूर्णाङ्क nsegs = 0;
  msegmentptr pred = &m->seg;
  msegmentptr sp = pred->अग्र;
  यावत् (sp != 0) {
    वर्ण* आधारः = sp->आधारः;
    size_t आकार = sp->आकार;
    msegmentptr अग्र = sp->अग्र;
    ++nsegs;
    यदि (is_mmapped_segment(sp) && !is_extern_segment(sp)) {
      mchunkptr p = align_as_chunk(आधारः);
      size_t psize = chunksize(p);
      /* Can unmap if first chunk holds entire segment and not pinned */
      यदि (!is_inuse(p) && (वर्ण*)p + psize >= आधारः + आकार - शिखरम्_FOOT_आकार) {
        tchunkptr tp = (tchunkptr)p;
        प्रतिज्ञा(segment_holds(sp, (वर्ण*)sp));
        यदि (p == m->dv) {
          m->dv = 0;
          m->dvsize = 0;
        }
        अन्यथा {
          unlink_large_chunk(m, tp);
        }
        यदि (CALL_MUNMAP(आधारः, आकार) == 0) {
          released += आकार;
          m->footprint -= आकार;
          /* unlink obsoleted record */
          sp = pred;
          sp->अग्र = अग्र;
        }
        अन्यथा { /* back out if cannot unmap */
          निवेशय_large_chunk(m, tp, psize);
        }
      }
    }
    यदि (NO_SEGMENT_TRAVERSAL) /* scan only first segment */
      भङ्ग;
    pred = sp;
    sp = अग्र;
  }
  /* Reset check counter */
  m->release_checks = (((size_t) nsegs > (size_t) MAX_RELEASE_CHECK_दरः)?
                       (size_t) nsegs : (size_t) MAX_RELEASE_CHECK_दरः);
  प्रत्यागम released;
}

स्थिर पूर्णाङ्क sys_trim(mstate m, size_t pad) {
  size_t released = 0;
  ensure_initialization();
  यदि (pad < MAX_REQUEST && is_initialized(m)) {
    pad += शिखरम्_FOOT_आकार; /* ensure enough room for segment overhead */

    यदि (m->topsize > pad) {
      /* Shrink top space in granularity-size units, keeping at least one */
      size_t unit = mparams.granularity;
      size_t extra = ((m->topsize - pad + (unit - आकार_T_ONE)) / unit -
                      आकार_T_ONE) * unit;
      msegmentptr sp = segment_holding(m, (वर्ण*)m->शिखरम्);

      यदि (sp == 0)
        CORRUPTION_ERROR_ACTION(m);
      यदि (!is_extern_segment(sp)) {
        यदि (is_mmapped_segment(sp)) {
          यदि (HAVE_MMAP &&
              sp->आकार >= extra &&
              !has_segment_link(m, sp)) { /* can't shrink if pinned */
            size_t newsize = sp->आकार - extra;
            (शून्य)newsize; /* placate people compiling -Wunused-variable */
            /* Prefer mremap, fall back to munmap */
            यदि ((CALL_MREMAP(sp->आधारः, sp->आकार, newsize, 0) != MFAIL) ||
                (CALL_MUNMAP(sp->आधारः + newsize, extra) == 0)) {
              released = extra;
            }
          }
        }
        अन्यथा यदि (HAVE_MORECORE) {
          यदि (extra >= HALF_MAX_आकार_T) /* Avoid wrapping negative */
            extra = (HALF_MAX_आकार_T) + आकार_T_ONE - unit;
          ACQUIRE_MALLOC_GLOBAL_तालकम्();
          {
            /* Make sure end of memory is where we last set it. */
            वर्ण* old_br = (वर्ण*)(CALL_MORECORE(0));
            यदि (old_br == sp->आधारः + sp->आकार) {
              वर्ण* rel_br = (वर्ण*)(CALL_MORECORE(-extra));
              वर्ण* new_br = (वर्ण*)(CALL_MORECORE(0));
              यदि (rel_br != CMFAIL && new_br < old_br)
                released = old_br - new_br;
            }
          }
          RELEASE_MALLOC_GLOBAL_तालकम्();
        }
      }

      यदि (released != 0) {
        sp->आकार -= released;
        m->footprint -= released;
        init_शिखरम्(m, m->शिखरम्, m->topsize - released);
        check_शिखरम्_chunk(m, m->शिखरम्);
      }
    }

    /* Unmap any unused mmapped segments */
    यदि (HAVE_MMAP)
      released += release_अप्रयुक्तम्_segments(m);

    /* On failure, disable autotrim to avoid repeated failed future calls */
    यदि (released == 0 && m->topsize > m->trim_check)
      m->trim_check = MAX_आकार_T;
  }

  प्रत्यागम (released != 0)? 1 : 0;
}

/* Consolidate and bin a chunk. Differs from exported versions
   of free mainly in that the chunk need not be marked as inuse.
*/
स्थिर शून्य dispose_chunk(mstate m, mchunkptr p, size_t psize) {
  mchunkptr अग्र = chunk_plus_विस्थापनम्(p, psize);
  यदि (!pinuse(p)) {
    mchunkptr पूर्व;
    size_t prevsize = p->पूर्व_foot;
    यदि (is_mmapped(p)) {
      psize += prevsize + MMAP_FOOT_PAD;
      यदि (CALL_MUNMAP((वर्ण*)p - prevsize, psize) == 0)
        m->footprint -= psize;
      प्रत्यागम;
    }
    पूर्व = chunk_minus_विस्थापनम्(p, prevsize);
    psize += prevsize;
    p = पूर्व;
    यदि (RTCHECK(ok_स्थानाङ्कः(m, पूर्व))) { /* consolidate backward */
      यदि (p != m->dv) {
        unlink_chunk(m, p, prevsize);
      }
      अन्यथा यदि ((अग्र->शीर्ष & INUSE_द्वयङ्काः) == INUSE_द्वयङ्काः) {
        m->dvsize = psize;
        समुच्चयः_free_with_pinuse(p, psize, अग्र);
        प्रत्यागम;
      }
    }
    अन्यथा {
      CORRUPTION_ERROR_ACTION(m);
      प्रत्यागम;
    }
  }
  यदि (RTCHECK(ok_स्थानाङ्कः(m, अग्र))) {
    यदि (!cinuse(अग्र)) {  /* consolidate forward */
      यदि (अग्र == m->शिखरम्) {
        size_t tsize = m->topsize += psize;
        m->शिखरम् = p;
        p->शीर्ष = tsize | PINUSE_BIT;
        यदि (p == m->dv) {
          m->dv = 0;
          m->dvsize = 0;
        }
        प्रत्यागम;
      }
      अन्यथा यदि (अग्र == m->dv) {
        size_t dsize = m->dvsize += psize;
        m->dv = p;
        समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(p, dsize);
        प्रत्यागम;
      }
      अन्यथा {
        size_t nsize = chunksize(अग्र);
        psize += nsize;
        unlink_chunk(m, अग्र, nsize);
        समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(p, psize);
        यदि (p == m->dv) {
          m->dvsize = psize;
          प्रत्यागम;
        }
      }
    }
    अन्यथा {
      समुच्चयः_free_with_pinuse(p, psize, अग्र);
    }
    निवेशय_chunk(m, p, psize);
  }
  अन्यथा {
    CORRUPTION_ERROR_ACTION(m);
  }
}

/* ---------------------------- malloc --------------------------- */

/* allocate a large request from the best fitting chunk in a treebin */
स्थिर शून्य* tmalloc_large(mstate m, size_t nb) {
  tchunkptr v = 0;
  size_t rsize = -nb; /* Unsigned negation */
  tchunkptr t;
  bindex_t idx;
  compute_वृक्षः_सूचकांक(nb, idx);
  यदि ((t = *treebin_at(m, idx)) != 0) {
    /* Traverse tree for this bin looking for node with size == nb */
    size_t sizebits = nb << leftshift_for_वृक्षः_सूचकांक(idx);
    tchunkptr rst = 0;  /* The deepest untaken right subtree */
    प्रति (;;) {
      tchunkptr rt;
      size_t trem = chunksize(t) - nb;
      यदि (trem < rsize) {
        v = t;
        यदि ((rsize = trem) == 0)
          भङ्ग;
      }
      rt = t->child[1];
      t = t->child[(sizebits >> (आकार_T_BITSIZE-आकार_T_ONE)) & 1];
      यदि (rt != 0 && rt != t)
        rst = rt;
      यदि (t == 0) {
        t = rst; /* set t to least subtree holding sizes > nb */
        भङ्ग;
      }
      sizebits <<= 1;
    }
  }
  यदि (t == 0 && v == 0) { /* set t to root of next non-empty treebin */
    binmap_t leftbits = वामः_द्वयङ्काः(idx2bit(idx)) & m->treemap;
    यदि (leftbits != 0) {
      bindex_t i;
      binmap_t leastbit = न्यूनतमम्_bit(leftbits);
      compute_bit2idx(leastbit, i);
      t = *treebin_at(m, i);
    }
  }

  यावत् (t != 0) { /* find smallest of tree or subtree */
    size_t trem = chunksize(t) - nb;
    यदि (trem < rsize) {
      rsize = trem;
      v = t;
    }
    t = leftmost_child(t);
  }

  /*  If dv is a better fit, return 0 so malloc will use it */
  यदि (v != 0 && rsize < (size_t)(m->dvsize - nb)) {
    यदि (RTCHECK(ok_स्थानाङ्कः(m, v))) { /*विभज*/
      mchunkptr r = chunk_plus_विस्थापनम्(v, nb);
      प्रतिज्ञा(chunksize(v) == rsize + nb);
      यदि (RTCHECK(ok_अग्र(v, r))) {
        unlink_large_chunk(m, v);
        यदि (rsize < MIN_CHUNK_आकार)
          समुच्चयः_inuse_and_pinuse(m, v, (rsize + nb));
        अन्यथा {
          समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(m, v, nb);
          समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(r, rsize);
          निवेशय_chunk(m, r, rsize);
        }
        प्रत्यागम chunk2mem(v);
      }
    }
    CORRUPTION_ERROR_ACTION(m);
  }
  प्रत्यागम 0;
}

/* allocate a small request from the best fitting chunk in a treebin */
स्थिर शून्य* tmalloc_small(mstate m, size_t nb) {
  tchunkptr t, v;
  size_t rsize;
  bindex_t i;
  binmap_t leastbit = न्यूनतमम्_bit(m->treemap);
  compute_bit2idx(leastbit, i);
  v = t = *treebin_at(m, i);
  rsize = chunksize(t) - nb;

  यावत् ((t = leftmost_child(t)) != 0) {
    size_t trem = chunksize(t) - nb;
    यदि (trem < rsize) {
      rsize = trem;
      v = t;
    }
  }

  यदि (RTCHECK(ok_स्थानाङ्कः(m, v))) {
    mchunkptr r = chunk_plus_विस्थापनम्(v, nb);
    प्रतिज्ञा(chunksize(v) == rsize + nb);
    यदि (RTCHECK(ok_अग्र(v, r))) {
      unlink_large_chunk(m, v);
      यदि (rsize < MIN_CHUNK_आकार)
        समुच्चयः_inuse_and_pinuse(m, v, (rsize + nb));
      अन्यथा {
        समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(m, v, nb);
        समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(r, rsize);
        replace_dv(m, r, rsize);
      }
      प्रत्यागम chunk2mem(v);
    }
  }

  CORRUPTION_ERROR_ACTION(m);
  प्रत्यागम 0;
}

#यदि !ONLY_MSPACES

शून्य* dlmalloc(size_t अष्टकानि) {
  /*
     Basic algorithm:
     If a small request (< 256 bytes minus per-chunk overhead):
       1. If one exists, use a remainderless chunk in associated smallbin.
          (Remainderless means that there are too few excess bytes to
          represent as a chunk.)
       2. If it is big enough, use the dv chunk, which is normally the
          chunk adjacent to the one used for the most recent small request.
       3. If one exists, split the smallest available chunk in a bin,
          saving remainder in dv.
       4. If it is big enough, use the top chunk.
       5. If available, get memory from system and use it
     Otherwise, for a large request:
       1. Find the smallest available binned chunk that fits, and use it
          if it is better fitting than dv chunk, splitting if necessary.
       2. If better fitting than any binned chunk, use the dv chunk.
       3. If it is big enough, use the top chunk.
       4. If request size >= mmap threshold, try to directly mmap this chunk.
       5. If available, get memory from system and use it

     The ugly goto's here ensure that postaction occurs along all paths.
  */

#यदि USE_LOCKS
  ensure_initialization(); /* initialize in sys_alloc if not using locks */
#यदि_अन्तः

  यदि (!PREACTION(gm)) {
    शून्य* mem;
    size_t nb;
    यदि (अष्टकानि <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (अष्टकानि < MIN_REQUEST)? MIN_CHUNK_आकार : pad_request(अष्टकानि);
      idx = small_सूचकांक(nb);
      smallbits = gm->smallmap >> idx;

      यदि ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(gm, idx);
        p = b->fd;
        प्रतिज्ञा(chunksize(p) == small_index2size(idx));
        unlink_first_small_chunk(gm, b, p, idx);
        समुच्चयः_inuse_and_pinuse(gm, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(gm, mem, nb);
        गच्छ postaction;
      }

      अन्यथा यदि (nb > gm->dvsize) {
        यदि (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & वामः_द्वयङ्काः(idx2bit(idx));
          binmap_t leastbit = न्यूनतमम्_bit(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(gm, i);
          p = b->fd;
          प्रतिज्ञा(chunksize(p) == small_index2size(i));
          unlink_first_small_chunk(gm, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          यदि (आकार_T_आकार != 4 && rsize < MIN_CHUNK_आकार)
            समुच्चयः_inuse_and_pinuse(gm, p, small_index2size(i));
          अन्यथा {
            समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(gm, p, nb);
            r = chunk_plus_विस्थापनम्(p, nb);
            समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(r, rsize);
            replace_dv(gm, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(gm, mem, nb);
          गच्छ postaction;
        }

        अन्यथा यदि (gm->treemap != 0 && (mem = tmalloc_small(gm, nb)) != 0) {
          check_malloced_chunk(gm, mem, nb);
          गच्छ postaction;
        }
      }
    }
    अन्यथा यदि (अष्टकानि >= MAX_REQUEST)
      nb = MAX_आकार_T; /* Too big to allocate. Force failure (in sys alloc) */
    अन्यथा {
      nb = pad_request(अष्टकानि);
      यदि (gm->treemap != 0 && (mem = tmalloc_large(gm, nb)) != 0) {
        check_malloced_chunk(gm, mem, nb);
        गच्छ postaction;
      }
    }

    यदि (nb <= gm->dvsize) {
      size_t rsize = gm->dvsize - nb;
      mchunkptr p = gm->dv;
      यदि (rsize >= MIN_CHUNK_आकार) { /* split dv */
        mchunkptr r = gm->dv = chunk_plus_विस्थापनम्(p, nb);
        gm->dvsize = rsize;
        समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(r, rsize);
        समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(gm, p, nb);
      }
      अन्यथा { /* exhaust dv */
        size_t dvs = gm->dvsize;
        gm->dvsize = 0;
        gm->dv = 0;
        समुच्चयः_inuse_and_pinuse(gm, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(gm, mem, nb);
      गच्छ postaction;
    }

    अन्यथा यदि (nb < gm->topsize) { /* Split top */
      size_t rsize = gm->topsize -= nb;
      mchunkptr p = gm->शिखरम्;
      mchunkptr r = gm->शिखरम् = chunk_plus_विस्थापनम्(p, nb);
      r->शीर्ष = rsize | PINUSE_BIT;
      समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(gm, p, nb);
      mem = chunk2mem(p);
      check_शिखरम्_chunk(gm, gm->शिखरम्);
      check_malloced_chunk(gm, mem, nb);
      गच्छ postaction;
    }

    mem = sys_alloc(gm, nb);

  postaction:
    POSTACTION(gm);
    प्रत्यागम mem;
  }

  प्रत्यागम 0;
}

/* ---------------------------- free --------------------------- */

शून्य dlfree(शून्य* mem) {
  /*
     Consolidate freed chunks with preceding or succeeding bordering
     free chunks, if they exist, and then place in a bin.  Intermixed
     with special cases for top, dv, mmapped chunks, and usage errors.
  */

  यदि (mem != 0) {
    mchunkptr p  = mem2chunk(mem);
#यदि FOOTERS
    mstate fm = get_mstate_for(p);
    यदि (!ok_magic(fm)) {
      USAGE_ERROR_ACTION(fm, p);
      प्रत्यागम;
    }
#अन्यथा /* FOOTERS */
#परिभाषय fm gm
#यदि_अन्तः /* FOOTERS */
    यदि (!PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      यदि (RTCHECK(ok_स्थानाङ्कः(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr अग्र = chunk_plus_विस्थापनम्(p, psize);
        यदि (!pinuse(p)) {
          size_t prevsize = p->पूर्व_foot;
          यदि (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            यदि (CALL_MUNMAP((वर्ण*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            गच्छ postaction;
          }
          अन्यथा {
            mchunkptr पूर्व = chunk_minus_विस्थापनम्(p, prevsize);
            psize += prevsize;
            p = पूर्व;
            यदि (RTCHECK(ok_स्थानाङ्कः(fm, पूर्व))) { /* consolidate backward */
              यदि (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              अन्यथा यदि ((अग्र->शीर्ष & INUSE_द्वयङ्काः) == INUSE_द्वयङ्काः) {
                fm->dvsize = psize;
                समुच्चयः_free_with_pinuse(p, psize, अग्र);
                गच्छ postaction;
              }
            }
            अन्यथा
              गच्छ erroraction;
          }
        }

        यदि (RTCHECK(ok_अग्र(p, अग्र) && ok_pinuse(अग्र))) {
          यदि (!cinuse(अग्र)) {  /* consolidate forward */
            यदि (अग्र == fm->शिखरम्) {
              size_t tsize = fm->topsize += psize;
              fm->शिखरम् = p;
              p->शीर्ष = tsize | PINUSE_BIT;
              यदि (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              यदि (should_trim(fm, tsize))
                sys_trim(fm, 0);
              गच्छ postaction;
            }
            अन्यथा यदि (अग्र == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(p, dsize);
              गच्छ postaction;
            }
            अन्यथा {
              size_t nsize = chunksize(अग्र);
              psize += nsize;
              unlink_chunk(fm, अग्र, nsize);
              समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(p, psize);
              यदि (p == fm->dv) {
                fm->dvsize = psize;
                गच्छ postaction;
              }
            }
          }
          अन्यथा
            समुच्चयः_free_with_pinuse(p, psize, अग्र);

          यदि (is_small(psize)) {
            निवेशय_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          अन्यथा {
            tchunkptr tp = (tchunkptr)p;
            निवेशय_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            यदि (--fm->release_checks == 0)
              release_अप्रयुक्तम्_segments(fm);
          }
          गच्छ postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
#यदि !FOOTERS
#अपरिभाषय fm
#यदि_अन्तः /* FOOTERS */
}

शून्य* dlcalloc(size_t n_elements, size_t elem_आकार) {
  शून्य* mem;
  size_t req = 0;
  यदि (n_elements != 0) {
    req = n_elements * elem_आकार;
    यदि (((n_elements | elem_आकार) & ~(size_t)0xffff) &&
        (req / n_elements != elem_आकार))
      req = MAX_आकार_T; /* force downstream failure on overflow */
  }
  mem = dlmalloc(req);
  यदि (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    स्मृतिं_पूरय(mem, 0, req);
  प्रत्यागम mem;
}

#यदि_अन्तः /* !ONLY_MSPACES */

/* ------------ Internal support for realloc, memalign, etc -------------- */

/* Try to realloc; only in-place unless can_move true */
स्थिर mchunkptr try_realloc_chunk(mstate m, mchunkptr p, size_t nb,
                                   पूर्णाङ्क can_move) {
  mchunkptr newp = 0;
  size_t oldsize = chunksize(p);
  mchunkptr अग्र = chunk_plus_विस्थापनम्(p, oldsize);
  यदि (RTCHECK(ok_स्थानाङ्कः(m, p) && ok_inuse(p) &&
              ok_अग्र(p, अग्र) && ok_pinuse(अग्र))) {
    यदि (is_mmapped(p)) {
      newp = mmap_resize(m, p, nb, can_move);
    }
    अन्यथा यदि (oldsize >= nb) {             /* already big enough */
      size_t rsize = oldsize - nb;
      यदि (rsize >= MIN_CHUNK_आकार) {      /* split off remainder */
        mchunkptr r = chunk_plus_विस्थापनम्(p, nb);
        समुच्चयः_inuse(m, p, nb);
        समुच्चयः_inuse(m, r, rsize);
        dispose_chunk(m, r, rsize);
      }
      newp = p;
    }
    अन्यथा यदि (अग्र == m->शिखरम्) {  /* extend into top */
      यदि (oldsize + m->topsize > nb) {
        size_t newsize = oldsize + m->topsize;
        size_t newtopsize = newsize - nb;
        mchunkptr newtop = chunk_plus_विस्थापनम्(p, nb);
        समुच्चयः_inuse(m, p, nb);
        newtop->शीर्ष = newtopsize |PINUSE_BIT;
        m->शिखरम् = newtop;
        m->topsize = newtopsize;
        newp = p;
      }
    }
    अन्यथा यदि (अग्र == m->dv) { /* extend into dv */
      size_t dvs = m->dvsize;
      यदि (oldsize + dvs >= nb) {
        size_t dsize = oldsize + dvs - nb;
        यदि (dsize >= MIN_CHUNK_आकार) {
          mchunkptr r = chunk_plus_विस्थापनम्(p, nb);
          mchunkptr n = chunk_plus_विस्थापनम्(r, dsize);
          समुच्चयः_inuse(m, p, nb);
          समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(r, dsize);
          clear_pinuse(n);
          m->dvsize = dsize;
          m->dv = r;
        }
        अन्यथा { /* exhaust dv */
          size_t newsize = oldsize + dvs;
          समुच्चयः_inuse(m, p, newsize);
          m->dvsize = 0;
          m->dv = 0;
        }
        newp = p;
      }
    }
    अन्यथा यदि (!cinuse(अग्र)) { /* extend into next free chunk */
      size_t nextsize = chunksize(अग्र);
      यदि (oldsize + nextsize >= nb) {
        size_t rsize = oldsize + nextsize - nb;
        unlink_chunk(m, अग्र, nextsize);
        यदि (rsize < MIN_CHUNK_आकार) {
          size_t newsize = oldsize + nextsize;
          समुच्चयः_inuse(m, p, newsize);
        }
        अन्यथा {
          mchunkptr r = chunk_plus_विस्थापनम्(p, nb);
          समुच्चयः_inuse(m, p, nb);
          समुच्चयः_inuse(m, r, rsize);
          dispose_chunk(m, r, rsize);
        }
        newp = p;
      }
    }
  }
  अन्यथा {
    USAGE_ERROR_ACTION(m, chunk2mem(p));
  }
  प्रत्यागम newp;
}

स्थिर शून्य* आन्तरिकम्_memalign(mstate m, size_t alignment, size_t अष्टकानि) {
  शून्य* mem = 0;
  यदि (alignment <  MIN_CHUNK_आकार) /* must be at least a minimum chunk size */
    alignment = MIN_CHUNK_आकार;
  यदि ((alignment & (alignment-आकार_T_ONE)) != 0) {/* Ensure a power of 2 */
    size_t a = MALLOC_ALIGNMENT << 1;
    यावत् (a < alignment) a <<= 1;
    alignment = a;
  }
  यदि (अष्टकानि >= MAX_REQUEST - alignment) {
    यदि (m != 0)  { /* Test isn't needed but avoids compiler warning */
      MALLOC_FAILURE_ACTION;
    }
  }
  अन्यथा {
    size_t nb = request2size(अष्टकानि);
    size_t req = nb + alignment + MIN_CHUNK_आकार - CHUNK_OVERHEAD;
    mem = आन्तरिकम्_malloc(m, req);
    यदि (mem != 0) {
      mchunkptr p = mem2chunk(mem);
      यदि (PREACTION(m))
        प्रत्यागम 0;
      यदि ((((size_t)(mem)) & (alignment - 1)) != 0) { /* misaligned */
        /*
          Find an aligned spot inside chunk.  Since we need to give
          back leading space in a chunk of at least MIN_CHUNK_SIZE, if
          the first calculation places us at a spot with less than
          MIN_CHUNK_SIZE leader, we can move to the next aligned spot.
          We've allocated enough total room so that this is always
          possible.
        */
        वर्ण* br = (वर्ण*)mem2chunk((size_t)(((size_t)((वर्ण*)mem + alignment -
                                                       आकार_T_ONE)) &
                                             -alignment));
        वर्ण* pos = ((size_t)(br - (वर्ण*)(p)) >= MIN_CHUNK_आकार)?
          br : br+alignment;
        mchunkptr newp = (mchunkptr)pos;
        size_t leadsize = pos - (वर्ण*)(p);
        size_t newsize = chunksize(p) - leadsize;

        यदि (is_mmapped(p)) { /* For mmapped chunks, just adjust offset */
          newp->पूर्व_foot = p->पूर्व_foot + leadsize;
          newp->शीर्ष = newsize;
        }
        अन्यथा { /* Otherwise, give back leader, use the rest */
          समुच्चयः_inuse(m, newp, newsize);
          समुच्चयः_inuse(m, p, leadsize);
          dispose_chunk(m, p, leadsize);
        }
        p = newp;
      }

      /* Give back spare room at the end */
      यदि (!is_mmapped(p)) {
        size_t आकार = chunksize(p);
        यदि (आकार > nb + MIN_CHUNK_आकार) {
          size_t remainder_आकार = आकार - nb;
          mchunkptr remainder = chunk_plus_विस्थापनम्(p, nb);
          समुच्चयः_inuse(m, p, nb);
          समुच्चयः_inuse(m, remainder, remainder_आकार);
          dispose_chunk(m, remainder, remainder_आकार);
        }
      }

      mem = chunk2mem(p);
      प्रतिज्ञा (chunksize(p) >= nb);
      प्रतिज्ञा(((size_t)mem & (alignment - 1)) == 0);
      check_inuse_chunk(m, p);
      POSTACTION(m);
    }
  }
  प्रत्यागम mem;
}

/*
  Common support for independent_X routines, handling
    all of the combinations that can result.
  The opts arg has:
    bit 0 set if all elements are same size (using sizes[0])
    bit 1 set if elements should be zeroed
*/
स्थिर शून्य** ialloc(mstate m,
                     size_t n_elements,
                     size_t* sizes,
                     पूर्णाङ्क opts,
                     शून्य* chunks[]) {

  size_t    element_आकार;   /* chunksize of each element, if all same */
  size_t    contents_आकार;  /* total size of elements */
  size_t    array_आकार;     /* request size of pointer array */
  शून्य*     mem;            /* malloced aggregate space */
  mchunkptr p;              /* corresponding chunk */
  size_t    remainder_आकार; /* remaining bytes while splitting */
  शून्य**    marray;         /* either "chunks" or malloced ptr array */
  mchunkptr array_chunk;    /* chunk for malloced ptr array */
  flag_t    was_enabled;    /* to disable mmap */
  size_t    आकार;
  size_t    i;

  ensure_initialization();
  /* compute array length, if needed */
  यदि (chunks != 0) {
    यदि (n_elements == 0)
      प्रत्यागम chunks; /* nothing to do */
    marray = chunks;
    array_आकार = 0;
  }
  अन्यथा {
    /* if empty req, must still return chunk representing empty array */
    यदि (n_elements == 0)
      प्रत्यागम (शून्य**)आन्तरिकम्_malloc(m, 0);
    marray = 0;
    array_आकार = request2size(n_elements * (परिमाण(शून्य*)));
  }

  /* compute total element size */
  यदि (opts & 0x1) { /* all-same-size */
    element_आकार = request2size(*sizes);
    contents_आकार = n_elements * element_आकार;
  }
  अन्यथा { /* add up all the sizes */
    element_आकार = 0;
    contents_आकार = 0;
    प्रति (i = 0; i != n_elements; ++i)
      contents_आकार += request2size(sizes[i]);
  }

  आकार = contents_आकार + array_आकार;

  /*
     Allocate the aggregate chunk.  First disable direct-mmapping so
     malloc won't use it, since we would not be able to later
     free/realloc space internal to a segregated mmap region.
  */
  was_enabled = use_mmap(m);
  disable_mmap(m);
  mem = आन्तरिकम्_malloc(m, आकार - CHUNK_OVERHEAD);
  यदि (was_enabled)
    enable_mmap(m);
  यदि (mem == 0)
    प्रत्यागम 0;

  यदि (PREACTION(m)) प्रत्यागम 0;
  p = mem2chunk(mem);
  remainder_आकार = chunksize(p);

  प्रतिज्ञा(!is_mmapped(p));

  यदि (opts & 0x2) {       /* optionally clear the elements */
    स्मृतिं_पूरय((size_t*)mem, 0, remainder_आकार - आकार_T_आकार - array_आकार);
  }

  /* If not provided, allocate the pointer array as final part of chunk */
  यदि (marray == 0) {
    size_t  array_chunk_आकार;
    array_chunk = chunk_plus_विस्थापनम्(p, contents_आकार);
    array_chunk_आकार = remainder_आकार - contents_आकार;
    marray = (शून्य**) (chunk2mem(array_chunk));
    समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(m, array_chunk, array_chunk_आकार);
    remainder_आकार = contents_आकार;
  }

  /* split out elements */
  प्रति (i = 0; ; ++i) {
    marray[i] = chunk2mem(p);
    यदि (i != n_elements-1) {
      यदि (element_आकार != 0)
        आकार = element_आकार;
      अन्यथा
        आकार = request2size(sizes[i]);
      remainder_आकार -= आकार;
      समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(m, p, आकार);
      p = chunk_plus_विस्थापनम्(p, आकार);
    }
    अन्यथा { /* the final element absorbs any overallocation slop */
      समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(m, p, remainder_आकार);
      भङ्ग;
    }
  }

#यदि DEBUG
  यदि (marray != chunks) {
    /* final element must have exactly exhausted chunk */
    यदि (element_आकार != 0) {
      प्रतिज्ञा(remainder_आकार == element_आकार);
    }
    अन्यथा {
      प्रतिज्ञा(remainder_आकार == request2size(sizes[i]));
    }
    check_inuse_chunk(m, mem2chunk(marray));
  }
  प्रति (i = 0; i != n_elements; ++i)
    check_inuse_chunk(m, mem2chunk(marray[i]));

#यदि_अन्तः /* DEBUG */

  POSTACTION(m);
  प्रत्यागम marray;
}

/* Try to free all pointers in the given array.
   Note: this could be made faster, by delaying consolidation,
   at the price of disabling some user integrity checks, We
   still optimize some consolidations by combining adjacent
   chunks before freeing, which will occur often if allocated
   with ialloc or the array is sorted.
*/
स्थिर size_t आन्तरिकम्_bulk_free(mstate m, शून्य* array[], size_t nelem) {
  size_t unfreed = 0;
  यदि (!PREACTION(m)) {
    शून्य** a;
    शून्य** fence = &(array[nelem]);
    प्रति (a = array; a != fence; ++a) {
      शून्य* mem = *a;
      यदि (mem != 0) {
        mchunkptr p = mem2chunk(mem);
        size_t psize = chunksize(p);
#यदि FOOTERS
        यदि (get_mstate_for(p) != m) {
          ++unfreed;
          अनुवर्त;
        }
#यदि_अन्तः
        check_inuse_chunk(m, p);
        *a = 0;
        यदि (RTCHECK(ok_स्थानाङ्कः(m, p) && ok_inuse(p))) {
          शून्य ** b = a + 1; /* try to merge with next chunk */
          mchunkptr अग्र = अग्र_chunk(p);
          यदि (b != fence && *b == chunk2mem(अग्र)) {
            size_t newsize = chunksize(अग्र) + psize;
            समुच्चयः_inuse(m, p, newsize);
            *b = chunk2mem(p);
          }
          अन्यथा
            dispose_chunk(m, p, psize);
        }
        अन्यथा {
          CORRUPTION_ERROR_ACTION(m);
          भङ्ग;
        }
      }
    }
    यदि (should_trim(m, m->topsize))
      sys_trim(m, 0);
    POSTACTION(m);
  }
  प्रत्यागम unfreed;
}

/* Traversal */
#यदि MALLOC_INSPECT_ALL
स्थिर शून्य आन्तरिकम्_inspect_all(mstate m,
                                 शून्य(*handler)(शून्य *start,
                                                शून्य *अन्तः,
                                                size_t प्रयुक्तम्_अष्टकानि,
                                                शून्य* callback_arg),
                                 शून्य* arg) {
  यदि (is_initialized(m)) {
    mchunkptr शिखरम् = m->शिखरम्;
    msegmentptr s;
    प्रति (s = &m->seg; s != 0; s = s->अग्र) {
      mchunkptr q = align_as_chunk(s->आधारः);
      यावत् (segment_holds(s, q) && q->शीर्ष != FENCEPOST_शीर्ष) {
        mchunkptr अग्र = अग्र_chunk(q);
        size_t sz = chunksize(q);
        size_t प्रयुक्तम्;
        शून्य* start;
        यदि (is_inuse(q)) {
          प्रयुक्तम् = sz - CHUNK_OVERHEAD; /* must not be mmapped */
          start = chunk2mem(q);
        }
        अन्यथा {
          प्रयुक्तम् = 0;
          यदि (is_small(sz)) {     /* offset by possible bookkeeping */
            start = (शून्य*)((वर्ण*)q + परिमाण(संरचना malloc_chunk));
          }
          अन्यथा {
            start = (शून्य*)((वर्ण*)q + परिमाण(संरचना malloc_वृक्षः_chunk));
          }
        }
        यदि (start < (शून्य*)अग्र)  /* skip if all space is bookkeeping */
          handler(start, अग्र, प्रयुक्तम्, arg);
        यदि (q == शिखरम्)
          भङ्ग;
        q = अग्र;
      }
    }
  }
}
#यदि_अन्तः /* MALLOC_INSPECT_ALL */

/* ------------------ Exported realloc, memalign, etc -------------------- */

#यदि !ONLY_MSPACES

शून्य* dlrealloc(शून्य* oldmem, size_t अष्टकानि) {
  शून्य* mem = 0;
  यदि (oldmem == 0) {
    mem = dlmalloc(अष्टकानि);
  }
  अन्यथा यदि (अष्टकानि >= MAX_REQUEST) {
    MALLOC_FAILURE_ACTION;
  }
#यदि_परिभाषितम् REALLOC_ZERO_अष्टकानि_FREES
  अन्यथा यदि (अष्टकानि == 0) {
    dlfree(oldmem);
  }
#यदि_अन्तः /* REALLOC_ZERO_BYTES_FREES */
  अन्यथा {
    size_t nb = request2size(अष्टकानि);
    mchunkptr oldp = mem2chunk(oldmem);
#यदि ! FOOTERS
    mstate m = gm;
#अन्यथा /* FOOTERS */
    mstate m = get_mstate_for(oldp);
    यदि (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m, oldmem);
      प्रत्यागम 0;
    }
#यदि_अन्तः /* FOOTERS */
    यदि (!PREACTION(m)) {
      mchunkptr newp = try_realloc_chunk(m, oldp, nb, 1);
      POSTACTION(m);
      यदि (newp != 0) {
        check_inuse_chunk(m, newp);
        mem = chunk2mem(newp);
      }
      अन्यथा {
        mem = आन्तरिकम्_malloc(m, अष्टकानि);
        यदि (mem != 0) {
          size_t oc = chunksize(oldp) - overhead_for(oldp);
          स्मृतिं_प्रतिलिख(mem, oldmem, (oc < अष्टकानि)? oc : अष्टकानि);
          आन्तरिकम्_free(m, oldmem);
        }
      }
    }
  }
  प्रत्यागम mem;
}

शून्य* dlrealloc_अन्तर्_place(शून्य* oldmem, size_t अष्टकानि) {
  शून्य* mem = 0;
  यदि (oldmem != 0) {
    यदि (अष्टकानि >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
    }
    अन्यथा {
      size_t nb = request2size(अष्टकानि);
      mchunkptr oldp = mem2chunk(oldmem);
#यदि ! FOOTERS
      mstate m = gm;
#अन्यथा /* FOOTERS */
      mstate m = get_mstate_for(oldp);
      यदि (!ok_magic(m)) {
        USAGE_ERROR_ACTION(m, oldmem);
        प्रत्यागम 0;
      }
#यदि_अन्तः /* FOOTERS */
      यदि (!PREACTION(m)) {
        mchunkptr newp = try_realloc_chunk(m, oldp, nb, 0);
        POSTACTION(m);
        यदि (newp == oldp) {
          check_inuse_chunk(m, newp);
          mem = oldmem;
        }
      }
    }
  }
  प्रत्यागम mem;
}

शून्य* dlmemalign(size_t alignment, size_t अष्टकानि) {
  यदि (alignment <= MALLOC_ALIGNMENT) {
    प्रत्यागम dlmalloc(अष्टकानि);
  }
  प्रत्यागम आन्तरिकम्_memalign(gm, alignment, अष्टकानि);
}

पूर्णाङ्क dlposix_memalign(शून्य** pp, size_t alignment, size_t अष्टकानि) {
  शून्य* mem = 0;
  यदि (alignment == MALLOC_ALIGNMENT)
    mem = dlmalloc(अष्टकानि);
  अन्यथा {
    size_t d = alignment / परिमाण(शून्य*);
    size_t r = alignment % परिमाण(शून्य*);
    यदि (r != 0 || d == 0 || (d & (d-आकार_T_ONE)) != 0)
      प्रत्यागम EINVAL;
    अन्यथा यदि (अष्टकानि <= MAX_REQUEST - alignment) {
      यदि (alignment <  MIN_CHUNK_आकार)
        alignment = MIN_CHUNK_आकार;
      mem = आन्तरिकम्_memalign(gm, alignment, अष्टकानि);
    }
  }
  यदि (mem == 0)
    प्रत्यागम ENOMEM;
  अन्यथा {
    *pp = mem;
    प्रत्यागम 0;
  }
}

शून्य* dlvalloc(size_t अष्टकानि) {
  size_t pagesz;
  ensure_initialization();
  pagesz = mparams.page_आकार;
  प्रत्यागम dlmemalign(pagesz, अष्टकानि);
}

शून्य* dlpvalloc(size_t अष्टकानि) {
  size_t pagesz;
  ensure_initialization();
  pagesz = mparams.page_आकार;
  प्रत्यागम dlmemalign(pagesz, (अष्टकानि + pagesz - आकार_T_ONE) & ~(pagesz - आकार_T_ONE));
}

शून्य** dlindependent_calloc(size_t n_elements, size_t elem_आकार,
                            शून्य* chunks[]) {
  size_t sz = elem_आकार; /* serves as 1-element array */
  प्रत्यागम ialloc(gm, n_elements, &sz, 3, chunks);
}

शून्य** dlindependent_comalloc(size_t n_elements, size_t sizes[],
                              शून्य* chunks[]) {
  प्रत्यागम ialloc(gm, n_elements, sizes, 0, chunks);
}

size_t dlbulk_free(शून्य* array[], size_t nelem) {
  प्रत्यागम आन्तरिकम्_bulk_free(gm, array, nelem);
}

#यदि MALLOC_INSPECT_ALL
शून्य dlmalloc_inspect_all(शून्य(*handler)(शून्य *start,
                                         शून्य *अन्तः,
                                         size_t प्रयुक्तम्_अष्टकानि,
                                         शून्य* callback_arg),
                          शून्य* arg) {
  ensure_initialization();
  यदि (!PREACTION(gm)) {
    आन्तरिकम्_inspect_all(gm, handler, arg);
    POSTACTION(gm);
  }
}
#यदि_अन्तः /* MALLOC_INSPECT_ALL */

पूर्णाङ्क dlmalloc_trim(size_t pad) {
  पूर्णाङ्क परिणाम = 0;
  ensure_initialization();
  यदि (!PREACTION(gm)) {
    परिणाम = sys_trim(gm, pad);
    POSTACTION(gm);
  }
  प्रत्यागम परिणाम;
}

size_t dlmalloc_footprint(शून्य) {
  प्रत्यागम gm->footprint;
}

size_t dlmalloc_max_footprint(शून्य) {
  प्रत्यागम gm->max_footprint;
}

size_t dlmalloc_footprint_सीमा(शून्य) {
  size_t maf = gm->footprint_सीमा;
  प्रत्यागम maf == 0 ? MAX_आकार_T : maf;
}

size_t dlmalloc_समुच्चयः_footprint_सीमा(size_t अष्टकानि) {
  size_t परिणाम;  /* invert sense of 0 */
  यदि (अष्टकानि == 0)
    परिणाम = granularity_align(1); /* Use minimal size */
  यदि (अष्टकानि == MAX_आकार_T)
    परिणाम = 0;                    /* disable */
  अन्यथा
    परिणाम = granularity_align(अष्टकानि);
  प्रत्यागम gm->footprint_सीमा = परिणाम;
}

#यदि !NO_MALLINFO
संरचना mallinfo dlmallinfo(शून्य) {
  प्रत्यागम आन्तरिकम्_mallinfo(gm);
}
#यदि_अन्तः /* NO_MALLINFO */

#यदि !NO_MALLOC_STATS
शून्य dlmalloc_stats(शून्य) {
  आन्तरिकम्_malloc_stats(gm);
}
#यदि_अन्तः /* NO_MALLOC_STATS */

पूर्णाङ्क dlmallopt(पूर्णाङ्क param_सङ्ख्या, पूर्णाङ्क मानम्) {
  प्रत्यागम change_mparam(param_सङ्ख्या, मानम्);
}

size_t dlmalloc_usable_आकार(शून्य* mem) {
  यदि (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    यदि (is_inuse(p))
      प्रत्यागम chunksize(p) - overhead_for(p);
  }
  प्रत्यागम 0;
}

#यदि_अन्तः /* !ONLY_MSPACES */

/* ----------------------------- user mspaces ---------------------------- */

#यदि MSPACES

स्थिर mstate init_user_mstate(वर्ण* tbase, size_t tsize) {
  size_t msize = pad_request(परिमाण(संरचना malloc_state));
  mchunkptr mn;
  mchunkptr msp = align_as_chunk(tbase);
  mstate m = (mstate)(chunk2mem(msp));
  स्मृतिं_पूरय(m, 0, msize);
  (शून्य)INITIAL_तालकम्(&m->mutex);
  msp->शीर्ष = (msize|INUSE_द्वयङ्काः);
  m->seg.आधारः = m->न्यूनतमम्_addr = tbase;
  m->seg.आकार = m->footprint = m->max_footprint = tsize;
  m->magic = mparams.magic;
  m->release_checks = MAX_RELEASE_CHECK_दरः;
  m->mflags = mparams.default_mflags;
  m->extp = 0;
  m->exts = 0;
  disable_contiguous(m);
  init_bins(m);
  mn = अग्र_chunk(mem2chunk(m));
  init_शिखरम्(m, mn, (size_t)((tbase + tsize) - (वर्ण*)mn) - शिखरम्_FOOT_आकार);
  check_शिखरम्_chunk(m, m->शिखरम्);
  प्रत्यागम m;
}

mspace create_mspace(size_t क्षमता, पूर्णाङ्क locked) {
  mstate m = 0;
  size_t msize;
  ensure_initialization();
  msize = pad_request(परिमाण(संरचना malloc_state));
  यदि (क्षमता < (size_t) -(msize + शिखरम्_FOOT_आकार + mparams.page_आकार)) {
    size_t rs = ((क्षमता == 0)? mparams.granularity :
                 (क्षमता + शिखरम्_FOOT_आकार + msize));
    size_t tsize = granularity_align(rs);
    वर्ण* tbase = (वर्ण*)(CALL_MMAP(tsize));
    यदि (tbase != CMFAIL) {
      m = init_user_mstate(tbase, tsize);
      समुच्चयः_segment_flags(&m->seg, USE_MMAP_BIT);
      समुच्चयः_तालकम्(m, locked);
    }
  }
  प्रत्यागम (mspace)m;
}

mspace create_mspace_with_आधारः(शून्य* आधारः, size_t क्षमता, पूर्णाङ्क locked) {
  mstate m = 0;
  size_t msize;
  ensure_initialization();
  msize = pad_request(परिमाण(संरचना malloc_state));
  यदि (क्षमता > msize + शिखरम्_FOOT_आकार &&
      क्षमता < (size_t) -(msize + शिखरम्_FOOT_आकार + mparams.page_आकार)) {
    m = init_user_mstate((वर्ण*)आधारः, क्षमता);
    समुच्चयः_segment_flags(&m->seg, EXTERN_BIT);
    समुच्चयः_तालकम्(m, locked);
  }
  प्रत्यागम (mspace)m;
}

पूर्णाङ्क mspace_track_large_chunks(mspace msp, पूर्णाङ्क enable) {
  पूर्णाङ्क ret = 0;
  mstate ms = (mstate)msp;
  यदि (!PREACTION(ms)) {
    यदि (!use_mmap(ms)) {
      ret = 1;
    }
    यदि (!enable) {
      enable_mmap(ms);
    } अन्यथा {
      disable_mmap(ms);
    }
    POSTACTION(ms);
  }
  प्रत्यागम ret;
}

size_t destroy_mspace(mspace msp) {
  size_t freed = 0;
  mstate ms = (mstate)msp;
  यदि (ok_magic(ms)) {
    msegmentptr sp = &ms->seg;
    (शून्य)DESTROY_तालकम्(&ms->mutex); /* destroy before unmapped */
    यावत् (sp != 0) {
      वर्ण* आधारः = sp->आधारः;
      size_t आकार = sp->आकार;
      flag_t flag = get_segment_flags(sp);
      (शून्य)आधारः; /* placate people compiling -Wunused-variable */
      sp = sp->अग्र;
      यदि ((flag & USE_MMAP_BIT) && !(flag & EXTERN_BIT) &&
          CALL_MUNMAP(आधारः, आकार) == 0)
        freed += आकार;
    }
  }
  अन्यथा {
    USAGE_ERROR_ACTION(ms,ms);
  }
  प्रत्यागम freed;
}

/*
  mspace versions of routines are near-clones of the global
  versions. This is not so nice but better than the alternatives.
*/

शून्य* mspace_malloc(mspace msp, size_t अष्टकानि) {
  mstate ms = (mstate)msp;
  यदि (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    प्रत्यागम 0;
  }
  यदि (!PREACTION(ms)) {
    शून्य* mem;
    size_t nb;
    यदि (अष्टकानि <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (अष्टकानि < MIN_REQUEST)? MIN_CHUNK_आकार : pad_request(अष्टकानि);
      idx = small_सूचकांक(nb);
      smallbits = ms->smallmap >> idx;

      यदि ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(ms, idx);
        p = b->fd;
        प्रतिज्ञा(chunksize(p) == small_index2size(idx));
        unlink_first_small_chunk(ms, b, p, idx);
        समुच्चयः_inuse_and_pinuse(ms, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(ms, mem, nb);
        गच्छ postaction;
      }

      अन्यथा यदि (nb > ms->dvsize) {
        यदि (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & वामः_द्वयङ्काः(idx2bit(idx));
          binmap_t leastbit = न्यूनतमम्_bit(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(ms, i);
          p = b->fd;
          प्रतिज्ञा(chunksize(p) == small_index2size(i));
          unlink_first_small_chunk(ms, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          यदि (आकार_T_आकार != 4 && rsize < MIN_CHUNK_आकार)
            समुच्चयः_inuse_and_pinuse(ms, p, small_index2size(i));
          अन्यथा {
            समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(ms, p, nb);
            r = chunk_plus_विस्थापनम्(p, nb);
            समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(r, rsize);
            replace_dv(ms, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(ms, mem, nb);
          गच्छ postaction;
        }

        अन्यथा यदि (ms->treemap != 0 && (mem = tmalloc_small(ms, nb)) != 0) {
          check_malloced_chunk(ms, mem, nb);
          गच्छ postaction;
        }
      }
    }
    अन्यथा यदि (अष्टकानि >= MAX_REQUEST)
      nb = MAX_आकार_T; /* Too big to allocate. Force failure (in sys alloc) */
    अन्यथा {
      nb = pad_request(अष्टकानि);
      यदि (ms->treemap != 0 && (mem = tmalloc_large(ms, nb)) != 0) {
        check_malloced_chunk(ms, mem, nb);
        गच्छ postaction;
      }
    }

    यदि (nb <= ms->dvsize) {
      size_t rsize = ms->dvsize - nb;
      mchunkptr p = ms->dv;
      यदि (rsize >= MIN_CHUNK_आकार) { /* split dv */
        mchunkptr r = ms->dv = chunk_plus_विस्थापनम्(p, nb);
        ms->dvsize = rsize;
        समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(r, rsize);
        समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(ms, p, nb);
      }
      अन्यथा { /* exhaust dv */
        size_t dvs = ms->dvsize;
        ms->dvsize = 0;
        ms->dv = 0;
        समुच्चयः_inuse_and_pinuse(ms, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(ms, mem, nb);
      गच्छ postaction;
    }

    अन्यथा यदि (nb < ms->topsize) { /* Split top */
      size_t rsize = ms->topsize -= nb;
      mchunkptr p = ms->शिखरम्;
      mchunkptr r = ms->शिखरम् = chunk_plus_विस्थापनम्(p, nb);
      r->शीर्ष = rsize | PINUSE_BIT;
      समुच्चयः_आकार_and_pinuse_सम्बन्धि_inuse_chunk(ms, p, nb);
      mem = chunk2mem(p);
      check_शिखरम्_chunk(ms, ms->शिखरम्);
      check_malloced_chunk(ms, mem, nb);
      गच्छ postaction;
    }

    mem = sys_alloc(ms, nb);

  postaction:
    POSTACTION(ms);
    प्रत्यागम mem;
  }

  प्रत्यागम 0;
}

शून्य mspace_free(mspace msp, शून्य* mem) {
  यदि (mem != 0) {
    mchunkptr p  = mem2chunk(mem);
#यदि FOOTERS
    mstate fm = get_mstate_for(p);
    (शून्य)msp; /* placate people compiling -Wunused */
#अन्यथा /* FOOTERS */
    mstate fm = (mstate)msp;
#यदि_अन्तः /* FOOTERS */
    यदि (!ok_magic(fm)) {
      USAGE_ERROR_ACTION(fm, p);
      प्रत्यागम;
    }
    यदि (!PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      यदि (RTCHECK(ok_स्थानाङ्कः(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr अग्र = chunk_plus_विस्थापनम्(p, psize);
        यदि (!pinuse(p)) {
          size_t prevsize = p->पूर्व_foot;
          यदि (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            यदि (CALL_MUNMAP((वर्ण*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            गच्छ postaction;
          }
          अन्यथा {
            mchunkptr पूर्व = chunk_minus_विस्थापनम्(p, prevsize);
            psize += prevsize;
            p = पूर्व;
            यदि (RTCHECK(ok_स्थानाङ्कः(fm, पूर्व))) { /* consolidate backward */
              यदि (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              अन्यथा यदि ((अग्र->शीर्ष & INUSE_द्वयङ्काः) == INUSE_द्वयङ्काः) {
                fm->dvsize = psize;
                समुच्चयः_free_with_pinuse(p, psize, अग्र);
                गच्छ postaction;
              }
            }
            अन्यथा
              गच्छ erroraction;
          }
        }

        यदि (RTCHECK(ok_अग्र(p, अग्र) && ok_pinuse(अग्र))) {
          यदि (!cinuse(अग्र)) {  /* consolidate forward */
            यदि (अग्र == fm->शिखरम्) {
              size_t tsize = fm->topsize += psize;
              fm->शिखरम् = p;
              p->शीर्ष = tsize | PINUSE_BIT;
              यदि (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              यदि (should_trim(fm, tsize))
                sys_trim(fm, 0);
              गच्छ postaction;
            }
            अन्यथा यदि (अग्र == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(p, dsize);
              गच्छ postaction;
            }
            अन्यथा {
              size_t nsize = chunksize(अग्र);
              psize += nsize;
              unlink_chunk(fm, अग्र, nsize);
              समुच्चयः_आकार_and_pinuse_सम्बन्धि_free_chunk(p, psize);
              यदि (p == fm->dv) {
                fm->dvsize = psize;
                गच्छ postaction;
              }
            }
          }
          अन्यथा
            समुच्चयः_free_with_pinuse(p, psize, अग्र);

          यदि (is_small(psize)) {
            निवेशय_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          अन्यथा {
            tchunkptr tp = (tchunkptr)p;
            निवेशय_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            यदि (--fm->release_checks == 0)
              release_अप्रयुक्तम्_segments(fm);
          }
          गच्छ postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
}

शून्य* mspace_calloc(mspace msp, size_t n_elements, size_t elem_आकार) {
  शून्य* mem;
  size_t req = 0;
  mstate ms = (mstate)msp;
  यदि (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    प्रत्यागम 0;
  }
  यदि (n_elements != 0) {
    req = n_elements * elem_आकार;
    यदि (((n_elements | elem_आकार) & ~(size_t)0xffff) &&
        (req / n_elements != elem_आकार))
      req = MAX_आकार_T; /* force downstream failure on overflow */
  }
  mem = आन्तरिकम्_malloc(ms, req);
  यदि (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    स्मृतिं_पूरय(mem, 0, req);
  प्रत्यागम mem;
}

शून्य* mspace_realloc(mspace msp, शून्य* oldmem, size_t अष्टकानि) {
  शून्य* mem = 0;
  यदि (oldmem == 0) {
    mem = mspace_malloc(msp, अष्टकानि);
  }
  अन्यथा यदि (अष्टकानि >= MAX_REQUEST) {
    MALLOC_FAILURE_ACTION;
  }
#यदि_परिभाषितम् REALLOC_ZERO_अष्टकानि_FREES
  अन्यथा यदि (अष्टकानि == 0) {
    mspace_free(msp, oldmem);
  }
#यदि_अन्तः /* REALLOC_ZERO_BYTES_FREES */
  अन्यथा {
    size_t nb = request2size(अष्टकानि);
    mchunkptr oldp = mem2chunk(oldmem);
#यदि ! FOOTERS
    mstate m = (mstate)msp;
#अन्यथा /* FOOTERS */
    mstate m = get_mstate_for(oldp);
    यदि (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m, oldmem);
      प्रत्यागम 0;
    }
#यदि_अन्तः /* FOOTERS */
    यदि (!PREACTION(m)) {
      mchunkptr newp = try_realloc_chunk(m, oldp, nb, 1);
      POSTACTION(m);
      यदि (newp != 0) {
        check_inuse_chunk(m, newp);
        mem = chunk2mem(newp);
      }
      अन्यथा {
        mem = mspace_malloc(m, अष्टकानि);
        यदि (mem != 0) {
          size_t oc = chunksize(oldp) - overhead_for(oldp);
          स्मृतिं_प्रतिलिख(mem, oldmem, (oc < अष्टकानि)? oc : अष्टकानि);
          mspace_free(m, oldmem);
        }
      }
    }
  }
  प्रत्यागम mem;
}

शून्य* mspace_realloc_अन्तर्_place(mspace msp, शून्य* oldmem, size_t अष्टकानि) {
  शून्य* mem = 0;
  यदि (oldmem != 0) {
    यदि (अष्टकानि >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
    }
    अन्यथा {
      size_t nb = request2size(अष्टकानि);
      mchunkptr oldp = mem2chunk(oldmem);
#यदि ! FOOTERS
      mstate m = (mstate)msp;
#अन्यथा /* FOOTERS */
      mstate m = get_mstate_for(oldp);
      (शून्य)msp; /* placate people compiling -Wunused */
      यदि (!ok_magic(m)) {
        USAGE_ERROR_ACTION(m, oldmem);
        प्रत्यागम 0;
      }
#यदि_अन्तः /* FOOTERS */
      यदि (!PREACTION(m)) {
        mchunkptr newp = try_realloc_chunk(m, oldp, nb, 0);
        POSTACTION(m);
        यदि (newp == oldp) {
          check_inuse_chunk(m, newp);
          mem = oldmem;
        }
      }
    }
  }
  प्रत्यागम mem;
}

शून्य* mspace_memalign(mspace msp, size_t alignment, size_t अष्टकानि) {
  mstate ms = (mstate)msp;
  यदि (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    प्रत्यागम 0;
  }
  यदि (alignment <= MALLOC_ALIGNMENT)
    प्रत्यागम mspace_malloc(msp, अष्टकानि);
  प्रत्यागम आन्तरिकम्_memalign(ms, alignment, अष्टकानि);
}

शून्य** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_आकार, शून्य* chunks[]) {
  size_t sz = elem_आकार; /* serves as 1-element array */
  mstate ms = (mstate)msp;
  यदि (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    प्रत्यागम 0;
  }
  प्रत्यागम ialloc(ms, n_elements, &sz, 3, chunks);
}

शून्य** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], शून्य* chunks[]) {
  mstate ms = (mstate)msp;
  यदि (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    प्रत्यागम 0;
  }
  प्रत्यागम ialloc(ms, n_elements, sizes, 0, chunks);
}

size_t mspace_bulk_free(mspace msp, शून्य* array[], size_t nelem) {
  प्रत्यागम आन्तरिकम्_bulk_free((mstate)msp, array, nelem);
}

#यदि MALLOC_INSPECT_ALL
शून्य mspace_inspect_all(mspace msp,
                        शून्य(*handler)(शून्य *start,
                                       शून्य *अन्तः,
                                       size_t प्रयुक्तम्_अष्टकानि,
                                       शून्य* callback_arg),
                        शून्य* arg) {
  mstate ms = (mstate)msp;
  यदि (ok_magic(ms)) {
    यदि (!PREACTION(ms)) {
      आन्तरिकम्_inspect_all(ms, handler, arg);
      POSTACTION(ms);
    }
  }
  अन्यथा {
    USAGE_ERROR_ACTION(ms,ms);
  }
}
#यदि_अन्तः /* MALLOC_INSPECT_ALL */

पूर्णाङ्क mspace_trim(mspace msp, size_t pad) {
  पूर्णाङ्क परिणाम = 0;
  mstate ms = (mstate)msp;
  यदि (ok_magic(ms)) {
    यदि (!PREACTION(ms)) {
      परिणाम = sys_trim(ms, pad);
      POSTACTION(ms);
    }
  }
  अन्यथा {
    USAGE_ERROR_ACTION(ms,ms);
  }
  प्रत्यागम परिणाम;
}

#यदि !NO_MALLOC_STATS
शून्य mspace_malloc_stats(mspace msp) {
  mstate ms = (mstate)msp;
  यदि (ok_magic(ms)) {
    आन्तरिकम्_malloc_stats(ms);
  }
  अन्यथा {
    USAGE_ERROR_ACTION(ms,ms);
  }
}
#यदि_अन्तः /* NO_MALLOC_STATS */

size_t mspace_footprint(mspace msp) {
  size_t परिणाम = 0;
  mstate ms = (mstate)msp;
  यदि (ok_magic(ms)) {
    परिणाम = ms->footprint;
  }
  अन्यथा {
    USAGE_ERROR_ACTION(ms,ms);
  }
  प्रत्यागम परिणाम;
}

size_t mspace_max_footprint(mspace msp) {
  size_t परिणाम = 0;
  mstate ms = (mstate)msp;
  यदि (ok_magic(ms)) {
    परिणाम = ms->max_footprint;
  }
  अन्यथा {
    USAGE_ERROR_ACTION(ms,ms);
  }
  प्रत्यागम परिणाम;
}

size_t mspace_footprint_सीमा(mspace msp) {
  size_t परिणाम = 0;
  mstate ms = (mstate)msp;
  यदि (ok_magic(ms)) {
    size_t maf = ms->footprint_सीमा;
    परिणाम = (maf == 0) ? MAX_आकार_T : maf;
  }
  अन्यथा {
    USAGE_ERROR_ACTION(ms,ms);
  }
  प्रत्यागम परिणाम;
}

size_t mspace_समुच्चयः_footprint_सीमा(mspace msp, size_t अष्टकानि) {
  size_t परिणाम = 0;
  mstate ms = (mstate)msp;
  यदि (ok_magic(ms)) {
    यदि (अष्टकानि == 0)
      परिणाम = granularity_align(1); /* Use minimal size */
    यदि (अष्टकानि == MAX_आकार_T)
      परिणाम = 0;                    /* disable */
    अन्यथा
      परिणाम = granularity_align(अष्टकानि);
    ms->footprint_सीमा = परिणाम;
  }
  अन्यथा {
    USAGE_ERROR_ACTION(ms,ms);
  }
  प्रत्यागम परिणाम;
}

#यदि !NO_MALLINFO
संरचना mallinfo mspace_mallinfo(mspace msp) {
  mstate ms = (mstate)msp;
  यदि (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
  }
  प्रत्यागम आन्तरिकम्_mallinfo(ms);
}
#यदि_अन्तः /* NO_MALLINFO */

size_t mspace_usable_आकार(ध्रुव शून्य* mem) {
  यदि (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    यदि (is_inuse(p))
      प्रत्यागम chunksize(p) - overhead_for(p);
  }
  प्रत्यागम 0;
}

पूर्णाङ्क mspace_mallopt(पूर्णाङ्क param_सङ्ख्या, पूर्णाङ्क मानम्) {
  प्रत्यागम change_mparam(param_सङ्ख्या, मानम्);
}

#यदि_अन्तः /* MSPACES */


/* -------------------- Alternative MORECORE functions ------------------- */

/*
  Guidelines for creating a custom version of MORECORE:

  * For best performance, MORECORE should allocate in multiples of pagesize.
  * MORECORE may allocate more memory than requested. (Or even less,
      but this will usually result in a malloc failure.)
  * MORECORE must not allocate memory when given argument zero, but
      instead return one past the end address of memory from previous
      nonzero call.
  * For best performance, consecutive calls to MORECORE with positive
      arguments should return increasing addresses, indicating that
      space has been contiguously extended.
  * Even though consecutive calls to MORECORE need not return contiguous
      addresses, it must be OK for malloc'ed chunks to span multiple
      regions in those cases where they do happen to be contiguous.
  * MORECORE need not handle negative arguments -- it may instead
      just return MFAIL when given negative arguments.
      Negative arguments are always multiples of pagesize. MORECORE
      must not misinterpret negative args as large positive unsigned
      args. You can suppress all such calls from even occurring by defining
      MORECORE_CANNOT_TRIM,

  As an example alternative MORECORE, here is a custom allocator
  kindly contributed for pre-OSX macOS.  It uses virtually but not
  necessarily physically contiguous non-paged memory (locked in,
  present and won't get swapped out).  You can use it by uncommenting
  this section, adding some #includes, and setting up the appropriate
  defines above:

      #define MORECORE osMoreCore

  There is also a shutdown routine that should somehow be called for
  cleanup upon program exit.

  #define MAX_POOL_ENTRIES 100
  #define MINIMUM_MORECORE_SIZE  (64 * 1024U)
  static int next_os_pool;
  void *our_os_pools[MAX_POOL_ENTRIES];

  void *osMoreCore(int size)
  {
    void *ptr = 0;
    static void *sbrk_top = 0;

    if (size > 0)
    {
      if (size < MINIMUM_MORECORE_SIZE)
         size = MINIMUM_MORECORE_SIZE;
      if (CurrentExecutionLevel() == kTaskLevel)
         ptr = PoolAllocateResident(size + RM_PAGE_SIZE, 0);
      if (ptr == 0)
      {
        return (void *) MFAIL;
      }
      // save ptrs so they can be freed during cleanup
      our_os_pools[next_os_pool] = ptr;
      next_os_pool++;
      ptr = (void *) ((((size_t) ptr) + RM_PAGE_MASK) & ~RM_PAGE_MASK);
      sbrk_top = (char *) ptr + size;
      return ptr;
    }
    else if (size < 0)
    {
      // we don't currently support shrink behavior
      return (void *) MFAIL;
    }
    else
    {
      return sbrk_top;
    }
  }

  // cleanup any allocated memory pools
  // called as last thing before shutting down driver

  void osCleanupMem(void)
  {
    void **ptr;

    for (ptr = our_os_pools; ptr < &our_os_pools[MAX_POOL_ENTRIES]; ptr++)
      if (*ptr)
      {
         PoolDeallocate(*ptr);
         *ptr = 0;
      }
  }

*/


/* -----------------------------------------------------------------------
History:
    v2.8.6 Wed Aug 29 06:57:58 2012  Doug Lea
      * fix bad comparison in dlposix_memalign
      * don't reuse adjusted asize in sys_alloc
      * add LOCK_AT_FORK -- thanks to Kirill Artamonov for the suggestion
      * reduce compiler warnings -- thanks to all who reported/suggested these

    v2.8.5 Sun May 22 10:26:02 2011  Doug Lea  (dl at gee)
      * Always perform unlink checks unless INSECURE
      * Add posix_memalign.
      * Improve realloc to expand in more cases; expose realloc_in_place.
        Thanks to Peter Buhr for the suggestion.
      * Add footprint_limit, inspect_all, bulk_free. Thanks
        to Barry Hayes and others for the suggestions.
      * Internal refactorings to avoid calls while holding locks
      * Use non-reentrant locks by default. Thanks to Roland McGrath
        for the suggestion.
      * Small fixes to mspace_destroy, reset_on_error.
      * Various configuration extensions/changes. Thanks
         to all who contributed these.

    V2.8.4a Thu Apr 28 14:39:43 2011 (dl at gee.cs.oswego.edu)
      * Update Creative Commons URL

    V2.8.4 Wed May 27 09:56:23 2009  Doug Lea  (dl at gee)
      * Use zeros instead of prev foot for is_mmapped
      * Add mspace_track_large_chunks; thanks to Jean Brouwers
      * Fix set_inuse in internal_realloc; thanks to Jean Brouwers
      * Fix insufficient sys_alloc padding when using 16byte alignment
      * Fix bad error check in mspace_footprint
      * Adaptations for ptmalloc; thanks to Wolfram Gloger.
      * Reentrant spin locks; thanks to Earl Chew and others
      * Win32 improvements; thanks to Niall Douglas and Earl Chew
      * Add NO_SEGMENT_TRAVERSAL and MAX_RELEASE_CHECK_RATE options
      * Extension hook in malloc_state
      * Various small adjustments to reduce warnings on some compilers
      * Various configuration extensions/changes for more platforms. Thanks
         to all who contributed these.

    V2.8.3 Thu Sep 22 11:16:32 2005  Doug Lea  (dl at gee)
      * Add max_footprint functions
      * Ensure all appropriate literals are size_t
      * Fix conditional compilation problem for some #define settings
      * Avoid concatenating segments with the one provided
        in create_mspace_with_base
      * Rename some variables to avoid compiler shadowing warnings
      * Use explicit lock initialization.
      * Better handling of sbrk interference.
      * Simplify and fix segment insertion, trimming and mspace_destroy
      * Reinstate REALLOC_ZERO_BYTES_FREES option from 2.7.x
      * Thanks especially to Dennis Flanagan for help on these.

    V2.8.2 Sun Jun 12 16:01:10 2005  Doug Lea  (dl at gee)
      * Fix memalign brace error.

    V2.8.1 Wed Jun  8 16:11:46 2005  Doug Lea  (dl at gee)
      * Fix improper #endif nesting in C++
      * Add explicit casts needed for C++

    V2.8.0 Mon May 30 14:09:02 2005  Doug Lea  (dl at gee)
      * Use trees for large bins
      * Support mspaces
      * Use segments to unify sbrk-based and mmap-based system allocation,
        removing need for emulation on most platforms without sbrk.
      * Default safety checks
      * Optional footer checks. Thanks to William Robertson for the idea.
      * Internal code refactoring
      * Incorporate suggestions and platform-specific changes.
        Thanks to Dennis Flanagan, Colin Plumb, Niall Douglas,
        Aaron Bachmann,  Emery Berger, and others.
      * Speed up non-fastbin processing enough to remove fastbins.
      * Remove useless cfree() to avoid conflicts with other apps.
      * Remove internal memcpy, memset. Compilers handle builtins better.
      * Remove some options that no one ever used and rename others.

    V2.7.2 Sat Aug 17 09:07:30 2002  Doug Lea  (dl at gee)
      * Fix malloc_state bitmap array misdeclaration

    V2.7.1 Thu Jul 25 10:58:03 2002  Doug Lea  (dl at gee)
      * Allow tuning of FIRST_SORTED_BIN_SIZE
      * Use PTR_UINT as type for all ptr->int casts. Thanks to John Belmonte.
      * Better detection and support for non-contiguousness of MORECORE.
        Thanks to Andreas Mueller, Conal Walsh, and Wolfram Gloger
      * Bypass most of malloc if no frees. Thanks To Emery Berger.
      * Fix freeing of old top non-contiguous chunk im sysmalloc.
      * Raised default trim and map thresholds to 256K.
      * Fix mmap-related #defines. Thanks to Lubos Lunak.
      * Fix copy macros; added LACKS_FCNTL_H. Thanks to Neal Walfield.
      * Branch-free bin calculation
      * Default trim and mmap thresholds now 256K.

    V2.7.0 Sun Mar 11 14:14:06 2001  Doug Lea  (dl at gee)
      * Introduce independent_comalloc and independent_calloc.
        Thanks to Michael Pachos for motivation and help.
      * Make optional .h file available
      * Allow > 2GB requests on 32bit systems.
      * new WIN32 sbrk, mmap, munmap, lock code from <Walter@GeNeSys-e.de>.
        Thanks also to Andreas Mueller <a.mueller at paradatec.de>,
        and Anonymous.
      * Allow override of MALLOC_ALIGNMENT (Thanks to Ruud Waij for
        helping test this.)
      * memalign: check alignment arg
      * realloc: don't try to shift chunks backwards, since this
        leads to  more fragmentation in some programs and doesn't
        seem to help in any others.
      * Collect all cases in malloc requiring system memory into sysmalloc
      * Use mmap as backup to sbrk
      * Place all internal state in malloc_state
      * Introduce fastbins (although similar to 2.5.1)
      * Many minor tunings and cosmetic improvements
      * Introduce USE_PUBLIC_MALLOC_WRAPPERS, USE_MALLOC_LOCK
      * Introduce MALLOC_FAILURE_ACTION, MORECORE_CONTIGUOUS
        Thanks to Tony E. Bennett <tbennett@nvidia.com> and others.
      * Include errno.h to support default failure action.

    V2.6.6 Sun Dec  5 07:42:19 1999  Doug Lea  (dl at gee)
      * return null for negative arguments
      * Added Several WIN32 cleanups from Martin C. Fong <mcfong at yahoo.com>
         * Add 'LACKS_SYS_PARAM_H' for those systems without 'sys/param.h'
          (e.g. WIN32 platforms)
         * Cleanup header file inclusion for WIN32 platforms
         * Cleanup code to avoid Microsoft Visual C++ compiler complaints
         * Add 'USE_DL_PREFIX' to quickly allow co-existence with existing
           memory allocation routines
         * Set 'malloc_getpagesize' for WIN32 platforms (needs more work)
         * Use 'assert' rather than 'ASSERT' in WIN32 code to conform to
           usage of 'assert' in non-WIN32 code
         * Improve WIN32 'sbrk()' emulation's 'findRegion()' routine to
           avoid infinite loop
      * Always call 'fREe()' rather than 'free()'

    V2.6.5 Wed Jun 17 15:57:31 1998  Doug Lea  (dl at gee)
      * Fixed ordering problem with boundary-stamping

    V2.6.3 Sun May 19 08:17:58 1996  Doug Lea  (dl at gee)
      * Added pvalloc, as recommended by H.J. Liu
      * Added 64bit pointer support mainly from Wolfram Gloger
      * Added anonymously donated WIN32 sbrk emulation
      * Malloc, calloc, getpagesize: add optimizations from Raymond Nijssen
      * malloc_extend_top: fix mask error that caused wastage after
        foreign sbrks
      * Add linux mremap support code from HJ Liu

    V2.6.2 Tue Dec  5 06:52:55 1995  Doug Lea  (dl at gee)
      * Integrated most documentation with the code.
      * Add support for mmap, with help from
        Wolfram Gloger (Gloger@lrz.uni-muenchen.de).
      * Use last_remainder in more cases.
      * Pack bins using idea from  colin@nyx10.cs.du.edu
      * Use ordered bins instead of best-fit threshold
      * Eliminate block-local decls to simplify tracing and debugging.
      * Support another case of realloc via move into top
      * Fix error occurring when initial sbrk_base not word-aligned.
      * Rely on page size for units instead of SBRK_UNIT to
        avoid surprises about sbrk alignment conventions.
      * Add mallinfo, mallopt. Thanks to Raymond Nijssen
        (raymond@es.ele.tue.nl) for the suggestion.
      * Add `pad' argument to malloc_trim and top_pad mallopt parameter.
      * More precautions for cases where other routines call sbrk,
        courtesy of Wolfram Gloger (Gloger@lrz.uni-muenchen.de).
      * Added macros etc., allowing use in linux libc from
        H.J. Lu (hjl@gnu.ai.mit.edu)
      * Inverted this history list

    V2.6.1 Sat Dec  2 14:10:57 1995  Doug Lea  (dl at gee)
      * Re-tuned and fixed to behave more nicely with V2.6.0 changes.
      * Removed all preallocation code since under current scheme
        the work required to undo bad preallocations exceeds
        the work saved in good cases for most test programs.
      * No longer use return list or unconsolidated bins since
        no scheme using them consistently outperforms those that don't
        given above changes.
      * Use best fit for very large chunks to prevent some worst-cases.
      * Added some support for debugging

    V2.6.0 Sat Nov  4 07:05:23 1995  Doug Lea  (dl at gee)
      * Removed footers when chunks are in use. Thanks to
        Paul Wilson (wilson@cs.texas.edu) for the suggestion.

    V2.5.4 Wed Nov  1 07:54:51 1995  Doug Lea  (dl at gee)
      * Added malloc_trim, with help from Wolfram Gloger
        (wmglo@Dent.MED.Uni-Muenchen.DE).

    V2.5.3 Tue Apr 26 10:16:01 1994  Doug Lea  (dl at g)

    V2.5.2 Tue Apr  5 16:20:40 1994  Doug Lea  (dl at g)
      * realloc: try to expand in both directions
      * malloc: swap order of clean-bin strategy;
      * realloc: only conditionally expand backwards
      * Try not to scavenge used bins
      * Use bin counts as a guide to preallocation
      * Occasionally bin return list chunks in first scan
      * Add a few optimizations from colin@nyx10.cs.du.edu

    V2.5.1 Sat Aug 14 15:40:43 1993  Doug Lea  (dl at g)
      * faster bin computation & slightly different binning
      * merged all consolidations to one part of malloc proper
         (eliminating old malloc_find_space & malloc_clean_bin)
      * Scan 2 returns chunks (not just 1)
      * Propagate failure in realloc if malloc returns 0
      * Add stuff to allow compilation on non-ANSI compilers
          from kpv@research.att.com

    V2.5 Sat Aug  7 07:41:59 1993  Doug Lea  (dl at g.oswego.edu)
      * removed potential for odd address access in prev_chunk
      * removed dependency on getpagesize.h
      * misc cosmetics and a bit more internal documentation
      * anticosmetics: mangled names in macros to evade debugger strangeness
      * tested on sparc, hp-700, dec-mips, rs6000
          with gcc & native cc (hp, dec only) allowing
          Detlefs & Zorn comparison study (in SIGPLAN Notices.)

    Trial version Fri Aug 28 13:14:29 1992  Doug Lea  (dl at g.oswego.edu)
      * Based loosely on libg++-1.2X malloc. (It retains some of the overall
         structure of old version,  but most details differ.)

*/
