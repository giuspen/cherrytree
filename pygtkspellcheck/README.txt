
This is pygtkspellcheck 4.0.1 written by Maximilian Köhl and Carlos Jenkins


Changes introduced by Giuseppe Penone:


###### 1) problem of tag added to tag table twice
# file spellcheck.py
-        self._table.add(self._misspelled)
+        if not self._table.lookup(self._misspelled.get_property("name")): self._table.add(self._misspelled)


###### 2) notification of language changed to cherrytree
# file spellcheck.py
-    def __init__(self, view, language='en', prefix='gtkspellchecker',
+    def __init__(self, view, cherrytree_instance, language='en', prefix='gtkspellchecker',
                  collapse=True, params={}):
         self._view = view
+        self._cherrytree_instance = cherrytree_instance
         self.collapse = collapse

     def _languages_menu(self):
         def _set_language(item, code):
             self.language = code
+            self._cherrytree_instance.spell_check_notify_new_lang(code)
         if _pygobject:


###### 3) problem of callbacks not to work when spell check is not enabled
# file spellcheck.py
     def _click_move_popup(self, *args):
+        if not self._enabled:
+            return False

     def _click_move_button(self, widget, event):
+        if not self._enabled:
+            return
