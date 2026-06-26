#include <assert.h>
#include <stdio.h>
#include "mom_jeans_voice.h"

static int failures = 0;
#define CHECK(cond) do { if(!(cond)){ printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); failures++; } } while(0)

static void test_sync_gate(void) {
    CHECK(mj_sync_gate(0.0f) == 0);   // below threshold
    CHECK(mj_sync_gate(2.5f) == 0);   // exactly threshold is NOT high ( > 2.5 )
    CHECK(mj_sync_gate(2.6f) == 1);   // above threshold
    CHECK(mj_sync_gate(5.0f) == 1);   // clearly high
}

int main(void) {
    test_sync_gate();
    if (failures) { printf("%d failures\n", failures); return 1; }
    printf("all tests passed\n");
    return 0;
}
