RNG
===

Small library for random number generation. Includes integer random number generation (essentially a glorified hash), and high-quality float/double generation that utilise all bit patterns between 0.0 and 1.0.

Usage
=====

```RNG_New()``` creates a new RNG. The RNG returned will have a unique 256-bit seed. Seeds will only wrap when 2^256 RNGs have been created, which should be more than enough for most applications.
```RNG_Destroy(rng_t *rng)``` destroys the RNG and all memory associated with it.

Dependencies
============

The default hashing function uses XXHash hashing library headers (https://github.com/Cyan4973/xxHash) for high-quality random number generation. XXHash is licensed under the BSD 2-clause license.

Compiling
=========

I personally use Microsoft Visual Studio - Community Edition 2022 to build. If you're building on an OS other than Windows, you will need to provide your own implementation of ```SpinLock_Lock``` and ```SpinLock_Unlock```, as well as a relevant ```spinlock_t``` type.

License
-------

MIT license. You can pretty much do what you like with this code, but please consider attributing me as the source.

Copyright (c) 2023 Craig Sutherland

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
