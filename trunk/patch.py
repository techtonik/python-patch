""" Patch utility to apply unified diffs written in Python """
""" Brute-force line-by-line parsing """

import logging
import re
from logging import debug, warning


logging.basicConfig(level=logging.DEBUG, format="%(levelname)8s %(message)s")

def read_patch(filename):
  # define possible file regions that will direct the parser flow
  header = False    # comments before the patch body
  filenames = False # lines starting with --- and +++

  hunkhead = False  # @@ -R +R @@ sequence
  hunkbody = False  #
  hunkskip = False  # skipping invalid hunk mode

  header = True
  files = dict(source=[], target=[], hunks=[])
  nextfileno = 0
  nexthunkno = 0
  lineends = dict(lf=0, crlf=0, cr=0)

  # hunkinfo holds parsed values, hunkactual - calculated
  hunkinfo = dict(startsrc=None, linessrc=None, starttgt=None, linestgt=None, invalid=False, text=[])
  hunkactual = dict(linessrc=None, linestgt=None)

  fp = open(filename, "rb")
  for lineno, line in enumerate(fp):

    # analyze state
    if header and line.startswith("--- "):
      header = False
      # switch to filenames state
      filenames = True

    #: skip hunkskip and hunkbody code until you read definition of hunkhead
    if hunkbody:
      if hunkinfo["linessrc"] == hunkactual["linessrc"] and hunkinfo["linestgt"] == hunkactual["linestgt"]:
          files["hunks"][nextfileno-1].append(hunkinfo.copy())
          # switch to hunkskip state
          hunkbody = False
          hunkskip = True
          nexthunkno += 1

          debuglines = dict(lineends)
          debuglines.update(file=files["target"][nextfileno-1], hunk=nexthunkno-1)
          debug("crlf: %(crlf)d  lf: %(lf)d  cr: %(cr)d\t - file: %(file)s hunk: %(hunk)d" % debuglines)
          if ((lineends["cr"]!=0) + (lineends["crlf"]!=0) + (lineends["lf"]!=0)) > 1:
            warning("inconsistent line endings")

      elif not re.match(r"^[- \+\\]", line) or hunkinfo["linessrc"] < hunkactual["linessrc"]:
          warning("invalid hunk no.%d for target file %s" % (nexthunkno-1, files["target"][nextfileno-1]))
          # add hunk status node
          files["hunks"][nextfileno-1].append(hunkinfo.copy())
          files["hunks"][nextfileno-1][nexthunkno-1]["invalid"] = True
          # switch to hunkskip state
          hunkbody = False
          hunkskip = True
          nexthunkno += 1
      else:
          # gather stats about line endings
          if line.endswith("\r\n"):
            lineends["crlf"] += 1
          elif line.endswith("\n"):
            lineends["lf"] += 1
          elif line.endswith("\r"):
            lineends["cr"] += 1
            
          if line.startswith("-"):
            hunkactual["linessrc"] += 1
          elif line.startswith("+"):
            hunkactual["linestgt"] += 1
          elif not line.startswith("\\"):
            hunkactual["linessrc"] += 1
            hunkactual["linestgt"] += 1
          hunkinfo["text"].append(line)
          # todo: handle \ No newline cases

    if hunkskip:
      match = re.match("^@@ -(\d+)(,(\d+))? \+(\d+)(,(\d+))?", line)
      if match:
        # switch to hunkhead state
        hunkskip = False
        hunkhead = True
      elif line.startswith("--- "):
        # switch to filenames state
        hunkskip = False
        filenames = True

    if filenames:
      if line.startswith("--- "):
        if nextfileno in files["source"]:
          warning("skipping invalid patch for %s" % files["source"][nextfileno])
          del files["source"][nextfileno]
          # double source filename line is encountered
          # attempt to restart from this second line
        re_filename = "^--- ([^\t]+)"
        match = re.match(re_filename, line)
        if not match:
          warning("skipping invalid filename at line %d" % lineno)
          # switch back to header state
          filenames = False
          header = True
        else:
          files["source"].append(match.group(1))
      elif not line.startswith("+++ "):
        if nextfileno in files["source"]:
          warning("skipping invalid patch with no target for %s" % files["source"][nextfileno])
          del files["source"][nextfileno]
        else:
          # this should be unreachable
          warning("skipping invalid target patch")
        filenames = False
        header = True
      else:
        if nextfileno in files["target"]:
          warning("skipping invalid patch - double target at line %d" % lineno)
          del files["source"][nextfileno]
          del files["target"][nextfileno]
          nextfileno -= 1
          # double target filename line is encountered
          # switch back to header state
          filenames = False
          header = True
        else:
          re_filename = "^\+\+\+ ([^\t]+)"
          match = re.match(re_filename, line)
          if not match:
            warning("skipping invalid patch - no target filename at line %d" % lineno)
            # switch back to header state
            filenames = False
            header = True
          else:
            files["target"].append(match.group(1))
            nextfileno += 1
            # switch to hunkhead state
            filenames = False
            hunkhead = True
            nexthunkno = 0
            files["hunks"].append([])
            lineends = dict(lf=0, crlf=0, cr=0)
            continue

    if hunkhead:
      match = re.match("^@@ -(\d+)(,(\d+))? \+(\d+)(,(\d+))?", line)
      if not match:
        if nextfileno-1 not in files["hunks"]:
          warning("skipping invalid patch with no hunks for file %s" % files["target"][nextfileno-1])
          # switch to header state
          hunkhead = False
          header = True
          continue
        else:
          # switch to header state
          hunkhead = False
          header = True
      else:
        hunkinfo["startsrc"] = int(match.group(1))
        hunkinfo["linessrc"] = int(match.group(3) if match.group(3) else 1)
        hunkinfo["starttgt"] = int(match.group(4))
        hunkinfo["linestgt"] = int(match.group(6) if match.group(6) else 1)
        hunkinfo["invalid"] = False
        hunkinfo["text"] = []

        hunkactual["linessrc"] = hunkactual["linestgt"] = 0

        # switch to hunkbody state
        hunkhead = False
        hunkbody = True
        nexthunkno += 1
        continue

  # todo detect invalid last chunk
  fp.close()
  return files


from os.path import exists, isfile
from pprint import pprint

def apply_patch(patch):
  for fileno, filename in enumerate(patch["source"]):
    f2patch = filename
    if not exists(f2patch):
      f2patch = patch["target"][fileno]
      if not exists(f2patch):
        warning("source/target file does not exist\n--- %s\n+++ %s" % (filename, f2patch))
        continue
    if not isfile(f2patch):
      warning("not a file - %s" % f2patch)
      continue

    # validate before patching
    f2fp = open(f2patch)
    hunkno = 0
    hunk = patch["hunks"][fileno][hunkno]
    hunkfind = []
    hunkreplace = []
    for lineno, line in enumerate(f2fp):
      if lineno+1 < hunk["startsrc"]:
        continue
      elif lineno+1 == hunk["startsrc"]:
        hunkfind = [x[1:].rstrip() for x in hunk["text"] if x[0] in " -"]
        #pprint(hunkfind)
        hunkreplace = [x[1:].rstrip() for x in hunk["text"] if x[0] in " +"]
        #pprint(hunkreplace)
        # todo No newline at end of file


      if line.rstrip() == hunk["text"][0][1:].rstrip():
        print "+", line
        print "+", hunk["text"][0]
      #print hunk["startsrc"], lineno


patch = read_patch("fix_devpak_install.patch")
apply_patch(patch)

#pprint(patch)


