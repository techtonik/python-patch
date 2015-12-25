Parse and apply unified diffs.

[![Build Status](https://img.shields.io/travis/techtonik/python-patch.svg)](https://travis-ci.org/techtonik/python-patch)

### Usage

Download **patch.py** and run it with Python. It is a self-contained
module without external dependencies.

    patch.py diff.patch

You can also run the .zip file.
    
    python patch-1.14.2.zip diff.patch

### Installation

**patch.py** is self sufficient. You can copy it into your repository
and use it from here. This setup will always be repeatable. But if
you need to add `patch` module as a dependency, make sure to use strict
specifiers to avoid hitting an API break:

    pip install "patch>=1,<2"

With pip 6.x.x and later it is possible to use the alternative syntax:

    pip install "patch==1.*"


### Other stuff

* [LICENSE](doc/LICENSE)
* [CREDITS](doc/CREDITS)
* <a href="https://gratipay.com/techtonik/"><img src='https://img.shields.io/gratipay/techtonik.svg'/></a>
