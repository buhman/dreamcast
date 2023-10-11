import io

def _render(out, lines):
    indent = " "
    level = 0
    for l in lines:
        if l and (l[0] == "}" or l[0] == ")"):
            level -= 2
            assert level >= 0, out.getvalue()

        out.write(indent * level + l + "\n")

        if l and (l[-1] == "{" or l[-1] == "("):
            level += 2

        if level == 0 and l and l[-1] == ";":
            if "static_assert" not in l:
                out.write("\n")
    return out

def renderer():
    out = io.StringIO()
    def render(lines):
        return _render(out, lines)
    return render, out
