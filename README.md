# libdot
A Library for DOT file format parsing (under development)

THE DOT FORMAT - Version 1, Spec Revision 4

Author:
=======
Anoop Kumar Narayanan

Document Format Design Document:
================================
Please Read DESIGN file in the source

Usage:
======
There is no API to add, modify or delete the Nodes at this point. And it 
is highly unlikely it will ever be added. As each DOT line itself can 
achieve the similar set of operations. Using a string to modify the DOT
datastructure itself makes more sense. Formulate a string to add/modify/
delete nodes and call parse line function with the string as the argument.

It simplifies, the operation.

Everything is achieved using parse line function.

