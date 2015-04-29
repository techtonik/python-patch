#!/usr/bin/env python
"""
Wrap Python module into executable .zip file

Public domain work by:
  anatoly techtonik <techtonik@gmail.com>
"""
import os
import sys

def get_version(path):
  '''Read version info from a file without importing it'''
  for line in open(path, 'rb'):
    # Decode to unicode for PY2/PY3 in a fail-safe way
    line = line.decode('cp437')
    if '__version__' in line:
      # __version__ = "0.9"
      return line.split('"')[1]

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

    def render(self, template, vardict=None, **kwargs):
        """returns unicode str"""
        data = vardict or {}
        data.update(kwargs)

        def lookup(match):
            return data[match.group('tag')]

        tpl = open(self.path + template).read()
        if not self.PY3K:
            return unicode(self.tag.sub(lookup, tpl))
        else:
            return self.tag.sub(lookup, tpl)

# ---

BASE = os.path.abspath(os.path.dirname(__file__))

if __name__ == '__main__':
  if not sys.argv[1:]:
    sys.exit("usage: pack.py <module.py>")

  modpath = sys.argv[1]
  modname = os.path.basename(modpath)[:-3] # also strip extension
  version  = get_version(modpath)
  packname = modname + "-" + version + ".zip"
  print("[*] Packing %s into %s" % (modpath, packname))
  if os.path.exists(packname):
    os.remove(packname)
  zf = zipadd(packname, modpath, os.path.basename(modpath))
  print("[*] Making %s executable" % (packname))
  # http://techtonik.rainforce.org/2015/01/shipping-python-tools-in-executable-zip.html
  text = MiniJinja(BASE).render('pack.mainpy.tpl', module=modname)
  zf.writestr('__main__.py', text)
  print("[*] Making %s installable" % (packname))
  text2 = MiniJinja(BASE).render('pack.setuppy.tpl', module=modname, version=version)
  zf.writestr('setup.py', text2)
  zf.close()

