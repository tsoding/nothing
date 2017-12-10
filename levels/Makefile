SVGS=$(wildcard ./*.svg)
TXTS=$(SVGS:.svg=.txt)

all: $(TXTS)

%.txt: %.svg
	xqilla ../scripts/svg2rects.xqe -i $< -o $@

.PHONY: clean

clean:
	rm -rfv $(TXTS)
