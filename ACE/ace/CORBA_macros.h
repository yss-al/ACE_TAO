// -*- C++ -*-

// ============================================================================
/**
 * @file CORBA_macros.h
 *
 *   Writing code that is portable between platforms with or without
 *   native C++ exceptions is hard.  The following macros offer some
 *   help on this task, mostly oriented to making the ORB code and the
 *   IDL generated code portable.
 *
 *  @author Nanbor Wang <nanbor@cs.wustl.edu>
 *  @author Aniruddha Gokhale  <gokhale@sahyadri.research.bell-labs.com>
 *  @author Carlos O'Ryan  <coryan@uci.edu>, et al.
 */
// ============================================================================

// Macros for handling CORBA exceptions.

#ifndef ACE_CORBA_MACROS_H
#define ACE_CORBA_MACROS_H

#include /**/ "ace/pre.h"

#include /**/ "ace/config-all.h"

# if !defined (ACE_LACKS_PRAGMA_ONCE)
#   pragma once
# endif /* ACE_LACKS_PRAGMA_ONCE */

// The Windows MFC exception mechanism requires that a caught CException
// (including the CMemoryException in use here) be freed using its Delete()
// method. Thus, when MFC is in use and we're catching exceptions as a result
// of new(), the exception's Delete() method has to be called. No other
// platform imposes this sort of restriction/requirement. The Windows
// config stuff (at least for MSVC/MFC) defines a ACE_del_bad_alloc macro
// that works with its ACE_bad_alloc macro to implement this cleanup
// requirement. Since no other platform requires this, define it as
// empty here.
#if !defined (ACE_del_bad_alloc)
#  define ACE_del_bad_alloc
#endif

#define ACE_NEW_THROW_EX(POINTER,CONSTRUCTOR,EXCEPTION) \
  do { POINTER = new (std::nothrow) CONSTRUCTOR;         \
    if (!POINTER) { throw EXCEPTION; }              \
  } while (0)

// FUZZ: disable check_for_ACE_Guard
# define ACE_GUARD_THROW_EX(MUTEX,OBJ,LOCK,EXCEPTION) \
  ACE_Guard< MUTEX > OBJ (LOCK); \
    if (OBJ.locked () == 0) throw EXCEPTION;

# define ACE_READ_GUARD_THROW_EX(MUTEX,OBJ,LOCK,EXCEPTION) \
  ACE_Read_Guard< MUTEX > OBJ (LOCK); \
    if (OBJ.locked () == 0) throw EXCEPTION;

# define ACE_WRITE_GUARD_THROW_EX(MUTEX,OBJ,LOCK,EXCEPTION) \
  ACE_Write_Guard< MUTEX > OBJ (LOCK); \
    if (OBJ.locked () == 0) throw EXCEPTION;
// FUZZ: enable check_for_ACE_Guard

#include /**/ "ace/post.h"

#endif /* ACE_CORBA_MACROS_H */
