#!/usr/bin/env racket
#lang racket/base

(printf "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n")
(printf "<svg xmlns=\"http://www.w3.org/2000/svg\">\n")
(let ((n (read)))
  (for ([_ n])
    (apply printf
           "    <rect x=\"~a\" y=\"~a\" width=\"~a\" height=\"~a\" style=\"fill:#0000ff\" />\n"
           (for/list ([_ 4]) (read)))))
(printf "</svg>\n")
