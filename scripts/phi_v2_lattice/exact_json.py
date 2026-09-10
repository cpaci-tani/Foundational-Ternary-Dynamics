"""Compact exact-record JSON without changing Python's global digit limit.

The integer spelling and sorted object encoding match the existing checkpoint
format. Large ordinals therefore need no new schema or changed ordinary bytes.
Floating-point values and behavior-bearing containers are outside this codec.
"""
from __future__ import annotations

import json


def integer_text(value: int) -> str:
    if type(value) is not int:
        raise TypeError("exact JSON requires a concrete integer")
    negative = value < 0
    value = abs(value)
    pieces = []
    while value >= 1_000_000_000:
        value, remainder = divmod(value, 1_000_000_000)
        pieces.append(f"{remainder:09d}")
    return ("-" if negative else "") + str(value) + "".join(reversed(pieces))


def integer_from_text(text: str) -> int:
    if type(text) is not str or not text:
        raise ValueError("expected a canonical decimal integer")
    negative = text[0] == "-"
    digits = text[1:] if negative else text
    if (not digits or any(c < "0" or c > "9" for c in digits)
            or (len(digits) > 1 and digits[0] == "0") or (negative and digits == "0")):
        raise ValueError("expected a canonical decimal integer")
    value = 0
    for offset in range(0, len(digits), 9):
        part = digits[offset:offset + 9]
        value = value * 10 ** len(part) + int(part)
    return -value if negative else value


def dumps(value) -> str:
    kind = type(value)
    if value is None:
        return "null"
    if kind is bool:
        return "true" if value else "false"
    if kind is int:
        return integer_text(value)
    if kind is str:
        return json.dumps(value, ensure_ascii=True)
    if kind is list or kind is tuple:
        return "[" + ",".join(dumps(item) for item in value) + "]"
    if kind is dict:
        if any(type(key) is not str for key in value):
            raise TypeError("exact JSON object keys must be concrete strings")
        return "{" + ",".join(dumps(key) + ":" + dumps(value[key])
                               for key in sorted(value)) + "}"
    raise TypeError(f"unsupported exact JSON value: {kind.__name__}")


def loads(data, *, object_pairs_hook=None):
    def reject_float(text):
        raise ValueError("floating-point values are outside exact-record JSON")

    return json.loads(data, parse_int=integer_from_text,
                      parse_float=reject_float, parse_constant=reject_float,
                      object_pairs_hook=object_pairs_hook)
