#include <hpda/engine/engine.h>
#include <hpda/engine/functor.h>

namespace hpda {
functor::functor() : m_has_value(false), m_has_more_data(true) {}

functor::~functor() {}

void functor::set_engine(engine *e) {
  m_engine = e;
  m_engine->add_functor(this);
}
} // namespace hpda
