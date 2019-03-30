SVGS=$(wildcard ./*.svg)
TXTS=$(SVGS:.svg=.txt)

all: $(TXTS)

.SECONDEXPANSION:

# TODO: devtools/scripts_of_svg.py must be rewritten in C
%.txt: %.svg $$(shell python3 ../devtools/scripts_of_svg.py $$*.svg)
	../build/svg2level $< $@

.PHONY: clean watch

clean:
	rm -rfv $(TXTS)

watch:
	@echo "Watching for changes to ${SVGS} ..."
	@inotifywait -m -q -e modify,move_self ${SVGS} | \
	while read -r filename event; do \
		make; \
	done

# TODO(#734): integrate svg2level written in C with the level building process
