# risotto
Examples of Arbor I/O and interop.

## Contra/nesci demo

The `contra-demo` directory contains an example of a multithreaded, single rank
arbor simulation in communication with a contra client, communicating voltage
traces while running.

If nesci or contra cannot be found at configure time, we build versions based
on the git submodules under `external/`. These are currently forks of the upstream
versions in order to work around some CMake and python version issues.

If nesci and/or contra are built locally, their corresponding python modules
will be installed under `external/lib/python3.7/site-packages`, relative to
the build directory. This path will need to be added to the python search path
for the `contra-demo-client.py` python client to run.

This code is very preliminary, and is essentially a cut down version of the work
committed in the 2018 Arbor Hackathon.



