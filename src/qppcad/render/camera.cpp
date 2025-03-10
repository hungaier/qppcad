#include <qppcad/render/camera.hpp>
#include <qppcad/core/app_state.hpp>
#include <qppcad/core/json_helpers.hpp>

using namespace qpp;
using namespace qpp::cad;

void camera_t::on_recording() {
  if (get_cur_rec_type() == hs_doc_rec_type_e::hs_doc_rec_init) {
    p_cam_states.clear();
  }
  p_cam_states[get_cur_epoch()] = p_cam_state;
}

camera_t::camera_t () {
  app_state_t* astate = app_state_t::get_inst();
  m_cur_proj = astate->m_default_cam_proj;
  reset_camera();
}

void camera_t::orthogonalize_gs () {
  p_cam_state.m_view_dir = p_cam_state.m_look_at - p_cam_state.m_view_point;
  p_cam_state.m_stored_dist = p_cam_state.m_view_dir.norm();
  vector3<float> view_dir_new = p_cam_state.m_view_dir.normalized();

  if (p_cam_state.m_view_dir.isMuchSmallerThan(camera_t::norm_eps)) {
    p_cam_state.m_look_up = vector3<float>(0.0, 0.0, 1.0);
  } else {
    p_cam_state.m_look_up -= view_dir_new * (view_dir_new.dot(p_cam_state.m_look_up));
    p_cam_state.m_look_up.normalize();
  }

  p_cam_state.m_right = -1.0*(p_cam_state.m_look_up.cross(p_cam_state.m_view_dir));

  if (p_cam_state.m_right.isMuchSmallerThan(camera_t::norm_eps)) {
    p_cam_state.m_right = vector3<float>(1.0, 0.0, 0.0);
  } else {
    p_cam_state.m_right.normalize();
  }

  p_cam_state.m_forward = p_cam_state.m_look_up.cross(p_cam_state.m_right);
}

void camera_t::rotate_camera_around_origin (const matrix3<float> &mat_rot,
                                            const vector3<float> origin) {
  translate_camera(-origin);
  p_cam_state.m_view_point = mat_rot * p_cam_state.m_view_point;
  p_cam_state.m_look_at    = mat_rot * p_cam_state.m_look_at;
  p_cam_state.m_look_up    = mat_rot * p_cam_state.m_look_up;
  translate_camera(origin);
  orthogonalize_gs();
}

void camera_t::rotate_camera_around_axis (const float angle, const vector3<float> axis) {
  Eigen::Matrix<float, 3, 3> mr{Eigen::AngleAxisf(angle, axis)};
  rotate_camera_around_origin(mr, p_cam_state.m_look_at);
}

void camera_t::rotate_camera_orbit_yaw (const float yaw) {
  rotate_camera_around_axis(yaw, vector3<float>(0.0, 0.0, 1.0));
}

void camera_t::rotate_camera_orbit_pitch (const float pitch) {
  rotate_camera_around_axis(pitch, p_cam_state.m_right);
}

void camera_t::rotate_camera_orbit_roll(const float roll) {
  rotate_camera_around_axis(roll, p_cam_state.m_forward);
}

void camera_t::translate_camera_forward (const float amount) {
  vector3<float> view_dir_new = p_cam_state.m_view_dir.normalized();
  p_cam_state.m_view_point += view_dir_new * amount;
  p_cam_state.m_look_at    += view_dir_new * amount;
}

void camera_t::translate_camera_right (const float amount) {
  vector3<float> _tmp_tr = p_cam_state.m_right * amount;
  p_cam_state.m_view_point += _tmp_tr;
  p_cam_state.m_look_at += _tmp_tr;
}

void camera_t::translate_camera_up (const float amount) {
  vector3<float> _tmp_tr = p_cam_state.m_look_up * amount;
  p_cam_state.m_view_point += _tmp_tr;
  p_cam_state.m_look_at += _tmp_tr;
}

void camera_t::translate_camera (const vector3<float> shift) {
  p_cam_state.m_view_point += shift;
  p_cam_state.m_look_at    += shift;
}

void camera_t::copy_from_camera(const camera_t &another) {
  p_cam_state = another.p_cam_state;
  m_cur_proj = another.m_cur_proj;
  update_camera();
}

void camera_t::push_cam_state() {
  //
}

void camera_t::pop_cam_state() {
  if (p_cam_states.empty()) {
    return;
  }
  //
  update_camera();
}

void camera_t::reset_camera () {
  p_cam_state.m_view_point = vector3<float>(0.0, 9.0, 0.0);
  p_cam_state.m_look_at    = vector3<float>(0.0, 0.0, 0.0);
  p_cam_state.m_look_up    = vector3<float>(0.0, 0.0, 1.0);
  orthogonalize_gs();
}

void camera_t::update_camera () {

  app_state_t* astate = app_state_t::get_inst();

  float x_dt = std::clamp(astate->mouse_x - astate->mouse_x_old, -11.0f, 10.0f);
  float y_dt = std::clamp(astate->mouse_y - astate->mouse_y_old, -11.0f, 10.0f);

  if (m_move_camera) {
    float move_right = -x_dt / camera_t::nav_div_step_translation;
    float move_up = y_dt / camera_t::nav_div_step_translation;
    if (fabs(move_right) > camera_t::nav_thresh) {
      translate_camera_right(move_right);
      astate->make_viewport_dirty();
    }
    if (fabs(move_up) > camera_t::nav_thresh) {
      translate_camera_up(move_up);
      astate->make_viewport_dirty();
    }
  }

  if (m_rotate_camera) {
    float rot_angle_x = y_dt / camera_t::nav_div_step_rotation;
    float rot_angle_y = x_dt / camera_t::nav_div_step_rotation;
    if (fabs(rot_angle_y) > camera_t::nav_thresh && !m_rotate_over) {
      rotate_camera_orbit_yaw(rot_angle_y);
      astate->make_viewport_dirty();
    }
    if (fabs(rot_angle_x) > camera_t::nav_thresh && !m_rotate_over) {
      rotate_camera_orbit_pitch(rot_angle_x);
      astate->make_viewport_dirty();
    }
    float med_rot = (rot_angle_y + rot_angle_x) * 0.5f;
    if (fabs(med_rot) > camera_t::nav_thresh && m_rotate_over) {
      rotate_camera_orbit_roll(med_rot);
      astate->make_viewport_dirty();
    }
  }

  if (m_cur_proj == cam_proj_t::proj_persp) {
    p_cam_state.m_look_at = (p_cam_state.m_view_point - p_cam_state.m_look_at).normalized();
    p_cam_state.m_mat_proj =
        perspective<float>(p_cam_state.m_fov,
                           astate->viewport_size(0) / astate->viewport_size(1),
                           p_cam_state.m_znear_persp, p_cam_state.m_zfar_persp);
  } else {
    p_cam_state.m_mat_view = look_at<float>(p_cam_state.m_view_point,
                                            p_cam_state.m_look_at,
                                            p_cam_state.m_look_up);
    float width   = astate->viewport_size(0);
    float height  = astate->viewport_size(1);
    float x_scale = 1.0f;
    float y_scale = 1.0f;
    if (width > height) {
      x_scale = width / (height);
      y_scale = 1;
    } else {
      x_scale = 1;
      y_scale = height / (width);
    }
    float left   = - x_scale * (p_cam_state.m_ortho_scale);
    float right  =   x_scale * (p_cam_state.m_ortho_scale);
    float bottom = - y_scale * (p_cam_state.m_ortho_scale);
    float top    =   y_scale * (p_cam_state.m_ortho_scale);
    //std::cout<<"ortho_scale"<<m_ortho_scale<<std::endl;
    p_cam_state.m_mat_proj = ortho<float>(left, right,
                                          bottom, top,
                                          p_cam_state.m_znear_ortho, p_cam_state.m_zfar_ortho);
  }

  p_cam_state.m_proj_view = p_cam_state.m_mat_proj *  p_cam_state.m_mat_view ;
  p_cam_state.m_view_inv_tr =
      mat4_to_mat3<float>((p_cam_state.m_mat_view.inverse()).transpose());
  p_cam_state.m3_proj_view = mat4_to_mat3<float>(p_cam_state.m_proj_view);

}

void camera_t::update_camera_zoom (const float dist) {
  if (m_cur_proj == cam_proj_t::proj_persp) {
    vector3<float> m_view_dir_n = - p_cam_state.m_view_point + p_cam_state.m_look_at;
    float f_dist = m_view_dir_n.norm();
    p_cam_state.m_stored_dist = f_dist;
    m_view_dir_n.normalize();
    float f_dist_delta = dist * m_mouse_whell_camera_step;
    if (f_dist + f_dist_delta > m_mouse_zoom_min_distance || f_dist_delta < 0.0f)
      p_cam_state.m_view_point += m_view_dir_n * f_dist_delta;
  } else {
    p_cam_state.m_ortho_scale -= dist;
    p_cam_state.m_ortho_scale = clamp(p_cam_state.m_ortho_scale, 1.0f, 150.0f);
  }
}

void camera_t::update_camera_translation (const bool move_camera) {
  m_move_camera = move_camera;
}

void camera_t::update_camera_rotation (bool rotate_camera) {
  m_rotate_camera = rotate_camera;
}

void camera_t::set_projection (cam_proj_t _proj_to_set) {
  if (m_cur_proj != _proj_to_set) {
    reset_camera();
    m_cur_proj = _proj_to_set;
  }
}

float camera_t::distance(const vector3<float> &point) {
  return (p_cam_state.m_mat_view * vector4<float>(point(0), point(1), point(2), 1.0f)).norm();
}

vector3<float> camera_t::unproject (const float _x, const float _y, const float _z) {
  matrix4<float> mat_mvp_inv = (p_cam_state.m_mat_proj * p_cam_state.m_mat_view).inverse();
  vector4<float> invec4(_x, _y, _z, 1.0f);
  vector4<float> rvec4 =  mat_mvp_inv * invec4;
  rvec4(3) = 1.0f / rvec4(3);
  rvec4(0) = rvec4(0) * rvec4(3);
  rvec4(1) = rvec4(1) * rvec4(3);
  rvec4(2) = rvec4(2) * rvec4(3);
  return vector3<float>(rvec4(0), rvec4(1), rvec4(2));
}

std::optional<vector2<float> > camera_t::project (const vector3<float> point) {
  app_state_t* astate = app_state_t::get_inst();
  vector4<float> tmpv =
      p_cam_state.m_proj_view * vector4<float>(point[0], point[1], point[2], 1.0f);
  if (std::fabs(tmpv[3]) < 0.00001f)
    return std::nullopt;
  if (tmpv[3] > 0.98f && tmpv[3] < 1.04f) {
    tmpv[0] /= tmpv[3];
    tmpv[1] /= tmpv[3];
    tmpv[2] /= tmpv[3];
  }
  tmpv[0] = (tmpv[0] + 1.0f) / 2.0f;
  tmpv[1] = (-tmpv[1] + 1.0f) / 2.0f;
  tmpv[2] = (tmpv[2] + 1.0f) / 2.0f;
  tmpv[0] = tmpv[0] * astate->viewport_size[0];
  tmpv[1] = (tmpv[1] * astate->viewport_size[1]) + 0.5f;
  vector2<float> ret_v2;
  ret_v2[0] = tmpv[0];
  ret_v2[1] = tmpv[1];
  return std::optional(ret_v2);
}

void camera_t::save_to_json(json &data) {
  json_io::save_vec3(JSON_WS_CAMERA_LOOK_AT, p_cam_state.m_look_at, data);
  json_io::save_vec3(JSON_WS_CAMERA_VIEW_POINT, p_cam_state.m_view_point, data);
  json_io::save_var(JSON_WS_CAMERA_ORTHO_SCALE, p_cam_state.m_ortho_scale, data);
}

void camera_t::load_from_json(json &data) {
  json_io::load_vec3(JSON_WS_CAMERA_LOOK_AT, p_cam_state.m_look_at, data);
  json_io::load_vec3(JSON_WS_CAMERA_VIEW_POINT, p_cam_state.m_view_point, data);
  json_io::load_var(JSON_WS_CAMERA_ORTHO_SCALE, p_cam_state.m_ortho_scale, data);
  m_already_loaded = true;
  orthogonalize_gs();
  update_camera();
}

hs_result_e camera_t::on_epoch_changed(hs_doc_base_t::epoch_t prev_epoch) {
  auto cur_epoch = get_cur_epoch();
  auto it = p_cam_states.find(cur_epoch);
  if (it == end(p_cam_states)) {
    return hs_result_e::hs_invalid_epoch;
  }
  p_cam_state = it->second;
  orthogonalize_gs();
  update_camera();
  return hs_result_e::hs_success;
}

hs_result_e camera_t::on_epoch_removed(hs_doc_base_t::epoch_t epoch_to_remove) {
  auto it = p_cam_states.find(epoch_to_remove);
  if (it != p_cam_states.end()) {
    p_cam_states.erase(it);
    return hs_result_e::hs_success;
  } else {
    return hs_result_e::hs_error;
  }
}

void camera_t::on_commit_exclusive() {
  auto cur_epoch = get_cur_epoch();
  p_cam_states[cur_epoch] = p_cam_state;
}

vector3<float> camera_t::get_look_at() {
  return p_cam_state.m_look_at;
}

vector3<float> camera_t::get_view_point() {
  return p_cam_state.m_view_point;
}

float camera_t::get_znear_ortho() {
  return p_cam_state.m_znear_ortho;
}

float camera_t::get_zfar_ortho() {
  return p_cam_state.m_zfar_ortho;
}

float camera_t::get_stored_dist() {
  return p_cam_state.m_stored_dist;
}

float camera_t::get_ortho_scale() {
  return p_cam_state.m_ortho_scale;
}

void camera_t::update_camera_state(const vector3<float> &new_look_at,
                                   const vector3<float> &new_look_pos) {
  p_cam_state.m_look_at = new_look_at;
  p_cam_state.m_view_point = new_look_pos;
}

void camera_t::update_camera_state_v2(const vector3<float> &new_look_at,
                                      const vector3<float> &new_look_pos,
                                      const vector3<float> &new_look_up) {
  p_cam_state.m_look_at = new_look_at;
  p_cam_state.m_view_point = new_look_pos;
  p_cam_state.m_look_up = new_look_up;
}

matrix4<float> &camera_t::get_mat_view() {
  return p_cam_state.m_mat_view;
}

matrix4<float> &camera_t::get_proj_view() {
  return p_cam_state.m_proj_view;
}

matrix4<float> &camera_t::get_mat_proj() {
  return p_cam_state.m_mat_proj;
}


