import os
import logging
from patch import fromfile, fromstring

class PatchLogHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self, logging.DEBUG)

    def emit(self, record):
        logstr = self.format(record)
        print logstr

patchlog = logging.getLogger("patch")
patchlog.handlers = []
patchlog.addHandler(PatchLogHandler())

patch = fromstring("""--- /dev/null
+++ b/newfile
@@ -0,0 +0,3 @@
+New file1
+New file2
+New file3
""")

patch.apply(root=os.getcwd(), strip=0)


with open("newfile", "rb") as f:
    newfile = f.read()
    assert "New file1\nNew file2\nNew file3\n" == newfile

patch = fromstring("""--- a/newfile
+++ /dev/null
@@ -0,3 +0,0 @@
-New file1
-New file2
-New file3
""")

result = patch.apply(root=os.getcwd(), strip=0)

assert os.path.exists("newfile") is False