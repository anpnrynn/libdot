#include <stdio.h>
#include <string.h>
#include <dot.h>


int main (int argc , char *argv[]) {
	DOT_PARSER *dotParser = dot_parser_new ();
	dot_parser_parse_file( dotParser, argv[1] );
	//dot_parser_dump( dotParser, dotParser->docRoot, 0 );
	fprintf(stdout, " >>>>>>>>>>>>>>>>>>>>>> \n");
	dot_parser_pretty_print( dotParser, dotParser->docRoot, 0 );
	dot_parser_delete( dotParser );
	return 0;
}
