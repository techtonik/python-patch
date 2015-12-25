Library to parse and apply unified diffs.

[![Build Status](https://img.shields.io/travis/techtonik/python-patch.svg)](https://travis-ci.org/techtonik/python-patch)

### Features

 * Automatic correction of
   * Linefeeds according to patched file
   * Diffs broken by stripping trailing whitespace
   * a/ and b/ prefixes
 * Single file, which is a command line tool and library
 * No dependencies outside Python stdlib
 * Patch format detection (SVN, HG, GIT)
 * Nice diffstat histogram
 * Linux / Windows / OX X
 * Python 2.5+ compatible, 2.6/2.7 tested
 * Test coverage

Things that don't work out of the box:

 * Python 3
 * File renaming, creation and removal
 * Directory tree operations
 * Version control specific properties
 * Non-unified diff formats


### Usage

Download **patch.py** and run it with Python. It is a self-contained
module without external dependencies.

    patch.py diff.patch

You can also run the .zip file.
    
    python patch-1.15.zip diff.patch

### Installation

**patch.py** is self sufficient. You can copy it into your repository
and use it from here. This setup will always be repeatable. But if
you need to add `patch` module as a dependency, make sure to use strict
specifiers to avoid hitting an API break:

    pip install "patch>=1,<2"

With pip 6.x.x and later it is possible to use the alternative syntax:

    pip install "patch==1.*"


### Other stuff
* [CHANGES](doc/CHANGES.md)
* [LICENSE](doc/LICENSE)
* [CREDITS](doc/CREDITS)
* <a href="https://gratipay.com/techtonik/"><img src='https://img.shields.io/gratipay/techtonik.svg'/></a>
