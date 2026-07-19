#ifndef IPTOASN_H
#define IPTOASN_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t asn;
    char description[256];
    char country[4];
} ASNRecord;

int iptoasn_lookup(const char *ip, ASNRecord *record);
int iptoasn_verify(const char *ip, const ASNRecord *record, const unsigned char *pk);

#endif
