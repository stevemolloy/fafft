# fafft
Performs FFT's on the data produced by my "[fars](https://github.com/stevemolloy/fars)" application.

# Building
- Make sure the [FFTW](https://www.fftw.org/download.html) library is available on your system.
- Edit the defns.h file if needed (to disable/enable threading and/or debugging output)

```bash
$ make
```
# Running
The following will run the tool on any file matching the input parameter.  Output files have the same name as the input (and so will be in the same directory as the input), but with `.fft` appended.
```bash
./bin/fafft ./example/fa_data.00*.dat
```
