
This is pygtkspellcheck 4.0.1 written by Maximilian Köhl and Carlos Jenkins

Changes introduced by Giuseppe Penone:

# file spellcheck.py
-        self._table.add(self._misspelled)
+        if not self._table.lookup(self._misspelled.get_property("name")): self._table.add(self._misspelled)
