#include "../enclave/train_type.h"
#include "toolkit/plugins/csv/csv_reader.h"
typedef ypc::plugins::typed_csv_reader<train_b_item_t> train_b_reader_t;
impl_csv_reader(train_b_reader_t)
