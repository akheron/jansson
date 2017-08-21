To build jansson in evc3.0:

1. Setup EVC++
    a. Tools -> Options -> Directories -> Include Files -> New
    b. Add win32/evc30 to include path (EVC doesn't have per-project include paths)
2. Ensure you have the minor patches from this branch (to silence errors about errno features Windows CE 3.0 doesn't have)
3. Build