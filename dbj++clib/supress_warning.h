// https://wandbox.org/permlink/5wMC29SXQPOrwp6w
// suppress_warning.h
// This inclusion file has no header guard because it may be multiply included.

#if __GNUC__
#   pragma GCC diagnostic push
#   ifdef __has_warning
#       define HAS_WARNING( X ) __has_warning( X )
#       if HAS_WARNING( SUPPRESS_WARNING )

			 // Use the _Pragma operator to access the macro flag from the directive.
#            define LITERAL_PRAGMA( X ) _Pragma( # X )
#            define EXPAND_PRAGMA( X ) LITERAL_PRAGMA( X )
EXPAND_PRAGMA(GCC diagnostic ignored SUPPRESS_WARNING)
#       endif
#   endif
#endif

#undef SUPPRESS_WARNING
