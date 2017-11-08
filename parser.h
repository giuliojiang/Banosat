#ifndef SAT_PARSER_H
#define SAT_PARSER_H

#include "clause.h"
#include "context.h"

clause_t* parser_parse_clause(char* line, context_t* context);

#endif
