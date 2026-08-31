#include "ocall_def.h"
#include "ypc/core/byte.h"

#include <glog/logging.h>
#include <opencv2/opencv.hpp>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-page.h>

// PDF解析算法存储结果的结构，包括总页数以及至多三个页面的数据
define_nt(pdf_page, std::vector<unsigned char>); // PDF页面数据
define_nt(total_pages, double);                  // PDF总页数
define_nt(page_number, double); // 截取PDF页面对应的页码
typedef ::ff::util::ntobject<pdf_page, total_pages, page_number> pdf_page_t;

extern int random_get_frame_index(int total_frames, int frame_capture_len);

uint32_t handle_pdf(const ypc::bytes &in, ypc::bytes &out) {
  LOG(INFO) << "handle pdf";
  const char *ifs = (const char *)in.data();
  size_t ifs_size = in.size();
  typename ypc::cast_obj_to_package<pdf_page_t>::type pkg;

  if (!ifs) {
    LOG(ERROR) << "Invalid input parameters";
    return 1;
  }

  std::string error;
  std::string path_str(ifs, ifs_size);
  std::unique_ptr<poppler::document> doc(
      poppler::document::load_from_file(path_str, "", ""));
  if (!doc) {
    LOG(ERROR) << "Failed to load PDF: " << error;
    return 1;
  }

  int total_pages = doc->pages();
  pkg.set<::total_pages>(total_pages);

  if (total_pages <= 0) {
    LOG(ERROR) << "PDF contains no pages";
    return 2;
  }

  // 随机选择一页
  int page_index = random_get_frame_index(total_pages, 1);
  std::unique_ptr<poppler::page> pg(doc->create_page(page_index));
  if (!pg) {
    LOG(ERROR) << "Failed to load page " << page_index;
    return 3;
  }

  poppler::page_renderer pr;
  pr.set_render_hint(poppler::page_renderer::antialiasing, true);
  pr.set_render_hint(poppler::page_renderer::text_antialiasing, true);
  poppler::image img = pr.render_page(pg.get(), 70, 70); // DPI可以根据需要调整

  if (!img.is_valid()) {
    LOG(ERROR) << "Rendering page failed";
    return 4;
  }

  cv::Mat mat(img.height(), img.width(), CV_8UC4, (void *)img.data());

  std::string text = "DIAN SHU";
  int fontFace = cv::FONT_HERSHEY_COMPLEX;
  double fontScale = 3.5; // 字体大小
  int thickness = 10;     // 线条粗细
  int baseline = 0;
  cv::Scalar textColor(166, 71, 73); // 水印颜色

  // 获取文本的宽度和高度
  cv::Size textSize =
      cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);

  // 计算文本水印在原图上的中心位置
  cv::Point textCenter(mat.cols / 2, mat.rows / 2);

  // 创建文本水印图层（与原图大小一致，并初始化为全透明）
  cv::Mat watermarkLayer = cv::Mat::zeros(mat.size(), CV_8UC4);

  // 计算文本的起始坐标（左下角点），使其居中放置
  cv::Point textOrg((watermarkLayer.cols - textSize.width) / 2,
                    (watermarkLayer.rows + textSize.height) / 2);

  // 在透明图层上绘制文本
  cv::putText(watermarkLayer, text, textOrg, fontFace, fontScale, textColor,
              thickness, cv::LINE_AA);

  // 创建旋转矩阵，以图像中心为旋转中心，旋转60度
  cv::Mat rotMat = cv::getRotationMatrix2D(textCenter, 60, 1.0);

  // 创建一个新图层，用于存放旋转后的文本水印
  cv::Mat rotatedWatermarkLayer;
  cv::warpAffine(watermarkLayer, rotatedWatermarkLayer, rotMat,
                 watermarkLayer.size(), cv::INTER_LINEAR,
                 cv::BORDER_TRANSPARENT);

  // 将旋转后的水印图层覆盖到原图 `mat` 上
  for (int y = 0; y < mat.rows; ++y) {
    for (int x = 0; x < mat.cols; ++x) {
      cv::Vec4b &watermarkPixel = rotatedWatermarkLayer.at<cv::Vec4b>(y, x);
      if (watermarkPixel[0] == 166) {
        mat.at<cv::Vec4b>(y, x) = watermarkPixel;
      }
    }
  }

  // 编码图像为JPEG
  std::vector<unsigned char> buffer;
  if (!cv::imencode(".jpg", mat, buffer)) {
    LOG(ERROR) << "Failed to encode image";
    return 5; // 图像编码失败
  }

  pkg.set<::pdf_page>(buffer);
  pkg.set<::page_number>(page_index);
  out = ypc::make_bytes<ypc::bytes>::for_package(pkg);
  return 0;
}
