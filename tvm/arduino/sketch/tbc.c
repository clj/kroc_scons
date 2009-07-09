#include "tvm-arduino.h"

/*{{{  TEncode Structures */
typedef struct _avr_tenc_element_t {
	char id[5];
	UWORD length;
	union {
		WORD s_int;
		UWORD u_int;
		const prog_char *str;
		const prog_char *bytes;
	} data;
	const prog_char *next;
} avr_tenc_element_t;
/*}}}*/
/*{{{  TEncode functions based on those in tencode.c */
static UWORD avr_tenc_decode_int (const prog_char *src) {
	return SwapTwoBytes (pgm_read_word ((const prog_int16_t *) src));
}

static int avr_tenc_decode_element (const prog_char *src, UWORD *length, avr_tenc_element_t *element)
{
	if (*length < (sizeof (WORD) + 4)) {
		return -1;
	}

	memcpy_P (element->id, src, 4);
	element->id[4] = '\0';
	element->length = avr_tenc_decode_int (src + 4);
	element->next = src + (sizeof (WORD) + 4);
	*length -= (sizeof (WORD) + 4);

	if (element->id[3] == 'I' || element->id[3] == 'U') {
		element->data.u_int = element->length;
		element->length = 0;
		return 0;
	} else if (*length < element->length) {
		return -1;
	} else {
		const int align_mask = (sizeof (WORD) - 1);
		int bytes = element->length;

		if (bytes & align_mask) {
			bytes += (sizeof (WORD)) - (bytes & align_mask);
		}

		element->data.bytes = element->next;
		element->next += bytes;
		*length -= bytes;

		return 0;
	}
}

static int ids_match (const char *a, const char *b)
{
	if (a[0] != b[0]) return 0;
	if (a[1] != b[1]) return 0;
	if (a[2] != b[2]) return 0;
	if (a[3] != b[3]) return 0;
	return 1;
}

static int avr_tenc_walk_to_element (const prog_char *data, UWORD *length, const char *id, avr_tenc_element_t *element)
{
	while (*length > 0) {
		int ret = avr_tenc_decode_element (data, length, element);

		if (ret < 0) {
			return ret;
		}

		if (ids_match (element->id, id)) {
			return 0;
		}
		
		data = element->next;
	}

	return -1;
}

static int load_uint (const prog_char **data, UWORD *length, const char *id, UWORD *dst)
{
	avr_tenc_element_t element;
	int ret;

	if ((ret = avr_tenc_walk_to_element (*data, length, id, &element)) < 0)
		return ret;
	
	*dst = element.data.u_int;
	*data = element.next;

	return 0;
}

static int avr_tbc_decode (const prog_char *data, UWORD length, ECTX context, WORDPTR memory, UWORD memory_size)
{
	UWORD ws_size, vs_size;
	const prog_char *bytecode;

	WORDPTR ws, vs;

	avr_tenc_element_t element;
	int ret;

	/* Decode the required elements */
	if ((ret = load_uint (&data, &length, "ws U", &ws_size)) < 0)
		return ret;
	if ((ret = load_uint (&data, &length, "vs U", &vs_size)) < 0)
		return ret;

	if ((ret = avr_tenc_walk_to_element (data, &length, "bc B", &element)) < 0)
		return ret;
	
	bytecode = element.data.bytes;
	data = element.next;

	/* FIXME: check TLP is empty */
#if 0
	/* Decode optional elements */
	tbc->tlp = NULL;

	while (length > 0) {
		if (tenc_decode_element (data, &length, &element) < 0)
			return 0; /* ignore errors */

		if (ids_match (element.id, "tlpL")) {
			tbc->tlp = decode_tlp (data, tbc->tlp, &element); 
		}

		data = element.next;
	}
#endif

	tvm_ectx_layout (
		context, memory,
		"", 0,
		ws_size, vs_size,
		&ws, &vs
	);
	/* FIXME: check memory hasn't been exhausted */
	ret = tvm_ectx_install_tlp (
		context, tvm_addr_from_progmem ((prog_void *) bytecode),
		ws, vs,
		"", 0, NULL
	);

	return ret;
}
/*}}}*/

// Initialise a Transputer context from a TBC file in program memory.
// Returns 0 on success, -1 on failure.
int init_context_from_tbc (ECTX context, const prog_char *data, WORDPTR memory, UWORD memory_size) {
	avr_tenc_element_t element;
	UWORD length;

	if (memcmp_P ("tenc", data, 4) != 0) {
		return -1;
	}
	length = avr_tenc_decode_int (data + 4);

	// Skip over the first header.
	data += 6;

	if (avr_tenc_walk_to_element (data, &length, "tbcL", &element) < 0) {
		return -1;
	}

	if (avr_tbc_decode (element.data.bytes, element.length, context, memory, memory_size)) {
		return -1;
	}

	return 0;
}
