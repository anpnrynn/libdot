# libdot
A Library for DOT file format parsing (under development)

THE DOT FORMAT - Version 1, Spec Revision 4

Author:
=======
Anoop Kumar Narayanan

Document Format Design Document:
================================
Please Read DESIGN file in the source

Why choose it over HTML/XML ?
=============================
1. Like XML, the DOT format is very self-explanatory .
2. Ever patched an XML file ?
3. Use Git ? Multiple people working on a Branch ? Merging is problem.
4. Its simple.

Howto overcome HTML/XML problems with Source Control Solutions ?
================================================================
Well, simple solution is store it as a DOT format and then convert it
HTML/XML while generating webpage. DOT format stores information in a
similar HTML/XML hierarchical format, but it is also line based when 
Compared to HTML/XML. Making it easier to edit and make changes and 
manage in a Source Control.
(This is just a suggestion)


