/*
 *   This file is part of the CxProlog system

 *   YourExtensions.c
 */

#if COMPASS

#include "ext/YourExtensions.c"

#else

#include "CxProlog.h"

/*
CAUTIONARY NOTES CONCERNING DEFINING BUILTIN PREDICATES WRITTEN IN C

Cautions concerning the X parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
All the X parameters must be dereferenced before they are used. To dereference
a X parameter use the function Drf(.), which is defined in the file "Term.c".
Exceptions: there is no need to dereference a X parameter if you pass it
to a function, such as Unify, that already dereferences it.

After a X parameter is dereferenced, the type of its value should be checked,
almost always. This is done using the macros IsAtom(.), IsVar(.), IsInt(.),
IsFloat(.), IsStruct(.), IsList(.), IsExtra(.), etc. All these macros are
defined in the file "Term.h".

Once you know the type of the deferenced term, you are able to extract
information from it, using the macros XStructArity(.), XStructAtom(.),
XStructArg(.,.), XListHead(.), XListTail(.), XListArg(.,.), XAtom(.),
etc., defined in file "Term.h", and the functions XInt(.) and XFloat(.)
defined in file "Number.c".

There are some handy functions that do these three operations (dereference,
check type, extract value) automatically for you, if there is a known expected
type for the X parameter. They constitute the "Test & Extract" family
of functions, defined in file "Term.c". These functions dereference the X
parameter, check its type, and extract information from it. The "Test & Extract"
functions are: XTestAtom, XTestAtomName, XTestInt, XTestPosInt, XTestNat,
XTestIntRange, XTestFloat, XTestBool, XTestOnOff, XTestVar, XTestNonVar,
XTestFunctor, XTestFunctor2.

Cautions concerning control flow
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
A deterministic built-in predicate written in C is made to succeed by calling
JumpNext() and is made to fail by calling DoFail().

A nondeterministic built-in predicate written in C is made to succeed be
calling JumpNext() and is made to stop generating more alternatives and fail
by calling Jump(DiscardAndFail). A nondeterministic built-in predicate
written in C sould not use the macro DoFail().

Cautions concerning the automatic grow features of the runtime stacks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Any built-in predicate that creates a new term (probably using the facilities
provided by file "Term.c") may generate a stack overflow, if the created
term is large enough. If you want to take advantage of the automatic
growing stacks of CxProlog and avoid "stack overflow" errors, this is what
you should know:

1- The system ensures that there are at least 512 words available in the
stacks at the start of each C-built-in: if this in enough for a your specific
predicate, nothing needs to be done. The vast majority of the CxProlog built-ins
are in this category.

2- If 512 words is not enough but you can estimate, upon predicate entry, the
size of the new terms, then you call the function ZEnsureFreeSpaceOnStacks(.,.)
at the beginning of the predicate. This functions reserve the space needed,
causing the stacks to grow immediately, if necessary.
The CxProlog built-ins in this category are:
os_args/1, free_vars/2, name/2, varnames/1, sort/2, msort/2, keysort/2.

3- If it is hard to estimate upon predicate entry the space needed for the new
terms, but if these terms are to be created via Z-functions - ZPushTerm,
ZReadTerm or ZReadTermFromStr - then you should know the Z-functions already
make the stacks grow for you. However, be careful because the stacks are moved
to a different area of memory when they grow. Therefore, a pointer to a term
stored in a C variable can easily become a dangling pointer, after such
relocation.

You overcome this by avoiding storing  pointers to terms in normal C variables,
or by using the special Z register, which contents is automatically relocated
along with the stacks. The CxProlog built-ins in this category are:
dict_get/3, dict_as_list/2, =:/2, current_ivar/2, clause/2, retract/1,
predicate_property/2, queue_get/2, queue_peek/2, queue_as_list/2,
stack_pop/2, stack_top/2, stack_as_list/2, copy_term/2, copy_term/3,
read/1, read/2, read_tokens/1, read_tokens/2,
atom_term/2, atom_termq/2, array_get/3, array_as_list/2.

*/

void YourPrologue()
{
	/* What you may do here is very limited because the system has not been
	   initialized yet. Here you may:
			- Set the aplication name, version, subversion and
			  revision through the function 'SetVersion'
			- Initialize your own flags
			- Fork some processes
			- Not much more.
	*/
}

void YourExtensions()
{
	/* Here, you can do a lot. The whole system has already been initialized
	   and the abstract machine is just about to be activated. Here you may:
			- Install your own deterministic C system predicates by
			  using the function 'InstallCBuiltinPred'
			- Install your own non-deterministic C system predicates by
			  using the function 'InstallNDeterCBuiltinPred'
			- Specify an alternative boot file using function 'SpecifyBootFile'
			- Create and initialize your own data structures
			- Etc.
	*/
}

#endif
