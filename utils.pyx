
import numpy as np

import cat
import re

cpdef str drop_brackets(str cat):
    if cat.startswith('(') and \
        cat.endswith(')') and \
        find_closing_bracket(cat, 0) == len(cat)-1:
        return cat[1:-1]
    else:
        return cat


cpdef int find_closing_bracket(str source, int start) except -1:
    cdef int open_brackets = 0
    for i, c in enumerate(source):
        if c == '(':
            open_brackets += 1
        elif c == ')':
            open_brackets -= 1

        if open_brackets == 0:
            return i

    raise Exception("Mismatched brackets in string")


cpdef int find_non_nested_char(str haystack, str needles):
    cdef int open_brackets = 0

    for i, c in enumerate(haystack):
        if c == '(':
            open_brackets += 1
        elif c == ')':
            open_brackets -= 1
        elif open_brackets == 0:
            for n in needles:
                if n == c: return i
    return -1


cpdef list get_context_by_window(
        list items, int window_size, object lpad, object rpad):
    cdef list res = []
    cdef list context
    cdef int i, j
    cdef object item
    for i, item in enumerate(items):
        context = []
        if window_size - i > 0:
            for j in xrange(window_size - i):
                context.append(lpad)
            for j in xrange(i):
                context.append(items[j])
        else:
            for j in xrange(i - window_size, i):
                context.append(items[j])
        context.append(item)
        if i + window_size >= len(items):
            for j in xrange(i + 1, len(items)):
                context.append(items[j])
            for j in xrange(i + window_size - len(items) + 1):
                context.append(rpad)
        else:
            for j in xrange(i + 1, i + window_size + 1):
                context.append(items[j])
        assert len(context) == window_size * 2 + 1

        res.append(context)
    return res


cpdef np.ndarray[FLOAT_T, ndim=2] read_pretrained_embeddings(str filepath):
    cdef object io
    cdef int i, dim
    cdef int nvocab = 0
    cdef str line
    cdef np.ndarray[FLOAT_T, ndim=2] res

    io = open(filepath)
    dim = len(io.readline().split())
    io.seek(0)
    for _ in io:
        nvocab += 1
    io.seek(0)
    res = np.empty((nvocab, dim), dtype=np.float32)
    for i, line in enumerate(io):
        line = line.strip()
        if len(line) == 0: continue
        res[i] = line.split()
    io.close()
    return res


cpdef dict read_model_defs(str filepath):
    """
    input file is made up of lines, "ITEM FREQUENCY".
    """
    cdef dict res = {}
    cdef int i
    cdef str line, word, _

    for i, line in enumerate(open(filepath)):
        word, _ = line.strip().split(" ")
        res[word] = i
    return res


cdef np.ndarray[FLOAT_T, ndim=2] compute_outsize_probs(list supertags):
    cdef int sent_size = len(supertags)
    cdef int i, j
    cdef:
        np.ndarray[FLOAT_T, ndim=2] res = \
            np.zeros((sent_size + 1, sent_size + 1), 'f')
        np.ndarray[FLOAT_T, ndim=1] from_left = \
            np.zeros((sent_size + 1,), 'f')
        np.ndarray[FLOAT_T, ndim=1] from_right = \
            np.zeros((sent_size + 1,), 'f')

    for i in xrange(sent_size - 1):
        j = sent_size - i
        from_left[i + 1]  = from_left[i] + supertags[i][0][1]
        from_right[j - 1] = from_right[j] + supertags[j - 1][0][1]

    for i in xrange(sent_size + 1):
        for j in xrange(i, sent_size + 1):
            res[i, j] = from_left[i] + from_right[j]

    return res


cpdef dict load_unary(str filename):
    cdef dict res = {}
    cdef str line
    cdef int comment
    cdef list items
    cdef object inp, out

    for line in open(filename):
        comment = line.find("#")
        if comment > -1:
            line = line[:comment]
        line = line.strip()
        if len(line) == 0:
            continue
        items = line.split()
        assert len(items) == 2
        inp = cat.Cat.parse(items[0])
        out = cat.Cat.parse(items[1])
        if res.has_key(inp):
            res[inp].append(out)
        else:
            res[inp] = [out]
    return res


feat = re.compile("\[nb\]|\[X\]")
cpdef dict load_seen_rules(str filename):
    cdef dict res = {}
    cdef str line
    cdef int comment
    cdef list items
    cdef object cat1, cat2

    for line in open(filename):
        comment = line.find("#")
        if comment > -1:
            line = line[:comment]
        line = line.strip()
        if len(line) == 0:
            continue
        items = line.split()
        assert len(items) == 2
        cat1 = cat.Cat.parse(feat.sub("", items[0]))
        cat2 = cat.Cat.parse(feat.sub("", items[1]))
        res[(cat1, cat2)] = True
    return res


