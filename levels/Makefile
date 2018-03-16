SVGS=$(wildcard ./*.svg)
TXTS=$(SVGS:.svg=.txt)

all: $(TXTS)

%.txt: %.svg
	xqilla ../devtools/svg2rects.xqe -i $< -o $@

.PHONY: clean watch

clean:
	rm -rfv $(TXTS)

watch:
	@echo "Watching for changes to ${SVGS} ..."
	@inotifywait -m -q -e modify,move_self ${SVGS} | \
	while read -r filename event; do \
		make; \
	done
