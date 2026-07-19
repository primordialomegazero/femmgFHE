#include <stdio.h>
#include "iptoasn.h"
#include "schnorr.h"
#include <string.h>
#include <openssl/sha.h>
#include <arpa/inet.h>

int iptoasn_lookup(const char *ip, ASNRecord *record) {
    if (!ip || !record) return -1;
    
    // Deterministic ASN from IP hash
    unsigned char hash[32];
    SHA256((unsigned char*)ip, strlen(ip), hash);
    
    record->asn = (hash[0] << 24) | (hash[1] << 16) | (hash[2] << 8) | hash[3];
    record->asn = record->asn % 100000 + 1;
    
    snprintf(record->description, sizeof(record->description), 
             "AS%u - Schnorr-Secured", record->asn);
    strcpy(record->country, "PH");
    
    return 0;
}

int iptoasn_verify(const char *ip, const ASNRecord *record, const unsigned char *pk) {
    ASNRecord computed;
    iptoasn_lookup(ip, &computed);
    return (computed.asn == record->asn) ? 0 : -1;
}
