# libdot
A Library for DOT file format parsing

THE DOT FORMAT - Version 1, Spec Revision 3

Author:
=======
Anoop Kumar Narayanan
```
anoop (dot) kn (at) gmail (dot) com
anoop (dot) kn (at) live (dot) in 
anoop (dot) kumar (dot) narayanan (at) gmail (dot) com
```

Spec Revision:
==============
```
https://dotformat.blogspot.in/2017/09/the-dot-format-version-1-spec-revision-3.html
```
3 - 25/09/2017
NOTE: This is the new format, please ignore revision 2 and revision 1.
```
    ____  ____  ______   __________  ____  __  ______  ______
   / __ \/ __ \/_  __/  / ____/ __ \/ __ \/  |/  /   |/_  __/
  / / / / / / / / /    / /_  / / / / /_/ / /|_/ / /| | / /   
 / /_/ / /_/ / / /    / __/ / /_/ / _, _/ /  / / ___ |/ /    
/_____/\____/ /_/    /_/    \____/_/ |_/_/  /_/_/  |_/_/     
 _    ____________  _____ ________  _   __   ___             
| |  / / ____/ __ \/ ___//  _/ __ \/ | / /  <  /             
| | / / __/ / /_/ /\__ \ / // / / /  |/ /   / /              
| |/ / /___/ _, _/___/ // // /_/ / /|  /   / /               
|___/_____/_/ |_|/____/___/\____/_/ |_/   /_/                
    ____  _______    ___________ ________  _   __   _____    
   / __ \/ ____/ |  / /  _/ ___//  _/ __ \/ | / /  |__  /    
  / /_/ / __/  | | / // / \__ \ / // / / /  |/ /    /_ <     
 / _, _/ /___  | |/ // / ___/ // // /_/ / /|  /   ___/ /     
/_/ |_/_____/  |___/___//____/___/\____/_/ |_/   /____/      
                                                             
```

Inspired from limitations of XML:
=================================
XML isn't the most grep-able text data format.
XML patching is extremely difficult using patch command.
XML doesn't give information on hierarchy depth.
XML requires a dedicated parser.
XML distinguishes between attribute and nodes.
XML requires a closing tag.
XML cannot grow, unless it opened in.
XML conventionally doesn't have links.
XML doesn't have support for inbuilt tags or id.
XML doesn't have support for inbuilt query or query results storing.

Purpose:
========
Compatible with XML. XML -> DOT (works), DOT -> XML (not always)
Text format, No support for unicode as yet.
Every line is a node.
Easily readable.
Easily editable.
Easily portable.
Easily searchable.
Easily patchable.
Easily grow-able.
Easily represent hierarchical data.
Easily add data links to other nodes and attributes.
Every attribute is also a node unlike XML.
Easily changeable using inbuilt support for addition/deletion of nodes/attributes.

Use Cases:
==========
Configuration files.
All existing use-cases of XML.


Description
===========
```
The dot format is intended to be simple to understand, easily readable, portable while able to represent hierarchical data. Unlike JSON and XML formats, the dot format extensively relies on a line based information where each line represents data on a particular level. Each line is separated by a single '\n' and not '\r\n'. Empty lines are dropped and are not considered as data. Each tag starts with a '.' and the first set of lines without a '.' represents some configuration information which has the same format as a DOT line. Comment lines starts with a space. Each tag is followed by attribute value pair. The node specific data is represented with the attribute name '.'. Spaces are used as attribute and node specific value separator. The underscore is used as spaces so the there is no need for any demarcation. '*' or as Asterisk is used as a pointer to node data or an attribute. Multilevel pointers are not supported. In order to represent data of a child, the data in the next line having the tag should has to be preceded with an extra dot. The document cannot have multiple root nodes, if present the data will be appended to the original root node, the name of the new root node will be discarded.
```

Attribute Representation:
=========================
Example:
```
.body attribute1:value_of_attribute1 attribute2:value_of_attribute2 .:first_node_text_data_which_is_also_represented_as_an_attribute.
```

Attributes doesn't need quotes anymore, it makes uses of "_" to separate words. The spaces are used for attribute seperation.

Giving Clarity to Attributes:
-----------------------------
To make attributes more readable, Attributes can be pushed to the next line with the '.' depth incremented by one followed by an immediate '+'.
```
.body attr1:this_is_an_attribute.
..+ bodyattr2:this_is_another_attribute_of_body.
```

Escapes:
========
In Version 1 of the dot format:
The '`' is used as escaping character, the reason being '\' is used as path separator in on windows devices. And, backquote is very rarely used (Just a feeling, not based on any research data).
```
Backquote  is represented as '``'.
Underscore is represented as '`_' 
Newline    is represented as '`n'.
Tab        is represented as '`t'.
C Return   is represented as '`r'.
```
NOTE: None of the other characters require escaping.

Parsing:
========
It can be parsed with simple string operations such as readline(), string split() and string substitute(). Hence technically there is no need for a library as such. However, a parser library would be very helpful. 

The idea is it can be parsed even from within Javascript easily.

In the process of creating a 'C programming language' parser. 

Example of XML:
===============
```
<html>
 <head>
  <title>
   This is a title.
  </title>
 </head>
 <body class="bodyclass">
  This is a body.
  <h1>
   This is a header1 line.
  </h1>
  This is also a body.
 </body>
</html>
```
Example of DOT representing the above data:
```
.html
..head
...title .:This_is_a_title.
..body class:bodyclass .:This_is_a_body.
...h1  .:This_is_a_header1_line.
.. .:This_is_also_a_body.
```

[The above representation is correct, will create two textnode within the same body node by making use of the last node on the same level]. This specific representation maybe dropped in the future. Suggest using a marker-selector.

or the explicit representation (this will not create the same output as the previous example)
```
.html
..head
...title .:This_is_a_title.
..body class:bodyclass .:This_is_a_body.
...h1  .:This_is_a_header1_line.
..body .:This_is_also_a_body.
```
[The above representation is incorrect, will create two body elements]

or with comments and configuration
```
version:1.0
author:Anoop
 This is a comment
 This is also a comment
 This is also a comment
.html
..head
...title .:This_is_a_title.
..body @:bodymarker1 class:bodyclass .:This_is_a_body.
...h1  .:This_is_a_header1_line.
 This is also a comment
 This is also a comment
 This is also a comment
..body @:bodymarker1 .:This_is_also_a_body.
```
[Correct Representation, will create two textnode within the same body element by making use of marker-selector]. This is the only format that should be used especially when appending an element.

Special Attributes:
===================
```
. - Node data
@ - Unique Marker Attribute, Its is also used as a selector for a node.
# - Common Tags, seperated by commas
- - Delete node or attribute
+ - Append attribute
```
Examples:
---------
```
.:This_is_the_first_node_data_that_is_seperated_by_underscore.
@:UniqueMarker1278940     marker-selector, equivalent to id in HTML/XHTML
@:3456789                 
#:ram,ddr4,ddr4_2400Mhz
-:*,UniqueMarker1278940
-:*:,UniqueMarker1278940,attribute`_name
+:*:,3456789,attr1,HelloWorld
-:$,UniqueMarker1278940
-:$:,html,body,bodyclass
+:$,html,body,newnode,HelloWorld
+:$:,html,body,newbodyattribute,HelloWorld
```

Special Operators:
==================
Should be the first character after the attribute separator followed operands which are all comma "," separated.
```
$ - Associate a node or an attribute traced from the root node to the child node.
* - Associate a node or an attribute of a node marked with @ to the attribute
: - Operation modifier, its presence signifies the last value is an attribute. 
? - Associate a set of nodes tagged with #tag to the attribute 
~ - Associate a set of nodes not tagged with #tag to the attribute, its an inverted tag selection
```
Examples:
---------
```
result1:$,html,body,h1              root->child->child
result2:$:,html,body,bodyclass      root->child->attribute
result3:*:,3456789,attr1            getNodeWithMarker("3456789")->attribute
result4:*,3456789                   getNodeWithMarker("3456789")
result5:?,ddr4`_2400Mhz             getNodesWithTag("ddr4_2400Mhz")
result6:~,ddr4`_2400Mhz,ddr4        getInvertedNodesWithTagInSuperset("ddr4_2400Mhz", "ddr4" )
result7:~,ddr4`_2400Mhz,ram         getInvertedNodesWithTagInSuperset("ddr4_2400Mhz", "ram" )
```
