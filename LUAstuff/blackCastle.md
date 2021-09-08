# blackCastle, a public domain JSON parser

blackCastle is a public domain JSON parser.

## The name

This is called "Black Castle" because Douglas Crockford's (same guy
who invented JSON) old game "Galahad and the Holy Grail" had a very
difficult puzzle where a secret passage was hidden in the black castle;
the passage was so difficult to find, I had to run the program through
a 6502 monitor to find it.

A 1984 review of "Galahad and the Holy Grail" had the same complaint:
In "The book of Atari Software 1984", the reviewer could not find this
secret passage until Atari sent them a map and hint sheet.

To Douglas Crockford's credit, the 1985 Antic version of the game has
an arrow in the black castle pointing to the secret passage.

## Usage

```lua
require("blackCastle")
foo = blackCastle("file.json")
```

## String handling

To correctly add support for \ in JSON strings, this library
would need to have an entire Unicode subsystem: Convert Unicode
codepoints to UTF-8; convert UTF-8 sequences in to Unicode
code points (or, at least, make sure we don’t have invalid UTF-8
in our strings); convert surrogate pairs in to single code points;
etc.  It would make this quick and dirty JSON library about four
times larger.

So, instead, we just pass the string the JSON gives us as-is as
a "binary blob" to Lua.  The UTF-8 infinity symbol `∞` will become
a literal UTF-8 infinity symbol (Hex: E2 88 9E); the sequence
'\u221e' in a JSON string (which represents the infinity symbol)
will remain the ASCII string "\u221e" with the backslash passes as-is.

## null handling

If blackCastle receives a null keyword, it will, by default convert the
null in the the string `--NULL--`.  This value can be changed by giving
blackCastle an optional second argument.

## Bugs

* This only parses JSON input 
* This parser assumes that strings are binary blobs 
* This parser can only handle JSON encoded using a character encoding that
  is either ASCII or a superset of ASCII (e.g. UTF-8, any ISO-8859 encoding,
  etc.).  It can not handle non-ASCII encodings such as UTF-16 (UCS-2).
* This does not generate JSON
* This only reads JSON from a file
* This uses `tonumber` for numeric generation, which may not be 100% JSON
  compatible with some corner cases.
* This parser doesn't care where or one puts (or doesn't put) a `,` or `:`
* This parser is a quick and dirty hack.

## Useful non-JSON extensions

* The parser allows comments (with `#`).  At least one whitespace character
  should be before the `#`.
* The parser allows bare words for object keys.  A bare word must start
  with an ASCII letter, and can have ASCII letters, numbers, and `_` in it.
* The parser allows a comma after the last item in a list of array or
  object members.

## Permissive parser

blackCastle is a permissive parser: Should blackCastle obtain an input
which is not correct JSON, its behavior is usually undefined, but it will 
often times still parse the input.  

Exceptions to bad JSON being undefined are:

* Comments which start with a `#` after a whitespace character or newline.
  Comments are ignored by the parser until a *NIX/Linux newline (`\n`) ends
  the comment (DOS linefeeds are OK, because the `\n` after the `\r` ends
  the line with the comment).
* Object keys can be unquoted if they start with an ASCII letter and 
  continue with ASCII letters, numbers, and the `_` character.
* Commas placed at the end of a list of array or object members before a
  `}` or `]` are ignored.

