#define RNG_ID_TYPE_STRING	1
#define RNG_ID_TYPE_U64		2
#define RNG_ID_TYPE_HASH	3
#define RNG_ID_TYPE_GENERIC	4

typedef struct rng_s
{
	uint8_t		*state;
	uint32_t	state_size;					// sizeof(uint64_t) * 4 + id_length + USER_DATA = state_size
	uint32_t	state_size_allocated_bytes;	// how many bytes have been allocated for *state
	uint32_t	max_state_size;				// how many bytes are allowed for *state
	uint32_t	id_length;
	uint32_t	id_type;
}rng_t;

rng_t RNG_New();
rng_t RNG_Clone(rng_t *old_rng);
void RNG_Destroy(rng_t *rng);
int RNG_IsValid(rng_t *rng);

int RNG_SetMaxStackSize(rng_t *rng, uint32_t size);

uint32_t RNG_GetStackDepth(rng_t *rng);

uint32_t RNG_GetIDLength(rng_t *rng);

int RNG_SetID(rng_t *rng, void *data, uint32_t data_len);
int RNG_SetIDString(rng_t *rng, char *string);
int RNG_SetIDu64(rng_t *rng, uint64_t id);
int RNG_SetIDStringHash(rng_t *rng, char *string);

int RNG_GetIDType(rng_t *rng);
int RNG_CopyID(rng_t *rng, void *buffer);

void RNG_ResetStack(rng_t *rng);

int RNG_SetRelative(rng_t *rng, uint32_t offset, void *data, uint32_t size);
int RNG_SetRelativeu64(rng_t *rng, uint32_t offset, uint64_t x);
int RNG_SetRelativeu32(rng_t *rng, uint32_t offset, uint32_t x);
int RNG_SetRelativeu16(rng_t *rng, uint32_t offset, uint16_t x);
int RNG_SetRelativeu8(rng_t *rng, uint32_t offset, uint8_t x);
int RNG_SetRelativei64(rng_t *rng, int32_t offset, int64_t x);
int RNG_SetRelativei32(rng_t *rng, int32_t offset, int32_t x);
int RNG_SetRelativei16(rng_t *rng, int32_t offset, int16_t x);
int RNG_SetRelativei8(rng_t *rng, int32_t offset, int8_t x);

int RNG_GetRelative(rng_t *rng, uint32_t offset, void *data, uint32_t size);
int RNG_GetRelativeu64(rng_t *rng, uint32_t offset, uint64_t *x);
int RNG_GetRelativeu32(rng_t *rng, uint32_t offset, uint32_t *x);
int RNG_GetRelativeu16(rng_t *rng, uint32_t offset, uint16_t *x);
int RNG_GetRelativeu8(rng_t *rng, uint32_t offset, uint8_t *x);
int RNG_GetRelativei64(rng_t *rng, int32_t offset, int64_t *x);
int RNG_GetRelativei32(rng_t *rng, int32_t offset, int32_t *x);
int RNG_GetRelativei16(rng_t *rng, int32_t offset, int16_t *x);
int RNG_GetRelativei8(rng_t *rng, int32_t offset, int8_t *x);

int RNG_Push(rng_t *rng, void *data, uint32_t size);
int RNG_Pushu64(rng_t *rng, uint64_t x);
int RNG_Pushu32(rng_t *rng, uint32_t x);
int RNG_Pushu16(rng_t *rng, uint16_t x);
int RNG_Pushu8(rng_t *rng, uint8_t x);
int RNG_Pushi64(rng_t *rng, int64_t x);
int RNG_Pushi32(rng_t *rng, int32_t x);
int RNG_Pushi16(rng_t *rng, int16_t x);
int RNG_Pushi8(rng_t *rng, int8_t x);

int RNG_Pop(rng_t *rng, void *data, uint32_t size);
int RNG_Popu64(rng_t *rng, uint64_t *x);
int RNG_Popu32(rng_t *rng, uint32_t *x);
int RNG_Popu16(rng_t *rng, uint16_t *x);
int RNG_Popu8(rng_t *rng, uint8_t *x);
int RNG_Popi64(rng_t *rng, int64_t *x);
int RNG_Popi32(rng_t *rng, int32_t *x);
int RNG_Popi16(rng_t *rng, int16_t *x);
int RNG_Popi8(rng_t *rng, int8_t *x);

uint64_t RNG_Randomu64(rng_t *rng);
uint32_t RNG_Randomu32(rng_t *rng);
uint16_t RNG_Randomu16(rng_t *rng);
uint8_t RNG_Randomu8(rng_t *rng);
int64_t RNG_Randomi64(rng_t *rng);
int32_t RNG_Randomi32(rng_t *rng);
int16_t RNG_Randomi16(rng_t *rng);
int8_t RNG_Randomi8(rng_t *rng);

float RNG_Randomf32(rng_t *rng);
double RNG_Randomf64(rng_t *rng);
