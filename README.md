Description
-----------

This library is a fork of the semaphore code from musl, except with the 256 named semaphores limit removed. It can be `LD_PRELOAD`ed to override the original `sem_*()` functions.


Building
--------

```sh
mkdir build
cd build
cmake ..
make -j4
ctest -VV
```


License
-------

This library is licensed under the same terms as musl itself. See [COPYRIGHT](COPYRIGHT) for more information.
