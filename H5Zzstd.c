#include <stdlib.h>
#include "hdf5.h"
#include "zstd.h"

static size_t H5Z_filter_zstd(unsigned int flags, size_t cd_nelmts,
                              const unsigned int cd_values[], size_t nbytes,
                              size_t *buf_size, void **buf);

#define H5Z_FILTER_ZSTD 32015

const H5Z_class_t H5Z_ZSTD[1] = {{
    H5Z_CLASS_T_VERS,                   /* H5Z_class_t version                */
    (H5Z_filter_t)(H5Z_FILTER_ZSTD),    /* Filter id number                   */
    1,                                  /* encoder_present flag (set to true) */
    1,                                  /* decoder_present flag (set to true) */
    "Zstandard compression: http://www.zstd.net",
                                        /* Filter name for debugging          */
    NULL,                               /* The "can apply" callback           */
    NULL,                               /* The "set local" callback           */
    (H5Z_func_t)(H5Z_filter_zstd)       /* The actual filter function         */
}};

H5PL_type_t H5PLget_plugin_type(void) { return H5PL_TYPE_FILTER; }
const void* H5PLget_plugin_info(void) { return H5Z_ZSTD; }

static size_t H5Z_filter_zstd(unsigned int flags, size_t cd_nelmts,
                              const unsigned int cd_values[], size_t nbytes,
                              size_t *buf_size, void **buf)
{
    void *outbuf = NULL;    /* Pointer to new output buffer */
    void *inbuf = NULL;     /* Pointer to input buffer */
    inbuf = *buf;

	size_t ret_value;
	size_t origSize = nbytes;     /* Number of bytes for output (compressed) buffer */

	if (flags & H5Z_FLAG_REVERSE)
	{
		size_t decompSize = ZSTD_getDecompressedSize(*buf, origSize);
		if (NULL == (outbuf = malloc(decompSize)))
			goto error;

		decompSize = ZSTD_decompress(outbuf, decompSize, inbuf, origSize);

		free(*buf);
		*buf = outbuf;
		outbuf = NULL;
		ret_value = (size_t)decompSize;
	}
	else
	{
		int aggression = 0;
		if (cd_nelmts > 0)
			aggression = (int)cd_values[0];
		if (aggression > 22)
			aggression = 22;

		size_t compSize = ZSTD_compressBound(origSize);
		if (NULL == (outbuf = malloc(compSize)))
			goto error;

		compSize = ZSTD_compress(outbuf, compSize, inbuf, origSize, aggression);

		free(*buf);
		*buf = outbuf;
		*buf_size = compSize;
		outbuf = NULL;
		ret_value = compSize;
	}
	if (outbuf != NULL)
		free(outbuf);
	return ret_value;

error:
	return 0;
}
