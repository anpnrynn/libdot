#include <stdio.h>
#include <string.h>
#include <dot.h>


int main () {
	DOT_PARSER *dotParser = dot_parser_new ();

	char *line001 = "version:1\n";
	char *line002 = "author:Anoop Kumar Narayanan\n";
	char *line003 = " \r\n";
	char *line004 = "	\n";
	char *line005 = " 						\n";
	char *line006 = "\n";
	char *line007 = "\r\n";
	//char *line008 = ".html .:TextAttribute; @:1234; first:1first;  second:2second;        third:3third;\n";
	//char *line008 = "..body .:TextAttribute; @:1234; first:1first;  second:2second;        third:3third;\n";
	//char *line008 = ".html .:TextAttribute\t\"`; @:1234\\\\; first:1first;  second:2second;        third:3third;\n";

	char *line008=".html\n";
	char *line009="..head\n";
	char *line010="...title .:This is a title.\\\\\\n\\;;\n";
	char *line011="..body @:bodymarker1; class:bodyclass; .:This is a body.;\n";
	char *line012="...h1  .:This is a header1 line.;\n";
	char *line013="..body @:bodymarker1; .:This is also a body.;\n";



	char buf[65536];
//#if 0
	strcpy( buf, line001);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line002);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line003);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line004);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line005);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line006);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line007);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line008);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line009);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line010);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line011);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line012);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line013);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line010);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line010);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line010);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );
	strcpy( buf, line012);
	dot_parser_parse_line ( dotParser, buf, strlen(buf) );

	fprintf(stderr,"INFO: main(): ======================================================\n");
	dot_parser_dump( dotParser, dotParser->docRoot, 0 );
	dot_parser_pretty_print( dotParser, dotParser->docRoot, 0 );


	dot_parser_delete( dotParser );
	//fprintf(stderr,"INFO: main(): ======================================================\n");
	//dot_parser_dump( dotParser->docRoot, 0 );
	return 0;
}
