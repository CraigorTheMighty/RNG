RNG
===

Small library for random number generation. Includes integer random number generation (essentially a glorified hash), and high-quality float/double generation that utilise all bit patterns between 0.0 and 1.0.

API
===

- ```RNG_IsValid(rng_t *rng)```

Checks whether an RNG is valid. Returns non-zero if the RNG is valid, or zero if the RNG is invalid.

- ```RNG_New()``` 

Returns a new RNG. The RNG returned will have a unique 256-bit seed. Seeds will only repeat after 2^256 RNGs have been created. Call ```RNG_IsValid(rng_t *rng)``` to determine whether the RNG is valid before using it.

- ```RNG_Destroy(rng_t *rng)``` 

Destroys the RNG and all memory associated with it.

- ```RNG_Clone(rng_t *rng)```

Clone an existing RNG. The cloned RNG will have an exact copy of the internal state of the original RNG, and will output the same sequence of numbers as the original RNG given the same sequence of operations. Call ```RNG_IsValid(rng_t *rng)``` to determine whether the RNG is valid before using it.


- ```RNG_SetTotalMaxStackSize(rng_t *rng, uint32_t size)```

Set the total size in bytes that the RNG can use for its internal state. ```size``` is silently modified internally to the closest power-of-two that is equal to or greater than ```size```. Returns zero on success, and non-zero on failure. Failure only occurs when ```size``` is less than the current amount of memory allocated for the state. The default stack size is determined by ```RNG_DEFAULT_MAX_STATE_SIZE```, which is defined as 65536 bytes.

- ```RNG_SetUserMaxStackSize(rng_t *rng, uint32_t size)```

Set the total size in bytes that the user data portion of the stack can grow to. ```size``` is silently modified internally to the closest power-of-two that is equal to or greater than ```size```. Returns zero on success, and non-zero on failure. Failure only occurs when ```size``` is less than the current amount of memory in the user data portion of the stack. The default user data size is determined by ```RNG_DEFAULT_MAX_STATE_SIZE```, which is defined as 65536 bytes.

- ```RNG_GetTotalMaxStackSize(rng_t *rng)```

Returns the maximum size in bytes that the RNG state can grow to.

- ```RNG_GetUserMaxStackSize(rng_t *rng)```

Returns the maximum size in bytes that the user portion of the RNG state can grow to. This will be, at most, the value set by ```RNG_SetUserMaxStackSize(rng_t *rng, uint32_t size)```, and depends on the size of the current user ID.

- ```RNG_GetTotalStackDepth(rng_t *rng)```

Returns the current number of bytes used by the RNG stack.

- ```RNG_GetUserStackDepth(rng_t *rng)```

Returns the current number of bytes used in the user portion of the RNG stack.

- ```RNG_ShrinkStack(rng_t *rng)```

Attempts to reduce memory usage by shrinking the RNG stack. Returns zero on success, and non-zero on failure. In the case of failure, the RNG retains its original unmodified state.

- ```RNG_SetID(rng_t *rng, void *data, uint32_t data_len)```
- ```RNG_SetIDString(rng_t *rng, char *string)```
- ```RNG_SetIDu64(rng_t *rng, uint64_t id)```
- ```RNG_SetIDStringHash(rng_t *rng, char *string)```

Set the user-defined ID of the RNG as either raw data, a NULL-terminated string, a ```uint64_t```, or the 64-bit hash of a NULL-terminated string. Return zero on success, and non-zero on failure. In the case of failure, the RNG retains its original unmodified state.

- ```RNG_GetIDType(rng_t *rng)```

Returns the type of the ID field of the RNG. This will always be one of the self-explantory values:

        RNG_ID_TYPE_STRING
        RNG_ID_TYPE_U64
        RNG_ID_TYPE_HASH
        RNG_ID_TYPE_GENERIC

- ```RNG_GetIDLength(rng_t *rng)```

Returns the length of the ID field of the RNG. In the case of ```RNG_ID_TYPE_STRING```, this is the length of the ID string plus the terminating NULL character. i.e. if the RNG has ID string "rngID", the returned length will be 5 + 1 = 6 bytes.

- ```RNG_CopyID(rng_t *rng, void *buffer)```

Copy the RNG ID field to a buffer of length equal to the value returned by ```RNG_GetIDLength(rng_t *rng)```. Returns zero on success, and non-zero on failure or when no ID field exists.

- ```RNG_ResetStack(rng_t *rng)```

Reset the user area of the RNG stack: silently pop the stack until no user data is left.

- ```RNG_Push(rng_t *rng, void *data, uint32_t size)```
- ```RNG_Push<type>(rng_t *rng, <type> x)```

Push generic data or sized-integer values onto the RNG stack. Returns zero on success, and non-zero on failure. In the case of failure, the RNG retains its original unmodified state. If ```data``` is NULL, this has the effect of allocating space on the stack for ```size``` bytes, but without initialising the extra space.

- ```RNG_Pop(rng_t *rng, void *data, uint32_t size)```
- ```RNG_Pop<type>(rng_t *rng, <type> *x)```

Pop generic data or sized-integer values from the RNG stack. Returns zero on success, and non-zero on failure. Failure occurs only when the size of the popped data would exceed the amount of user data left on the stack. If ```data``` or ```x``` is non-NULL, the data popped from the stack will be stored in this address.

- ```RNG_SetRelative(rng_t *rng, uint32_t offset, void *data, uint32_t size)```
- ```RNG_SetRelative<type>(rng_t *rng, uint32_t offset, <type> x)```

Set the user area of the stack relative to an offset from the top of the stack. For sized-integer types of the function, ```offset``` represents a byte-offset equal to ```offset * sizeof(<type>)```, otherwise it represents a byte-offset from the top of the stack. The size in bytes of the user area can be queried with ```RNG_GetStackDepth(rng_t *rng)```. ```data``` must be a valid address of size ```size```. Returns zero on success, and non-zero on failure. In the case of failure, the RNG retains its original unmodified state.

- ```RNG_GetRelative(rng_t *rng, uint32_t offset, void *data, uint32_t size)```
- ```RNG_GetRelativeu<type>(rng_t *rng, uint32_t offset, <type> *x)```

Get data from the user area of stack relative to an offset from the top of the stack. See ```RNG_SetRelative(rng_t *rng, uint32_t offset, void *data, uint32_t size)``` for details. ```data``` or ```x``` must be valid pointers to memory of ```size``` or ```sizeof(<type>)``` bytes. Returns zero on success, and non-zero on failure. In the case of failure, the data in the pointers given to the functions will remain unchanged.

- ```RNG_Random<integer-type>(rng_t *rng)```

Returns a random integer of the specific type. Range is [0, 2^bits - 1] for unsigned types, and [-2^(bits - 1), 2^(bits - 1) - 1] for signed types.

- ```RNG_Random<float-type>(rng_t *rng)```

Returns a random float in the half-open range [0.0, 1.0).

Usage example
=============

```
	rng_t rng = RNG_New();
	int retval = RNG_IsValid(&rng);
	float value;
	if (retval == 0)
		return -1.0f;
	retval = RNG_SetIDString(&rng, "TestRNGID");
	if (retval)
		return -1.0f;
	retval = RNG_SetUserMaxStackSize(&rng, 64);
	if (retval)
		return -1.0f;

	RNG_Pushu64(&rng, 12345); // ignore return value
	RNG_Pushu64(&rng, 123456); // ignore return value
	RNG_Pushu64(&rng, 1234567); // ignore return value
	value = RNG_Randomf32(&rng);
	RNG_Popu64(&rng, 0);
	value += RNG_Randomf32(&rng);
	RNG_SetRelativeu64(&rng, 1, 12345678);
	value += RNG_Randomf32(&rng);
	RNG_Popu64(&rng, 0);
	RNG_Popu64(&rng, 0);
		
	RNG_Destroy(&rng);

```

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
