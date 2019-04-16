CXX         = clang++
LIBARYFLAGS = 
CXXFLAGS    = -std=c++1z -Wall -Wextra -Wparentheses -g $(SANS)

.PHONY:all seg msan
all:
	@make -j1 format --no-print-directory
	@make -j1 TAGS --no-print-directory
	@make -j1 deps --no-print-directory
	@make -j1 mains --no-print-directory
seg: clean msan
msan:
	make --no-print-directory all SANS=-fsanitize=address

# generate the ctags file
TAGS:
	@rm -f TAGS
	@ls|grep "pp$$"|xargs -r etags -a
	@echo "Generated Tags"

# use the ctags file to find all excicutables
.PHONY:mains
mains:
	@for f in `ls *.c*` ; do \
		if etags -o - $$f | grep "int main(" - > /dev/null; \
			then echo $$f | sed -e 's/[.][^.]*$$/.bin/' -e 's/.*/make --no-print-directory &/' |sh; \
		fi ; \
	done

.PHONY:deps
deps:
	-@for f in `ls *.cpp` ; do \
		echo $$f | sed -e 's,cpp$$,d,' -e 's/.*/make -s .d\/&/'|sh; \
	done

# dependancy making
DEPDIR      = .d
$(shell mkdir -p $(DEPDIR) > /dev/null)
.PRECIOUS: $(DEPDIR)/%.d
$(DEPDIR)/%.d: %.cpp
	@set -e; rm -f $@; \
	 $(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 sed -e 's,[.]o\([ :]\),.bin\1,g' -e 's,[.][hc]pp\>,.o,g' -e 's,\<[a-zA-Z]*[.]tpp\>,,g' < $@.$$$$ >> $@; \
	 printf '\t$$(CXX) $$(CXXFLAGS) $$(LIBARYFLAGS) -o $$@ $$^' >> $@; \
	 echo >> $@; \
	 rm -f $@.$$$$
	@echo "remade $@"

# emacs flycheck-mode
.PHONY:check-syntax csyntax
check-syntax: csyntax
csyntax:
	$(CXX) $(CXXFLAGS) -c ${CHK_SOURCES} -o /dev/null

.PHONY: clean
clean:
	rm -f *.o *.bin .d/*.d
	rmdir .d

.PHONY: format
format:
	@find|egrep '.*[.](cpp|cxx|cc|c++|c|tpp|txx)$$'|sed 's/[] ()'\''\\[&;]/\\&/g'|xargs clang-format -i -style=file
	@echo "reformatted code"

include $(wildcard $(DEPDIR)/*.d)
include $(wildcard *.d)
