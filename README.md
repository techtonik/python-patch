Library to parse and apply unified diffs.

[![Build Status](https://img.shields.io/travis/techtonik/python-patch/master)](https://travis-ci.org/techtonik/python-patch/branches) [![PyPI](https://img.shields.io/pypi/v/patch)](https://pypi.python.org/pypi/patch)

### Features

 * Python 2 and 3 compatible
 * Automatic correction of
   * Linefeeds according to patched file
   * Diffs broken by stripping trailing whitespace
   * a/ and b/ prefixes
 * Single file, which is a command line tool and a library
 * No dependencies outside Python stdlib
 * Patch format detection (SVN, HG, GIT)
 * Nice diffstat histogram
 * Linux / Windows / OS X
 * Test coverage

Things that don't work out of the box:

 * File renaming, creation and removal
 * Directory tree operations
 * Version control specific properties
 * Non-unified diff formats


### Usage

Download **patch.py** and run it with Python. It is a self-contained
module without external dependencies.

    patch.py diff.patch

You can also run the .zip file.
    
    python patch-1.16.zip diff.patch

### Installation

**patch.py** is self sufficient. You can copy it into your repository
and use it from here. This setup will always be repeatable. But if
you need to add `patch` module as a dependency, make sure to use strict
specifiers to avoid hitting an API break when version 2 is released:

    pip install "patch==1.*"


### Other stuff

* [CHANGES](doc/CHANGES.md)
* [LICENSE](doc/LICENSE)
* [CREDITS](doc/CREDITS)

* [test coverage](http://techtonik.github.io/python-patch/tests/coverage/)
