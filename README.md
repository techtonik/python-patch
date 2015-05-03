Cross-platform **patch** utility to apply unified diffs.

### How to use?

Download **patch.py** and run it with Python. It is a self-contained file with not external dependencies.

    python patch.py diff.patch

### How to install?

It is better to just stuff **patch.py** into your repository and use it from here. This way your setup
will always be repeatable. If you still want to install it, make sure to use strict dependencies to
avoid hitting an API break:

    pip install "patch>=1,<2"

With pip 6.x.x and later it is possible to use the alternative syntax:


    pip install "patch==1.*"


### Other stuff

* <a href="https://gratipay.com/techtonik/"><img src='https://img.shields.io/gratipay/techtonik.svg'/></a>
* [LICENSE](doc/LICENSE)
* [CREDITS](doc/CREDITS)
