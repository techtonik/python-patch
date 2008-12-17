""" Patch utility to apply unified diffs """
""" Brute-force line-by-line parsing 

    Project home: http://code.google.com/p/python-patch/
    $Id$
    $HeadURL$

"""

__author__ = "techtonik.rainforce.org"
__version__ = "8.12-1"

import copy
import logging
import re
from StringIO import StringIO
from logging import debug, info, warning


debugmode = False


class HunkInfo(object):
  """ parsed hunk data (hunk starts with @@ -R +R @@) """

  def __init__(self):
    # define HunkInfo data members
    self.startsrc=None
    self.linessrc=None
    self.starttgt=None
    self.linestgt=None
    self.invalid=False
    self.text=[]

  def copy(self):
    return copy.copy(self)


def patch_from_file(filename):
  """
  read and parse unified diff file into python structure - dict table
  where entries are columns and each row corresponds to one source file

  {
    source, # list of source filenames
    target, # list of target filenames (not used)
    hunks,  # list of lists of hunks
    hunkends, # file endings statistics in hunks
  }

  this structure is essentialy a table with a row for every source file
  """

  files = dict(source=[], target=[], hunks=[], hunkends=[])

  # define possible file regions that will direct the parser flow
  header = False    # comments before the patch body
  filenames = False # lines starting with --- and +++

  hunkhead = False  # @@ -R +R @@ sequence
  hunkbody = False  #
  hunkskip = False  # skipping invalid hunk mode

  header = True
  lineends = dict(lf=0, crlf=0, cr=0)
  nextfileno = 0
  nexthunkno = 0    #: even if index starts with 0 user messages number hunks from 1

  # hunkinfo holds parsed values, hunkactual - calculated
  hunkinfo = HunkInfo()
  hunkactual = dict(linessrc=None, linestgt=None)

  info("reading patch %s" % filename)

  fp = open(filename, "rb")
  for lineno, line in enumerate(fp):

    # analyze state
    if header and line.startswith("--- "):
      header = False
      # switch to filenames state
      filenames = True
    #: skip hunkskip and hunkbody code until you read definition of hunkhead
    if hunkbody:
      # process line first
      if re.match(r"^[- \+\\]", line):
          # gather stats about line endings
          if line.endswith("\r\n"):
            files["hunkends"][nextfileno-1]["crlf"] += 1
          elif line.endswith("\n"):
            files["hunkends"][nextfileno-1]["lf"] += 1
          elif line.endswith("\r"):
            files["hunkends"][nextfileno-1]["cr"] += 1
            
          if line.startswith("-"):
            hunkactual["linessrc"] += 1
          elif line.startswith("+"):
            hunkactual["linestgt"] += 1
          elif not line.startswith("\\"):
            hunkactual["linessrc"] += 1
            hunkactual["linestgt"] += 1
          hunkinfo.text.append(line)
          # todo: handle \ No newline cases
      else:
          warning("invalid hunk no.%d at %d for target file %s" % (nexthunkno, lineno+1, files["target"][nextfileno-1]))
          # add hunk status node
          files["hunks"][nextfileno-1].append(hunkinfo.copy())
          files["hunks"][nextfileno-1][nexthunkno-1]["invalid"] = True
          # switch to hunkskip state
          hunkbody = False
          hunkskip = True

      # check exit conditions
      if hunkactual["linessrc"] > hunkinfo.linessrc or hunkactual["linestgt"] > hunkinfo.linestgt:
          warning("extra hunk no.%d lines at %d for target %s" % (nexthunkno, lineno+1, files["target"][nextfileno-1]))
          # add hunk status node
          files["hunks"][nextfileno-1].append(hunkinfo.copy())
          files["hunks"][nextfileno-1][nexthunkno-1]["invalid"] = True
          # switch to hunkskip state
          hunkbody = False
          hunkskip = True
      elif hunkinfo.linessrc == hunkactual["linessrc"] and hunkinfo.linestgt == hunkactual["linestgt"]:
          files["hunks"][nextfileno-1].append(hunkinfo.copy())
          # switch to hunkskip state
          hunkbody = False
          hunkskip = True

          # detect mixed window/unix line ends
          ends = files["hunkends"][nextfileno-1]
          if ((ends["cr"]!=0) + (ends["crlf"]!=0) + (ends["lf"]!=0)) > 1:
            warning("inconsistent line ends in patch hunks for %s" % files["source"][nextfileno-1])
          if debugmode:
            debuglines = dict(ends)
            debuglines.update(file=files["target"][nextfileno-1], hunk=nexthunkno)
            debug("crlf: %(crlf)d  lf: %(lf)d  cr: %(cr)d\t - file: %(file)s hunk: %(hunk)d" % debuglines)

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
        if debugmode and len(files["source"]) > 0:
          debug("- %2d hunks for %s" % (len(files["hunks"][nextfileno-1]), files["source"][nextfileno-1]))

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
            files["hunkends"].append(lineends.copy())
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
        hunkinfo.startsrc = int(match.group(1))
        hunkinfo.linessrc = int(match.group(3) if match.group(3) else 1)
        hunkinfo.starttgt = int(match.group(4))
        hunkinfo.linestgt = int(match.group(6) if match.group(6) else 1)
        hunkinfo.invalid = False
        hunkinfo.text = []

        hunkactual["linessrc"] = hunkactual["linestgt"] = 0

        # switch to hunkbody state
        hunkhead = False
        hunkbody = True
        nexthunkno += 1
        continue
  else:
    if not hunkskip:
      warning("patch file incomplete - %s" % filename)
      # sys.exit(?)
    else:
      # duplicated message when an eof is reached
      if debugmode and len(files["source"]) > 0:
          debug("- %2d hunks for %s" % (len(files["hunks"][nextfileno-1]), files["source"][nextfileno-1]))

  info("total files: %d  total hunks: %d" % (len(files["source"]), sum(len(hset) for hset in files["hunks"])))
  fp.close()
  return files


def check_patched(filename, hunks):
  matched = True
  fp = open(filename)

  class NoMatch(Exception):
    pass

  lineno = 1
  line = fp.readline()
  hno = None
  try:
    if not len(line):
      raise NoMatch
    for hno, h in enumerate(hunks):
      # skip to line just before hunk starts
      while lineno < h.starttgt-1:
        line = fp.readline()
        lineno += 1
        if not len(line):
          raise NoMatch
      for hline in h.text:
        # todo: \ No newline at the end of file
        if not hline.startswith("-") and not hline.startswith("\\"):
          line = fp.readline()
          lineno += 1
          if not len(line):
            raise NoMatch
          if line.rstrip("\r\n") != hline[1:].rstrip("\r\n"):
            warning("file is not patched - failed hunk: %d" % (hno+1))
            raise NoMatch
  except NoMatch:
    matched = False
    # todo: display failed hunk, i.e. expected/found

  fp.close()
  return matched



def patch_stream(instream, hunks):
  """ given a source stream and hunks iterable, yield patched stream
  
      converts lineends in hunk lines to the best suitable format
      autodetected from input
  """

  # todo: At the moment substituted lineends may not be the same
  #       at the start and at the end of patching. Also issue a
  #       warning/throw about mixed lineends (is it really needed?)

  hunks = iter(hunks)

  srclineno = 1

  lineends = {'\n':0, '\r\n':0, '\r':0}
  def get_line():
    """
    local utility function - return line from source stream
    collecting line end statistics on the way
    """
    line = instream.readline()
      # 'U' mode works only with text files
    if line.endswith("\r\n"):
      lineends["\r\n"] += 1
    elif line.endswith("\n"):
      lineends["\n"] += 1
    elif line.endswith("\r"):
      lineends["\r"] += 1
    return line

  for hno, h in enumerate(hunks):
    debug("hunk %d" % (hno+1))
    # skip to line just before hunk starts
    while srclineno < h.startsrc:
      yield get_line()
      srclineno += 1

    for hline in h.text:
      # todo: check \ No newline at the end of file
      if hline.startswith("-") or hline.startswith("\\"):
        get_line()
        srclineno += 1
        continue
      else:
        if not hline.startswith("+"):
          get_line()
          srclineno += 1
        line2write = hline[1:]
        # detect if line ends are consistent in source file
        if sum([bool(lineends[x]) for x in lineends]) == 1:
          newline = [x for x in lineends if lineends[x] != 0][0]
          yield line2write.rstrip("\r\n")+newline
        else: # newlines are mixed
          yield line2write
   
  for line in instream:
    yield line



def patch_hunks(srcname, tgtname, hunks):
  src = open(srcname, "rb")
  tgt = open(tgtname, "wb")

  debug("processing target file %s" % tgtname)

  tgt.writelines(patch_stream(src, hunks))

  tgt.close()
  src.close()
  return True
  


from os.path import exists, isfile
from os import unlink
from pprint import pprint

def apply_patch(patch):
  total = len(patch["source"])
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
    filename = f2patch

    info("processing %d/%d:\t %s" % (fileno+1, total, filename))

    # validate before patching
    f2fp = open(filename)
    hunkno = 0
    hunk = patch["hunks"][fileno][hunkno]
    hunkfind = []
    hunkreplace = []
    validhunks = 0
    canpatch = False
    for lineno, line in enumerate(f2fp):
      if lineno+1 < hunk.startsrc:
        continue
      elif lineno+1 == hunk.startsrc:
        hunkfind = [x[1:].rstrip("\r\n") for x in hunk.text if x[0] in " -"]
        hunkreplace = [x[1:].rstrip("\r\n") for x in hunk.text if x[0] in " +"]
        #pprint(hunkreplace)
        hunklineno = 0

        # todo \ No newline at end of file

      # check hunks in source file
      if lineno+1 < hunk.startsrc+len(hunkfind)-1:
        if line.rstrip("\r\n") == hunkfind[hunklineno]:
          hunklineno+=1
        else:
          debug("hunk no.%d doesn't match source file %s" % (hunkno+1, filename))
          # file may be already patched, but we will check other hunks anyway
          hunkno += 1
          if hunkno < len(patch["hunks"][fileno]):
            hunk = patch["hunks"][fileno][hunkno]
            continue
          else:
            break

      # check if processed line is the last line
      if lineno+1 == hunk.startsrc+len(hunkfind)-1:
        debug("file %s hunk no.%d -- is ready to be patched" % (filename, hunkno+1))
        hunkno+=1
        validhunks+=1
        if hunkno < len(patch["hunks"][fileno]):
          hunk = patch["hunks"][fileno][hunkno]
        else:
          if validhunks == len(patch["hunks"][fileno]):
            # patch file
            canpatch = True
            break
    else:
      if hunkno < len(patch["hunks"][fileno]):
        warning("premature end of source file %s at hunk %d" % (filename, hunkno+1))

    f2fp.close()

    if validhunks < len(patch["hunks"][fileno]):
      if check_patched(filename, patch["hunks"][fileno]):
        warning("already patched  %s" % filename)
      else:
        warning("source file is different - %s" % filename)
    if canpatch:
      backupname = filename+".orig"
      if exists(backupname):
        warning("can't backup original file to %s - aborting" % backupname)
      else:
        import shutil
        shutil.move(filename, backupname)
        if patch_hunks(backupname, filename, patch["hunks"][fileno]):
          warning("successfully patched %s" % filename)
          unlink(backupname)
        else:
          warning("error patching file %s" % filename)
          shutil.copy(filename, filename+".invalid")
          warning("invalid version is saved to %s" % filename+".invalid")
          # todo: proper rejects
          shutil.move(backupname, filename)

  # todo: check for premature eof



from optparse import OptionParser
from os.path import exists
import sys

if __name__ == "__main__":
  opt = OptionParser(usage="%prog [options] unipatch-file", version="python-patch %s" % __version__)
  opt.add_option("-d", action="store_true", dest="debugmode", help="debug mode")
  (options, args) = opt.parse_args()

  if not args:
    opt.print_version()
    print("")
    opt.print_help()
    sys.exit()
  debugmode = options.debugmode
  patchfile = args[0]
  if not exists(patchfile) or not isfile(patchfile):
    sys.exit("patch file does not exist - %s" % patchfile)


  if debugmode:
    logging.basicConfig(level=logging.DEBUG, format="%(levelname)8s %(message)s")
  else:
    logging.basicConfig(level=logging.INFO, format="%(message)s")



  patch = patch_from_file(patchfile)
  #pprint(patch)
  apply_patch(patch)

  # todo: document and test line ends handling logic - patch.py detects proper line-endings
  #       for inserted hunks and issues a warning if patched file has incosistent line ends
