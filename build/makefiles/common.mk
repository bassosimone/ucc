
$(BuildDir)/common_allocator.o: $(TopDir)/src/common/allocator.c
	@$(ECHO) "  [COMPILE] common/allocator.c"
	@$(COMPILE) $(TopDir)/src/common/allocator.c -o $(BuildDir)/common_allocator.o


$(BuildDir)/common_logger.o: $(TopDir)/src/common/logger.c
	@$(ECHO) "  [COMPILE] common/logger.c"
	@$(COMPILE) $(TopDir)/src/common/logger.c -o $(BuildDir)/common_logger.o


$(BuildDir)/common.a:  $(BuildDir)/common_allocator.o $(BuildDir)/common_logger.o
	@$(ECHO) "  [ARCHIVE] common.a"
	@$(AR) $(BuildDir)/common.a  $(BuildDir)/common_allocator.o $(BuildDir)/common_logger.o

