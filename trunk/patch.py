""" Patch utility to apply unified diffs written in Python """
""" Brute-force regex approach """

import logging
from logging import debug, warning


logging.basicConfig(level=logging.DEBUG, format="%(levelname)8s %(message)s")

def read_patch(filename):
  lineends = dict(lf=0, crlf=0, cr=0)

  fp = open(filename, "rb")
  for line in fp:
    # gather stats about line endings
    # todo: account only source/target content lines
    if line.endswith("\r\n"):
      lineends["crlf"] += 1
    elif line.endswith("\n"):
      lineends["lf"] += 1
    elif line.endswith("\r"):
      lineends["cr"] += 1

    # analyze state 
  fp.close()

  debug("patch stats - crlf: %(crlf)d  lf: %(lf)d  cr: %(cr)d" % lineends)
  if ((lineends["cr"]!=0) + (lineends["crlf"]!=0) + (lineends["lf"]!=0)) > 1:
    warning("inconsistent line endings")


read_patch("fix_devpak_install.patch")
