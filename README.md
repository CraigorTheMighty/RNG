RNG
===

Small library for random number generation. Includes integer random number generation (essentially a glorified hash), and high-quality float/double generation that utilise all bit patterns between 0.0 and 1.0.

Usage
=====

-```RNG_IsValid(rng_t *rng)```

Checks whether an RNG is valid. Returns non-zero if the RNG is valid, or zero if the RNG is invalid.

-```RNG_New()``` 

Returns a new RNG. The RNG returned will have a unique 256-bit seed. Seeds will only repeat after 2^256 RNGs have been created. Call ```RNG_IsValid(rng_t *rng)``` to determine whether the RNG is valid before using it.

-```RNG_Destroy(rng_t *rng)``` 

Destroys the RNG and all memory associated with it.

-```RNG_Clone(rng_t *rng)```

Clone an existing RNG. The cloned RNG will have an exact copy of the internal state of the original RNG, and will output the same sequence of numbers as the original RNG given the same sequence of operations. Call ```RNG_IsValid(rng_t *rng)``` to determine whether the RNG is valid before using it.

-```RNG_SetMaxStackSize(rng_t *rng, uint32_t size)```

Set the maximum stack size that an RNG can have. Returns zero on success, and non-zero on failure. In the case of failure, the RNG retains its original unmodified state. The default stack size is determined by ```RNG_DEFAULT_MAX_STATE_SIZE```, which is 65536 bytes.

-```RNG_GetMaxStackSize(rng_t *rng)```

Get the maximum stack size of an RNG.

-```RNG_GetStackDepth(rng_t *rng)```

Return the size of the user portion of the RNG stack. This is defined as the maximum stack size, minus 32 bytes for the RNG seed, minus the length of the user-defined ID, if any. This value will always be >= 0.

-```RNG_SetID(rng_t *rng, void *data, uint32_t data_len)```
-```RNG_SetIDString(rng_t *rng, char *string)```
-```RNG_SetIDu64(rng_t *rng, uint64_t id)```
-```RNG_SetIDStringHash(rng_t *rng, char *string)```

Set the user-defined ID of the RNG as either raw data, a NULL-terminated string, a ```uint64_t```, or the 64-bit hash of a NULL-terminated string. Return zero on success, and non-zero on failure. In the case of failure, the RNG retains its original unmodified state.

-```RNG_GetIDType(rng_t *rng)```

Returns the type the ID field of the RNG. This will always be one of the values:

        -```RNG_ID_TYPE_STRING```
        -```RNG_ID_TYPE_U64```
        -```RNG_ID_TYPE_HASH```
        -```RNG_ID_TYPE_GENERIC```


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
