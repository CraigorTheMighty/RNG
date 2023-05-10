#include <windows.h>
#include <stdint.h>
#include <memory.h>
#include <math.h>

#include "..\inc\xxh3.h"
#include "..\inc\rng.h"

#define INLINE_DEF __forceinline

#define SPINLOCK_INIT	0

#define RNG_HASH_BITS_LOG2					6
#define RNG_HASH_BITS						(1 << (RNG_HASH_BITS_LOG2))

#define HASH_FUNCTION64(data, len, seed)	XXH128(data, len, seed).low64

#define MALLOC_FUNC			malloc
#define FREE_FUNC			free
#define REALLOC_FUNC		realloc

#define FP32_MANTISSA_BITS	23
#define FP32_EXPONENT_BITS	8
#define FP32_EXPONENT_MASK	0x7F800000
#define FP32_MANTISSA_MASK	0x007FFFFF

#define FP64_MANTISSA_MASK	0x000FFFFFFFFFFFFFULL
#define FP64_MANTISSA_BITS	52
#define FP64_EXPONENT_BITS	11

#define RNG_DEFAULT_MAX_STATE_SIZE		(1 << 16)

#define RNG_EXPAND_BASE	1
#define RNG_EXPAND_ID	2
#define RNG_EXPAND_USER	3

typedef volatile uint32_t spinlock_t;

static spinlock_t g_rng_lock = SPINLOCK_INIT;
static uint64_t g_rng_counter256[4] = {0};	// 256 bits

#define USE_BUILTIN_FUNCTIONS

#ifdef USE_BUILTIN_FUNCTIONS

#define LZCNT8(x)	return (int)__lzcnt16((uint16_t)x);
#define LZCNT16(x)	return (int)__lzcnt16(x);
#define LZCNT32(x)	return (int)__lzcnt(x);
#define LZCNT64(x)	return (int)__lzcnt64(x);

#define POPCNT8(x)	return (int)__popcnt16((uint16_t)x);
#define POPCNT16(x)	return (int)__popcnt16(x);
#define POPCNT32(x)	return (int)__popcnt(x);
#define POPCNT64(x)	return (int)__popcnt64(x);

#define ROTL8(x, shift)		(x << shift) | (x >> (8 - shift))
#define ROTL16(x, shift)	(x << shift) | (x >> (16 - shift))
#define ROTL32(x, shift)	_rotl(x, shift)
#define ROTL64(x, shift)	_rotl64(x, shift)

#else

#define POPCNT8(x)	x = (x & 0x55) + ((x & 0xAA) >> 1);\
					x = (x & 0x33) + ((x & 0xCC) >> 2);\
					x = (x & 0x0F) + ((x & 0xF0) >> 4);\
					return (int)x;

#define POPCNT16(x)	x = (x & 0x5555) + ((x & 0xAAAA) >> 1);\
					x = (x & 0x3333) + ((x & 0xCCCC) >> 2);\
					x = (x & 0x0F0F) + ((x & 0xF0F0) >> 4);\
					x = (x & 0x00FF) + ((x & 0xFF00) >> 8);\
					return (int)x;

#define POPCNT32(x)	x = (x & 0x55555555) + ((x & 0xAAAAAAAA) >> 1);\
					x = (x & 0x33333333) + ((x & 0xCCCCCCCC) >> 2);\
					x = (x & 0x0F0F0F0F) + ((x & 0xF0F0F0F0) >> 4);\
					x = (x & 0x00FF00FF) + ((x & 0xFF00FF00) >> 8);\
					x = (x & 0x0000FFFF) + ((x & 0xFFFF0000) >> 16);\
					return (int)x;

#define POPCNT64(x)	x = (x & 0x5555555555555555ULL) + ((x & 0xAAAAAAAAAAAAAAAAULL) >> 1);\
					x = (x & 0x3333333333333333ULL) + ((x & 0xCCCCCCCCCCCCCCCCULL) >> 2);\
					x = (x & 0x0F0F0F0F0F0F0F0FULL) + ((x & 0xF0F0F0F0F0F0F0F0ULL) >> 4);\
					x = (x & 0x00FF00FF00FF00FFULL) + ((x & 0xFF00FF00FF00FF00ULL) >> 8);\
					x = (x & 0x0000FFFF0000FFFFULL) + ((x & 0xFFFF0000FFFF0000ULL) >> 16);\
					x = (x & 0x00000000FFFFFFFFULL) + ((x & 0xFFFFFFFF00000000ULL) >> 32);\
					return (int)x;

#define LZCNT8(x)	x = x | (x >> 1);\
				    x = x | (x >> 2);\
					x = x | (x >> 4);\
					return 8 - Math_PopCnt8(x);

#define LZCNT16(x)	x = x | (x >> 1);\
				    x = x | (x >> 2);\
					x = x | (x >> 4);\
					x = x | (x >> 8);\
					return 16 - Math_PopCnt16(x);

#define LZCNT32(x)	x = x | (x >> 1);\
				    x = x | (x >> 2);\
					x = x | (x >> 4);\
					x = x | (x >> 8);\
					x = x | (x >> 16);\
					return 32 - Math_PopCnt32(x);

#define LZCNT64(x)	x = x | (x >> 1);\
				    x = x | (x >> 2);\
					x = x | (x >> 4);\
					x = x | (x >> 8);\
					x = x | (x >> 16);\
					x = x | (x >> 32);\
					return 64 - Math_PopCnt64(x);

#define ROTL8(x, shift)		(x << shift) | (x >> (8 - shift))
#define ROTL16(x, shift)	(x << shift) | (x >> (16 - shift))
#define ROTL32(x, shift)	(x << shift) | (x >> (32 - shift))
#define ROTL64(x, shift)	(x << shift) | (x >> (64 - shift))

#endif

static INLINE_DEF uint32_t Math_CeilPow2u32(uint32_t x)
{
	x--;
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	x++;

	return x;
}

static INLINE_DEF int Math_LZCnt64(uint64_t x)
{
	LZCNT64(x);
}

static INLINE_DEF void SpinLock_Lock(spinlock_t *lock)
{
	while (InterlockedExchange(lock, 1) == 1)
	{
	}
}
static INLINE_DEF void SpinLock_Unlock(spinlock_t *lock)
{
	InterlockedExchange(lock, 0);
}

static INLINE_DEF void RNG_IncrementCounter256(uint64_t *counter256)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		counter256[i]++;
		if (counter256[i])
			break;
	}
}

static INLINE_DEF int RNG_ExpandStateBuffer(rng_t *rng, uint32_t size, int type)
{
	uint32_t old_size = rng->state_size_allocated_bytes;
	uint32_t new_size = size;
	void *ptr;

	if (type == RNG_EXPAND_USER)
	{
		uint32_t req_user_size = rng->state_size - rng->id_length - sizeof(uint64_t) * 4;
		if (req_user_size + size < req_user_size)	// integer overflow
			return -1;
		req_user_size += size;
		if (req_user_size > rng->user_state_required_size)
			return -1;
		new_size = req_user_size + rng->id_length + sizeof(uint64_t) * 4;
	}

	if (old_size >= new_size)
		return 0;

	if (old_size == 0)
		old_size = 1;

	while ((old_size < new_size) && (old_size != 0) && (old_size < rng->max_state_size))
	{
		old_size <<= 1;
	}
	if ((old_size == 0) || (old_size > rng->max_state_size) || (old_size < new_size))
		return -1;

	rng->state_size_allocated_bytes = old_size;
	ptr = REALLOC_FUNC(rng->state, rng->state_size_allocated_bytes);

	if (!ptr)
		return -1;

	rng->state = ptr;

	return 0;
}

void RNG_Destroy(rng_t *rng)
{
	if (!rng)
		return;
	FREE_FUNC(rng->state);
	memset(rng, 0, sizeof(rng_t));
}

int RNG_IsValid(rng_t *rng)
{
	return (rng->state != 0 && rng->state_size >= sizeof(uint64_t)*4) ? 1 : 0;
}

int RNG_SetTotalMaxStackSize(rng_t *rng, uint32_t size)
{
	size = Math_CeilPow2u32(size);

	if (size == 0)
		return -1;
	if (size < rng->state_size_allocated_bytes)
		return -1;

	rng->max_state_size = size;

	return 0;
}
int RNG_SetUserMaxStackSize(rng_t *rng, uint32_t size)
{
	uint32_t current_offset = rng->state_size - rng->id_length - sizeof(uint64_t) * 4;
	
	size = Math_CeilPow2u32(size);

	if (size == 0)
		return -1;
	if (size < current_offset)
		return -1;

	rng->user_state_required_size = size;

	return 0;
}
int RNG_ShrinkStack(rng_t *rng)
{
	uint32_t old_size = rng->state_size_allocated_bytes;
	uint32_t target_size = Math_CeilPow2u32(rng->state_size);
	void *ptr;

	if (target_size == rng->state_size_allocated_bytes)
		return 0;

	while ((old_size >> 1) >= target_size)
		old_size >>= 1;

	if (old_size == rng->state_size_allocated_bytes)
		return 0;

	ptr = REALLOC_FUNC(rng->state, old_size);

	if (!ptr)
		return -1;

	rng->state = ptr;
	rng->state_size_allocated_bytes = old_size;

	return 0;
}
uint32_t RNG_GetTotalMaxStackSize(rng_t *rng)
{
	return rng->max_state_size;
}
uint32_t RNG_GetUserMaxStackSize(rng_t *rng)
{
	uint32_t overhead = rng->id_length + sizeof(uint64_t) * 4;

	if (rng->max_state_size - overhead < rng->user_state_required_size)
		return rng->max_state_size - overhead;
	else
		return rng->user_state_required_size;
}
rng_t RNG_New()
{
	rng_t rng = {0};

	rng.state_size = (uint32_t)sizeof(uint64_t) * 4;
	rng.max_state_size = RNG_DEFAULT_MAX_STATE_SIZE;
	rng.user_state_required_size = RNG_DEFAULT_MAX_STATE_SIZE;

	if (RNG_ExpandStateBuffer(&rng, (uint32_t)sizeof(uint64_t) * 4, RNG_EXPAND_BASE))
	{
		FREE_FUNC(rng.state);
		memset(&rng, 0, (uint32_t)sizeof(rng_t));
		return rng; // non-valid RNG
	}

	SpinLock_Lock(&g_rng_lock);
	memcpy(rng.state, g_rng_counter256, (uint32_t)sizeof(uint64_t) * 4); // C6387, false positive
	RNG_IncrementCounter256(g_rng_counter256);
	SpinLock_Unlock(&g_rng_lock);

	return rng;
}
rng_t RNG_Clone(rng_t *old_rng)
{
	rng_t rng = *old_rng;

	rng.state = 0;
	rng.state_size_allocated_bytes = 0;

	if (RNG_ExpandStateBuffer(&rng, old_rng->state_size_allocated_bytes, RNG_EXPAND_BASE))
	{
		FREE_FUNC(rng.state);
		memset(&rng, 0, (uint32_t)sizeof(rng_t));
		return rng; // non-valid RNG
	}

	memcpy(rng.state, old_rng->state, rng.state_size_allocated_bytes);

	return rng;
}

int RNG_SetID(rng_t *rng, void *data, uint32_t data_len)
{
	uint32_t new_id_length = data_len;
	uint32_t old_userdata_size = rng->state_size - rng->id_length - (uint32_t)sizeof(uint64_t) * 4;
	uint32_t req_size = old_userdata_size + new_id_length + (uint32_t)sizeof(uint64_t) * 4;
	uint8_t *old_userdata_p;
	uint8_t *new_userdata_p;

	if (RNG_ExpandStateBuffer(rng, req_size, RNG_EXPAND_ID))
		return -1; // valid RNG but without ID updated

	old_userdata_p = &rng->state[(uint32_t)sizeof(uint64_t) * 4 + rng->id_length];
	new_userdata_p = &rng->state[(uint32_t)sizeof(uint64_t) * 4 + new_id_length];

	memmove(new_userdata_p, old_userdata_p, old_userdata_size);
	memcpy(&rng->state[(uint32_t)sizeof(uint64_t) * 4], data, new_id_length);

	rng->state_size = rng->state_size + new_id_length - rng->id_length;
	rng->id_length = new_id_length;
	rng->id_type = RNG_ID_TYPE_GENERIC;

	return 0;
}
int RNG_SetIDString(rng_t *rng, char *string)
{
	int ret = RNG_SetID(rng, string, (uint32_t)strlen(string));
	if (!ret)
		rng->id_type = RNG_ID_TYPE_STRING;
	return ret;
}
int RNG_SetIDu64(rng_t *rng, uint64_t id)
{
	int ret = RNG_SetID(rng, &id, (uint32_t)sizeof(uint64_t));
	if (!ret)
		rng->id_type = RNG_ID_TYPE_U64;
	return ret;
}
int RNG_SetIDStringHash(rng_t *rng, char *string)
{
	uint64_t val = HASH_FUNCTION64(string, strlen(string), 0);
	int ret = RNG_SetID(rng, &val, (uint32_t)sizeof(uint64_t));
	if (!ret)
		rng->id_type = RNG_ID_TYPE_HASH;
	return ret;
}

uint32_t RNG_GetIDLength(rng_t *rng)
{
	return rng->id_length + (rng->id_type == RNG_ID_TYPE_STRING ? 1 : 0);
}
int RNG_GetIDType(rng_t *rng)
{
	return rng->id_type;
}
int RNG_CopyID(rng_t *rng, void *buffer)
{
	if (rng->id_length)
	{
		memcpy(buffer, &rng->state[(uint32_t)sizeof(uint64_t) * 4], rng->id_length);
		if (rng->id_type == RNG_ID_TYPE_STRING)
			((uint8_t*)buffer)[rng->id_length] = '\0';
		return 0;
	}
	else
		return -1;
}

int RNG_SetRelative(rng_t *rng, uint32_t offset, void *data, uint32_t size)
{
	uint32_t user_size = rng->state_size - rng->id_length - sizeof(uint64_t) * 4;

	if (offset > user_size)
		return -1;
	if (offset < size)
		return -1;

	memcpy(&rng->state[(uint32_t)sizeof(uint64_t) * 4 + rng->id_length + (user_size - offset)], data, size);

	return 0;
}

uint32_t RNG_GetTotalStackDepth(rng_t *rng)
{
	return rng->state_size;
}
uint32_t RNG_GetUserStackDepth(rng_t *rng)
{
	return rng->state_size - rng->id_length - sizeof(uint64_t) * 4;
}

int RNG_SetRelativeu64(rng_t *rng, uint32_t offset, uint64_t x)
{
	return RNG_SetRelative(rng, offset * sizeof(uint64_t), &x, sizeof(uint64_t));
}
int RNG_SetRelativeu32(rng_t *rng, uint32_t offset, uint32_t x)
{
	return RNG_SetRelative(rng, offset * sizeof(uint32_t), &x, sizeof(uint32_t));
}
int RNG_SetRelativeu16(rng_t *rng, uint32_t offset, uint16_t x)
{
	return RNG_SetRelative(rng, offset * sizeof(uint16_t), &x, sizeof(uint16_t));
}
int RNG_SetRelativeu8(rng_t *rng, uint32_t offset, uint8_t x)
{
	return RNG_SetRelative(rng, offset * sizeof(uint8_t), &x, sizeof(uint8_t));
}
int RNG_SetRelativei64(rng_t *rng, int32_t offset, int64_t x)
{
	return RNG_SetRelative(rng, offset * sizeof(int64_t), &x, sizeof(int64_t));
}
int RNG_SetRelativei32(rng_t *rng, int32_t offset, int32_t x)
{
	return RNG_SetRelative(rng, offset * sizeof(int32_t), &x, sizeof(int32_t));
}
int RNG_SetRelativei16(rng_t *rng, int32_t offset, int16_t x)
{
	return RNG_SetRelative(rng, offset * sizeof(int16_t), &x, sizeof(int16_t));
}
int RNG_SetRelativei8(rng_t *rng, int32_t offset, int8_t x)
{
	return RNG_SetRelative(rng, offset * sizeof(int8_t), &x, sizeof(int8_t));
}
int RNG_GetRelative(rng_t *rng, uint32_t offset, void *data, uint32_t size)
{
	uint32_t user_size = rng->state_size - rng->id_length - sizeof(uint64_t) * 4;

	if (offset > user_size)
		return -1;
	if (offset < size)
		return -1;

	memcpy(data, &rng->state[(uint32_t)sizeof(uint64_t) * 4 + rng->id_length + (user_size - offset)], size);

	return 0;
}

int RNG_GetRelativeu64(rng_t *rng, uint32_t offset, uint64_t *x)
{
	return RNG_GetRelative(rng, offset * sizeof(uint64_t), x, sizeof(uint64_t));
}
int RNG_GetRelativeu32(rng_t *rng, uint32_t offset, uint32_t *x)
{
	return RNG_GetRelative(rng, offset * sizeof(uint32_t), x, sizeof(uint32_t));
}
int RNG_GetRelativeu16(rng_t *rng, uint32_t offset, uint16_t *x)
{
	return RNG_GetRelative(rng, offset * sizeof(uint16_t), x, sizeof(uint16_t));
}
int RNG_GetRelativeu8(rng_t *rng, uint32_t offset, uint8_t *x)
{
	return RNG_GetRelative(rng, offset * sizeof(uint8_t), x, sizeof(uint8_t));
}
int RNG_GetRelativei64(rng_t *rng, int32_t offset, int64_t *x)
{
	return RNG_GetRelative(rng, offset * sizeof(int64_t), x, sizeof(int64_t));
}
int RNG_GetRelativei32(rng_t *rng, int32_t offset, int32_t *x)
{
	return RNG_GetRelative(rng, offset * sizeof(int32_t), x, sizeof(int32_t));
}
int RNG_GetRelativei16(rng_t *rng, int32_t offset, int16_t *x)
{
	return RNG_GetRelative(rng, offset * sizeof(int16_t), x, sizeof(int16_t));
}
int RNG_GetRelativei8(rng_t *rng, int32_t offset, int8_t *x)
{
	return RNG_GetRelative(rng, offset * sizeof(int8_t), x, sizeof(int8_t));
}
int RNG_Push(rng_t *rng, void *data, uint32_t size)
{
	if (RNG_ExpandStateBuffer(rng, size, RNG_EXPAND_USER))
		return -1; // valid RNG but without value pushed

	if (data)
		memcpy(&rng->state[rng->state_size], data, size);

	rng->state_size += size;

	return 0;
}

int RNG_Pushu64(rng_t *rng, uint64_t x)
{
	return RNG_Push(rng, &x, (uint32_t)sizeof(uint64_t));
}
int RNG_Pushu32(rng_t *rng, uint32_t x)
{
	return RNG_Push(rng, &x, (uint32_t)sizeof(uint32_t));
}
int RNG_Pushu16(rng_t *rng, uint16_t x)
{
	return RNG_Push(rng, &x, (uint32_t)sizeof(uint16_t));
}
int RNG_Pushu8(rng_t *rng, uint8_t x)
{
	return RNG_Push(rng, &x, (uint32_t)sizeof(uint8_t));
}
int RNG_Pushi64(rng_t *rng, int64_t x)
{
	return RNG_Push(rng, &x, (uint32_t)sizeof(int64_t));
}
int RNG_Pushi32(rng_t *rng, int32_t x)
{
	return RNG_Push(rng, &x, (uint32_t)sizeof(int32_t));
}
int RNG_Pushi16(rng_t *rng, int16_t x)
{
	return RNG_Push(rng, &x, (uint32_t)sizeof(int16_t));
}
int RNG_Pushi8(rng_t *rng, int8_t x)
{
	return RNG_Push(rng, &x, (uint32_t)sizeof(int8_t));
}

int RNG_Pop(rng_t *rng, void *data, uint32_t size)
{
	if (rng->state_size < size + (uint32_t)sizeof(uint64_t)*4 + rng->id_length)
		return -1;
	
	if (data)
		memcpy(data, &rng->state[rng->state_size - size], size);
	
	rng->state_size -= size;

	return 0;
}
int RNG_Popu64(rng_t *rng, uint64_t *x)
{
	return RNG_Pop(rng, x, (uint32_t)sizeof(uint64_t));
}
int RNG_Popu32(rng_t *rng, uint32_t *x)
{
	return RNG_Pop(rng, x, (uint32_t)sizeof(uint32_t));
}
int RNG_Popu16(rng_t *rng, uint16_t *x)
{
	return RNG_Pop(rng, x, (uint32_t)sizeof(uint16_t));
}
int RNG_Popu8(rng_t *rng, uint8_t *x)
{
	return RNG_Pop(rng, x, (uint32_t)sizeof(uint8_t));
}
int RNG_Popi64(rng_t *rng, int64_t *x)
{
	return RNG_Pop(rng, x, (uint32_t)sizeof(int64_t));
}
int RNG_Popi32(rng_t *rng, int32_t *x)
{
	return RNG_Pop(rng, x, (uint32_t)sizeof(int32_t));
}
int RNG_Popi16(rng_t *rng, int16_t *x)
{
	return RNG_Pop(rng, x, (uint32_t)sizeof(int16_t));
}
int RNG_Popi8(rng_t *rng, int8_t *x)
{
	return RNG_Pop(rng, x, (uint32_t)sizeof(int8_t));
}
void RNG_ResetStack(rng_t *rng)
{
	rng->state_size = (uint32_t)sizeof(uint64_t)*4 + rng->id_length;
}

uint64_t RNG_Randomu64(rng_t *rng)
{
	return (uint64_t)HASH_FUNCTION64(rng->state, rng->state_size, 0);
}
uint32_t RNG_Randomu32(rng_t *rng)
{
	return (uint32_t)HASH_FUNCTION64(rng->state, rng->state_size, 0);
}
uint16_t RNG_Randomu16(rng_t *rng)
{
	return (uint16_t)HASH_FUNCTION64(rng->state, rng->state_size, 0);
}
uint8_t RNG_Randomu8(rng_t *rng)
{
	return (uint8_t)HASH_FUNCTION64(rng->state, rng->state_size, 0);
}

int64_t RNG_Randomi64(rng_t *rng)
{
	return (int64_t)HASH_FUNCTION64(rng->state, rng->state_size, 0);
}
int32_t RNG_Randomi32(rng_t *rng)
{
	return (int32_t)HASH_FUNCTION64(rng->state, rng->state_size, 0);
}
int16_t RNG_Randomi16(rng_t *rng)
{
	return (int16_t)HASH_FUNCTION64(rng->state, rng->state_size, 0);
}
int8_t RNG_Randomi8(rng_t *rng)
{
	return (int8_t)HASH_FUNCTION64(rng->state, rng->state_size, 0);
}

float RNG_Randomf32(rng_t *rng)
{
	uint64_t current;
	uint32_t cnt;
	uint32_t pw2;    
	uint32_t m;

	current = HASH_FUNCTION64(rng->state, rng->state_size, 0);
	cnt = (uint32_t)Math_LZCnt64(current);
	pw2 = cnt;

	if (cnt == RNG_HASH_BITS)
	{
		current = HASH_FUNCTION64(rng->state, rng->state_size, 1);
		cnt = (uint32_t)Math_LZCnt64(current);
		pw2 += cnt;
	}

	// if less than "FP32_MANTISSA_BITS" bits left, we need to generate a new hash to fill the mantissa
	if (RNG_HASH_BITS - cnt - 1 < FP32_MANTISSA_BITS)
	{
		HASH_FUNCTION64(rng->state, rng->state_size, 2);
	}

	if (pw2 < (uint32_t)((1 << (FP32_EXPONENT_BITS - 1)) - 2))
		m = ((1 << (FP32_EXPONENT_BITS - 1)) - 2 - pw2) << FP32_MANTISSA_BITS;
	else // subnormal
		m = 0;

	m |= FP32_MANTISSA_MASK & current;

	return *((float*)&m);
}

double RNG_Randomf64(rng_t *rng)
{
	uint64_t current;
	int32_t cnt;
	uint32_t pw2;    
	// ceil((2^(ebits - 1) - 2) / 2^hashbits)
	const uint32_t maxrec = ((uint32_t)(1 << (FP64_EXPONENT_BITS - 1)) - 2 + RNG_HASH_BITS - 1) >> RNG_HASH_BITS_LOG2;
	uint64_t m;
	uint32_t i;

	current = HASH_FUNCTION64(rng->state, rng->state_size, 0);
	cnt = (uint32_t)Math_LZCnt64(current);
	pw2 = cnt;

	for (i = 1; (i < maxrec) && (cnt == RNG_HASH_BITS); i++)
	{
		current = HASH_FUNCTION64(rng->state, rng->state_size, i);
		cnt = (uint32_t)Math_LZCnt64(current);
		pw2 += cnt;
	}

	// if less than "FP64_MANTISSA_BITS" bits left, we need to generate a new hash to fill the mantissa
	if (RNG_HASH_BITS - cnt - 1 < FP64_MANTISSA_BITS)
	{
		current = HASH_FUNCTION64(rng->state, rng->state_size, i);
	}

	if (pw2 < (uint32_t)((1 << (FP64_EXPONENT_BITS - 1)) - 2))
		m = ((((uint64_t)1) << (FP64_EXPONENT_BITS - 1)) - 2 - pw2) << FP64_MANTISSA_BITS;
	else // subnormal
		m = 0;

	m |= FP64_MANTISSA_MASK & current;

	return *((double*)&m);
}
