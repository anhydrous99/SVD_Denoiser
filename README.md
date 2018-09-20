# SVD Denoiser

Uses the selective elimination of singular values from SVD Decomposition to try to clean up noise from Images. 
To compile cmake is needed!

## Linux
```
git clone https://github.com/anhydrous99/SVD_Denoiser
cd SVD_Denoiser
mkdir build && cd build
cmake ..
make
```
## Windows
Use the CMake GUI and set the toolkit to the 64bit one. Afterwhich, use the Visual Studio project cmake creates to compile.
