#include "output/user_type.h"
#include<toolkit/plugins/csv/csv_reader.h>
typedef ypc::plugins::typed_csv_reader<iris_item_t> iris_reader_t;
impl_csv_reader(iris_reader_t)
