
$(TopDir)/src/compiler/scanner.c: $(TopDir)/src/compiler/scanner.l
	@$(ECHO) "  [LEX] compiler/scanner.l"
	@$(LEX) -o $(TopDir)/src/compiler/scanner.c $(TopDir)/src/compiler/scanner.l


$(TopDir)/src/compiler/parser.c: $(TopDir)/src/compiler/parser.y
	@$(ECHO) "  [YACC] compiler/parser.y"
	@$(YACC) $(TopDir)/src/compiler/parser.y -o $(TopDir)/src/compiler/parser.c


$(BuildDir)/compiler_main.o: $(TopDir)/src/compiler/main.c
	@$(ECHO) "  [COMPILE] compiler/main.c"
	@$(COMPILE) $(TopDir)/src/compiler/main.c -o $(BuildDir)/compiler_main.o


$(BuildDir)/compiler_parser.o: $(TopDir)/src/compiler/parser.c
	@$(ECHO) "  [COMPILE] compiler/parser.c"
	@$(COMPILE) $(TopDir)/src/compiler/parser.c -o $(BuildDir)/compiler_parser.o


$(BuildDir)/compiler_scanner.o: $(TopDir)/src/compiler/scanner.c
	@$(ECHO) "  [COMPILE] compiler/scanner.c"
	@$(COMPILE) $(TopDir)/src/compiler/scanner.c -o $(BuildDir)/compiler_scanner.o


$(BuildDir)/compiler.a:  $(BuildDir)/compiler_main.o $(BuildDir)/compiler_parser.o $(BuildDir)/compiler_scanner.o
	@$(ECHO) "  [ARCHIVE] compiler.a"
	@$(AR) $(BuildDir)/compiler.a  $(BuildDir)/compiler_main.o $(BuildDir)/compiler_parser.o $(BuildDir)/compiler_scanner.o

