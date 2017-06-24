
$(TopDir)/src/optimizer/scanner.c: $(TopDir)/src/optimizer/scanner.l
	@$(ECHO) "  [LEX] optimizer/scanner.l"
	@$(LEX) -o $(TopDir)/src/optimizer/scanner.c $(TopDir)/src/optimizer/scanner.l


$(TopDir)/src/optimizer/parser.c: $(TopDir)/src/optimizer/parser.y
	@$(ECHO) "  [YACC] optimizer/parser.y"
	@$(YACC) $(TopDir)/src/optimizer/parser.y -o $(TopDir)/src/optimizer/parser.c


$(BuildDir)/optimizer_main.o: $(TopDir)/src/optimizer/main.c
	@$(ECHO) "  [COMPILE] optimizer/main.c"
	@$(COMPILE) $(TopDir)/src/optimizer/main.c -o $(BuildDir)/optimizer_main.o


$(BuildDir)/optimizer_parser.o: $(TopDir)/src/optimizer/parser.c
	@$(ECHO) "  [COMPILE] optimizer/parser.c"
	@$(COMPILE) $(TopDir)/src/optimizer/parser.c -o $(BuildDir)/optimizer_parser.o


$(BuildDir)/optimizer_scanner.o: $(TopDir)/src/optimizer/scanner.c
	@$(ECHO) "  [COMPILE] optimizer/scanner.c"
	@$(COMPILE) $(TopDir)/src/optimizer/scanner.c -o $(BuildDir)/optimizer_scanner.o


$(BuildDir)/optimizer.a:  $(BuildDir)/optimizer_main.o $(BuildDir)/optimizer_parser.o $(BuildDir)/optimizer_scanner.o
	@$(ECHO) "  [ARCHIVE] optimizer.a"
	@$(AR) $(BuildDir)/optimizer.a  $(BuildDir)/optimizer_main.o $(BuildDir)/optimizer_parser.o $(BuildDir)/optimizer_scanner.o

