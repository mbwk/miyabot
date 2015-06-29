# miyabot Makefile
# 

CC					= clang
CXX					= clang++
CFLAGS					= -fdiagnostics-show-location=every-line \
					-std=c89 -Wall -Werror -pedantic \
					-Wfatal-errors -g -iquote include/
LD					= $(CC)
LDFLAGS				= 
STRIP				= strip --strip-unneeded
RM					= rm
CTAGS				= ctags -R

# project directories
SRCDIR				= src
INCDIR				= include
OBJDIR				= build
BINDIR				= bin

# files, deduced by make every time it is run
SRCS				= ${wildcard $(SRCDIR)/*.c}
HDRS				= ${wildcard $(INCDIR)/*.h}
OBJS				= ${patsubst src/%.c,build/%.o,$(SRCS)}
EXE				= $(BINDIR)/miyabot


build/%.o: src/%.c
				$(CC) $(CFLAGS) -c $< -o $@

all: $(EXE)

run: $(EXE)
				$(EXE)

$(EXE): $(OBJS)
				$(LD) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
				$(CTAGS) $(SRCS) $(HDRS)

# all object files in OBJDIR are purged along with executable
clean:
				$(RM) -f $(EXE) ${wildcard $(OBJDIR)/*.o}

dist: $(EXE)
				$(STRIP) $(EXE)

