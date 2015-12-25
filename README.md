## Status ##

[![](https://drone.io/techtonik/python-patch/status.png)](https://drone.io/techtonik/python-patch)

**API status**: API is unstable, so **use strict dependencies on major
version number** when using this tool as a library.

It understands only _unified diffs_. Currently it doesn't support file
renames, creation and removals.

Note that **patch.py** was not designed to reproduce original files. Parsing
is a lossy process where data is normalized to be cross-platform. Absolute
paths are stripped as well as references to parent directories, backslashes
are converted to forward slashes and so on.

**patch.py** is designed to transparently handle line end differences. Line
endings from patch are converted into
best suitable format for patched file. patch.py scans line endings in source
file, and if they are consistent - lines from patch are applied with the
same ending. If source linefeeds are inconsistend - lines from patch are
applied "as is".


Parsing of diff is done in a in very straightforward manner as an exercise
to approach the problem of parsing on my own before learning the 'proper
ways'. Thanks creators, _the format of unified diff_ is rather simple (an
illustration of Subversion style unified diff is included in
[source doc/](http://python-patch.googlecode.com/svn/trunk/doc/) directory).

## Library usage ##

See [APIUseCases](APIUseCases.md).

## Future ##

Patch utility in Python makes it possible to implement online "submit,
review and apply" module. Similar to [Review Board](http://www.reviewboard.org/)
for code, but suitable for all kind of textual content that uses
unified diffs as an interchange format between users, website, and version
control system. With this system patches can be applied after on site
review, automatically storing the names of patch contributors in SVN
history logs without requiring write access for these contributors. This
system is not the scope of this project though.

Additional unified diff parsers may be added in future to compare different
parsing techniques (with [pyparsing](http://pyparsing.wikispaces.com/),
[SPARK](http://www.ibm.com/developerworks/library/l-spark.html) or
[others](http://www.google.com/Top/Computers/Programming/Languages/Python/Modules/Text_Processing/)
as example).

See also https://code.google.com/p/rainforce/wiki/ModerationQueue

It would be nice to further simplify parser, make it more modular to allow easy
customization and extension, but the primary focus for now is to figure out
an API that will make it usable as a library. There is separate TODO item to
check behavior of "\ No newline at end of file" cases. Other goals is to
expand test coverage, and try to make script more interactive.
