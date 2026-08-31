#include "decoded_bytes.h"
#include "ocall_def.h"
#include "ypc/core/byte.h"

#include <QImage>
#include <QPainter>
#include <archive.h>
#include <archive_entry.h>
#include <boost/filesystem.hpp>
#include <glog/logging.h>
#include <opencv2/freetype.hpp>
#include <opencv2/opencv.hpp>
#include <zip.h>

// ZIP解析算法存储结果的结构，包括总文件数、文件数量占比统计图、文件大小占比统计图以及zip文件路径
define_nt(total_file_num, int);                       // 总文件数
define_nt(summary_gragh, std::vector<unsigned char>); // 文件数量占比统计图
define_nt(size_gragh, std::vector<unsigned char>); // 文件大小占比统计图
define_nt(zip_file_path, std::string);             // zip文件路径
typedef ::ff::util::ntobject<total_file_num, summary_gragh, size_gragh,
                             zip_file_path>
    zip_info_t;

typename ypc::cast_obj_to_package<zip_info_t>::type global_zip_info_pkg;
std::vector<uint8_t> global_buf;

// 选择并加载支持中文的字体；可通过环境变量 DIANSHU_FONT_PATH 覆盖
std::string pick_cjk_font_path() {
  const char *env_font = std::getenv("DIANSHU_FONT_PATH");
  if (env_font && boost::filesystem::exists(env_font)) {
    return std::string(env_font);
  }

  const std::vector<std::string> candidates = {
      "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
      "/usr/share/fonts/opentype/noto/NotoSerifCJK-Regular.ttc",
  };

  for (const auto &path : candidates) {
    if (boost::filesystem::exists(path)) {
      return path;
    }
  }

  return {};
}

cv::Ptr<cv::freetype::FreeType2> create_freetype_renderer() {
  static cv::Ptr<cv::freetype::FreeType2> renderer;
  static bool attempted = false;
  if (attempted) {
    return renderer;
  }
  attempted = true;

  std::string font_path = pick_cjk_font_path();
  if (font_path.empty()) {
    LOG(WARNING) << "No CJK font found; set DIANSHU_FONT_PATH to a valid .ttf/.ttc.";
    return renderer;
  }

  try {
    renderer = cv::freetype::createFreeType2();
    renderer->loadFontData(font_path, 0);
    LOG(INFO) << "Using font: " << font_path;
  } catch (const cv::Exception &e) {
    LOG(WARNING) << "Failed to init FreeType: " << e.what();
    renderer.release();
  }

  return renderer;
}

// 使用 FreeType 优先绘制文本；如果不可用则退回 Hershey 字体
void draw_text_utf8(cv::Mat &mat, const std::string &text, const cv::Point &org,
                    double font_scale, const cv::Scalar &color, int thickness) {
  auto ft = create_freetype_renderer();
  if (ft) {
    int pixel_height = static_cast<int>(font_scale * 32.0);

    // FreeType 只接受 8U/三通道或单通道，这里统一转 BGR，绘制后再转回
    if (mat.type() != CV_8UC3) {
      cv::Mat tmpBgr;
      if (mat.channels() == 4) {
        cv::cvtColor(mat, tmpBgr, cv::COLOR_BGRA2BGR);
      } else if (mat.channels() == 1) {
        cv::cvtColor(mat, tmpBgr, cv::COLOR_GRAY2BGR);
      } else {
        mat.copyTo(tmpBgr);
      }

      ft->putText(tmpBgr, text, org, pixel_height, color, thickness, cv::LINE_AA,
                  true);

      if (mat.channels() == 4) {
        cv::cvtColor(tmpBgr, mat, cv::COLOR_BGR2BGRA);
      } else if (mat.channels() == 1) {
        cv::cvtColor(tmpBgr, mat, cv::COLOR_BGR2GRAY);
      } else {
        tmpBgr.copyTo(mat);
      }
    } else {
      ft->putText(mat, text, org, pixel_height, color, thickness, cv::LINE_AA,
                  true);
    }
  } else {
    cv::putText(mat, text, org, cv::FONT_HERSHEY_TRIPLEX, font_scale, color,
                thickness, cv::LINE_AA);
  }
}

uint32_t handle_zip(const ypc::bytes &in, ypc::bytes &out) {
  LOG(INFO) << "handle zip";
  auto pkg = ypc::make_package<
      typename ypc::cast_obj_to_package<zip_info_t>::type>::from_bytes(in);
  std::string filename = pkg.get<::zip_file_path>();

  std::map<std::string, int> file_map; // 用于存储文件名和文件数量
  std::map<std::string, long long> file_size; // 用于存储文件名和文件大小

  // 现在filename指代的就是完整的文件路径，直接对这个处理就好

  int file_count = 0;            // 文件数量
  long long total_file_size = 0; // 文件总大小

  int max_length = 0; // 最长扩展名的长度

  int err = 0;
  zip *z = zip_open(filename.c_str(), ZIP_RDONLY, &err);

  if (z == nullptr) {
    zip_error_t ziperror;
    zip_error_init_with_code(&ziperror, err);
    LOG(ERROR) << "Failed to open archive: " << filename
               << " Error: " << zip_error_strerror(&ziperror);
    zip_error_fini(&ziperror);
    return 1;
  }

  // 获取ZIP文件中的实体数目
  zip_int64_t n = zip_get_num_entries(z, 0);
  LOG(INFO) << "Total item: " << n;

  // 遍历ZIP文件中的每个实体
  for (zip_int64_t i = 0; i < n; ++i) {
    zip_stat_t sb;
    if (zip_stat_index(z, i, 0, &sb) == 0) {
      const char *extension = strrchr(sb.name, '.');
      if (extension == nullptr) {
        continue; // 如果没有找到'.'，跳过当前循环
      }
      std::string file_temp(extension);

      if (file_temp.length() > max_length) {
        max_length = file_temp.length();
      }

      bool flag = true;

      for (int j = 0; j < file_temp.size(); j++) {
        if (file_temp[j] == '/' || file_temp[j] == '\\') {
          flag = false;
        }

        if (file_temp[j] >= 'A' && file_temp[j] <= 'Z') {
          file_temp[j] = file_temp[j] - 'A' + 'a';
        }
      }

      if (flag) { // 确保文件不是目录
        file_map[file_temp]++;
        file_size[file_temp] += sb.size;
        file_count++;
        total_file_size += sb.size;
      }
    }
  }

  // 关闭ZIP文件
  zip_close(z);

  // 最长扩展名的长度
  LOG(INFO) << "Max Extension length: " << max_length;

  struct ExtStat {
    std::string ext;
    int count;
    long long size;
  };

  std::vector<ExtStat> stats;
  stats.reserve(file_map.size());
  for (const auto &kv : file_map) {
    long long sz = 0;
    auto it = file_size.find(kv.first);
    if (it != file_size.end()) {
      sz = it->second;
    }
    stats.push_back({kv.first, kv.second, sz});
  }

  // 按文件大小降序（再按数量降序）排序
  std::sort(stats.begin(), stats.end(), [](const ExtStat &a, const ExtStat &b) {
    if (a.size == b.size) {
      return a.count > b.count;
    }
    return a.size > b.size;
  });

  // 只展示前9种，其余合并为“其他”
  const size_t kMaxTypes = 9;
  std::vector<ExtStat> display_stats;
  display_stats.reserve(std::min(kMaxTypes, stats.size()) + 1);
  long long other_size = 0;
  int other_count = 0;
  for (size_t i = 0; i < stats.size(); ++i) {
    if (i < kMaxTypes) {
      display_stats.push_back(stats[i]);
    } else {
      other_size += stats[i].size;
      other_count += stats[i].count;
    }
  }
  if (stats.size() > kMaxTypes) {
    display_stats.push_back({"其他", other_count, other_size});
  }

  int display_max_length = max_length;
  for (const auto &s : display_stats) {
    if (s.ext.length() > display_max_length) {
      display_max_length = s.ext.length();
    }
  }

  int extra_length = 0; // 如果最长扩展名大于7时，需要额外的长度

  if (display_max_length > 7) {
    extra_length = (display_max_length - 7) * 40;
  }

  LOG(INFO) << "File num: " << file_count;

  global_zip_info_pkg.set<::total_file_num>(file_count);

  // archive_read_data_skip(a);  // 跳过文件内容
  // 释放资源
  // archive_read_free(a);

  if (file_count == 0) {
    global_zip_info_pkg.set<::summary_gragh>(decoded_bytes);
    global_zip_info_pkg.set<::size_gragh>(decoded_bytes);
    return 0;
  }

  //--------------------添加数量占比统计图--------------------

  // 控制图像尺寸，避免超过 OpenCV 65500 像素的编码上限
  const int kMaxEncoderDim = 65000; // 稍小于 65500，给编码预留空间
  int listStartX = 260;             // 列表开始的横坐标
  int listStartY = 650;             // 列表开始的纵坐标
  int itemHeight = 120;             // 每项的高度
  int columnWidth = 500;            // 每列的宽度

  // 计算行/列数量，尽量压缩高度，防止超过 kMaxEncoderDim
  int maxRows =
      std::max(1, (kMaxEncoderDim - (650 + 100)) / itemHeight); // 650+100 为固定区域高度
  int itemsPerRow =
      std::max(1, (static_cast<int>(display_stats.size()) + maxRows - 1) / maxRows);
  int rowCount =
      std::max(1, (static_cast<int>(display_stats.size()) + itemsPerRow - 1) / itemsPerRow);

  int imageWidth = 1080 + extra_length + (itemsPerRow - 1) * columnWidth;
  int imageHeight = 650 + rowCount * itemHeight + 100;

  // 添加背景图像
  QImage image(imageWidth, imageHeight, QImage::Format_ARGB32);

  image.fill(Qt::white); // 设置背景为白色
  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿

  int startAngle = 0;
  int outerRadius = 500; // 外圈半径
  int innerRadius = 300; // 内圈半径
  QRect outerRect((1080 - outerRadius) / 2, (1080 - outerRadius) / 2 - 250,
                  outerRadius, outerRadius);
  QRect innerRect((1080 - innerRadius) / 2, (1080 - innerRadius) / 2 - 250,
                  innerRadius, innerRadius);

  std::vector<QColor> colors; // 记录颜色，用于后面生成统计表格
  std::map<std::string, QColor> colorMap; // 记录颜色的map

  int hueStep = 360 / display_stats.size(); // 计算色相步长
  int currentHue = 0;

  for (const auto &kv : display_stats) {
    int i = 0;
    QColor color = QColor::fromHsv(currentHue, 150, 255); // 动态生成颜色
    colors.push_back(color);                              // 存入颜色
    colorMap[kv.ext] = color;                             // 存入map
    currentHue += hueStep; // 更新色相，以确保颜色分布均匀

    // 计算角度，QT中的角度是1度的1/16，因此需要乘以16
    int angle = 360 * kv.count * 16 / file_count;

    painter.setBrush(color);
    painter.setPen(Qt::NoPen); // 无边框
    painter.drawPie(outerRect, startAngle, angle);
    startAngle += angle;
    i++;
  }

  // 用最大占比的文件类型补全剩余的角度
  painter.setBrush(colorMap[display_stats[0].ext]);
  painter.setPen(Qt::NoPen); // 无边框
  painter.drawPie(outerRect, startAngle, 5760 - startAngle);

  // 绘制内圆，覆盖中心，形成环状
  painter.setBrush(Qt::white); // 设置内部填充为白色
  painter.setPen(Qt::NoPen);   // 无边框
  painter.drawEllipse(innerRect);

  // 添加列表，显示每种颜色的类型、数量和占比
  painter.setPen(QPen(Qt::black));

  for (size_t i = 0; i < display_stats.size(); ++i) {
    const auto &kv = display_stats[i];

    int x = listStartX + (i % itemsPerRow) * columnWidth;
    int y = listStartY + (i / itemsPerRow) * itemHeight;

    painter.setBrush(colorMap[kv.ext]); // 设置颜色
    painter.drawRect(x, y, 60, 60);       // 绘制颜色方块
  }

  painter.end(); // 结束当前的painter，释放资源以便重新初始化

  // 转换 QImage 到 OpenCV Mat
  cv::Mat mat(image.height(), image.width(), CV_8UC4,
              const_cast<uchar *>(image.bits()), image.bytesPerLine());

  for (size_t i = 0; i < display_stats.size(); ++i) {
    const auto &kv = display_stats[i];

    std::string nProportion = std::to_string(100.0 * kv.count / file_count);

    std::string label = kv.ext + ": " + std::to_string(kv.count) + " (" +
                        nProportion.substr(0, nProportion.length() - 4) + "%)";

    int x = listStartX + (i % itemsPerRow) * columnWidth;
    int y = listStartY + (i / itemsPerRow) * itemHeight;
    cv::Scalar textColor = cv::Scalar(0, 0, 0); // 黑色

    draw_text_utf8(mat, label, cv::Point(x + 70, y + 40), 1.6, textColor, 2);
  }

  // 压缩图像以满足大小限制
  std::vector<int> compression_params;
  compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
  int quality = 100; // 初始高质量
  do {
    global_buf.clear();
    compression_params.back() = quality;
    cv::imencode(".jpg", mat, global_buf, compression_params);
    quality -= 2;                                      // 逐步降低质量
  } while (global_buf.size() > 1e6 && quality > 0); // 保证图像文件小于1MB

  global_zip_info_pkg.set<::summary_gragh>(global_buf);
  //--------------------结束添加数量占比统计图--------------------

  //--------------------添加大小占比统计图--------------------
  std::vector<std::string> formed_size_vec;
  for (auto it = display_stats.begin(); it != display_stats.end(); it++) {
    std::string temp;
    if (it->size < 1024) {
      temp = std::to_string(it->size) + "B";
    } else if (it->size < 1024 * 1024) {
      temp = std::to_string(it->size / 1024.0);
      temp = temp.substr(0, temp.length() - 4) + "KB";
    } else if (it->size < 1024 * 1024 * 1024) {
      temp = std::to_string(it->size / 1024.0 / 1024.0);
      temp = temp.substr(0, temp.length() - 4) + "MB";
    } else {
      temp = std::to_string(it->size / 1024.0 / 1024.0 / 1024.0);
      temp = temp.substr(0, temp.length() - 4) + "GB";
    }
    // LOG(INFO) << it -> first << " : " << temp;
    formed_size_vec.push_back(temp);
  }

  // 重新计算大小图布局，可能与数量图的行/列不同
  int maxRowsSize =
      std::max(1, (kMaxEncoderDim - (650 + 100)) / itemHeight);
  int itemsPerRowSize =
      std::max(1, (static_cast<int>(display_stats.size()) + maxRowsSize - 1) / maxRowsSize);
  int rowCountSize = std::max(
      1, (static_cast<int>(display_stats.size()) + itemsPerRowSize - 1) / itemsPerRowSize);

  int image2Width = 1080 + extra_length + (itemsPerRowSize - 1) * columnWidth;
  int image2Height = 650 + rowCountSize * itemHeight + 100;

  // 添加背景图像
  QImage image2(image2Width, image2Height, QImage::Format_ARGB32);

  image2.fill(Qt::white); // 设置背景为白色
  QPainter painter2(&image2);
  painter2.setRenderHint(QPainter::Antialiasing); // 抗锯齿

  startAngle = 0; // 除了这个变量需要重新初始化其他都不需要

  for (const auto &kv : display_stats) {
    int i = 0;

    // 共有5760个角度
    int angle = 360 * kv.size * 16 / total_file_size;

    painter2.setBrush(colorMap[kv.ext]);
    painter2.setPen(Qt::NoPen); // 无边框
    painter2.drawPie(outerRect, startAngle, angle);
    startAngle += angle;
    i++;
  }

  // 用最大占比的文件类型补全剩余的角度
  painter2.setBrush(colorMap[display_stats[0].ext]);
  painter2.setPen(Qt::NoPen); // 无边框
  painter2.drawPie(outerRect, startAngle, 5760 - startAngle);

  // 绘制内圆，覆盖中心，形成环状
  painter2.setBrush(Qt::white); // 设置内部填充为白色
  painter2.setPen(Qt::NoPen);   // 无边框
  painter2.drawEllipse(innerRect);

  // 添加列表，显示每种颜色的类型、数量和占比
  painter2.setPen(QPen(Qt::black));

  for (size_t i = 0; i < display_stats.size(); ++i) {
    const auto &kv = display_stats[i];

    int x = listStartX + (i % itemsPerRowSize) * columnWidth;
    int y = listStartY + (i / itemsPerRowSize) * itemHeight;

    painter2.setBrush(colorMap[kv.ext]); // 设置颜色
    painter2.drawRect(x, y, 60, 60);       // 绘制颜色方块
  }

  painter2.end(); // 结束当前的painter2，释放资源以便重新初始化

  // 转换 QImage 到 OpenCV Mat
  cv::Mat mat2(image2.height(), image2Width, CV_8UC4,
               const_cast<uchar *>(image2.bits()), image2.bytesPerLine());

  for (size_t i = 0; i < display_stats.size(); ++i) {
    const auto &kv = display_stats[i];

    std::string nProportion =
        std::to_string(100.0 * kv.size / total_file_size);

    std::string label = kv.ext + ": " + formed_size_vec[i] + " (" +
                        nProportion.substr(0, nProportion.length() - 4) + "%)";

    int x = listStartX + (i % itemsPerRowSize) * columnWidth;
    int y = listStartY + (i / itemsPerRowSize) * itemHeight;
    cv::Scalar textColor = cv::Scalar(0, 0, 0); // 黑色

    draw_text_utf8(mat2, label, cv::Point(x + 70, y + 40), 1.6, textColor, 2);
  }

  // 压缩图像以满足大小限制
  quality = 100; // 初始高质量
  do {
    global_buf.clear();
    compression_params.back() = quality;
    cv::imencode(".jpg", mat2, global_buf, compression_params);
    quality -= 2;                                      // 逐步降低质量
  } while (global_buf.size() > 1e6 && quality > 0); // 保证图像文件小于1MB

  global_zip_info_pkg.set<::size_gragh>(global_buf);
  out = ypc::make_bytes<ypc::bytes>::for_package(global_zip_info_pkg);
  return 0;
}

const char *get_filename_no_directory(const char *filename) {
  const char *p = NULL;
  if (!filename)
    return NULL;

  p = strrchr(filename, '/');
  if (!p)
    return filename;

  if (p[1] == '\0')
    return NULL;

  return p + 1;
}
uint32_t handle_rar(const ypc::bytes &in, ypc::bytes &out) {
  LOG(INFO) << "handle rar";
  auto pkg = ypc::make_package<
      typename ypc::cast_obj_to_package<zip_info_t>::type>::from_bytes(in);
  std::string filename = pkg.get<::zip_file_path>();

  std::map<std::string, int> file_map; // 用于存储文件名和文件数量
  std::map<std::string, long long> file_size; // 用于存储文件名和文件大小

  // 现在filename指代的就是完整的文件路径，直接对这个处理就好

  int file_count = 0;            // 文件数量
  long long total_file_size = 0; // 文件总大小

  int max_length = 0; // 最长扩展名的长度

  //--rar格式压缩文件处理逻辑
  archive *rar_archive = archive_read_new();
  if (rar_archive == NULL) {
    LOG(ERROR) << "Failed to initialize RAR archive";
    return 1;
  }
  archive_read_support_filter_all(rar_archive);
  archive_read_support_format_rar(rar_archive);
  archive_read_support_format_rar5(rar_archive);
  if (archive_read_open_filename(rar_archive, filename.c_str(), 10240) !=
      ARCHIVE_OK) {
    LOG(ERROR) << "Failed to open archive: "
               << archive_error_string(rar_archive);
    archive_read_free(rar_archive);
    return 1;
  }

  int redundant_num = 0;
  archive_entry *entry = NULL;
  int archive_status = ARCHIVE_OK;
  while ((archive_status = archive_read_next_header(rar_archive, &entry)) ==
         ARCHIVE_OK) {
    ++file_count;
    const char *name = archive_entry_pathname_utf8(entry);
    if (name == NULL) {
      name = archive_entry_pathname(entry);
    }
    const char *base_name = get_filename_no_directory(name);
    if (base_name == NULL) {
      ++redundant_num;
      archive_read_data_skip(rar_archive);
      continue;
    }

    std::string file_temp(base_name);
    bool flag = false;
    for (char &ch : file_temp) {
      if (ch == '/' || ch == '\\') {
        flag = false;
      }
      if (ch == '.') {
        flag = true;
      }
      if (ch >= 'A' && ch <= 'Z') {
        ch = ch - 'A' + 'a';
      }
    }
    if (!flag) {
      ++redundant_num;
    }
    if (flag && archive_entry_filetype(entry) != AE_IFDIR) {
      file_temp = strrchr(file_temp.c_str(), '.');
      if (static_cast<int>(file_temp.length()) > max_length) {
        max_length = static_cast<int>(file_temp.length());
      }
      const long long entry_size =
          archive_entry_size_is_set(entry) ? archive_entry_size(entry) : 0;
      file_map[file_temp]++;
      file_size[file_temp] += entry_size;
      total_file_size += entry_size;
    }
    archive_read_data_skip(rar_archive);
  }
  if (archive_status != ARCHIVE_EOF) {
    LOG(ERROR) << "Failed to read archive: "
               << archive_error_string(rar_archive);
    archive_read_free(rar_archive);
    return 1;
  }
  file_count -= redundant_num;
  archive_read_free(rar_archive);
  //--rar格式压缩文件处理逻辑结束
  //
  // 最长扩展名的长度
  LOG(INFO) << "Max Extension length: " << max_length;

  struct ExtStat {
    std::string ext;
    int count;
    long long size;
  };

  std::vector<ExtStat> stats;
  stats.reserve(file_map.size());
  for (const auto &kv : file_map) {
    long long sz = 0;
    auto it = file_size.find(kv.first);
    if (it != file_size.end()) {
      sz = it->second;
    }
    stats.push_back({kv.first, kv.second, sz});
  }

  // 按文件大小降序（再按数量降序）排序
  std::sort(stats.begin(), stats.end(), [](const ExtStat &a, const ExtStat &b) {
    if (a.size == b.size) {
      return a.count > b.count;
    }
    return a.size > b.size;
  });

  // 只展示前9种，其余合并为“其他”
  const size_t kMaxTypes = 9;
  std::vector<ExtStat> display_stats;
  display_stats.reserve(std::min(kMaxTypes, stats.size()) + 1);
  long long other_size = 0;
  int other_count = 0;
  for (size_t i = 0; i < stats.size(); ++i) {
    if (i < kMaxTypes) {
      display_stats.push_back(stats[i]);
    } else {
      other_size += stats[i].size;
      other_count += stats[i].count;
    }
  }
  if (stats.size() > kMaxTypes) {
    display_stats.push_back({"其他", other_count, other_size});
  }

  int display_max_length = max_length;
  for (const auto &s : display_stats) {
    if (s.ext.length() > display_max_length) {
      display_max_length = s.ext.length();
    }
  }

  int extra_length = 0; // 如果最长扩展名大于7时，需要额外的长度

  if (display_max_length > 7) {
    extra_length = (display_max_length - 7) * 40;
  }

  LOG(INFO) << "File num: " << file_count;

  global_zip_info_pkg.set<::total_file_num>(file_count);

  // archive_read_data_skip(a);  // 跳过文件内容
  // 释放资源
  // archive_read_free(a);

  if (file_count == 0) {
    global_zip_info_pkg.set<::summary_gragh>(decoded_bytes);
    global_zip_info_pkg.set<::size_gragh>(decoded_bytes);
    return 0;
  }

  //--------------------添加数量占比统计图--------------------

  int listStartX = 260;  // 列表开始的横坐标
  int listStartY = 650;  // 列表开始的纵坐标
  int itemHeight = 120;  // 每项的高度
  int columnWidth = 500; // 每列的宽度
  int itemsPerRow = 1;   // 每列的项数

  // 重新计算高度/列数以限制尺寸
  const int kMaxEncoderDim = 65000;
  int maxRows =
      std::max(1, (kMaxEncoderDim - (650 + 100)) / itemHeight); // 650+100 固定区域
  itemsPerRow =
      std::max(1, (static_cast<int>(display_stats.size()) + maxRows - 1) / maxRows);
  int rowCount =
      std::max(1, (static_cast<int>(display_stats.size()) + itemsPerRow - 1) / itemsPerRow);

  int imageWidth = 1080 + extra_length + (itemsPerRow - 1) * columnWidth;
  int imageHeight = 650 + rowCount * itemHeight + 100;

  // 添加背景图像
  QImage image(imageWidth, imageHeight, QImage::Format_ARGB32);

  image.fill(Qt::white); // 设置背景为白色
  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿

  int startAngle = 0;
  int outerRadius = 500; // 外圈半径
  int innerRadius = 300; // 内圈半径
  QRect outerRect((1080 - outerRadius) / 2, (1080 - outerRadius) / 2 - 250,
                  outerRadius, outerRadius);
  QRect innerRect((1080 - innerRadius) / 2, (1080 - innerRadius) / 2 - 250,
                  innerRadius, innerRadius);

  std::vector<QColor> colors; // 记录颜色，用于后面生成统计表格
  std::map<std::string, QColor> colorMap; // 记录颜色的map

  int hueStep = 360 / display_stats.size(); // 计算色相步长
  int currentHue = 0;

  for (const auto &kv : display_stats) {
    int i = 0;
    QColor color = QColor::fromHsv(currentHue, 150, 255); // 动态生成颜色
    colors.push_back(color);                              // 存入颜色
    colorMap[kv.ext] = color;                             // 存入map
    currentHue += hueStep; // 更新色相，以确保颜色分布均匀

    // 计算角度，QT中的角度是1度的1/16，因此需要乘以16
    int angle = 360 * kv.count * 16 / file_count;

    painter.setBrush(color);
    painter.setPen(Qt::NoPen); // 无边框
    painter.drawPie(outerRect, startAngle, angle);
    startAngle += angle;
    i++;
  }

  // 用最大占比的文件类型补全剩余的角度
  painter.setBrush(colorMap[display_stats[0].ext]);
  painter.setPen(Qt::NoPen); // 无边框
  painter.drawPie(outerRect, startAngle, 5760 - startAngle);

  // 绘制内圆，覆盖中心，形成环状
  painter.setBrush(Qt::white); // 设置内部填充为白色
  painter.setPen(Qt::NoPen);   // 无边框
  painter.drawEllipse(innerRect);

  // 添加列表，显示每种颜色的类型、数量和占比
  painter.setPen(QPen(Qt::black));

  for (size_t i = 0; i < display_stats.size(); ++i) {
    const auto &kv = display_stats[i];

    int x = listStartX + (i % itemsPerRow) * columnWidth;
    int y = listStartY + (i / itemsPerRow) * itemHeight;

    painter.setBrush(colorMap[kv.ext]); // 设置颜色
    painter.drawRect(x, y, 60, 60);       // 绘制颜色方块
  }

  painter.end(); // 结束当前的painter，释放资源以便重新初始化

  // 转换 QImage 到 OpenCV Mat
  cv::Mat mat(image.height(), image.width(), CV_8UC4,
              const_cast<uchar *>(image.bits()), image.bytesPerLine());

  for (size_t i = 0; i < display_stats.size(); ++i) {
    const auto &kv = display_stats[i];

    std::string nProportion = std::to_string(100.0 * kv.count / file_count);

    std::string label = kv.ext + ": " + std::to_string(kv.count) + " (" +
                        nProportion.substr(0, nProportion.length() - 4) + "%)";

    int x = listStartX + (i % itemsPerRow) * columnWidth;
    int y = listStartY + (i / itemsPerRow) * itemHeight;
    cv::Scalar textColor = cv::Scalar(0, 0, 0); // 黑色

    draw_text_utf8(mat, label, cv::Point(x + 70, y + 40), 1.6, textColor, 2);
  }

  // 压缩图像以满足大小限制
  std::vector<int> compression_params;
  compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
  int quality = 100; // 初始高质量
  do {
    global_buf.clear();
    compression_params.back() = quality;
    cv::imencode(".jpg", mat, global_buf, compression_params);
    quality -= 2;                                      // 逐步降低质量
  } while (global_buf.size() > 1e6 && quality > 0); // 保证图像文件小于1MB

  global_zip_info_pkg.set<::summary_gragh>(global_buf);
  //--------------------结束添加数量占比统计图--------------------

  //--------------------添加大小占比统计图--------------------
  std::vector<std::string> formed_size_vec;
  for (auto it = display_stats.begin(); it != display_stats.end(); it++) {
    std::string temp;
    if (it->size < 1024) {
      temp = std::to_string(it->size) + "B";
    } else if (it->size < 1024 * 1024) {
      temp = std::to_string(it->size / 1024.0);
      temp = temp.substr(0, temp.length() - 4) + "KB";
    } else if (it->size < 1024 * 1024 * 1024) {
      temp = std::to_string(it->size / 1024.0 / 1024.0);
      temp = temp.substr(0, temp.length() - 4) + "MB";
    } else {
      temp = std::to_string(it->size / 1024.0 / 1024.0 / 1024.0);
      temp = temp.substr(0, temp.length() - 4) + "GB";
    }
    // LOG(INFO) << it -> first << " : " << temp;
    formed_size_vec.push_back(temp);
  }

  // 重新计算大小图布局，可能与数量图的行/列不同
  int maxRowsSize =
      std::max(1, (kMaxEncoderDim - (650 + 100)) / itemHeight);
  int itemsPerRowSize =
      std::max(1, (static_cast<int>(display_stats.size()) + maxRowsSize - 1) / maxRowsSize);
  int rowCountSize = std::max(
      1, (static_cast<int>(display_stats.size()) + itemsPerRowSize - 1) / itemsPerRowSize);

  int image2Width = 1080 + extra_length + (itemsPerRowSize - 1) * columnWidth;
  int image2Height = 650 + rowCountSize * itemHeight + 100;

  // 添加背景图像
  QImage image2(image2Width, image2Height, QImage::Format_ARGB32);

  image2.fill(Qt::white); // 设置背景为白色
  QPainter painter2(&image2);
  painter2.setRenderHint(QPainter::Antialiasing); // 抗锯齿

  startAngle = 0; // 除了这个变量需要重新初始化其他都不需要

  for (const auto &kv : display_stats) {
    int i = 0;

    // 共有5760个角度
    int angle = 360 * kv.size * 16 / total_file_size;

    painter2.setBrush(colorMap[kv.ext]);
    painter2.setPen(Qt::NoPen); // 无边框
    painter2.drawPie(outerRect, startAngle, angle);
    startAngle += angle;
    i++;
  }

  // 用最大占比的文件类型补全剩余的角度
  painter2.setBrush(colorMap[display_stats[0].ext]);
  painter2.setPen(Qt::NoPen); // 无边框
  painter2.drawPie(outerRect, startAngle, 5760 - startAngle);

  // 绘制内圆，覆盖中心，形成环状
  painter2.setBrush(Qt::white); // 设置内部填充为白色
  painter2.setPen(Qt::NoPen);   // 无边框
  painter2.drawEllipse(innerRect);

  // 添加列表，显示每种颜色的类型、数量和占比
  painter2.setPen(QPen(Qt::black));

  for (size_t i = 0; i < display_stats.size(); ++i) {
    const auto &kv = display_stats[i];

    int x = listStartX + (i % itemsPerRowSize) * columnWidth;
    int y = listStartY + (i / itemsPerRowSize) * itemHeight;

    painter2.setBrush(colorMap[kv.ext]); // 设置颜色
    painter2.drawRect(x, y, 60, 60);       // 绘制颜色方块
  }

  painter2.end(); // 结束当前的painter2，释放资源以便重新初始化

  // 转换 QImage 到 OpenCV Mat
  cv::Mat mat2(image2.height(), image2Width, CV_8UC4,
               const_cast<uchar *>(image2.bits()), image2.bytesPerLine());

  for (size_t i = 0; i < display_stats.size(); ++i) {
    const auto &kv = display_stats[i];

    std::string nProportion =
        std::to_string(100.0 * kv.size / total_file_size);

    std::string label = kv.ext + ": " + formed_size_vec[i] + " (" +
                        nProportion.substr(0, nProportion.length() - 4) + "%)";

    int x = listStartX + (i % itemsPerRowSize) * columnWidth;
    int y = listStartY + (i / itemsPerRowSize) * itemHeight;
    cv::Scalar textColor = cv::Scalar(0, 0, 0); // 黑色

    draw_text_utf8(mat2, label, cv::Point(x + 70, y + 40), 1.6, textColor, 2);
  }

  // 压缩图像以满足大小限制
  quality = 100; // 初始高质量
  do {
    global_buf.clear();
    compression_params.back() = quality;
    cv::imencode(".jpg", mat2, global_buf, compression_params);
    quality -= 2;                                      // 逐步降低质量
  } while (global_buf.size() > 1e6 && quality > 0); // 保证图像文件小于1MB

  global_zip_info_pkg.set<::size_gragh>(global_buf);
  out = ypc::make_bytes<ypc::bytes>::for_package(global_zip_info_pkg);
  return 0;
}
