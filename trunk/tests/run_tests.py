"""
TestSuite

Files/directories that comprise one test all have the same name, but a different extensions:
*.patch
*.from
*.to

*.doctest   - self contained doctest patch

"""


import sys
import re
import shutil
from os import listdir
from os.path import abspath, dirname, join, isdir
from tempfile import mkdtemp



#: full path for directory with tests
tests_dir = dirname(abspath(__file__))


# import patch.py from parent directory
save_path = sys.path
sys.path.insert(0, dirname(tests_dir))
import patch
sys.path = save_path


# run testcases - every test starts with number
testptn = re.compile(r"^(?P<name>\d{2,}.+)\.(?P<ext>[^\.]+)")
testset = sorted( set([testptn.match(e).group('name') for e in listdir(tests_dir) if testptn.match(e)]) )

for testname in testset:
  # 1. create temp test directory
  # 2. copy files
  # 3. execute file-based patch 
  # 4. cleanup on success

  tmpdir = mkdtemp(prefix="%s."%testname)

  patch_file = join(tmpdir, "%s.patch" % testname)
  shutil.copy(join(tests_dir, "%s.patch" % testname), patch_file)
  
  from_src = join(tests_dir, "%s.from" % testname)

  if not isdir(from_src):
    shutil.copy(from_src, join(tmpdir, "%s.from" % testname))
  else:
    for e in listdir(from_src):
      if e == ".svn":
        continue
      if not isdir(e):
        shutil.copy(join(from_src, e), join(tmpdir, e))
      else:
        shutil.copytree(join(from_src, e), join(tmpdir, e))

  # 3. :todo

  shutil.rmtree(tmpdir)
