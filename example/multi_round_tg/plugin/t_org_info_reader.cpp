#include "../enclave/user_type.h"
#include "toolkit/plugins/csv/csv_reader.h"

typedef ypc::plugins::typed_csv_reader<t_org_info_item_t> t_org_info_reader_t;
impl_csv_reader(t_org_info_reader_t)
