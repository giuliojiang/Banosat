#ifndef SAT_PARSER_H
#define SAT_PARSER_H

#include "clause.h"
#include "arraymap.h"

clause_t* parser_parse_clause(char* line, arraymap_t* variables);

#endif
