#include <qppcad/sflow/sflow_parameter_ws_item.hpp>

using namespace qpp;
using namespace qpp::cad;

sf_parameter_e sf_parameter_ws_item_t::get_param_meta() {
  return sf_parameter_e::sfpar_ws_item;
}

std::shared_ptr<sf_parameter_t> sf_parameter_ws_item_t::clone() {
  auto _clone = std::make_shared<sf_parameter_ws_item_t>();
  _clone->m_value = m_value;
  return std::move(_clone);
}
