# object files for this project
objects = $(addprefix $(build_dir)/,variadic-function.o)
# name of the executable
target = $(build_dir)/,variadic-function.out

# directory section
build_dir:=./build
src_dir:=.

# C compiler and linker to be invoked
# -g: Produce debugging information, -Wall: enable resonable warnings
CC := gcc -Wall -g  
# C compiler switches
switches :=

#
# #### \end of parameter section
#

# the default target
$(target): $(objects) 
	$(CC) $(objects) -o $(target) 

# all: | $(build_dir)

# $(build_dir):
#	mkdir $(build_dir)

# rule to make objects out of sources living in another directory
$(build_dir)/%.o: $(src_dir)/%.c
	$(CC) $(switches) -c $< -o $@

# include dependency files, 
-include $(subst .o,.d,$(objects))
# rule to build dependecy files; sed includes dependency %.o : %.d  
$(build_dir)/%.d: $(src_dir)/%.c
	mkdir -p $(build_dir); \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# Declare "clean" as .PHONY to avoid confusion with a file called "clean" 
.PHONY : clean 
clean:
	rm $(target) $(objects) $(subst .o,.d,$(objects)) *~



