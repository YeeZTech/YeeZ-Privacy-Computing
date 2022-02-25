#include <hpda/engine/engine.h>
#include <hpda/engine/functor.h>

namespace hpda {
functor::functor() : m_has_value(false) {}

functor::~functor() {}

void functor::set_engine(engine *e) {
  m_engine = e;
  m_engine->add_functor(this);
}
void functor::done_value() { m_has_value = true; }
} // namespace hpda
