SVGS=$(wildcard ./*.svg)
TXTS=$(SVGS:.svg=.txt)

all: $(TXTS)

.SECONDEXPANSION:

%.txt: %.svg $$(shell ../build/svg2level deps $$*.svg)
	../build/svg2level compile $< $@

.PHONY: clean watch

clean:
	rm -rfv $(TXTS)

watch:
	@echo "Watching for changes to ${SVGS} ..."
	@inotifywait -m -q -e modify,move_self ${SVGS} | \
	while read -r filename event; do \
		make; \
	done

