from svg.tags import *

def svg_rects(svg_root):
    return [rect for rect in svg_root.iter(RECT_TAG_NAME)]


def svg_texts(svg_root):
    return [rect for rect in svg_root.iter(TEXT_TAG_NAME)]
