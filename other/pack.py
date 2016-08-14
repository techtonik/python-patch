#!/usr/bin/env python
"""
Wrap Python module into executable .zip package.

Extracts required meta-data (author|maintainer, name, version,
url) and optional fields (description) from module without
importing it.

  * [x] name, version
  * [x] author
  * [x] license
  * [ ] url
  * [ ] description (first line of module docstring)

Public domain work by:
  anatoly techtonik <techtonik@gmail.com>
"""
import os
import sys

def get_field(path, name='__version__'):
  '''Read named string from module without importing it'''
  for line in open(path, 'rb'):
    # Decode to unicode for PY2/PY3 in a fail-safe way
    line = line.decode('cp437')
    if name in line:
      # __version__ = "0.9"
      delim = '\"' if '\"' in line else '\''
      return line.split(delim)[1]

def get_description(path):
  '''Return first non-empty line from module docstring'''
  mf = open(path, 'rb')
  for i, line in enumerate(mf):
    if i > 10:
      # stop looking after 10 lines
      break
    line = line.decode('utf-8').strip()
    if '"""' in line or "'''" in line:
      while line.strip('\n\t\r \'\"') == '':
        line = mf.next()
        line = line.decode('utf-8').strip()
      return line

def zipadd(archive, filename, newname):
  '''Add filename to archive. `newname` is required. Otherwise
     zipfile may create unsafe entries, such as "../patch.py".
     Returns open ZipFile object.
  '''
  import zipfile
  zf = zipfile.ZipFile(archive, 'a', zipfile.ZIP_DEFLATED)
  zf.write(filename, newname)
  return zf

class MiniJinja(object):
    """Template engine that knows how to render {{ tag }}"""

    def __init__(self, templates='.'):
        """templates  - template path"""
        import re
        import sys
        self.PY3K = sys.version_info[0] == 3

        self.path = templates + '/'
        self.tag  = re.compile('{{ *(?P<tag>\w+) *}}')

    def render(self, tplfile, vardict=None, **kwargs):
        """returns unicode str"""
        tpltext = open(self.path + tplfile).read()
        return self.render_string(tpltext, vardict, **kwargs)

    def render_string(self, tpltext, vardict=None, **kwargs):
        data = vardict or {}
        data.update(kwargs)

        def lookup(match):
            return data[match.group('tag')]

        if not self.PY3K:
            return unicode(self.tag.sub(lookup, tpltext))
        else:
            return self.tag.sub(lookup, tpltext)

# ---

BASE = os.path.abspath(os.path.dirname(__file__))

MAINTPL = """\
import sys

import {{ module }}
sys.exit({{ module }}.main())
"""

if __name__ == '__main__':
  if not sys.argv[1:]:
    sys.exit("usage: pack.py <module.py>")

  modpath = sys.argv[1]
  tplvars = dict(
    module = os.path.basename(modpath)[:-3], # also strip extension
    version = get_field(modpath, '__version__'),
    author = get_field(modpath, '__author__'),
    license = get_field(modpath, '__license__'),
    description = get_description(modpath)
  )

  if tplvars['version'] == None:
    sys.exit("error: no __version__ specifier found in %s" % modpath)
  if tplvars['author'] == None:
    sys.exit("error: no __author__ specifier found in %s" % modpath)
  packname = tplvars['module'] + "-" + tplvars['version'] + ".zip"
  print("[*] Packing %s into %s" % (modpath, packname))
  if os.path.exists(packname):
    os.remove(packname)
  zf = zipadd(packname, modpath, os.path.basename(modpath))
  print("[*] Making %s executable" % (packname))
  # http://techtonik.rainforce.org/2015/01/shipping-python-tools-in-executable-zip.html
  text = MiniJinja(BASE).render_string(MAINTPL, **tplvars)
  zf.writestr('__main__.py', text)
  print("[*] Making %s installable" % (packname))
  text2 = MiniJinja(BASE).render('pack.setuppy.tpl', **tplvars)
  zf.writestr('setup.py', text2)
  print("[*] Making %s uploadable to PyPI" % (packname))
  zf.writestr('PKG-INFO', '')
  zf.close()

