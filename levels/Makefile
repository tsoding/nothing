SVGS=$(wildcard ./*.svg)
TXTS=$(SVGS:.svg=.txt)
SVG2LEVEL=../build/devtools/svg2level/svg2level

all: $(TXTS)

.SECONDEXPANSION:

%.txt: %.svg $$(shell $(SVG2LEVEL) deps $$*.svg)
	$(SVG2LEVEL) compile $< $@

.PHONY: clean watch

clean:
	rm -rfv $(TXTS)

watch:
	@echo "Watching for changes to ${SVGS} ..."
	@inotifywait -m -q -e modify,move_self ${SVGS} | \
	while read -r filename event; do \
		make; \
	done

