#include "tmpelement.h"
#include <stdlib.h>
TMPElement *TMPElement_new(void) { return (TMPElement*) calloc(1, sizeof(TMPElement)); }
void        TMPElement_free(TMPElement *s) { free(s); }
