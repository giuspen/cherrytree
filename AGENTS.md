# AGENTS.md

## Project Overview

CherryTree is a C++ hierarchical note-taking application built with GTKmm and
GtkSourceView. The main application code is in `src/ct`, tests are in `tests`,
and CMake/Ninja is the supported build path.

GTK3 is the primary build. GTK4 support is guarded with
`GTKMM_MAJOR_VERSION` and is still work in progress. Preserve both code paths
when changing shared UI code.

## Repository Layout

- `src/ct/`: application code
- `tests/`: GoogleTest test executables and test data
- `data/`, `icons/`, `styles/`, `language-specs/`: runtime resources
- `po/`: translations
- `src/7za`, `src/spdlog`, `tests/googletest`: bundled dependencies/submodules
- `BUILDING.md`: platform-specific dependency instructions
- `MULTI_NODE_EDITOR.md`: multi-node editor architecture and implementation notes
- `build.sh`: standard configure/build wrapper

Avoid editing bundled third-party code unless the task explicitly concerns it.

## Dependencies

Initialize submodules before the first build:

```sh
git submodule update --init --recursive
```

On Debian/Ubuntu, the main build dependencies are:

```sh
sudo apt install build-essential cmake ninja-build libgtkmm-3.0-dev \
  libgtksourceview-4-dev libxml++2.6-dev libsqlite3-dev gettext \
  libgspell-1-dev libcurl4-openssl-dev libuchardet-dev libfribidi-dev \
  libvte-2.91-dev libfmt-dev libspdlog-dev file libxml2-utils xvfb
```

See `BUILDING.md` for other platforms and optional LaTeX support.

## Build

The standard development build is:

```sh
./build.sh debug
```

For an explicit testable configuration without automatically running tests:

```sh
cmake -S . -B build -GNinja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON \
  -DAUTO_RUN_TESTING=OFF
ninja -C build
```

Useful variants:

```sh
./build.sh release
./build.sh debug bundledspdfmt
./build.sh debug gtk4
```

Do not delete or recreate the build directory unless configuration state is
actually the problem. Prefer an incremental `ninja -C build` build.

## Tests

Run all three project test executables after behavior changes:

```sh
build/run_tests_no_x
xvfb-run -a build/run_tests_with_x_1
xvfb-run -a build/run_tests_with_x_2
```

- `run_tests_no_x`: utility and non-GUI tests
- `run_tests_with_x_1`: export tests
- `run_tests_with_x_2`: storage/read-write and GTK integration tests

GUI tests require an X server. In headless environments, use `xvfb-run -a`.
Focused GoogleTest runs are encouraged while iterating:

```sh
xvfb-run -a build/run_tests_with_x_2 \
  --gtest_filter='ReadWriteTests/ReadWriteMultipleParametersTests.ChecksReadWrite/0'
```

Before finishing, also run:

```sh
git diff --check
```

## Coding Guidelines

- Follow the surrounding C++ style and existing ownership patterns.
- Keep GTK3 and GTK4 branches adjacent and narrowly scoped.
- Use `CtTreeIter` and `CtTreeStore` APIs instead of bypassing the tree model.
- Use `CtTextView` helpers instead of directly configuring only the permanent
  main GtkSourceView when behavior must apply to every editor view.
- Wrap user-visible strings with `_()` and avoid unnecessary translation churn.
- Add comments only for non-obvious lifetime, reentrancy, or data-model logic.
- Keep changes focused; do not reformat unrelated code.

## Multi-Node Editor

The implementation is split by responsibility:

- `src/ct/ct_main_win_events.cc`: tree selection routing and active-editor state
- `src/ct/ct_main_win_multi_node.cc`: section widgets, paging, sizing, teardown,
  preference propagation, and anchored-widget editor activation
- `src/ct/ct_main_win.h`: section ownership and multi-node state
- `src/ct/ct_treestore.cc`: binding real node buffers and embedded widgets to
  each editor view
- `tests/tests_multi_node.cpp`: focused integration and stress coverage

See `MULTI_NODE_EDITOR.md` for the full design and data flow.

Important invariants:

- Selected nodes are displayed in tree order.
- Shared nodes with the same data holder are displayed only once.
- Every section edits the node's real text buffer; content is not copied into a
  synthetic combined buffer.
- Text actions target the focused section. Structural tree actions target the
  tree cursor.
- `curr_tree_iter()` and `get_text_view()` represent the focused editor;
  `tree_cursor_iter()` represents the structural tree target;
  `selected_tree_iters()` returns unique data holders in tree order.
- `_pActiveTextview` must never point to a temporary view during or after
  multi-editor teardown.
- Disconnect section focus, buffer, and tree-store signal connections before
  removing or destroying section widgets.
- Callbacks must not retain raw section pointers across rebuilds. Use the
  generation guard and resolve the current section before access.
- Guard rebuilds against synchronous focus and selection callbacks emitted by
  GTK while widgets are removed.
- Preferences affecting editor presentation must be applied to every section,
  not just `_ctTextview`.
- `Scroll Beyond Last Line` applies only to the final multi-node section and is
  restored normally when returning to a single editor.
- Large selections are paged at 25 rendered sections. Keep the complete tree
  selection intact while loading buffers and embedded widgets only for the
  current page.
- When a selection exceeds one page or its estimated layout height is unsafe,
  place sections in inner scrollers capped at 600 pixels. Do not restore an
  unbounded vertical GtkTextView layout; it can violate GTK text-tree layout
  invariants on large documents.

When modifying this area, extend the stress coverage in
`tests/tests_multi_node.cpp`. The scenarios are invoked from the existing
read/write application lifecycle in `tests/tests_read_write.cpp`. Exercise
repeated section focus, tree focus, single-selection teardown, multi-selection
rebuilds, shared-node deduplication, paging, and large-document scrollers.

## Debugging Crashes

Build in debug mode and reproduce under GDB:

```sh
./build.sh debug
gdb ./build/cherrytree
```

For GUI crashes in a headless environment:

```sh
xvfb-run -a gdb ./build/cherrytree
```

Focus-related crashes are often synchronous GTK callback or object-lifetime
issues. Inspect signal disconnection order and temporary widget ownership before
adding defensive null checks.

## Working Tree Safety

The working tree may contain user changes. Do not reset, discard, or rewrite
unrelated modifications. Review the current diff before editing files that are
already modified, and preserve existing work.
