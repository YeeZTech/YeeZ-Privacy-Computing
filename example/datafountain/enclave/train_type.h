#pragma once
#include <ff/util/ntobject.h>
#include <string>
define_nt(flag, std::string);
define_nt(id, std::string);
define_nt(A1, std::string);
define_nt(A2, std::string);
define_nt(A3, std::string);
define_nt(A4, std::string);
define_nt(A5, std::string);
define_nt(A6, std::string);
define_nt(B1, std::string);
define_nt(B2, std::string);
define_nt(B3, std::string);
define_nt(B4, std::string);
define_nt(B5, std::string);
define_nt(B6, std::string);
define_nt(B7, std::string);
define_nt(B8, std::string);
define_nt(B9, std::string);
define_nt(B10, std::string);
define_nt(B11, std::string);
define_nt(B12, std::string);
define_nt(B13, std::string);
typedef ff::util::ntobject<flag, id, A1, A2, A3, A4, A5, A6, B1, B2, B3, B4, B5,
                           B6, B7, B8, B9, B10, B11, B12, B13>
    train_a_item_t;

define_nt(C1, std::string);
define_nt(C2, std::string);
define_nt(C3, std::string);
define_nt(C4, std::string);
define_nt(C5, std::string);
define_nt(C6, std::string);
define_nt(C7, std::string);
define_nt(D1, std::string);
define_nt(D2, std::string);
define_nt(D3, std::string);
define_nt(D4, std::string);
define_nt(D5, std::string);
define_nt(D6, std::string);
define_nt(D7, std::string);
define_nt(D8, std::string);
define_nt(D9, std::string);
define_nt(D10, std::string);
define_nt(D11, std::string);
define_nt(D12, std::string);
define_nt(D13, std::string);
typedef ff::util::ntobject<id, C1, C2, C3, C4, C5, C6, C7, D1, D2, D3, D4, D5,
                           D6, D7, D8, D9, D10, D11, D12, D13>
    train_b_item_t;

typedef ff::util::ntobject<id, flag, A1, A2, A3, A4, A5, A6, B1, B2, B3, B4, B5,
                           B6, B7, B8, B9, B10, B11, B12, B13, C1, C2, C3, C4,
                           C5, C6, C7, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10,
                           D11, D12, D13>
    train_merge_item_t;
