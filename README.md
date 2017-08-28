# Protocols for Secure Genomic Computation

This library contains a proof-of-concept implementation of several protocols for
privacy-preserving genome computation. This implementation is a research prototype and not intended
for production-level code.
All of the code is provided under the GPL license.

--------------------------------
Paper Reference
--------------------------------

Our protocols are described in the following paper:
* Karthik A. Jagadeesh, David J. Wu, Johannes A. Birgmeier, Dan Boneh, and Gill Bejerano.
  **_Deriving genomic diagnoses without revealing patient genomes_**.
  *Science*, 357(6352):692-695, 2017.
  \[[Abstract](http://science.sciencemag.org/cgi/content/abstract/357/6352/692?ijkey=QfLj2XCXHf3Ww&keytype=ref&siteid=sci)\]
  \[[PDF](http://science.sciencemag.org/cgi/rapidpdf/357/6352/692?ijkey=QfLj2XCXHf3Ww&keytype=ref&siteid=sci)\]
  [\[Full Text](http://science.sciencemag.org/cgi/content/full/357/6352/692?ijkey=QfLj2XCXHf3Ww&keytype=ref&siteid=sci)\]

Note that the paper links are only active from the main repository
site: https://github.com/dwu4/genome-privacy

--------------------------------
Building the Library
--------------------------------

To compile the code, use the following commands
    
    ./build_libs
    make

The `./build_libs` command will compile the garbled circuit and oblivious transfer (OT)
extensions libraries. We use a modified version of the 
[JustGarble](https://github.com/irdan/justGarble) library for the garbled circuits
and the [OTExtension](https://github.com/encryptogroup/OTExtension) library for
the OT implementation.

--------------------------------
Using the Code
--------------------------------

Client and server modules are provided for evaluation of the three different
vector operations described in the paper:
* INTERSECTION (`BasicIntersectionClient` and `BasicIntersectionServer`)
* SETDIFF (`SetDiffClient` and `SetDiffServer`)
* MAX (`ArgMaxClient` and `ArgMaxServer`)

Simply run the associated program to obtain the usage details (e.g., required
parameter) for the particular program. We provide some toy input files for the
different tasks in the `inputs/` directory. These examples are intended to
demo the different functionalities, and are substantially smaller than the real
data we used in the paper. The following set of commands can be used to run
each program on the provided example input files:
* INTERSECTION
  * Server: `./tests/BasicIntersectionServer inputs/input_alice_intersection.txt 50000`
  * Client: `./tests/BasicIntersectionClient inputs/input_bob_intersection.txt 50000`
* SETDIFF
  * Server: `./tests/SetDiffServer inputs/input_alice_setdiff.txt 20000 2`
  * Client: `./tests/SetDiffClient inputs/input_bob_setdiff.txt 20000 2`
* MAX
  * Server: `./tests/ArgMaxServer inputs/input_alice_argmax.txt 20000 2`
  * Client: `./tests/ArgMaxClient inputs/input_bob_argmax.txt   20000 2`

