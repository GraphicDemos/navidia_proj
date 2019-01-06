@echo on

bison -d -o %1/src/_ps1.0_parser.cpp -p ps10_ bison/ps1.0_grammar.y

bison -d -o %1/src/_rc1.0_parser.cpp -p rc10_ bison/rc1.0_grammar.y

bison -d -o %1/src/_ts1.0_parser.cpp -p ts10_ bison/ts1.0_grammar.y

bison -d -o %1/src/_vs1.0_parser.cpp -p vs10_ bison/vs1.0_grammar.y

flex -P ps10_ --wincompat -o %1/src/_ps1.0_lexer.cpp flex/ps1.0_tokens.l
flex -P rc10_ --wincompat -o %1/src/_rc1.0_lexer.cpp flex/rc1.0_tokens.l
flex -P ts10_ --wincompat -o %1/src/_ts1.0_lexer.cpp flex/ts1.0_tokens.l
flex -P vs10_ --wincompat -o %1/src/_vs1.0_lexer.cpp flex/vs1.0_tokens.l
