SVGS=$(wildcard ./*.svg)
LISPS=$(wildcard ./*.lisp)
TXTS=$(SVGS:.svg=.txt)

all: $(TXTS)

%.txt: %.svg $(LISPS)
	python3 ../devtools/svg2rects.py $< $@

.PHONY: clean watch

clean:
	rm -rfv $(TXTS)

watch:
	@echo "Watching for changes to ${SVGS} ..."
	@inotifywait -m -q -e modify,move_self ${SVGS} | \
	while read -r filename event; do \
		make; \
	done
