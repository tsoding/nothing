#!/usr/bin/env python3

import sys
import xml.etree.ElementTree as ET
from svg.selectors import *
import itertools


def usage():
    print("Usage: ./scripts_of_svg.py <svg-file-name>")


def scripts_of_rects(svg_root, prefix):
    return [title.text.split()[0]
            for rect in svg_rects(svg_root)
            if rect.attrib['id'].startswith(prefix)
            for title in rect]


def scripts_of_svg(svg_file_name):
    svg_tree = ET.parse(svg_file_name)
    svg_root = svg_tree.getroot()
    return itertools.chain(
        scripts_of_rects(svg_root, 'script'),
        scripts_of_rects(svg_root, 'player'))


if __name__ == '__main__':
    if len(sys.argv) < 2:
        usage()

    print(" ".join(scripts_of_svg(sys.argv[1])))
