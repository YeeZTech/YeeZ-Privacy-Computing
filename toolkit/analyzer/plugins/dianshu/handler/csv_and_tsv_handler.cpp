#include "csv.hpp"
#include "ocall_def.h"
#include "ypc/core/byte.h"

#include <glog/logging.h>
#include <opencv2/opencv.hpp>

define_nt(row_num, int);                          // 总行数
define_nt(col_num, int);                          // 总列数
define_nt(total_nulls, long long);                // 总缺失值数量
define_nt(null_proportion, std::string);          // 缺失值比例
define_nt(heat_map, std::vector<unsigned char>);  // 热力图
define_nt(missNullNumRowCount, std::vector<int>); // 行缺失值数量
define_nt(missNumColCount, std::vector<int>);     // 列缺失值数量
define_nt(csv_file_path, std::string);            // CSV文件路径
typedef ::ff::util::ntobject<row_num, col_num, total_nulls, null_proportion,
                             heat_map, missNullNumRowCount, missNumColCount,
                             csv_file_path>
    csv_info_t;

std::vector<uint8_t> global_buffer;
typename ypc::cast_obj_to_package<csv_info_t>::type global_csv_info_pkg;

std::vector<int8_t> g_arr;                    // 存储所有传入的数据
std::vector<std::vector<int8_t>> global_bArr; // 由0,1表示的源数据集缺失情况

// 用于生成热力图的算法
uint32_t csv_to_image() {

  int rows = global_csv_info_pkg.get<::row_num>();
  int cols = global_csv_info_pkg.get<::col_num>();

  cv::Scalar def_color = cv::Scalar(180, 170, 170);  // 默认非缺失颜色
  cv::Scalar miss_color = cv::Scalar(144, 123, 122); // 缺失颜色

  // LOG(INFO) << "Inside ocall scale: rows: " << rows << ", cols: " << cols;

  // 参数校验
  if (rows <= 0 || cols <= 0) {
    return 1; // 无效的输入参数
  }

  // 每个单元格的默认尺寸
  int defaultCellWidth = 40;
  int defaultCellHeight;

  defaultCellWidth = int(500 / cols); // 500是默认的图像宽度
  if (defaultCellWidth < 25) {
    defaultCellWidth = 25;
  } // 最小单元格宽度为5

  if (rows > 2000) {
    defaultCellHeight = 1;
  } else {
    defaultCellHeight = int(2000 / rows);
  }
  int lineWidth = 1; // 分隔线宽度

  // 根据行数动态调整图像的高度
  long long imageHeight = rows * defaultCellHeight; // 图片的高度可能会很大
  int imageWidth = cols * (defaultCellWidth + lineWidth);

  int si_row = 100000; // single image row 单张完整图片的行数

  int imageCount =
      rows /
      si_row; // 定义分片生成的图片一共需要个完整的100,000行数据构成的图片
  LOG(INFO) << "imageCount: " << imageCount;

  bool still_remain = false; // 是否还有剩余的行数未处理
  if (rows % si_row != 0) {
    still_remain = true;
  } // 最后这部分跟随最后一行统一处理

  std::vector<cv::Mat> image(imageCount + 1);

  if (still_remain) {
    image[imageCount] =
        cv::Mat::zeros(rows % si_row * defaultCellHeight, imageWidth, CV_8UC3);
  } // 最后剩余的不满100,000行的数据

  for (int i = 0; i < imageCount * si_row; ++i) {
    // 在处理完毕一个后才初始化下一个，保证不产生内存不足的问题。
    if (i % si_row == 0) {
      image[i / si_row] = cv::Mat::zeros(si_row, imageWidth, CV_8UC3);
    }

    for (int j = 0; j < cols; ++j) {
      int start_x = j * (defaultCellWidth + lineWidth);
      int start_y = (i % si_row) * defaultCellHeight;
      cv::Rect rect(start_x, start_y, defaultCellWidth,
                    defaultCellHeight); // 修改这里可以去除分隔线
      cv::Scalar color = (g_arr[i * cols + j] == 1) ? cv::Scalar(180, 170, 170)
                                                    : cv::Scalar(144, 123, 122);
      cv::rectangle(image[i / si_row], rect, color, cv::FILLED);
    }

    // 每处理完100,000行的数据，就将图片缩小到720*1440，以减小内存占用
    if (i % si_row == si_row - 1) {
      cv::resize(image[i / si_row], image[i / si_row], cv::Size(720, 1440), 0,
                 0, cv::INTER_LINEAR);
    }
  }

  // 处理剩余的不满100,000行的数据
  if (still_remain) {
    for (int i = imageCount * si_row; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        int start_x = j * (defaultCellWidth + lineWidth);
        int start_y = (i % si_row) * defaultCellHeight;
        cv::Rect rect(start_x, start_y, defaultCellWidth,
                      defaultCellHeight); // 修改这里可以去除分隔线
        cv::Scalar color = (g_arr[i * cols + j] == 1)
                               ? cv::Scalar(180, 170, 170)
                               : cv::Scalar(144, 123, 122);
        cv::rectangle(image[imageCount], rect, color, cv::FILLED);
      }
    }

    int tiHeight = rows % si_row * 1440 / si_row;
    if (tiHeight == 0) {
      tiHeight = 1; // 防止出现高度为0的情况
    }

    // 按最后剩余行数与si_row的比例压缩图片，以保证最后合成图片的一致性
    if (imageCount != 0) {
      cv::resize(image[imageCount], image[imageCount], cv::Size(720, tiHeight),
                 0, 0, cv::INTER_LINEAR);
    }
  }

  // 新建一个final_image，按顺序拼接上面生成的所有图片，开始时为空，只按顺序在其下面拼接其他图片。
  cv::Mat final_image = cv::Mat::zeros(1, 1, CV_8UC3);

  for (int i = 1; i <= imageCount; i++) {
    if (!still_remain && i == imageCount) {
      break;
    }
    cv::vconcat(image[0], image[i], image[0]);
  }
  LOG(INFO) << "Finish img process.";

  cv::resize(image[0], final_image, cv::Size(720, 720), 0, 0, cv::INTER_LINEAR);

  // 压缩图像以满足大小限制
  std::vector<int> compression_params;
  compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
  int quality = 100; // 初始高质量
  do {
    global_buffer.clear();
    compression_params.back() = quality;
    cv::imencode(".jpg", final_image, global_buffer, compression_params);
    quality -= 2;                                      // 逐步降低质量
  } while (global_buffer.size() > 1e6 && quality > 0); // 保证图像文件小于1MB

  LOG(INFO) << "global_buffer size in process: " << global_buffer.size();

  return 0; // 成功
}

uint32_t handle_csv(const ypc::bytes &in, ypc::bytes &out) {
  LOG(INFO) << "handle csv";
  auto pkg = ypc::make_package<
      typename ypc::cast_obj_to_package<csv_info_t>::type>::from_bytes(in);
  std::string filename = pkg.get<::csv_file_path>();

  // 处理filename即可
  csv::CSVFormat format_deli;
  format_deli.delimiter({',', '|', ';', '^'})
      .no_header()
      .quote(true)
      .variable_columns(1)
      .guess_delim();
  csv::CSVReader reader_deli(filename, format_deli);
  char actual_delim = reader_deli.get_format().get_delim();
  LOG(INFO) << "actual_delim: \"" << actual_delim << "\"";

  csv::CSVFormat format;
  format.delimiter({actual_delim}).no_header().quote(true).variable_columns(1);
  csv::CSVReader reader(filename, format);

  int r = 0, tr = 0, c, max_c = 0;
  int max_column_start = 0;

  // 第一次循环，只为了获取最大列的行开始的位置以及最大行的列数
  for (csv::CSVRow &row : reader) {
    c = 0;
    // 直接输出整行的全部内容
    std::string row_contents;

    for (csv::CSVField &field : row) {
      if (!row_contents.empty())
        row_contents += ','; // 添加逗号作为字段分隔符
      row_contents += field.get<>();

      c++;
    }
    if (c > max_c) {
      max_c = c;
      // max_column_start = tr;
    }
    tr++;
  }

  std::vector<int> missNumRow(tr - max_column_start, 0); // 第p行缺失q列
  std::vector<int> missNumCol(max_c, 0);                 // 第x列缺失y行
  std::vector<int> missNullNumRowCount(1002, 0); // 有y个缺失值的行数量
  std::vector<int> missNumColCount(1002, 0);     // 有q个缺失值的列数量

  long long total_nulls = 0;

  csv::CSVReader reader2(filename, format);
  for (csv::CSVRow &row : reader2) {

    if (r < max_column_start) {
      r++;
      continue;
    }

    int g_arr_i = 0; // 记录g_arr的下标
    std::vector<int8_t> temp_bArr(max_c, 0);
    std::vector<int8_t> temp_gArr(
        max_c,
        0); //先对每一个字段进行初始化，赋值为0，若循环中发现该字段存在，则赋值为1

    for (csv::CSVField &field :
         row) { // 这里面做的是对每一行的每一个字段进行处理

      // 二维数组用于统计行列缺失，根据原始数据的缺失情况，缺失标记为0，否则标记为1
      // global_barr是全局二维数组 一维数组用于生成热力图,变量名为 g_arr

      std::string temp = field.get<>();
      if (temp.size() != 0) {
        temp_gArr[g_arr_i] = 1;
        temp_bArr[g_arr_i] = 1;
      }
      c++;
      g_arr_i++;
    }

    for (int k = 0; k < max_c; k++) {
      if (temp_bArr[k] == 0)
        total_nulls++;
    }

    g_arr.insert(g_arr.end(), temp_gArr.begin(), temp_gArr.end());
    global_bArr.push_back(temp_bArr);
    r++;
  }

  std::string nullProportion = std::to_string(
      ((total_nulls * 10000) / ((tr - max_column_start) * max_c)) / 100.0);

  global_csv_info_pkg.set<::row_num>(
      tr - max_column_start); // 从最大行开始的有效行数
  global_csv_info_pkg.set<::col_num>(max_c);           // 最大行的列数
  global_csv_info_pkg.set<::total_nulls>(total_nulls); // 总缺失值数量
  global_csv_info_pkg.set<::null_proportion>(
      nullProportion.substr(0, nullProportion.length() - 4)); // 缺失值比例

  // 具体缺失情况统计，构造json串 ,globa_bArr中存储的是0,1表示的源数据集缺失情况
  for (int i = 0; i < global_csv_info_pkg.get<::row_num>(); i++) {
    for (int j = 0; j < global_csv_info_pkg.get<::col_num>(); j++) {
      if (global_bArr[i][j] == 0) {
        missNumRow[i]++;
        missNumCol[j]++;
      }
    }
  }

  for (int i = 0; i < global_csv_info_pkg.get<::col_num>(); i++) {
    if (missNumCol[i] > 1000) {
      missNumColCount[1001]++;
    } else {
      missNumColCount[missNumCol[i]]++;
    }
  }

  for (int i = 0; i < global_csv_info_pkg.get<::row_num>(); i++) {
    if (missNumRow[i] > 1000) {
      missNullNumRowCount[1001]++;
    } else {
      missNullNumRowCount[missNumRow[i]]++;
    }
  }

  std::remove(filename.c_str());
  global_csv_info_pkg.set<::missNullNumRowCount>(missNullNumRowCount);
  global_csv_info_pkg.set<::missNumColCount>(missNumColCount);

  csv_to_image();

  // global_csv_info_pkg
  // TODO seams no use
  // auto b = ypc::make_bytes<ypc::bytes>::for_package(global_csv_info_pkg);
  // g_mem_buf = std::unique_ptr<uint8_t[]>(new uint8_t[b.size()]);
  // memcpy(g_mem_buf.get(), b.data(), b.size());

  typename ypc::cast_obj_to_package<csv_info_t>::type csv_info_pkg;

  csv_info_pkg.set<::row_num>(global_csv_info_pkg.get<::row_num>());
  csv_info_pkg.set<::col_num>(global_csv_info_pkg.get<::col_num>());
  csv_info_pkg.set<::total_nulls>(global_csv_info_pkg.get<::total_nulls>());
  csv_info_pkg.set<::null_proportion>(
      global_csv_info_pkg.get<::null_proportion>());
  csv_info_pkg.set<::missNullNumRowCount>(
      global_csv_info_pkg.get<::missNullNumRowCount>());
  csv_info_pkg.set<::missNumColCount>(
      global_csv_info_pkg.get<::missNumColCount>());

  csv_info_pkg.set<::heat_map>(global_buffer);

  out = ypc::make_bytes<ypc::bytes>::for_package(csv_info_pkg);
  return 0;
}

uint32_t handle_tsv(const ypc::bytes &in, ypc::bytes &out) {
  LOG(INFO) << "handle tsv";
  auto pkg = ypc::make_package<
      typename ypc::cast_obj_to_package<csv_info_t>::type>::from_bytes(in);
  std::string filename = pkg.get<::csv_file_path>();

  csv::CSVFormat format;
  char final_delimiter = '\t';
  int r = 0, temp_tr = 0, c, max_c = 0, tr = 0;
  int max_column_start = 0;

  LOG(INFO) << "final_delimiter: \"" << final_delimiter << "\"";

  r = 0;
  max_c = 0;
  bool need_quote = false;

  format.delimiter({final_delimiter})
      .no_header()
      .quote(need_quote)
      .variable_columns(1);
  csv::CSVReader reader1(filename, format);
  for (csv::CSVRow &row : reader1) {
    c = 0;
    for (csv::CSVField &field : row) {
      c++;
    }
    if (c > max_c) {
      max_c = c;
    }
    tr++;
  }

  LOG(INFO) << "total rows: " << tr << "     column: " << max_c;

  std::vector<int> missNumRow(tr - max_column_start, 0); // 第p行缺失q列
  std::vector<int> missNumCol(max_c, 0);                 // 第x列缺失y行
  std::vector<int> missNullNumRowCount(1002, 0); // 有y个缺失值的行数量
  std::vector<int> missNumColCount(1002, 0);     // 有q个缺失值的列数量

  long long total_nulls = 0;

  format.delimiter({final_delimiter})
      .no_header()
      .quote(need_quote)
      .variable_columns(1);
  csv::CSVReader reader2(filename, format);
  for (csv::CSVRow &row : reader2) {

    int g_arr_i = 0; // 记录g_arr的下标
    std::vector<int8_t> temp_bArr(max_c, 0);
    std::vector<int8_t> temp_gArr(max_c, 0);

    for (csv::CSVField &field : row) {
      std::string temp = field.get<>();
      if (temp.size() != 0) {
        temp_gArr[g_arr_i] = 1;
        temp_bArr[g_arr_i] = 1;
      }
      c++;
      g_arr_i++;
    }

    for (int k = 0; k < max_c; k++) {
      if (temp_bArr[k] == 0)
        total_nulls++;
    }

    g_arr.insert(g_arr.end(), temp_gArr.begin(), temp_gArr.end());
    global_bArr.push_back(temp_bArr);
    r++;
  }

  std::remove(filename.c_str());
  std::string nullProportion = std::to_string(
      ((total_nulls * 10000) / ((tr - max_column_start) * max_c)) / 100.0);

  global_csv_info_pkg.set<::row_num>(
      tr - max_column_start); // 从最大行开始的有效行数
  global_csv_info_pkg.set<::col_num>(max_c);           // 最大行的列数
  global_csv_info_pkg.set<::total_nulls>(total_nulls); // 总缺失值数量
  global_csv_info_pkg.set<::null_proportion>(
      nullProportion.substr(0, nullProportion.length() - 4)); // 缺失值比例

  csv_to_image();

  // global_csv_info_pkg
  // auto b = ypc::make_bytes<ypc::bytes>::for_package(global_csv_info_pkg);
  // g_mem_buf = std::unique_ptr<uint8_t[]>(new uint8_t[b.size()]);
  // memcpy(g_mem_buf.get(), b.data(), b.size());

  typename ypc::cast_obj_to_package<csv_info_t>::type csv_info_pkg;

  csv_info_pkg.set<::row_num>(global_csv_info_pkg.get<::row_num>());
  csv_info_pkg.set<::col_num>(global_csv_info_pkg.get<::col_num>());
  csv_info_pkg.set<::total_nulls>(global_csv_info_pkg.get<::total_nulls>());
  csv_info_pkg.set<::null_proportion>(
      global_csv_info_pkg.get<::null_proportion>());
  csv_info_pkg.set<::missNullNumRowCount>(
      global_csv_info_pkg.get<::missNullNumRowCount>());
  csv_info_pkg.set<::missNumColCount>(
      global_csv_info_pkg.get<::missNumColCount>());

  csv_info_pkg.set<::heat_map>(global_buffer);
  out = ypc::make_bytes<ypc::bytes>::for_package(csv_info_pkg);
  return 0;
}
