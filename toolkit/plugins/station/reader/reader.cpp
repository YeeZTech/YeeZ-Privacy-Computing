#include "output/user_type.h"
#include<toolkit/plugins/csv/csv_reader.h>
typedef ypc::plugins::typed_csv_reader<station_item_t> station_reader_t;
impl_csv_reader(station_reader_t)