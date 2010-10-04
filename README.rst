============
libexception
============

:Author: `Benedikt Böhm <bb@xnull.de>`_
:Version: 0.2_beta
:Web: http://github.com/hollow/libexception
:Git: ``git clone https://github.com/hollow/libexception.git``
:Download: http://github.com/hollow/libexception/downloads

libexception is an exception handling library for C. It uses the ``setjmp`` and
``longjmp`` calls to build a stack of jump buffers and exceptions and provides
high level semantics implemented as macros to make exception handling in C
unobtrusive and easy to use.

Rationale
=========

Error handling is an important issue in programming languages, and it can
account for a substantial portion of a project's code. The classic C approach
to this problem is return codes. Each function returns a value indicating
success or failure. However, with a nontrivial function call hierarchy, this
approach clutters the code significantly. Every function must check the return
code of every function call it makes and take care of errors. In most cases,
the function will merely pass any errors back up to its caller.

Programming languages such as Ada or C++ address this issue with exceptions.
Exceptions make it easy to separate error handling from the rest of the code.
Intermediate functions can completely ignore errors occurring in functions they
call, if they can't handle them anyway.

The solution to this problem is to implement a simple exception-handling library
in C.

Implementation
==============

libexception uses two stacks implemented as a `doubly-linked list
<http://isis.poly.edu/kulesh/stuff/src/klist/>`_ to record exceptions and jump
buffers needed by ``setjmp``/``longjmp``. The work-flow is as follows:

.. image:: http://github.com/hollow/libexception/raw/master/flow.png

#. ``try`` will call ``setjmp`` and push the resulting ``jmp_buf`` object onto the tryenv stack.
#. usually ``setjmp`` returns zero and the following block is executed. if no
   exception occured nothing else happens.
#. if an exception is raised ``throw`` will push information about the error
   onto the exception stack and call ``longjmp`` to jump to the topmost tryenv
   buffer.
#. in this case ``setjmp`` returns again, but with a non-zero return code and the
   following ``except`` block is executed.
#. an except block consists of zero or more ``on`` blocks to handle a single
   exception and zero or one ``finally`` block to handle a yet unhandled but
   unknown exception
#. if an ``on`` block handled the exception the exception stack is cleared and
   nothing else happens.
#. if no ``on`` block handled the exception but a ``finally`` block handled the
   exception the exception stack is cleared and nothing else happens.
#. if an unhandled exception is found at the end of an ``except`` block,
   control is transfered to the next jump buffer (a.k.a ``except`` block).
#. if there are no more jump buffers available, the default exception handler
   is called to print an exception trace to ``STDERR`` and the program will be
   aborted.

Installation
============

To install libexception call ``./configure``, then ``make`` and finally ``make install``.

Documentation
=============

The libexception API reference can be found `on-line
<http://bb.xnull.de/projects/libexception/doc/html/>`_ or in `PDF
<http://bb.xnull.de/projects/libexception/doc/latex/refman.pdf>`_.

Examples
========

Examples can be found in the `test-suite
<http://github.com/hollow/libexception/tree/master/test/>`_.
