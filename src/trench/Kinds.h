#pragma once

#include "config.h'

#define KIND(TYPE, MNEMONIC) 					\
	const TYPE *as##TYPE() const { 				\
		if (mnemonic() == MNEMONIC) { 			\
			return static_cast<const TYPE *>(this); \
		} else { 					\
			return NULL; 				\
		} 						\
	}
#undef AS
