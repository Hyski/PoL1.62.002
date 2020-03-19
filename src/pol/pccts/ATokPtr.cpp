/* ATokPtr.C
 *
 * ANTLRToken MUST be defined before entry to this file.
 *
 * SOFTWARE RIGHTS
 *
 * We reserve no LEGAL rights to the Purdue Compiler Construction Tool
 * Set (PCCTS) -- PCCTS is in the public domain.  An individual or
 * company may do whatever they wish with source code distributed with
 * PCCTS or the code generated by PCCTS, including the incorporation of
 * PCCTS, or its output, into commerical software.
 *
 * We encourage users to develop software with PCCTS.  However, we do ask
 * that credit is given to us for developing PCCTS.  By "credit",
 * we mean that if you incorporate our source code into one of your
 * programs (commercial product, research project, or otherwise) that you
 * acknowledge this fact somewhere in the documentation, research report,
 * etc...  If you like PCCTS and have developed a nice tool with the
 * output, please mention that you developed it using PCCTS.  In
 * addition, we ask that this header remain intact in our source code.
 * As long as these guidelines are kept, we expect to continue enhancing
 * this system and expect to make other tools available as they are
 * completed.
 *
 * ANTLR 1.33
 * Written by Russell Quong June 30, 1995
 * Adapted by Terence Parr to ANTLR stuff
 * Parr Research Corporation
 * with Purdue University and AHPCRC, University of Minnesota
 * 1989-1998
 */

#include "pcctscfg.h"

#include "ATokPtr.h"

void ANTLRTokenPtr::ref() const
{
    if (ptr_ != NULL) {
		ptr_->ref();
	}
}

void ANTLRTokenPtr::deref()
{
    if (ptr_ != NULL)
    {
		ptr_->deref();
		if ( ptr_->nref()==0 )
		{
		    delete ptr_;
			ptr_ = NULL;
		}
    }
}

ANTLRTokenPtr::~ANTLRTokenPtr()
{
    deref();
}

//
//  8-Apr-97	MR1	Make operator -> a const member function
//			  as weall as some other member functions
//
void ANTLRTokenPtr::operator = (const ANTLRTokenPtr & lhs)	// MR1
{
    lhs.ref();	// protect against "xp = xp"; ie same underlying object
    deref();
    ptr_ = lhs.ptr_;
}

void ANTLRTokenPtr::operator = (ANTLRAbstractToken *addr)
{
    if (addr != NULL) {
	addr->ref();
    }
    deref();
    ptr_ = addr;
}
