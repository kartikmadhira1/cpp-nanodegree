## Testing

For exercises that have unit tests, the project must be built with the approprate test cpp file. This can be done by using `cmake_tests` instead of `cmake`. For example, from the build directory:
```
cmake_tests ..
make
```
The tests can then be run from the `build` directory as follows:
```
../bin/test
```
