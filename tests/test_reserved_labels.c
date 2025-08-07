#include <assert.h>
#include "utils.h"

int main(void) {
    assert(is_reserved_word("mov"));
    assert(is_reserved_word("r7"));
    assert(is_reserved_word("data"));
    assert(!is_reserved_word("mylabel"));

    assert(!is_valid_label("mov"));
    assert(!is_valid_label("r7"));
    assert(is_valid_label("my_label"));
    return 0;
}
