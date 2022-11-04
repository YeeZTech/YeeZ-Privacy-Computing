#include "output/user_type.h"
#include<toolkit/plugins/csv/csv_reader.h>
typedef ypc::plugins::typed_csv_reader<stats_item_t> stats_reader_t;
impl_csv_reader(stats_reader_t)