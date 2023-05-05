#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include "xgboost/regression/xgboost_reg.h"
#include "xgboost/regression/xgboost_reg_data.h"

namespace xgboost {
namespace regression {
class RegBoostTask {
public:
  RegBoostTask(const std::string &task,
               const std::vector<std::vector<std::pair<int, float>>> &rows,
               const std::vector<std::string> &ids)
      : m_task(task), m_silent(0), m_num_round(2) {
    m_fs = std::make_shared<utils::FileStream>();
    this->split_dataset(rows, ids);
    this->set_param("booster_type", "0");
    this->set_param("loss_type", "2");
    this->set_param("bst:eta", "0.3");
    this->set_param("bst:gamma", "1.0");
    this->set_param("bst:min_child_weight", "2");
    this->set_param("bst:max_depth", "6");
    this->set_param("num_round", "10");
    this->set_param("data", "normal_train_merge.train");
    this->set_param("test:data", "normal_train_merge.train");
  }
  ~RegBoostTask() = default;

  inline void run() {
    this->init_data();
    this->init_learner();
    if (m_task == "train") {
      this->train();
    }
    if (m_task == "pred") {
      this->pred();
    }
  }

  inline const std::string &get_model() const { return m_fs->get(); }
  inline void set_model(const std::string &model) { m_fs->set(model); }

  inline const std::vector<float> &get_preds() const { return m_preds; }
  inline const std::vector<std::string> &get_pred_ids() const { return m_ids; }

protected:
  inline void set_param(const char *name, const char *val) {
    if (!strcmp("num_round", name)) {
      m_num_round = atoi(val);
    }
    if (!strcmp("task", name)) {
      m_task = val;
    }
    if (!strcmp("data", name)) {
      m_train_path = val;
    }
    if (!strcmp("test:data", name)) {
      m_test_path = val;
      m_eval_data_names.push_back("test");
      m_eval_data_paths.push_back(std::string(val));
    }
    m_cfg.PushBack(name, val);
  }

  inline void
  split_dataset(const std::vector<std::vector<std::pair<int, float>>> &rows,
                const std::vector<std::string> &ids) {
    for (int i = 0; i < rows.size(); i++) {
      if (random::NextDouble() < 0.2f) {
        m_ids.push_back(ids[i]);
        m_libsvm_test_rows.push_back(rows[i]);
      } else {
        m_libsvm_train_rows.push_back(rows[i]);
      }
    }
  }

  inline void init_data() {
    if (m_task == "train") {
      m_data.CacheLoad(m_libsvm_train_rows, m_silent != 0, false);
      for (size_t i = 0; i < m_eval_data_names.size(); ++i) {
        m_deval.push_back(std::shared_ptr<DMatrix>(new DMatrix()));
        m_deval.back()->CacheLoad(m_libsvm_test_rows, m_silent != 0, false);
      }
    }
    if (m_task == "pred") {
      m_data.CacheLoad(m_libsvm_test_rows, m_silent != 0, false);
    }
    m_learner.SetData(&m_data, m_deval, m_eval_data_names);
  }

  inline void init_learner() {
    m_cfg.BeforeFirst();
    while (m_cfg.Next()) {
      m_learner.SetParam(m_cfg.name(), m_cfg.val());
    }
    if (m_task == "train") {
      utils::Assert(m_task == "train", "model_in not specified");
      m_learner.InitModel();
    }
    if (m_task == "pred") {
      m_learner.LoadModel(*m_fs);
    }
    m_learner.InitTrainer();
  }

  inline void train() {
    for (int i = 0; i < m_num_round; i++) {
      if (!m_silent) {
        LOG(INFO) << "\nboosting round " << i;
      }
      m_learner.UpdateOneIter(i);
      m_learner.EvalOneIter(i);
    }
    // always save final round
    m_learner.SaveModel(*m_fs);
    if (!m_silent) {
      LOG(INFO) << "updating end";
    }
  }

  inline void pred() {
    if (!m_silent) {
      LOG(INFO) << "start prediction...";
    }
    m_learner.Predict(m_preds, m_data);
    if (!m_silent) {
      LOG(INFO) << "accuracy: " << 100 * eval_pred() << "%";
    }
  }

  inline float eval_pred() {
    utils::Assert(
        m_preds.size() == m_libsvm_test_rows.size(),
        "the number of prediction results should be equal to test rows");
    int cnt = 0;
    int total = std::min(size_t(100), m_preds.size());
    for (int i = 0; i < total; i++) {
      int idx = random::NextDouble() * total;
      float val = m_libsvm_test_rows[idx][0].second;
      LOG(INFO) << "expected: " << val << ", actual: " << m_preds[idx];
      if (m_preds[idx] < 0.5f && val == 0.0f) {
        cnt++;
      }
      if (m_preds[idx] > 0.5f && val == 1.0f) {
        cnt++;
      }
    }
    return 1.0 * cnt / total;
  }

protected:
  /* \brief whether silent */
  int m_silent;
  /* \brief number of boosting iterations */
  int m_num_round;
  /* \brief the path of training/test data set */
  std::string m_train_path, m_test_path;
  /* \brief the paths of validation data sets */
  std::vector<std::string> m_eval_data_paths;
  /* \brief the names of the evaluation data used in output log */
  std::vector<std::string> m_eval_data_names;
  /* \brief task to perform */
  std::string m_task;
  /*! \brief saves configurations */
  utils::ConfigSaver m_cfg;

protected:
  std::shared_ptr<utils::FileStream> m_fs;
  std::vector<std::string> m_ids;
  std::vector<std::vector<std::pair<int, float>>> m_libsvm_train_rows;
  std::vector<std::vector<std::pair<int, float>>> m_libsvm_test_rows;
  DMatrix m_data;
  std::vector<float> m_preds;
  std::vector<std::shared_ptr<DMatrix>> m_deval;
  RegBoostLearner m_learner;
};
} // namespace regression
} // namespace xgboost
