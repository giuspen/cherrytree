# Multi-Node Editor

## Purpose

The multi-node editor allows more than one selected tree node to be displayed
and edited at the same time. Each selected node appears as an editor section in
the main text area.

This is not a merged document view. Every section is backed by the node's real
`Gtk::TextBuffer`, so edits, undo state, syntax highlighting, embedded widgets,
and storage notifications continue to use the normal CherryTree data model.

The implementation preserves the existing single-node editor and reuses its
`CtTextView` for one section. Additional visible sections receive temporary
`CtTextView` instances owned by `CtMainWin`.

## Main Files

- `src/ct/ct_main_win_events.cc` handles tree-selection changes and tracks the
  editor section that currently has focus.
- `src/ct/ct_main_win_multi_node.cc` builds and tears down the multi-node UI,
  manages section signals, applies presentation preferences, and implements
  paging and bounded layouts.
- `src/ct/ct_main_win.h` owns the widgets, section records, active editor
  pointers, rebuild guards, and page state.
- `src/ct/ct_treestore.cc` binds each `CtTextView` to a node's real buffer and
  connects buffer, cursor, and embedded-widget behavior to the correct section.
- `src/ct/ct_text_view.cc` contains view-level helpers that work for both the
  permanent editor and temporary section editors.
- `src/ct/ct_actions_tree.cc` distinguishes structural tree operations from
  operations that act on the focused editor.
- `tests/tests_multi_node.cpp` contains the integration and stress scenarios.

## Implementation Changes

The feature required a few supporting changes outside the section-building
code:

- The tree selection mode changed from single to multiple selection, and the
  main window now handles `Gtk::TreeSelection::signal_changed()` instead of the
  tree cursor signal.
- Active-editor state was separated from the tree cursor so editor actions and
  structural tree actions can target different selected nodes safely.
- Tree-store buffer and widget callbacks were generalized to carry the
  destination `CtTextView`, rather than assuming the permanent `_ctTextview`.
- View presentation helpers were generalized so line numbers, scrolling
  behavior, CSS, and event connections can be applied to temporary editors.
- Structural actions were updated to use `tree_cursor_iter()` explicitly.
- Multi-node section lifecycle and presentation were extracted from
  `ct_main_win_events.cc` into `ct_main_win_multi_node.cc`; event routing stays
  in the events translation unit.
- Integration scenarios were separated into `tests/tests_multi_node.cpp` and
  remain part of the existing `run_tests_with_x_2` application lifecycle.

## Targeting Model

Multi-selection introduces three related but different concepts:

- `curr_tree_iter()` is the node represented by the focused editor section.
- `get_text_view()` is the focused section's `CtTextView`.
- `tree_cursor_iter()` is the row carrying the GTK tree cursor and is the target
  for structural operations such as adding, moving, duplicating, or deleting a
  node.
- `selected_tree_iters()` is the complete logical selection, returned in tree
  order and deduplicated by data-holder ID.

Text editing, formatting, clipboard operations, status information, and
embedded-widget actions use the focused editor pair. Structural tree actions
use the tree cursor even when focus is inside a different selected section.

`CtMainWin::_set_active_editor()` updates `_activeTreeIter` and
`_pActiveTextview` together. It also stores the previous editor's modified and
cursor state, updates the window header and status bar, and records selection
history when requested.

## Selection Flow

The tree view uses GTK multiple-selection mode. `CtMainWin` listens to
`Gtk::TreeSelection::signal_changed()` through
`_on_treeview_selection_changed()`, matching the signal that reports changes
to the complete selected set rather than only movement of the tree cursor.

When the selection changes:

1. `selected_tree_iters()` converts the selected paths to `CtTreeIter` values.
2. Shared nodes resolving to the same data holder are collapsed to one entry.
3. More than one unique entry calls `_show_multi_node_editor()`.
4. Zero or one entry calls `_clear_multi_node_editor()` and restores the normal
   single editor.
5. The single selected row, or tree cursor when no row is explicitly selected,
   is bound to the permanent `_ctTextview`.

GTK returns selected paths in tree order. Deduplication retains the first path
for each data holder, so the visible sections also remain in tree order.

## Section Ownership

`CtMainWin::CtMultiNodeSection` records everything owned or referenced by one
rendered section:

- the section's `CtTreeIter`;
- an optional owned temporary `CtTextView`;
- the actual `CtTextView` pointer, which may refer to `_ctTextview`;
- optional separator and title widgets;
- an optional section-local `Gtk::ScrolledWindow`;
- focus and buffer-change signal connections.

The section records are held in `_multiNodeSections` as `unique_ptr` values.
One section reuses `_ctTextview`; all other sections own their temporary views.
Reusing the permanent view avoids maintaining a second special editor and
keeps the single-node transition straightforward.

The reused view is selected in this order:

1. the previously active editor node, when it is on the rendered page;
2. the tree cursor node, when it is on the page;
3. the first node on the page.

## Buffer and Widget Binding

Each section calls `CtTreeStore::text_view_apply_textbuffer()` with its node and
view. The method applies syntax configuration, sets the node's real text
buffer, updates editability and spell checking, lays out embedded widgets, and
connects buffer signals with both the node and the destination `CtTextView`.

The tree store keeps all current editor-buffer connections and disconnects them
as a group before a multi-node rebuild. Signal callbacks receive the relevant
`CtTextView` instead of assuming `_ctTextview`, allowing cursor updates, column
editing, and anchored-widget handling to act on the correct section.

Embedded widgets are still owned by the node data holder. When a widget action
needs an active editor, `activate_editor_for_widget()` finds the current section
containing that widget and activates its node/view pair first.

## Building the View

`_setup_multi_node_editor()` configures the page bar and its Previous and Next
buttons. The constructor calls this helper after creating the permanent text
view.

`_show_multi_node_editor()` performs a full rebuild:

1. Preserve the active node and tree cursor IDs.
2. Clear any previous multi-node UI.
3. Mark the editor as rebuilding and advance the generation counter.
4. Replace the permanent text-view child with `_multiNodeBox`.
5. Select the page and decide whether bounded section scrollers are required.
6. Create, bind, and insert each rendered section.
7. Connect focus and buffer-height callbacks.
8. Show the box, end the rebuild guard, and restore the active section.

Optional section titles use either the node name or hierarchical path according
to existing header preferences. The `multi_node_show_titles` setting controls
whether these titles are rendered and refreshes active multi-node windows when
changed.

## Lifetime and Reentrancy

Widget removal can emit GTK focus and selection callbacks synchronously. A
temporary section must therefore never remain visible through
`_pActiveTextview` while its widget is being disconnected or destroyed.

Teardown follows this order:

1. Set `_multiNodeEditorRebuilding` and advance `_multiNodeEditorGeneration`.
2. Point `_pActiveTextview` back to `_ctTextview`.
3. Disconnect tree-store buffer connections.
4. Disconnect every section's focus and height connections.
5. Remove section widgets from their containers.
6. Clear `_multiNodeSections` and restore `_ctTextview` to the outer scroller.
7. Restore normal scrolling and presentation settings.
8. Clear the rebuild guard.

Section callbacks capture stable node and view identifiers, not a raw section
pointer. Before accessing a section they verify the rebuild generation and use
`_find_multi_node_section()` to resolve the current record. This prevents an old
callback from dereferencing a section destroyed by a synchronous rebuild.

## Paging and Layout Limits

Only 25 sections are rendered at once. The complete GTK tree selection remains
selected; paging changes only which buffers and widgets are currently loaded in
the text area.

The initial page contains the tree cursor when possible. Previous and Next
rebuild the view at the neighboring page boundary. The page bar is outside the
outer text scroller so it remains visible while the sections scroll.

An unbounded stack of large `GtkTextView` widgets can exceed safe GTK text-tree
layout dimensions. The implementation estimates page height from buffer line
counts. Bounded mode is used when either:

- the selection exceeds one 25-section page; or
- the estimated rendered page height exceeds 30,000 pixels.

In bounded mode every section is placed in its own scroller capped at 600
pixels. Overlay scrolling is disabled for both the outer and section-local
scrollers so their scrollbars do not obscure each other. Normal outer-scroller
settings are restored when multi-node mode ends.

For small pages, sections expand to an estimated text height and update that
height when their buffer changes.

## Presentation Preferences

Code that formerly configured only `_ctTextview` was generalized where the
setting must affect every editor section.

- Line numbers are applied by `CtMainWin::set_show_line_numbers()`.
- Scroll Beyond Last Line is applied by
  `CtMainWin::set_scroll_beyond_last_line()`.
- Only the final rendered section receives Scroll Beyond Last Line; applying it
  to intermediate sections would create large artificial gaps between nodes.
- GTK3 uses the `ct-view-no-scroll-beyond` CSS class to suppress bottom padding
  on intermediate sections. GTK4 uses the text-view bottom margin.
- Temporary views receive the same event connections and `ct-view-panel` CSS
  class as the permanent view.

Buffer or syntax changes that replace editor state call
`refresh_multi_node_editor()` when multi-node mode is active, ensuring every
section is rebound consistently.

## Tests

`tests/tests_multi_node.cpp` is compiled into `run_tests_with_x_2` and invoked
from the existing read/write application lifecycle. It covers:

- tree-order selection and shared-data-holder deduplication;
- focused editor buffer routing versus structural tree cursor targeting;
- line-number and Scroll Beyond Last Line propagation;
- repeated focus changes, rebuilds, and single-selection teardown;
- inner scrollers for large documents;
- 25-section paging while retaining all selected paths.

Run the focused integration case while iterating:

```sh
xvfb-run -a build/run_tests_with_x_2 \
  --gtest_filter='ReadWriteTests/ReadWriteMultipleParametersTests.ChecksReadWrite/0'
```

Before finishing a change, run all three project test executables and
`git diff --check` as described in `AGENTS.md`.
