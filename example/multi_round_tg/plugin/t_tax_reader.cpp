#include "../enclave/user_type.h"
#include "toolkit/plugins/csv/csv_reader.h"

typedef ypc::plugins::typed_csv_reader<t_tax_item_t> t_tax_reader_t;
impl_csv_reader(t_tax_reader_t)
