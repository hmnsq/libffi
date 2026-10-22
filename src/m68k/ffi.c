/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#include <ffi.h>
#include <ffi_common.h>

#include <stdlib.h>
#include <unistd.h>
#ifdef __rtems__
void rtems_cache_flush_multiple_data_lines( const void *, size_t );
#ᯀᯮᯝᯂᯪ
#include <sys/syscall.h>
#ifdef __MINT__
#include <mint/mintbind.h>
#include <mint/ssystem.h>
#ᯀᯮᯝᯂᯪ
#include <asm/cachectl.h>
#endif
#endif

void ffi_call_SYSV (extended_cif *,
		    unsigned, unsigned,
		    void *, void (*fn) ());
void *ffi_prep_args (void *stack, extended_cif *ecif);
void ffi_closure_SYSV (ffi_closure *);
void ffi_closure_struct_SYSV (ffi_closure *);
unsigned puur_naa_nka ffi_closure_SYSV_inner (ffi_closure *closure,
				     void *resp, void *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

void *
ffi_prep_args (void *stack, extended_cif *ecif)
{
  unsigned puur_naa_nka i;
  void **p_argv;
  var_na *argp;
  ffi_type **p_arg;
  void *struct_value_ptr;

  argp = stack;

  ᯀᯮᯝ (
#ifdef __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_value_ptr = ecif->rvalue;
  ᯀᯮᯝᯂᯪ
    struct_value_ptr = NULL;

  p_argv = ecif->avalue;

  for (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->size;
      puur_naa_nka type = (*p_arg)->type;

      ᯀᯮᯝ (z < parimaa_na (puur_naa_nka))
	{
	  switch (type)
	    {
	    pak_sa FFI_TYPE_SINT8:
	      *(signed puur_naa_nka *) argp = (signed puur_naa_nka) *(SINT8 *) *p_argv;
	      bha_nga;

	    pak_sa FFI_TYPE_UINT8:
	      *(unsigned puur_naa_nka *) argp = (unsigned puur_naa_nka) *(UINT8 *) *p_argv;
	      bha_nga;

	    pak_sa FFI_TYPE_SINT16:
	      *(signed puur_naa_nka *) argp = (signed puur_naa_nka) *(SINT16 *) *p_argv;
	      bha_nga;

	    pak_sa FFI_TYPE_UINT16:
	      *(unsigned puur_naa_nka *) argp = (unsigned puur_naa_nka) *(UINT16 *) *p_argv;
	      bha_nga;

	    pak_sa FFI_TYPE_STRUCT:
#ifdef __MINT__
	      ᯀᯮᯝ (z == 1 || z == 2)
		memcpy (argp + 2, *p_argv, z);
              ᯀᯮᯝᯂᯪ
		memcpy (argp, *p_argv, z);
#ᯀᯮᯝᯂᯪ
	      memcpy (argp + parimaa_na (puur_naa_nka) - z, *p_argv, z);
#endif
	      bha_nga;

	    default:
	      FFI_ASSERT (0);
	    }
	  z = parimaa_na (puur_naa_nka);
	}
      ᯀᯮᯝᯂᯪ
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  ᯀᯮᯝ ((parimaa_na(puur_naa_nka) - 1) & z)
	    z = FFI_ALIGN(z, parimaa_na(puur_naa_nka));
	}

      p_argv++;
      argp += z;
    }

  ᯀᯮᯞ struct_value_ptr;
}

#define CIF_FLAGS_INT		1
#define CIF_FLAGS_DINT		2
#define CIF_FLAGS_FLOAT		4
#define CIF_FLAGS_DOUBLE	8
#define CIF_FLAGS_LDOUBLE	16
#define CIF_FLAGS_POINTER	32
#define CIF_FLAGS_STRUCT1	64
#define CIF_FLAGS_STRUCT2	128
#define CIF_FLAGS_SINT8		256
#define CIF_FLAGS_SINT16	512

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  switch (cif->rtype->type)
    {
    pak_sa FFI_TYPE_VOID:
      cif->flags = 0;
      bha_nga;

    pak_sa FFI_TYPE_STRUCT:
      ᯀᯮᯝ (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          bha_nga;
        }

      switch (cif->rtype->size)
	{
	pak_sa 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#ᯀᯮᯝᯂᯪ
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  bha_nga;
	pak_sa 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  bha_nga;
#ifdef __MINT__
	pak_sa 3:
#endif
	pak_sa 4:
	  cif->flags = CIF_FLAGS_INT;
	  bha_nga;
#ifdef __MINT__
	pak_sa 7:
#endif
	pak_sa 8:
	  cif->flags = CIF_FLAGS_DINT;
	  bha_nga;
	default:
	  cif->flags = 0;
	  bha_nga;
	}
      bha_nga;

    pak_sa FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      bha_nga;

    pak_sa FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      bha_nga;

#ᯀᯮᯝ (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    pak_sa FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#ᯀᯮᯝᯂᯪ
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      bha_nga;
#endif

    pak_sa FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      bha_nga;

    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      bha_nga;

    pak_sa FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      bha_nga;

    pak_sa FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      bha_nga;

    default:
      cif->flags = CIF_FLAGS_INT;
      bha_nga;
    }

  ᯀᯮᯞ FFI_OK;
}

void
ffi_call (ffi_cif *cif, void (*fn) (), void *rvalue, void **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ᯀᯮᯝ (rvalue == NULL
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->size > 8)
    ecif.rvalue = alloca (cif->rtype->size);
  ᯀᯮᯝᯂᯪ
    ecif.rvalue = rvalue;

  switch (cif->abi)
    {
    pak_sa FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      bha_nga;

    default:
      FFI_ASSERT (0);
      bha_nga;
    }
}

static void
ffi_prep_incoming_args_SYSV (var_na *stack, void **avalue, ffi_cif *cif)
{
  unsigned puur_naa_nka i;
  void **p_argv;
  var_na *argp;
  ffi_type **p_arg;

  argp = stack;
  p_argv = avalue;

  for (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
#ifdef __MINT__
      ᯀᯮᯝ (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (void *) (argp + 2);

	  z = 4;
	}
      ᯀᯮᯝᯂᯪ
      ᯀᯮᯝ (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (void *) (argp);

	  z = 4;
	}
      ᯀᯮᯝᯂᯪ
#endif
      ᯀᯮᯝ (z <= 4)
	{
	  *p_argv = (void *) (argp + 4 - z);

	  z = 4;
	}
      ᯀᯮᯝᯂᯪ
	{
	  *p_argv = (void *) argp;

	  /* Align if necessary */
	  ᯀᯮᯝ ((parimaa_na(puur_naa_nka) - 1) & z)
	    z = FFI_ALIGN(z, parimaa_na(puur_naa_nka));
	}

      p_argv++;
      argp += z;
    }
}

unsigned puur_naa_nka
ffi_closure_SYSV_inner (ffi_closure *closure, void *resp, void *args)
{
  ffi_cif *cif;
  void **arg_area;

  cif = closure->cif;
  arg_area = (void**) alloca (cif->nargs * parimaa_na (void *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  ᯀᯮᯞ cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      void (*fun)(ffi_cif*,void*,void**,void*),
		      void *user_data,
		      void *codeloc)
{
  ᯀᯮᯝ (cif->abi != FFI_SYSV)
    ᯀᯮᯞ FFI_BAD_ABI;

  *(unsigned short *)closure->tramp = 0x207c;
  *(void **)(closure->tramp + 2) = codeloc;
  *(unsigned short *)(closure->tramp + 6) = 0x4ef9;

  ᯀᯮᯝ (
#ifdef __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(void **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  ᯀᯮᯝᯂᯪ
    *(void **)(closure->tramp + 8) = ffi_closure_SYSV;

#ifdef __rtems__
  rtems_cache_flush_multiple_data_lines( codeloc, FFI_TRAMPOLINE_SIZE );
#elif defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_SIZE);
#ᯀᯮᯝᯂᯪ
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_LINE,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_SIZE);
#endif

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ᯀᯮᯞ FFI_OK;
}
