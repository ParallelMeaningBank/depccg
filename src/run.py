
from __future__ import print_function, unicode_literals
import argparse
import codecs
import sys

if sys.version_info.major == 2:
    sys.stdin = codecs.getreader('utf-8')(sys.stdin)

from depccg import PyAStarParser, PyJaAStarParser

Parsers = {"en": PyAStarParser, "ja": PyJaAStarParser}

class Token:
    def __init__(self, word, lemma, pos, chunk, entity):
        self.word   = word
        self.lemma  = lemma
        self.pos    = pos
        self.chunk  = chunk
        self.entity = entity

    @staticmethod
    def from_piped(string):
        # WORD|POS|NER or WORD|LEMMA|POS|NER
        items = string.split("|")
        if len(items) == 4:
            w, l, p, n = items
            return Token(w, l, p, "XX", n)
        else:
            w, p, n = items
            return Token(w, "XX", p, "XX", n)

def to_xml(trees, tagged_doc):
    print("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
    print("<?xml-stylesheet type=\"text/xsl\" href=\"candc.xml\"?>")
    print("<candc>")
    for tree, tagged in zip(trees, tagged_doc):
        assert len(tree) == len(tagged)
        print("<ccg>")
        print(tree.xml.format(*tagged))
        print("</ccg>")
    print("</candc>")

if __name__ == "__main__":
    parser = argparse.ArgumentParser("A* CCG parser")
    parser.add_argument("model", help="model directory")
    parser.add_argument("lang", help="language", choices=["en", "ja"])
    parser.add_argument("--input", default=None,
            help="a file with tokenized sentences in each line")
    parser.add_argument("--batchsize", type=int, default=32,
            help="batchsize in supertagger")
    parser.add_argument("--input-format", default="raw",
            choices=["raw", "POSandNERtagged"],
            help="input format")
    parser.add_argument("--format", default="auto",
            choices=["auto", "deriv", "xml", "ja", "conll"],
            help="output format")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    fin = sys.stdin if args.input is None else codecs.open(args.input, encoding="utf-8")
    if args.input_format == "POSandNERtagged":
        tagged_doc = [[Token.from_piped(token) for token in sent.strip().split(" ")]for sent in fin]
        doc = [" ".join([token.word for token in sent]) for sent in tagged_doc]
    else:
        assert args.format != "xml", \
                "XML output format is supported only with --input-format POSandNERtagged."
        doc = [l.strip() for l in fin]


    parser = Parsers[args.lang](args.model,
                               batchsize=args.batchsize,
                               loglevel=1 if args.verbose else 3)


    res = parser.parse_doc(doc)
    if args.format == "xml":
        to_xml(res, tagged_doc)
    else:
        for i, r in enumerate(res):
            print("ID={}".format(i))
            r.suppress_feat = True
            print(getattr(r, args.format))
