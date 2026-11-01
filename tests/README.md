# Component tests

The headless test drives real Dear ImGui frames and keyboard input. It checks
completion in two views of one TextEditor, both rendering orders, acceptance,
shared undo history, and moving focus when the originating view is hidden.

```sh
cmake -S tests -B build-tests -DIMGUI_DIR=/path/to/imgui
cmake --build build-tests --config Release
ctest --test-dir build-tests -C Release --output-on-failure --no-tests=error
```

CI runs on Linux and Windows against the same pinned ImGui 1.93 integration
used by Lumengine. No platform renderer or GPU is needed. This test does not
qualify all independent view state (for example find/replace or scrolling).
