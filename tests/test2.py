def json_parse(j):
    def is_number(s):
        i = 0
        c = 0
        while i < len(s):
            if s[i] == '.':
                if not c:
                    c += 1
                else:
                    return False
            if s[i] not in '0123456789.':
                return False
            i += 1
        return True

    def to_number(s):
        p = 0.0

        d = {}
        for i in range(11):
            d[str(i)] = i

        i = 0
        saw_dot = False
        while i < len(s):
            sv = s[i]
            if sv == '.':
                saw_dot = True
                break
            p = p * 10 + d[sv]
            i += 1

        if saw_dot:
            i += 1
            q = 10.0
            while i < len(s):
                sv = s[i]
                p = p + (d[sv] / q)
                q *= 10
                i += 1

        return p

    jp = 0

    while j[jp] in ' \t\n':
        jp += 1

    if j[jp] != '{':
        raise ValueError(f"Invalid character '{j[jp]}'")

    jp += 1
    res = {}

    while jp < len(j):
        while j[jp] in ' \t\n':
            jp += 1

        if j[jp] != '"':
            raise ValueError(f"Invalid character '{j[jp]}'")

        jp += 1
        key = ""
        while j[jp] != '"':
            key += j[jp]
            jp += 1

        jp += 1

        while j[jp] in ' \t\n':
            jp += 1

        if j[jp] != ':':
            raise ValueError(f"Invalid character '{j[jp]}'")

        jp += 1

        in_string = False
        quote_str = ""
        in_bracket = 0
        value = ""

        while jp < len(j):
            jv = j[jp]

            if jv == '}' and not in_bracket:
                break

            if jv in '[{':
                in_bracket += 1
            if jv in ']}':
                in_bracket -= 1

            if jv in '\'"':
                if not in_string:
                    in_string = True
                    quote_str = jv
                else:
                    if j[jp - 1] != '\\':
                        in_string = False
                    elif jp > 1 and j[jp - 2] == '\\':
                        in_string = False

            if jv == ',' and not in_string and not in_bracket:
                break

            value += jv
            jp += 1

        vl = 0
        vr = len(value) - 1

        while value[vl] in ' \t\n':
            vl += 1
        while value[vr] in ' \t\n':
            vr -= 1

        value = value[vl:vr + 1]

        if value[0] in '\'"':
            res[key] = value[1:-1]

        elif is_number(value):
            res[key] = to_number(value)

        elif value[0] == '{':
            res[key] = json_parse(value)

        elif value in ('true', 'false'):
            res[key] = value == 'true'

        elif value == 'none':
            res[key] = None

        elif value[0] == '[':
            vp = 1
            idx = ""

            in_string = False
            quote_str = ""
            in_bracket = 0
            idcs = []

            while vp < len(value):
                vv = value[vp]

                if vv == ']' and not in_bracket:
                    idcs.append(idx)
                    idx = ""
                    break

                if vv in '[{':
                    in_bracket += 1
                if vv in ']}':
                    in_bracket -= 1

                if vv in '\'"':
                    if not in_string:
                        in_string = True
                        quote_str = vv
                    else:
                        if value[vp - 1] != '\\':
                            in_string = False
                        elif vp > 1 and value[vp - 2] == '\\':
                            in_string = False

                if vv == ',' and not in_string and not in_bracket:
                    idcs.append(idx)
                    idx = ""
                    vp += 1
                    continue

                idx += vv
                vp += 1

            for i in range(len(idcs)):
                iv = idcs[i]
                il = 0
                ir = len(iv) - 1

                while iv[il] in ' \t\n':
                    il += 1
                while iv[ir] in ' \t\n':
                    ir -= 1

                iv = iv[il:ir + 1]
                idcs[i] = json_parse('{"." :' + iv + '}')["."]

            res[key] = idcs

        jp += 1

    return res


# ---- test ----
j = '{\n\t"b": 10,\n\t"a": {"c": [20, true, none]}}'
i = 1
while True:
    print(i)
    print(json_parse(j))
    i += 1
